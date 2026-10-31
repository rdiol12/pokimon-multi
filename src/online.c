#include "global.h"
#include "coins.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "online.h"
#include "pokemon.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "wild_encounter.h"
#include "world.h"
#include "random.h"
#include "battle.h"
#include "battle_setup.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"

// Sixth batch of online features: the shared part. Requests go over the
// Online Plaza's channel (NpxRpc_* in src/netplay_ext.c) and every answer
// has the same shape (docs/online.md):
//
//   row 0:   message|code|var1|var2|var3
//            message goes to gStringVar4 ("~" is a new line, "^" a new
//            page), code to VAR_0x800B, var1-3 to gStringVar1-3.
//   actions: +ITEM|id|count, -ITEM|id|count, +MONEY|n, -MONEY|n, +COINS|n, -COINS|n,
//            +MON|species|level|shiny: applied as soon as the answer comes.
//   N|text:  a notice, shown once the player is free.
//   S|key|.. the status of a feature (Online_Status).
//   others:  data rows, for Online_ShowBoard and Online_Choose; their count
//            goes to VAR_0x800A.
//
// In the background the game sends queued reports (OEV) and asks for the
// status of the live features (OSTAT) every few seconds.

#define DEFAULT_TIMEOUT   (60 * 6)
#define STATUS_INTERVAL   (60 * 10)
#define REPORT_QUEUE      8
#define REPORT_LENGTH     48
#define NOTICE_QUEUE      4
#define NOTICE_LENGTH     120
#define STATUS_KEYS       12
#define STATUS_FIELDS     6
#define STATUS_FIELD_LEN  16

struct OnlineStatusEntry
{
    char key[8];
    char fields[STATUS_FIELDS][STATUS_FIELD_LEN];
};

struct OnlineState
{
    u8 dataRows[NPX_MAX_ROWS];
    u8 dataCount;
    u8 reportCount;
    u8 noticeCount;
    u8 statusCount;
    bool8 background; // a report or status request is out
    bool8 statusRequest; // ... and it's OSTAT, whose answer is the whole status
    u16 statusTimer;
    char reports[REPORT_QUEUE][REPORT_LENGTH];
    u8 notices[NOTICE_QUEUE][NOTICE_LENGTH];
    struct OnlineStatusEntry status[STATUS_KEYS];
};

static EWRAM_DATA struct OnlineState sOnline = {0};

// The wild Pokémon of the current battle (gEnemyParty may be cleared by the
// time the battle ends).
struct OnlineWild
{
    u16 species;
    bool8 fished;
    u8 filler;
    u32 personality;
};

static EWRAM_DATA struct OnlineWild sWild = {0};

// The screenshot driver (tools/online_shots/) writes a scene here; the next
// free overworld frame plays it.
enum
{
    SHOWCASE_NONE,
    SHOWCASE_ROAMER, // a roaming species/level battle starts
    SHOWCASE_WILD,   // a wild species/level (shiny if set) battle starts
    SHOWCASE_ITEM,   // `level` of item `species` go in the bag
    SHOWCASE_BADGES, // the player has the first `level` badges
};

struct OnlineShowcase
{
    u8 kind;
    u8 level;
    u16 species;
    u8 shiny;
    u8 filler[3];
};

EWRAM_DATA struct OnlineShowcase gOnlineShowcase = {0};

extern const u8 Online_EventScript_Hub[];
extern const u8 Online_EventScript_Notice[];

static bool32 IsOnline(void)
{
    return gNetplayMailbox.bridgeState == NETPLAY_ONLINE;
}

static bool32 StrEq(const char *a, const char *b)
{
    while (*a != 0 && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
}

static void CopyAscii(char *dest, const char *src, u32 size)
{
    u32 i;

    for (i = 0; i + 1 < size && src[i] != 0; i++)
        dest[i] = src[i];
    dest[i] = 0;
}

static s32 AsciiToNum(const char *s)
{
    s32 n = 0;
    bool32 negative = (*s == '-');

    if (negative)
        s++;
    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return negative ? -n : n;
}

// Server text in the game's charset: "~" is a new line, "^" a new page,
// "#" is "é", "$" the money sign, ">" itself.
u8 *Online_CopyText(u8 *dest, const char *src, u32 maxChars)
{
    char one[2] = {0, 0};

    while (*src != 0 && maxChars-- != 0)
    {
        if (*src == '~')
            *dest++ = CHAR_NEWLINE;
        else if (*src == '^')
            *dest++ = CHAR_PROMPT_CLEAR;
        else if (*src == '#')
            *dest++ = CHAR_e_ACUTE;
        else if (*src == '$')
            *dest++ = CHAR_CURRENCY;
        else if (*src == '>')
            *dest++ = CHAR_GREATER_THAN;
        else
        {
            one[0] = *src;
            Npx_AsciiToGame(dest, one, 1);
            dest++;
        }
        src++;
    }
    *dest = EOS;
    return dest;
}

// Answers ------------------------------------------------------------------

u32 Online_DataRows(void)
{
    return sOnline.dataCount;
}

const char *Online_DataField(u32 row, u32 col)
{
    if (row >= sOnline.dataCount)
        return "";
    return Npx_Field(sOnline.dataRows[row], col);
}

s32 Online_DataNum(u32 row, u32 col)
{
    return AsciiToNum(Online_DataField(row, col));
}

static void GiveMon(u16 species, u8 level, bool32 shiny)
{
    struct Pokemon mon;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    CreateMonWithIVs(&mon, species, level, GetMonPersonality(species, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
        OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    if (shiny)
        SetMonData(&mon, MON_DATA_IS_SHINY, &shiny);
    GiveCapturedMonToPlayer(&mon);
}

static void QueueNotice(const char *text)
{
    if (sOnline.noticeCount >= NOTICE_QUEUE)
        return;
    Online_CopyText(sOnline.notices[sOnline.noticeCount++], text, NOTICE_LENGTH - 1);
}

static void SetStatus(u32 row)
{
    const char *key = Npx_Field(row, 1);
    u32 i, f;

    for (i = 0; i < sOnline.statusCount; i++)
    {
        if (StrEq(sOnline.status[i].key, key))
            break;
    }
    if (i == sOnline.statusCount)
    {
        if (i >= STATUS_KEYS)
            return;
        sOnline.statusCount++;
        CopyAscii(sOnline.status[i].key, key, sizeof(sOnline.status[i].key));
    }
    for (f = 0; f < STATUS_FIELDS; f++)
        CopyAscii(sOnline.status[i].fields[f], Npx_Field(row, f + 2), STATUS_FIELD_LEN);
}

// Applies the action, notice and status rows of the answer just taken, and
// keeps the others as data rows. Returns FALSE for a refusal's plain text.
static void TakeRows(bool32 background)
{
    u32 row;

    sOnline.dataCount = 0;
    if (background && sOnline.statusRequest)
        sOnline.statusCount = 0;
    for (row = 1; row < Npx_RowCount(); row++)
    {
        const char *what = Npx_Field(row, 0);
        s32 a = Npx_Num(row, 1), b = Npx_Num(row, 2);

        if (StrEq(what, "+ITEM"))
            AddBagItem(a, b);
        else if (StrEq(what, "-ITEM"))
            RemoveBagItem(a, b);
        else if (StrEq(what, "+MONEY"))
            AddMoney(&gSaveBlock1Ptr->money, a);
        else if (StrEq(what, "-MONEY"))
            RemoveMoney(&gSaveBlock1Ptr->money, a);
        else if (StrEq(what, "+COINS"))
            AddCoins(a);
        else if (StrEq(what, "-COINS"))
            RemoveCoins(a);
        else if (StrEq(what, "+MON"))
            GiveMon(a, b, Npx_Num(row, 3));
        else if (StrEq(what, "N"))
            QueueNotice(Npx_Field(row, 1));
        else if (StrEq(what, "S"))
            SetStatus(row);
        else if (!background)
            sOnline.dataRows[sOnline.dataCount++] = row;
    }
    if (!background)
        gSpecialVar_0x800A = sOnline.dataCount;
}

static void TakeMessageRow(void)
{
    Online_CopyText(gStringVar4, Npx_Field(0, 0), 250);
    gSpecialVar_0x800B = Npx_Num(0, 1);
    Online_CopyText(gStringVar1, Npx_Field(0, 2), 30);
    Online_CopyText(gStringVar2, Npx_Field(0, 3), 30);
    Online_CopyText(gStringVar3, Npx_Field(0, 4), 30);
}

const char *Online_Status(const char *key, u32 field)
{
    u32 i;

    if (field >= STATUS_FIELDS)
        return "";
    for (i = 0; i < sOnline.statusCount; i++)
    {
        if (StrEq(sOnline.status[i].key, key))
            return sOnline.status[i].fields[field];
    }
    return "";
}

s32 Online_StatusNum(const char *key, u32 field)
{
    return AsciiToNum(Online_Status(key, field));
}

// Online_Request -----------------------------------------------------------
//
// VAR_0x8004: ONL_REQ_*. Sends the request and waits for the answer.
// VAR_RESULT: ONLINE_OK, _REFUSED or _FAILED; the server's message (or the
// reason it failed) is in gStringVar4.

#define tReq   data[0]
#define tState data[1]

static void Task_Request(u8 taskId);

void Online_Request(void)
{
    u8 taskId = CreateTask(Task_Request, 80);
    gTasks[taskId].tReq = gSpecialVar_0x8004;
    gTasks[taskId].tState = 0;
}

static void FinishRequest(u8 taskId, u16 result)
{
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void AddMonArgs(u32 slot)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][slot < PARTY_SIZE ? slot : 0];
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];

    NpxRpc_Num(GetMonData(mon, MON_DATA_SPECIES));
    NpxRpc_Num(GetMonData(mon, MON_DATA_LEVEL));
    NpxRpc_Num(GetMonData(mon, MON_DATA_IS_SHINY));
    NpxRpc_Num(GetMonData(mon, MON_DATA_PERSONALITY) & 0xFFFF);
    GetMonData(mon, MON_DATA_NICKNAME, nickname);
    NpxRpc_Name(nickname);
}

static void BuildRequest(const struct OnlineRequest *req)
{
    static const u16 *const sVars[] = {&gSpecialVar_0x8005, &gSpecialVar_0x8006, &gSpecialVar_0x8007};
    u32 i;

    for (i = 0; i < req->numbers && i < ARRAY_COUNT(sVars); i++)
        NpxRpc_Num(*sVars[i]);
    if (req->flags & ONL_ARG_CHOSEN)
        NpxRpc_Word(Npx_Chosen(1)[0] != 0 ? Npx_Chosen(1) : "0");
    if (req->flags & ONL_ARG_MON)
        AddMonArgs(gSpecialVar_0x8005);
    if (req->flags & ONL_ARG_MONEY)
        NpxRpc_Num(GetMoney(&gSaveBlock1Ptr->money));
    if (req->flags & ONL_ARG_NAME)
        NpxRpc_Name(gSaveBlock2Ptr->playerName);
    if (req->flags & ONL_ARG_COINS)
        NpxRpc_Num(GetCoins());
    if (req->build != NULL)
        req->build();
}

static void Task_Request(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    const struct OnlineRequest *req = &gOnlineRequests[tReq];

    switch (tState)
    {
    case 0:
        if (!IsOnline())
        {
            StringCopy(gStringVar4, COMPOUND_STRING("You're not online. Pick JOIN ONLINE\non the title menu to play online."));
            FinishRequest(taskId, ONLINE_FAILED);
            return;
        }
        // The plaza's title poll or one of our reports may still be out.
        if (sOnline.background || !NpxRpc_Begin(req->text))
            return;
        BuildRequest(req);
        NpxRpc_Send();
        tState++;
        break;
    case 1:
        switch (NpxRpc_Poll())
        {
        case NPX_RPC_DONE:
            Npx_TakeAnswer();
            NpxRpc_Release();
            if (gNetplayExtMailbox.status == 0)
            {
                TakeMessageRow();
                TakeRows(FALSE);
                if (req->after != NULL)
                    req->after();
                FinishRequest(taskId, ONLINE_OK);
            }
            else
            {
                Online_CopyText(gStringVar4, Npx_Field(0, 0), 200);
                gSpecialVar_0x800A = 0;
                sOnline.dataCount = 0;
                FinishRequest(taskId, ONLINE_REFUSED);
            }
            break;
        case NPX_RPC_FAILED:
            NpxRpc_Release();
            StringCopy(gStringVar4, COMPOUND_STRING("The server didn't answer."));
            FinishRequest(taskId, ONLINE_FAILED);
            break;
        }
        break;
    }
}

#undef tReq
#undef tState

// Boards -------------------------------------------------------------------
//
// Online_ShowBoard: a board from the data rows of the last answer, closed
// with A or B. Data row 0 is the title, row 1 the columns ("x:LABEL" each),
// then one row per line. A line whose first field starts with "*" is the
// player's own and is drawn in red. A line "%|percent|text|right text|
// count" is a progress bar, with its text above it and its count beside it.

#define BOARD_TOP_ROWS 2
#define BOARD_LINE     11
#define BOARD_LINES    8
#define BOARD_WIDTH    26

static const struct WindowTemplate sBoardWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 1,
    .width = BOARD_WIDTH,
    .height = 15,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sColorsNormal[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sColorsMine[3]   = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};
static const u8 sColorsHeader[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};

static u32 ColumnX(u32 col)
{
    return AsciiToNum(Online_DataField(1, col));
}

static const char *ColumnLabel(u32 col)
{
    const char *f = Online_DataField(1, col);

    while (*f != 0 && *f != ':')
        f++;
    return *f == ':' ? f + 1 : f;
}

#define BAR_WIDTH (BOARD_WIDTH * 8 - 64)

static void DrawProgressBar(u8 windowId, u32 y, u32 percent)
{
    u32 filled;

    if (percent > 100)
        percent = 100;
    filled = (BAR_WIDTH - 2) * percent / 100;
    FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), 10, y, BAR_WIDTH, 6);
    FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_WHITE), 11, y + 1, BAR_WIDTH - 2, 4);
    if (filled != 0)
        FillWindowPixelRect(windowId, PIXEL_FILL(percent >= 100 ? TEXT_COLOR_GREEN : TEXT_COLOR_BLUE), 11, y + 1, filled, 4);
}

static void DrawBoard(u8 windowId)
{
    u32 cols, row, line = 0, c;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(TEXT_COLOR_WHITE));
    Online_CopyText(gStringVar4, Online_DataField(0, 0), 40);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 6, 1, TEXT_SKIP_DRAW, NULL);

    for (cols = 0; cols < NPX_MAX_COLS && Online_DataField(1, cols)[0] != 0; cols++)
    {
        Online_CopyText(gStringVar4, ColumnLabel(cols), 40);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, ColumnX(cols), 16, sColorsHeader, TEXT_SKIP_DRAW, gStringVar4);
    }

    for (row = BOARD_TOP_ROWS; row < Online_DataRows() && line < BOARD_LINES; row++)
    {
        const char *first = Online_DataField(row, 0);
        u32 y = 28 + line * BOARD_LINE;

        if (first[0] == '%')
        {
            Online_CopyText(gStringVar4, Online_DataField(row, 2), 40);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, 10, y, sColorsNormal, TEXT_SKIP_DRAW, gStringVar4);
            Online_CopyText(gStringVar4, Online_DataField(row, 3), 24);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, BOARD_WIDTH * 8 - 14 - GetStringWidth(FONT_SMALL, gStringVar4, 0),
                y, sColorsMine, TEXT_SKIP_DRAW, gStringVar4);
            DrawProgressBar(windowId, y + BOARD_LINE + 1, Online_DataNum(row, 1));
            Online_CopyText(gStringVar4, Online_DataField(row, 4), 12);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, 16 + BAR_WIDTH, y + BOARD_LINE - 2, sColorsNormal, TEXT_SKIP_DRAW, gStringVar4);
            line += 2;
            continue;
        }
        for (c = 0; c < cols; c++)
        {
            const char *f = Online_DataField(row, c);
            if (c == 0 && f[0] == '*')
                f++;
            Online_CopyText(gStringVar4, f, 22);
            AddTextPrinterParameterized3(windowId, FONT_SMALL, ColumnX(c), y,
                first[0] == '*' ? sColorsMine : sColorsNormal, TEXT_SKIP_DRAW, gStringVar4);
        }
        line++;
    }
    if (Online_DataRows() <= BOARD_TOP_ROWS)
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 10, 28, sColorsNormal, TEXT_SKIP_DRAW,
            COMPOUND_STRING("Nothing here yet."));
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

#define tWindowId data[0]

static void Task_Board(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
        ClearWindowTilemap(task->tWindowId);
        RemoveWindow(task->tWindowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

void Online_ShowBoard(void)
{
    u8 taskId = CreateTask(Task_Board, 80);
    struct Task *task = &gTasks[taskId];

    task->tWindowId = AddWindow(&sBoardWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    DrawBoard(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
}

#undef tWindowId

// Pick lists ---------------------------------------------------------------
//
// Online_Choose: picks one of the data rows of the last answer by its first
// field. VAR_RESULT: the row, or ONLINE_CHOSE_NOTHING. The row's fields are
// kept (Npx_Chosen; ONL_ARG_CHOSEN sends field 1) and its first field goes
// to gStringVar3.

#define LIST_SHOWN 6

#define tListTaskId data[0]
#define tWindowId   data[1]

// A scrolling list in a window, run by func (which calls
// ListMenu_ProcessInput on tListTaskId and Online_CloseList when done).
// items has count rows, the last usually Cancel.
u8 Online_OpenList(const struct ListMenuItem *items, u32 count, void (*func)(u8 taskId))
{
    struct ListMenuTemplate template = gMultiuseListMenuTemplate;
    struct WindowTemplate window = {.bg = 0, .tilemapLeft = 4, .tilemapTop = 1, .width = 22, .paletteNum = 15, .baseBlock = 0x008};
    u8 taskId = CreateTask(func, 80);
    struct Task *task = &gTasks[taskId];

    window.height = min(count, LIST_SHOWN) * 2;
    task->tWindowId = AddWindow(&window);
    DrawStdWindowFrame(task->tWindowId, FALSE);

    template.items = items;
    template.totalItems = count;
    template.maxShowed = min(count, LIST_SHOWN);
    template.windowId = task->tWindowId;
    template.header_X = 0;
    template.item_X = 8;
    template.cursor_X = 0;
    template.upText_Y = 1;
    template.fontId = FONT_NORMAL;
    template.itemVerticalPadding = 0;
    template.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    template.cursorKind = CURSOR_BLACK_ARROW;
    template.moveCursorFunc = NULL;
    template.itemPrintFunc = NULL;
    task->tListTaskId = ListMenuInit(&template, 0, 0);
    CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    return taskId;
}

void Online_CloseList(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Choose(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL)
    {
        gSpecialVar_Result = ONLINE_CHOSE_NOTHING;
    }
    else
    {
        gSpecialVar_Result = input;
        Npx_ChooseAnswerRow(sOnline.dataRows[input]);
        Online_CopyText(gStringVar3, Online_DataField(input, 0), 30);
    }
    Online_CloseList(taskId);
}

void Online_Choose(void)
{
    u32 i, count = min(Online_DataRows(), NPX_MAX_ROWS);

    for (i = 0; i < count; i++)
    {
        Online_CopyText(gNpxListText[i], Online_DataField(i, 0), 24);
        gNpxListItems[i].name = gNpxListText[i];
        gNpxListItems[i].id = i;
    }
    gNpxListItems[count].name = gText_Cancel;
    gNpxListItems[count].id = LIST_CANCEL;
    Online_OpenList(gNpxListItems, count + 1, Task_Choose);
}

#undef tListTaskId
#undef tWindowId

// Background ---------------------------------------------------------------

void Online_Report(const char *word, u32 argCount, const s32 *args)
{
    char *dest;
    u32 len = 0, i;

    if (!IsOnline() || sOnline.reportCount >= REPORT_QUEUE)
        return;
    dest = sOnline.reports[sOnline.reportCount++];
    for (i = 0; word[i] != 0 && len < REPORT_LENGTH - 1; i++)
        dest[len++] = word[i];
    for (i = 0; i < argCount && i < ONLINE_REPORT_ARGS; i++)
    {
        char buf[12];
        u32 n = ARRAY_COUNT(buf) - 1;
        bool32 negative = args[i] < 0;
        u32 v = negative ? -args[i] : args[i];

        buf[n] = 0;
        do
        {
            buf[--n] = '0' + v % 10;
            v /= 10;
        } while (v != 0 && n > 1);
        if (negative)
            buf[--n] = '-';
        if (len < REPORT_LENGTH - 1)
            dest[len++] = ' ';
        while (buf[n] != 0 && len < REPORT_LENGTH - 1)
            dest[len++] = buf[n++];
    }
    dest[len] = 0;
}

static void SendBackground(void)
{
    if (sOnline.reportCount != 0)
    {
        u32 i;
        if (!NpxRpc_Begin("OEV"))
            return;
        NpxRpc_Word(sOnline.reports[0]);
        for (i = 1; i < sOnline.reportCount; i++)
            memcpy(sOnline.reports[i - 1], sOnline.reports[i], REPORT_LENGTH);
        sOnline.reportCount--;
        sOnline.statusRequest = FALSE;
    }
    else
    {
        if (++sOnline.statusTimer < STATUS_INTERVAL || !NpxRpc_Begin("OSTAT"))
            return;
        sOnline.statusTimer = 0;
        Online_CheckBadges(); // queued for the next round
        sOnline.statusRequest = TRUE;
    }
    NpxRpc_Send();
    sOnline.background = TRUE;
}

static void PollBackground(void)
{
    switch (NpxRpc_Poll())
    {
    case NPX_RPC_DONE:
        Npx_TakeAnswer();
        NpxRpc_Release();
        if (gNetplayExtMailbox.status == 0)
            TakeRows(TRUE);
        sOnline.background = FALSE;
        break;
    case NPX_RPC_FAILED:
        NpxRpc_Release();
        sOnline.background = FALSE;
        break;
    }
}

static bool32 RunShowcase(void)
{
    u32 kind = gOnlineShowcase.kind;

    gOnlineShowcase.kind = SHOWCASE_NONE;
    switch (kind)
    {
    case SHOWCASE_ROAMER:
    case SHOWCASE_WILD:
        ZeroEnemyPartyMons();
        CreateMonWithIVs(&gParties[B_TRAINER_OPPONENT_A][0], gOnlineShowcase.species, gOnlineShowcase.level,
            GetMonPersonality(gOnlineShowcase.species, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
            OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
        if (gOnlineShowcase.shiny)
        {
            bool32 shiny = TRUE;
            SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_IS_SHINY, &shiny);
        }
        if (kind == SHOWCASE_ROAMER)
            BattleSetup_StartRoamerBattle();
        else
            BattleSetup_StartWildBattle();
        return TRUE;
    case SHOWCASE_ITEM:
        AddBagItem(gOnlineShowcase.species, gOnlineShowcase.level);
        break;
    case SHOWCASE_BADGES:
    {
        u32 i;
        for (i = 0; i < NUM_BADGES; i++)
        {
            if (i < gOnlineShowcase.level)
                FlagSet(FLAG_BADGE01_GET + i);
            else
                FlagClear(FLAG_BADGE01_GET + i);
        }
        break;
    }
    }
    return FALSE;
}

// Asks for the status within a few frames (after a request that changes it).
void Online_PollStatusSoon(void)
{
    if (sOnline.statusTimer < STATUS_INTERVAL - 10)
        sOnline.statusTimer = STATUS_INTERVAL - 10;
}

// Called every overworld frame. Returns TRUE if it started a script.
bool32 Online_OverworldUpdate(void)
{
    if (Online_RaceUpdate())
        return TRUE;
    if (!IsOnline())
    {
        sOnline.background = FALSE;
        sOnline.reportCount = 0;
        sOnline.statusCount = 0;
        sOnline.statusTimer = STATUS_INTERVAL - 60;
        return FALSE;
    }
    if (sOnline.background)
        PollBackground();
    // Only while no script runs: a script may be reading the last answer.
    else if (!ArePlayerFieldControlsLocked())
        SendBackground();

    // Racers aren't stopped for notices or invites; they wait for the finish.
    if (Online_RaceIsOn())
        return FALSE;

    if (gOnlineShowcase.kind != SHOWCASE_NONE && !ArePlayerFieldControlsLocked())
        return RunShowcase();

    if (!ArePlayerFieldControlsLocked() && Online_TryCoFishInvite())
        return TRUE;

    if (!ArePlayerFieldControlsLocked() && IsPlayerStandingStill() && Online_PlayerQuestUpdate())
        return TRUE;

    if (sOnline.noticeCount != 0 && !ArePlayerFieldControlsLocked() && IsPlayerStandingStill())
    {
        StringCopy(gStringVar4, sOnline.notices[0]);
        memmove(sOnline.notices[0], sOnline.notices[1], sizeof(sOnline.notices[0]) * (NOTICE_QUEUE - 1));
        sOnline.noticeCount--;
        ScriptContext_SetupScript(Online_EventScript_Notice);
        return TRUE;
    }
    return FALSE;
}

// A wild battle is starting: a roaming Pokémon or a legend is a sighting
// for the server's board (and a notice for everyone online).
void Online_OnWildBattleStart(void)
{
    u16 species;
    s32 args[4];

    if (gIsFishingEncounter)
        Online_TournamentBite();
    species = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
    sWild.species = species;
    sWild.personality = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PERSONALITY);
    sWild.fished = gIsFishingEncounter;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    if (!(gBattleTypeFlags & BATTLE_TYPE_ROAMER) && !gSpeciesInfo[species].isRestrictedLegendary
     && !gSpeciesInfo[species].isSubLegendary && !gSpeciesInfo[species].isMythical)
        return;
    args[0] = species;
    args[1] = gSaveBlock1Ptr->location.mapGroup;
    args[2] = gSaveBlock1Ptr->location.mapNum;
    args[3] = (gBattleTypeFlags & BATTLE_TYPE_ROAMER) != 0;
    Online_Report("SIGHT", ARRAY_COUNT(args), args);
}

// Back from a wild battle where the player caught the Pokémon.
void Online_OnCatch(void)
{
    s32 args[5];

    args[0] = gBattleResults.caughtMonSpecies;
    args[1] = gBattleResults.shinyWildMon;
    args[2] = gSaveBlock1Ptr->location.mapGroup;
    args[3] = gSaveBlock1Ptr->location.mapNum;
    args[4] = sWild.fished && sWild.species == gBattleResults.caughtMonSpecies;
    Online_Report("CATCH", ARRAY_COUNT(args), args);
    if (args[4])
    {
        args[1] = Records_SizeOf(sWild.species, sWild.personality);
        Online_Report("FISH", 2, args);
    }
}

// For the ONLINE menu.
void Online_OpenHubFromMenu(void)
{
    ScriptContext_SetupScript(Online_EventScript_Hub);
}
