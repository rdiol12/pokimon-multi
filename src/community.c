#include "global.h"
#include "battle_setup.h"
#include "community.h"
#include "data.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "main.h"
#include "menu.h"
#include "netplay.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

// The shared side of the community features: the mailbox the bridge syncs
// (include/community.h), requests that a script can wait on, a list window
// over the server's text rows, START > ONLINE > COMMUNITY, and levelled
// trainer battles. Each feature keeps the rest in its own file.

EWRAM_DATA struct CommunityMailbox gCommunityMailbox = {0};
EWRAM_DATA struct CommunityTrainerScale gCommunityTrainerScale = {0};
EWRAM_DATA struct CommunityShowcase gCommunityShowcase = {0};

#define RESPONSE_TIMEOUT (60 * 8)
#define OFFLINE_GRACE    (60 * 2)

static EWRAM_DATA u8 sExpectedSeq = 0;
static EWRAM_DATA u8 sMenuCursor = 0;

bool32 Community_IsOnline(void)
{
    return Netplay_IsOnline();
}

void Community_InitMailbox(void)
{
    static const char magic[8] = {'P', 'K', 'M', 'N', 'C', 'O', 'M', '1'};

    if (memcmp(gCommunityMailbox.magic, magic, sizeof(magic)) != 0)
    {
        memset(&gCommunityMailbox, 0, sizeof(gCommunityMailbox));
        gCommunityMailbox.version = COMMUNITY_MAILBOX_VERSION;
        memcpy(gCommunityMailbox.magic, magic, sizeof(magic));
    }
    gCommunityMailbox.partyAddr = (u32)gParties[B_TRAINER_PLAYER];
    gCommunityMailbox.partyCount = gPartiesCount[B_TRAINER_PLAYER];
    gCommunityMailbox.monSize = sizeof(struct Pokemon);
}

void Community_Request(u8 type, u32 a0, u32 a1, u32 a2, u32 a3, const void *data, u16 len)
{
    Community_InitMailbox();
    if (len > COMMUNITY_DATA_SIZE)
        len = COMMUNITY_DATA_SIZE;
    gCommunityMailbox.reqType = type;
    gCommunityMailbox.reqArgs[0] = a0;
    gCommunityMailbox.reqArgs[1] = a1;
    gCommunityMailbox.reqArgs[2] = a2;
    gCommunityMailbox.reqArgs[3] = a3;
    gCommunityMailbox.reqLen = len;
    if (len != 0)
        memcpy(gCommunityMailbox.reqData, data, len);
    sExpectedSeq = gCommunityMailbox.respSeq;
    gCommunityMailbox.reqSeq++; // last: the bridge sends it now
}

bool32 Community_HasResponse(u8 type)
{
    return gCommunityMailbox.respSeq != sExpectedSeq && gCommunityMailbox.respType == type;
}

static void RunShowcase(void)
{
    struct CommunityShowcase *s = &gCommunityShowcase;
    u32 i;

    s->pending = FALSE;
    if (s->gameClear)
        FlagSet(FLAG_SYS_GAME_CLEAR);
    for (i = 0; i < s->badges && i < 8; i++)
        FlagSet(FLAG_BADGE01_GET + i);
    for (i = 0; i < ARRAY_COUNT(s->flags); i++)
    {
        if (s->flags[i] != 0)
            FlagSet(s->flags[i]);
    }
    for (i = 0; i < ARRAY_COUNT(s->vars); i++)
    {
        if (s->vars[i] != 0)
            VarSet(s->vars[i], s->varValues[i]);
    }
    if (s->species[0] != SPECIES_NONE)
    {
        ZeroPlayerPartyMons();
        for (i = 0; i < PARTY_SIZE && s->species[i] != SPECIES_NONE; i++)
        {
            CreateMonWithIVs(&gParties[B_TRAINER_PLAYER][i], s->species[i], s->level,
                GetMonPersonality(s->species[i], MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
                OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
            GiveMonInitialMoveset(&gParties[B_TRAINER_PLAYER][i]);
        }
        CalculatePlayerPartyCount();
    }
    memset(s, 0, sizeof(*s));
}

bool32 Community_OverworldUpdate(void)
{
    Community_InitMailbox();
    if (gCommunityShowcase.pending && !ArePlayerFieldControlsLocked())
        RunShowcase();
    if (CommunityChat_OverworldUpdate != NULL && CommunityChat_OverworldUpdate())
        return TRUE;
    if (CommunityTour_OverworldUpdate != NULL && CommunityTour_OverworldUpdate())
        return TRUE;
    if (CommunityBoss_OverworldUpdate != NULL && CommunityBoss_OverworldUpdate())
        return TRUE;
    return FALSE;
}

// --- Waiting from a script ----------------------------------------------

#define tType  data[0]
#define tTimer data[1]
#define tSent  data[2]

static EWRAM_DATA struct {
    u32 args[3];
    u16 len;
    u8 data[COMMUNITY_DATA_SIZE];
} sPending = {0};

// The request goes out once the bridge is online: right after a screen
// change (the Easy Chat screen, a battle) it can take a moment.
static void Task_WaitResponse(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (!task->tSent && Community_IsOnline())
    {
        Community_Request(task->tType, sPending.args[0], sPending.args[1], sPending.args[2], 0, sPending.data, sPending.len);
        task->tSent = TRUE;
    }
    if (task->tSent && Community_HasResponse(task->tType))
    {
        gSpecialVar_Result = gCommunityMailbox.respStatus;
    }
    else if (++task->tTimer > (task->tSent ? RESPONSE_TIMEOUT : OFFLINE_GRACE))
    {
        gSpecialVar_Result = COMMUNITY_OFFLINE;
        gCommunityMailbox.rowCount = 0;
    }
    else
    {
        return;
    }
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Features: send a request, then waitstate until it's answered.
void Community_RequestAndWait(u8 type, u32 a0, u32 a1, u32 a2, const void *data, u16 len)
{
    u8 taskId = CreateTask(Task_WaitResponse, 80);

    gTasks[taskId].tType = type;
    gTasks[taskId].tTimer = 0;
    gTasks[taskId].tSent = FALSE;
    sPending.args[0] = a0;
    sPending.args[1] = a1;
    sPending.args[2] = a2;
    sPending.len = min(len, COMMUNITY_DATA_SIZE);
    if (sPending.len != 0)
        memcpy(sPending.data, data, sPending.len);
}

#undef tType
#undef tTimer
#undef tSent

// Script: VAR_0x8004 = COMMUNITY_REQ_*, VAR_0x8005..0x8007 = its numbers.
// waitstate; VAR_RESULT = the answer's status (COMMUNITY_OFFLINE if none).
void Community_ScriptRequest(void)
{
    Community_RequestAndWait(gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006, gSpecialVar_0x8007, NULL, 0);
}

// Script: rows 1-3 of the answer into STR_VAR_1..3 (row 0 is its title).
void Community_BufferRows(void)
{
    u8 *const vars[] = {gStringVar1, gStringVar2, gStringVar3};
    u32 i;

    for (i = 0; i < ARRAY_COUNT(vars); i++)
    {
        if (i + 1 < gCommunityMailbox.rowCount)
            StringCopyN(vars[i], gCommunityMailbox.rows[i + 1], COMMUNITY_ROW_LENGTH);
        else
            vars[i][0] = EOS;
        vars[i][COMMUNITY_ROW_LENGTH] = EOS;
    }
}

// Script: VAR_RESULT = the answer's number VAR_0x8004 (0-3).
void Community_GetRespArg(void)
{
    gSpecialVar_Result = gCommunityMailbox.respArgs[gSpecialVar_0x8004 & 3];
}

// --- The row list ---------------------------------------------------------

#define LIST_ROWS        8
#define LIST_LINE_HEIGHT 14
#define LIST_TOP         16

static const struct WindowTemplate sListWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

#define tWindowId data[0]
#define tCursor   data[1]
#define tScroll   data[2]
#define tChoose   data[3]

static u32 ListCount(void)
{
    return gCommunityMailbox.rowCount > 1 ? gCommunityMailbox.rowCount - 1 : 0;
}

static void DrawList(struct Task *task)
{
    u8 line[COMMUNITY_ROW_LENGTH + 1];
    u32 i, count = ListCount();

    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(1));
    if (gCommunityMailbox.rowCount != 0)
    {
        StringCopyN(line, gCommunityMailbox.rows[0], COMMUNITY_ROW_LENGTH);
        line[COMMUNITY_ROW_LENGTH] = EOS;
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, line, 4, 0, TEXT_SKIP_DRAW, NULL);
    }
    for (i = 0; i < LIST_ROWS && task->tScroll + i < count; i++)
    {
        u32 index = task->tScroll + i;
        u32 y = LIST_TOP + i * LIST_LINE_HEIGHT;

        StringCopyN(line, gCommunityMailbox.rows[index + 1], COMMUNITY_ROW_LENGTH);
        line[COMMUNITY_ROW_LENGTH] = EOS;
        if (task->tChoose && index == task->tCursor)
            AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, gText_SelectorArrow2, 0, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, line, 10, y, TEXT_SKIP_DRAW, NULL);
    }
    if (gCommunityMailbox.rowCount == 0)
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, COMPOUND_STRING("The server didn't answer."), 10, LIST_TOP, TEXT_SKIP_DRAW, NULL);
    else if (count == 0)
        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, COMPOUND_STRING("Nothing here yet."), 10, LIST_TOP, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
}

static void Task_RowList(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s16 old = task->tCursor;
    u32 count = ListCount();
    bool32 picked = task->tChoose && count != 0 && JOY_NEW(A_BUTTON);

    if (picked || JOY_NEW(B_BUTTON) || (!task->tChoose && JOY_NEW(A_BUTTON)))
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = picked ? task->tCursor : 0xFF;
        ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
        ClearWindowTilemap(task->tWindowId);
        RemoveWindow(task->tWindowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (count == 0)
        return;
    if (task->tChoose)
    {
        if (JOY_REPEAT(DPAD_DOWN) && task->tCursor + 1 < count)
            task->tCursor++;
        else if (JOY_REPEAT(DPAD_UP) && task->tCursor > 0)
            task->tCursor--;
    }
    else
    {
        if (JOY_REPEAT(DPAD_DOWN) && task->tCursor + LIST_ROWS < count)
            task->tCursor += LIST_ROWS;
        else if (JOY_REPEAT(DPAD_UP) && task->tCursor >= LIST_ROWS)
            task->tCursor -= LIST_ROWS;
    }
    if (task->tCursor != old)
    {
        if (task->tCursor < task->tScroll)
            task->tScroll = task->tCursor;
        else if (task->tCursor >= task->tScroll + LIST_ROWS)
            task->tScroll = task->tCursor - LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawList(task);
    }
}

void Community_ShowRowList(bool32 choose)
{
    u8 taskId = CreateTask(Task_RowList, 80);
    struct Task *task = &gTasks[taskId];

    task->tChoose = choose;
    task->tCursor = 0;
    task->tScroll = 0;
    task->tWindowId = AddWindow(&sListWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    DrawList(task);
    ScheduleBgCopyTilemapToVram(0);
}

// Script, waitstate: show the answer's rows.
void Community_ShowRows(void)
{
    Community_ShowRowList(FALSE);
}

// Script, waitstate: pick one of the answer's rows. VAR_RESULT: its index or 0xFF.
void Community_ChooseRow(void)
{
    Community_ShowRowList(TRUE);
}

#undef tWindowId
#undef tCursor
#undef tScroll
#undef tChoose

// --- START > ONLINE > COMMUNITY -------------------------------------------

struct CommunityMenuRow
{
    const u8 *text;
    void (*open)(void);
};

static const struct CommunityMenuRow sMenuRows[COMMUNITY_MENU_COUNT] =
{
    [COMMUNITY_MENU_TOURNAMENT] = {COMPOUND_STRING("TOURNAMENT"), Community_OpenTournament},
    [COMMUNITY_MENU_DAYCARE]    = {COMPOUND_STRING("DAYCARE"),    Community_OpenDaycare},
    [COMMUNITY_MENU_BOSS]       = {COMPOUND_STRING("BOSS"),       Community_OpenBoss},
    [COMMUNITY_MENU_CARDS]      = {COMPOUND_STRING("CARDS"),      Community_OpenCards},
    [COMMUNITY_MENU_CONTEST]    = {COMPOUND_STRING("CONTEST"),    Community_OpenContest},
    [COMMUNITY_MENU_CHAT]       = {COMPOUND_STRING("CHAT"),       Community_OpenChat},
    [COMMUNITY_MENU_LEGAL]      = {COMPOUND_STRING("CHECK TEAM"), Community_OpenLegalCheck},
};

#define tWindowId data[0]
#define tCount    data[1]
#define tRows     data[2] // .. data[2 + COMMUNITY_MENU_COUNT - 1]: sMenuRows index per line

static void Task_CommunityMenu(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s8 input = Menu_ProcessInputNoWrap();
    void (*open)(void);

    if (input == MENU_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    ClearStdWindowAndFrame(task->tWindowId, TRUE);
    RemoveWindow(task->tWindowId);
    if (input == MENU_B_PRESSED || task->tCount == 0)
    {
        DestroyTask(taskId);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        return;
    }
    sMenuCursor = input;
    open = sMenuRows[task->data[2 + input]].open;
    DestroyTask(taskId);
    open();
}

void ShowCommunityMenu(void)
{
    static const struct WindowTemplate template =
    {
        .bg = 0,
        .tilemapLeft = 18,
        .tilemapTop = 1,
        .width = 11,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x139,
    };
    struct MenuAction items[COMMUNITY_MENU_COUNT];
    struct WindowTemplate window = template;
    u8 taskId = CreateTask(Task_CommunityMenu, 80);
    struct Task *task = &gTasks[taskId];
    u32 i, count = 0;

    Community_InitMailbox();
    for (i = 0; i < COMMUNITY_MENU_COUNT; i++)
    {
        if (sMenuRows[i].open == NULL)
            continue;
        items[count].text = sMenuRows[i].text;
        task->data[2 + count] = i;
        count++;
    }
    if (count == 0)
    {
        items[0].text = COMPOUND_STRING("NOTHING YET");
        task->data[2] = 0;
    }
    task->tCount = count;
    window.height = max(count, 1) * 2;
    task->tWindowId = AddWindow(&window);
    if (sMenuCursor >= max(count, 1))
        sMenuCursor = 0;
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PrintMenuTable(task->tWindowId, max(count, 1), items);
    InitMenuInUpperLeftCornerNormal(task->tWindowId, max(count, 1), sMenuCursor);
    ScheduleBgCopyTilemapToVram(0);
}

#undef tWindowId
#undef tCount
#undef tRows

// --- Levelled trainer battles ----------------------------------------------

u8 Community_GetPlayerMaxLevel(void)
{
    u32 i, best = 1;

    for (i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        u32 level;
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES_OR_EGG) == SPECIES_EGG)
            continue;
        level = GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_LEVEL);
        if (level > best)
            best = level;
    }
    return best;
}

// Script: VAR_0x8004 = trainer, VAR_0x8005 = party size (0: as defined),
// VAR_0x8006 = level bonus over the player's best (signed). The next battle
// against that trainer uses it.
void Community_SetTrainerScale(void)
{
    gCommunityTrainerScale.trainerId = gSpecialVar_0x8004;
    gCommunityTrainerScale.partySize = gSpecialVar_0x8005;
    gCommunityTrainerScale.levelBonus = (s16)gSpecialVar_0x8006;
    gCommunityTrainerScale.aceSpecies = SPECIES_NONE;
}

// Called by CreateNPCTrainerParty (battle_setup.c).
bool32 Community_OverrideTrainerParty(struct Pokemon *party, u16 trainerId)
{
    struct Trainer trainer;
    u32 i, level;

    if (gCommunityTrainerScale.trainerId == 0 || gCommunityTrainerScale.trainerId != trainerId)
        return FALSE;
    memcpy(&trainer, GetTrainerStructFromId(trainerId), sizeof(trainer));
    if (gCommunityTrainerScale.partySize != 0 && gCommunityTrainerScale.partySize < trainer.partySize)
        trainer.partySize = gCommunityTrainerScale.partySize;
    CreateNPCTrainerPartyFromTrainer(party, &trainer);
    if (gCommunityTrainerScale.aceSpecies != SPECIES_NONE && trainer.partySize < PARTY_SIZE)
    {
        CreateMonWithIVs(&party[trainer.partySize], gCommunityTrainerScale.aceSpecies, 5,
            GetMonPersonality(gCommunityTrainerScale.aceSpecies, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
            OTID_STRUCT_RANDOM_NO_SHINY, MAX_PER_STAT_IVS);
    }

    for (i = 0; i < PARTY_SIZE; i++)
    {
        enum Species species = GetMonData(&party[i], MON_DATA_SPECIES);
        u32 exp;

        if (species == SPECIES_NONE)
            continue;
        // The last one, their ace, is two levels above the rest.
        level = Community_GetPlayerMaxLevel() + gCommunityTrainerScale.levelBonus;
        if (i + 1 == PARTY_SIZE || GetMonData(&party[i + 1], MON_DATA_SPECIES) == SPECIES_NONE)
            level += 2;
        level = min(max(level, 5), MAX_LEVEL);
        exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
        SetMonData(&party[i], MON_DATA_EXP, &exp);
        SetMonData(&party[i], MON_DATA_LEVEL, &level);
        CalculateMonStats(&party[i]);
        if (i == trainer.partySize)
            GiveMonInitialMoveset(&party[i]); // the ace, at its new level
    }
    gCommunityTrainerScale.trainerId = 0;
    gCommunityTrainerScale.aceSpecies = SPECIES_NONE;
    return TRUE;
}
