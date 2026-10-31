#include "global.h"
#include "community.h"
#include "contest_effect.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "move.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/songs.h"

// Online contests (server/community_contest.py): START > ONLINE >
// COMMUNITY > CONTEST enters one Pokémon in a category. The server judges it
// against other players' latest entries and sends back the results, shown
// here; a win earns that category's Master Rank ribbon.

extern const u8 Community_EventScript_Contest[];

#define CONTEST_ENTRANTS 4
#define RESULT_SIZE      26 // name 8, nickname 12, species 2, three rounds, is-me
#define ENTRY_SIZE       18 // species 2, condition, sheen, level, move appeal, nickname 12
#define MASTER_RIBBON    4

static const u8 sConditionFields[CONTEST_CATEGORIES_COUNT] =
{
    MON_DATA_COOL, MON_DATA_BEAUTY, MON_DATA_CUTE, MON_DATA_SMART, MON_DATA_TOUGH,
};

static const u8 sRibbonFields[CONTEST_CATEGORIES_COUNT] =
{
    MON_DATA_COOL_RIBBON, MON_DATA_BEAUTY_RIBBON, MON_DATA_CUTE_RIBBON, MON_DATA_SMART_RIBBON, MON_DATA_TOUGH_RIBBON,
};

static EWRAM_DATA u8 sSlot = 0;
static EWRAM_DATA u8 sCategory = 0;

void Community_OpenContest(void)
{
    ScriptContext_SetupScript(Community_EventScript_Contest);
}

// How well a Pokémon's moves suit the category: full appeal for moves of
// that category, a quarter for the rest.
static u32 GetMoveAppeal(struct Pokemon *mon, u32 category)
{
    u32 i, total = 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(mon, MON_DATA_MOVE1 + i);
        u32 appeal;

        if (move == MOVE_NONE)
            continue;
        appeal = gContestEffects[GetMoveContestEffect(move)].appeal;
        total += GetMoveContestCategory(move) == category ? appeal : appeal / 4;
    }
    return min(total, 255);
}

// Script, waitstate: VAR_0x8004 = party slot, VAR_0x8005 = category.
void CommunityContest_Enter(void)
{
    u8 data[ENTRY_SIZE];
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);

    sSlot = gSpecialVar_0x8004;
    sCategory = gSpecialVar_0x8005 % CONTEST_CATEGORIES_COUNT;
    data[0] = species;
    data[1] = species >> 8;
    data[2] = GetMonData(mon, sConditionFields[sCategory]);
    data[3] = GetMonData(mon, MON_DATA_SHEEN);
    data[4] = GetMonData(mon, MON_DATA_LEVEL);
    data[5] = GetMoveAppeal(mon, sCategory);
    memset(data + 6, EOS, 12);
    GetMonData(mon, MON_DATA_NICKNAME, data + 6);
    data[ENTRY_SIZE - 1] = EOS;
    Community_RequestAndWait(COMMUNITY_REQ_CONTEST_ENTER, sCategory, 0, 0, data, sizeof(data));
}

// Script: the winner's ribbon. VAR_RESULT: TRUE if it's new.
void CommunityContest_GiveRibbon(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sSlot];
    u8 ribbon = MASTER_RIBBON;

    gSpecialVar_Result = GetMonData(mon, sRibbonFields[sCategory]) < MASTER_RIBBON;
    if (gSpecialVar_Result)
        SetMonData(mon, sRibbonFields[sCategory], &ribbon);
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
}

// --- The results -------------------------------------------------------------

static const struct WindowTemplate sResultsWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sPlaceColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};
static const u8 sMeColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sOtherColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 *const sPlaces[CONTEST_ENTRANTS] =
{
    COMPOUND_STRING("1st"), COMPOUND_STRING("2nd"), COMPOUND_STRING("3rd"), COMPOUND_STRING("4th"),
};

#define ROW_TOP     20
#define ROW_HEIGHT  30
#define BAR_X       62
#define BAR_SCALE   4 // points per pixel

struct ContestResults
{
    u8 windowId;
    u8 icons[CONTEST_ENTRANTS];
    u16 frame;
};

static EWRAM_DATA struct ContestResults sResults = {0};

static void DrawResults(u32 shownRounds)
{
    u8 text[40];
    u8 *end;
    u32 i, r, count = min(gCommunityMailbox.respArgs[2], CONTEST_ENTRANTS);

    FillWindowPixelBuffer(sResults.windowId, PIXEL_FILL(1));
    StringCopyN(text, gCommunityMailbox.rows[0], COMMUNITY_ROW_LENGTH);
    text[COMMUNITY_ROW_LENGTH] = EOS;
    AddTextPrinterParameterized(sResults.windowId, FONT_NORMAL, text, 4, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < count; i++)
    {
        const u8 *entry = &gCommunityMailbox.respData[i * RESULT_SIZE];
        bool32 me = entry[25];
        u32 y = ROW_TOP + i * ROW_HEIGHT, x = BAR_X, score = 0;

        AddTextPrinterParameterized3(sResults.windowId, FONT_NORMAL, 4, y + 6, sPlaceColor, TEXT_SKIP_DRAW, sPlaces[i]);
        // "TRAINER's Nickname" over the score bar.
        end = StringCopyN(text, entry, 7); // padded with EOS
        *end = EOS;
        end = StringAppend(text, COMPOUND_STRING("'s "));
        end = StringCopyN(end, entry + 8, 11);
        *end = EOS;
        AddTextPrinterParameterized3(sResults.windowId, FONT_NARROW, BAR_X, y, me ? sMeColor : sOtherColor, TEXT_SKIP_DRAW, text);
        // One bar segment per round.
        for (r = 0; r < shownRounds && r < 3; r++)
        {
            static const u8 roundColors[] = {TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED, TEXT_COLOR_BLUE};
            u32 width = entry[22 + r] / BAR_SCALE;

            FillWindowPixelRect(sResults.windowId, PIXEL_FILL(roundColors[r]), x, y + 16, width, 6);
            x += width;
            score += entry[22 + r];
        }
        if (shownRounds >= 3)
        {
            ConvertIntToDecimalStringN(text, score, STR_CONV_MODE_LEFT_ALIGN, 3);
            AddTextPrinterParameterized3(sResults.windowId, FONT_SMALL, x + 4, y + 13, sOtherColor, TEXT_SKIP_DRAW, text);
        }
    }
    CopyWindowToVram(sResults.windowId, COPYWIN_FULL);
}

static void Task_Results(u8 taskId)
{
    u32 i;

    // The rounds fill in one after another, like the judge's tally.
    sResults.frame++;
    if (sResults.frame == 40 || sResults.frame == 80 || sResults.frame == 120)
    {
        PlaySE(sResults.frame == 120 ? SE_EXP_MAX : SE_CONTEST_HEART);
        DrawResults(sResults.frame / 40);
    }
    if (sResults.frame < 120 || !JOY_NEW(A_BUTTON | B_BUTTON))
        return;
    PlaySE(SE_SELECT);
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        if (sResults.icons[i] != SPRITE_NONE)
            FreeAndDestroyMonIconSprite(&gSprites[sResults.icons[i]]);
    }
    FreeMonIconPalettes();
    ClearStdWindowAndFrameToTransparent(sResults.windowId, FALSE);
    ClearWindowTilemap(sResults.windowId);
    RemoveWindow(sResults.windowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Script, waitstate: the results in the last answer.
void CommunityContest_ShowResults(void)
{
    u32 i, count = min(gCommunityMailbox.respArgs[2], CONTEST_ENTRANTS);

    sResults.frame = 0;
    sResults.windowId = AddWindow(&sResultsWindowTemplate);
    DrawStdWindowFrame(sResults.windowId, FALSE);
    PutWindowTilemap(sResults.windowId);
    DrawResults(0);
    LoadMonIconPalettes();
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        const u8 *entry = &gCommunityMailbox.respData[i * RESULT_SIZE];
        u16 species = entry[20] | (entry[21] << 8);

        sResults.icons[i] = SPRITE_NONE;
        if (i >= count || species == SPECIES_NONE || species >= NUM_SPECIES)
            continue;
        sResults.icons[i] = CreateMonIcon(species, SpriteCB_MonIcon, 52, 8 + ROW_TOP + i * ROW_HEIGHT + 12, 0, 0);
        gSprites[sResults.icons[i]].oam.priority = 0;
    }
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Results, 80);
}
