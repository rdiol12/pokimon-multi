#include "global.h"
#include "community.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "pokemon.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"

// The server boss (server/community_boss.py): one huge wild Pokémon whose
// HP everyone on the server shares. START > ONLINE > COMMUNITY > BOSS shows
// it with its shared HP; a battle against it is an ordinary wild battle
// (it can't be caught), and the damage dealt is sent to the server.

extern const u8 Community_EventScript_Boss[];
extern const u8 Community_EventScript_BossFell[];

#define BOSS_STATUS_ACTIVE 2

// gCommunityMailbox.events[COMMUNITY_EVENT_BOSS] (and the status answer's
// respData, one byte earlier): id, alive, species (u16), hp (u32), max
// (u32), who brought it down (8 bytes). Read byte by byte.
#define BOSS_EVENT gCommunityMailbox.events[COMMUNITY_EVENT_BOSS]

static EWRAM_DATA u8 sBossId = 0;
static EWRAM_DATA u16 sBossMaxHp = 0;
static EWRAM_DATA u8 sSeenSeq = 0;

void Community_OpenBoss(void)
{
    sSeenSeq = BOSS_EVENT[0];
    ScriptContext_SetupScript(Community_EventScript_Boss);
}

// --- The status window -------------------------------------------------------

static const struct WindowTemplate sStatusWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 12,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

#define tWindowId data[0]
#define tSpriteId data[1]
#define tActive   data[2]

#define BAR_X 8
#define BAR_Y 74
#define BAR_WIDTH 80

static void DrawStatus(struct Task *task)
{
    u8 text[COMMUNITY_ROW_LENGTH + 1];
    u32 i, hp = gCommunityMailbox.respArgs[2], max = gCommunityMailbox.respArgs[3];

    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    for (i = 0; i < 4 && i < gCommunityMailbox.rowCount; i++)
    {
        StringCopyN(text, gCommunityMailbox.rows[i], COMMUNITY_ROW_LENGTH);
        text[COMMUNITY_ROW_LENGTH] = EOS;
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, text, 4, i == 0 ? 0 : 6 + i * 16, TEXT_SKIP_DRAW, NULL);
    }
    if (task->tActive && max != 0)
    {
        u32 filled = BAR_WIDTH * hp / max;
        // Green, then pink under half, red under a fifth, like a battle HP bar.
        u8 color = hp * 5 < max ? TEXT_COLOR_RED : hp * 2 < max ? TEXT_COLOR_LIGHT_RED : TEXT_COLOR_GREEN;

        AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, COMPOUND_STRING("HP"), BAR_X, BAR_Y - 3, TEXT_SKIP_DRAW, NULL);
        FillWindowPixelRect(task->tWindowId, PIXEL_FILL(2), BAR_X + 14, BAR_Y, BAR_WIDTH + 2, 6);
        FillWindowPixelRect(task->tWindowId, PIXEL_FILL(1), BAR_X + 15, BAR_Y + 1, BAR_WIDTH, 4);
        if (filled == 0 && hp != 0)
            filled = 1;
        FillWindowPixelRect(task->tWindowId, PIXEL_FILL(color), BAR_X + 15, BAR_Y + 1, filled, 4);
        ConvertIntToDecimalStringN(text, hp, STR_CONV_MODE_LEFT_ALIGN, 6);
        StringAppend(text, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(text + StringLength(text), max, STR_CONV_MODE_LEFT_ALIGN, 6);
        AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, text, BAR_X + BAR_WIDTH + 20, BAR_Y - 3, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(task->tWindowId, FONT_SMALL, COMPOUND_STRING("A: FIGHT IT   B: BACK"), 4, 84, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
}

static void Task_Status(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (!JOY_NEW(A_BUTTON | B_BUTTON))
        return;
    PlaySE(SE_SELECT);
    gSpecialVar_Result = task->tActive && JOY_NEW(A_BUTTON);
    if (task->tSpriteId != SPRITE_NONE)
        FreeAndDestroyMonPicSprite(task->tSpriteId);
    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script, waitstate: the boss and its shared HP, from the last status answer.
// VAR_RESULT: TRUE to fight it.
void CommunityBoss_ShowStatus(void)
{
    u8 taskId = CreateTask(Task_Status, 80);
    struct Task *task = &gTasks[taskId];
    u16 species = gCommunityMailbox.respArgs[0];

    task->tActive = gCommunityMailbox.respStatus == BOSS_STATUS_ACTIVE;
    task->tSpriteId = SPRITE_NONE;
    sBossId = gCommunityMailbox.respData[0];
    task->tWindowId = AddWindow(&sStatusWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    DrawStatus(task);
    if (species != SPECIES_NONE && species < NUM_SPECIES)
    {
        // The palette is loaded under the species as its tag (trainer_pokemon_sprites.c).
        task->tSpriteId = CreateMonPicSprite(species, FALSE, 0, TRUE, 192, 56, 15, species);
        if (task->tSpriteId != 0xFFFF)
            gSprites[task->tSpriteId].oam.priority = 0;
        else
            task->tSpriteId = SPRITE_NONE;
    }
    ScheduleBgCopyTilemapToVram(0);
}

#undef tWindowId
#undef tSpriteId
#undef tActive

// --- The battle --------------------------------------------------------------

// Script: sets up the wild battle (dowildbattle next).
void CommunityBoss_SetUpBattle(void)
{
    u16 species = gCommunityMailbox.respArgs[0];
    u8 level = gCommunityMailbox.respArgs[1];

    CreateScriptedWildMon(species, level, ITEM_NONE);
    sBossMaxHp = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_MAX_HP);
    FlagSet(FLAG_COMMUNITY_NO_CATCHING);
}

// Script, waitstate: after the battle, sends the damage dealt.
void CommunityBoss_ReportDamage(void)
{
    u16 hp = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HP);
    u32 damage = hp < sBossMaxHp ? sBossMaxHp - hp : 0;

    FlagClear(FLAG_COMMUNITY_NO_CATCHING);
    gSpecialVar_0x8005 = damage;
    Community_RequestAndWait(COMMUNITY_REQ_BOSS_DAMAGE, sBossId, damage, 0, NULL, 0);
}

// Tells everyone online when the boss falls.
bool32 CommunityBoss_OverworldUpdate(void)
{
    if (BOSS_EVENT[0] == sSeenSeq || ArePlayerFieldControlsLocked())
        return FALSE;
    sSeenSeq = BOSS_EVENT[0];
    if (BOSS_EVENT[2] != 0 || BOSS_EVENT[13] == EOS || BOSS_EVENT[13] == 0)
        return FALSE;
    StringCopyN(gStringVar1, &BOSS_EVENT[13], 7);
    gStringVar1[7] = EOS;
    StringCopy(gStringVar2, GetSpeciesName(BOSS_EVENT[3] | (BOSS_EVENT[4] << 8)));
    ScriptContext_SetupScript(Community_EventScript_BossFell);
    return TRUE;
}
