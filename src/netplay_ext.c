#include "global.h"
#include "netplay.h"
#include "netplay_ext.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/songs.h"

// Second batch of online features: the request channel to the server,
// boards and pick lists built from the server's answers, and the title
// badges shown above players on the overworld. The features themselves are
// in src/netplay_ext_features.c, the Online Plaza screen in
// src/netplay_ext_plaza.c. See docs/netplay_ext.md.

#define DEFAULT_TIMEOUT (60 * 6)
#define TITLE_POLL_FRAMES 150
#define NPX_FIELD_LENGTH 24

// One answer, split into rows of fields. The text part ends at the first
// NUL; anything after it is binary (a Pokémon, a team, a replay part).
struct NpxAnswer
{
    u8 buf[NETPLAY_EXT_RESP_SIZE + 1];
    u16 len;
    u16 binOffset; // 0: no binary part
    u8 rowCount;
    u8 colCount[NPX_MAX_ROWS];
    u16 fields[NPX_MAX_ROWS][NPX_MAX_COLS];
};

EWRAM_DATA struct NetplayExtMailbox gNetplayExtMailbox = {0};

static EWRAM_DATA struct NpxAnswer sAnswer = {0};
static EWRAM_DATA u8 sRpcState = 0;
static EWRAM_DATA u8 sRpcTextLen = 0;
static EWRAM_DATA u16 sRpcTimer = 0;
static EWRAM_DATA u16 sRpcTimeout = 0;
static EWRAM_DATA char sChosen[NPX_MAX_COLS][NPX_FIELD_LENGTH] = {0};
static EWRAM_DATA u8 sItemText[NPX_MAX_ROWS + 1][32] = {0};
static EWRAM_DATA struct ListMenuItem sItems[NPX_MAX_ROWS + 1] = {0};

// Badges: the title each player on the map shows.
static EWRAM_DATA u16 sTitleIds[NETPLAY_MAX_REMOTES] = {0};   // remote player id
static EWRAM_DATA u8 sTitleBadges[NETPLAY_MAX_REMOTES] = {0}; // badge + 1, 0: unknown
static EWRAM_DATA u8 sBadgeSprites[NETPLAY_MAX_REMOTES + 1] = {0}; // last: our own
static EWRAM_DATA u8 sMyBadge = 0; // badge + 1
static EWRAM_DATA u16 sTitlePollTimer = 0;
static EWRAM_DATA bool8 sTitlePolling = FALSE;
static EWRAM_DATA u8 sLastNotes = 0;
static EWRAM_DATA bool8 sNewMail = FALSE;

extern const u8 NetplayExt_EventScript_Plaza[];

#define NPX_TEST_SEED 0xEE

void NpxFeatures_OverworldUpdate(void);
void NpxFeatures_SeedTestData(void);
bool32 NpxFeatures_IsBusy(void);

void NetplayExt_Init(void)
{
    static const char magic[8] = {'P', 'K', 'M', 'N', 'E', 'X', 'T', '1'};

    CpuFill32(0, &gNetplayExtMailbox, sizeof(gNetplayExtMailbox));
    memcpy(gNetplayExtMailbox.magic, magic, sizeof(magic));
    gNetplayExtMailbox.version = NETPLAY_EXT_VERSION;
    sRpcState = NPX_RPC_IDLE;
    sMyBadge = 0;
    sTitlePolling = FALSE;
    sLastNotes = 0;
    sNewMail = FALSE;
}

bool32 Npx_IsOnline(void)
{
    return gNetplayMailbox.bridgeState == NETPLAY_ONLINE;
}

// Text -------------------------------------------------------------------

static u8 AsciiToGameChar(char c)
{
    if (c >= '0' && c <= '9')
        return CHAR_0 + (c - '0');
    if (c >= 'A' && c <= 'Z')
        return CHAR_A + (c - 'A');
    if (c >= 'a' && c <= 'z')
        return CHAR_a + (c - 'a');
    switch (c)
    {
    case '.': return CHAR_PERIOD;
    case '!': return CHAR_EXCL_MARK;
    case '?': return CHAR_QUESTION_MARK;
    case '-': return CHAR_HYPHEN;
    case '/': return CHAR_SLASH;
    case ':': return CHAR_COLON;
    case ',': return CHAR_COMMA;
    case '\'': return CHAR_SGL_QUOTE_RIGHT;
    case '(': return CHAR_LEFT_PAREN;
    case ')': return CHAR_RIGHT_PAREN;
    case '&': return CHAR_AMPERSAND;
    case '+': return CHAR_PLUS;
    case '%': return CHAR_PERCENT;
    case '\n': return CHAR_NEWLINE;
    }
    return CHAR_SPACE;
}

static char GameCharToAscii(u8 c)
{
    if (c >= CHAR_0 && c <= CHAR_0 + 9)
        return '0' + (c - CHAR_0);
    if (c >= CHAR_A && c <= CHAR_A + 25)
        return 'A' + (c - CHAR_A);
    if (c >= CHAR_a && c <= CHAR_a + 25)
        return 'a' + (c - CHAR_a);
    switch (c)
    {
    case CHAR_PERIOD: return '.';
    case CHAR_EXCL_MARK: return '!';
    case CHAR_QUESTION_MARK: return '?';
    case CHAR_HYPHEN: return '-';
    case CHAR_SPACE: return ' ';
    }
    return 0;
}

u8 *Npx_AsciiToGame(u8 *dest, const char *src, u32 maxChars)
{
    while (*src != 0 && maxChars-- != 0)
        *dest++ = AsciiToGameChar(*src++);
    *dest = EOS;
    return dest;
}

// Request channel --------------------------------------------------------

static void RpcAppend(const char *s)
{
    while (*s != 0 && sRpcTextLen < NETPLAY_EXT_TEXT_LENGTH - 1)
        gNetplayExtMailbox.reqText[sRpcTextLen++] = *s++;
    gNetplayExtMailbox.reqText[sRpcTextLen] = 0;
}

// Starts a request. FALSE while another request is on its way or its
// answer hasn't been taken yet (NpxRpc_Release).
bool32 NpxRpc_Begin(const char *word)
{
    if (sRpcState == NPX_RPC_WAITING || sRpcState == NPX_RPC_DONE)
        return FALSE;
    sRpcTextLen = 0;
    sRpcTimeout = DEFAULT_TIMEOUT;
    gNetplayExtMailbox.reqLen = 0;
    RpcAppend(word);
    sRpcState = NPX_RPC_IDLE;
    return TRUE;
}

void NpxRpc_Word(const char *word)
{
    RpcAppend(" ");
    RpcAppend(word);
}

void NpxRpc_Num(s32 n)
{
    char buf[12];
    u32 i = ARRAY_COUNT(buf) - 1;
    bool32 negative = n < 0;
    u32 v = negative ? -n : n;

    buf[i] = 0;
    do
    {
        buf[--i] = '0' + v % 10;
        v /= 10;
    } while (v != 0 && i > 1);
    if (negative)
        buf[--i] = '-';
    NpxRpc_Word(&buf[i]);
}

// A player name or other game text, as letters, digits and a few signs.
void NpxRpc_Name(const u8 *gameText)
{
    char buf[NPX_FIELD_LENGTH];
    u32 i, n = 0;

    for (i = 0; gameText[i] != EOS && n < ARRAY_COUNT(buf) - 1; i++)
    {
        char c = GameCharToAscii(gameText[i]);
        if (c != 0)
            buf[n++] = c;
    }
    buf[n] = 0;
    if (n != 0)
        NpxRpc_Word(buf);
}

void NpxRpc_Data(const void *data, u32 size)
{
    u32 len = gNetplayExtMailbox.reqLen;

    if (len + size > NETPLAY_EXT_DATA_SIZE)
        size = NETPLAY_EXT_DATA_SIZE - len;
    memcpy(&gNetplayExtMailbox.reqData[len], data, size);
    gNetplayExtMailbox.reqLen = len + size;
}

void NpxRpc_Send(void)
{
    if (!Npx_IsOnline())
    {
        sRpcState = NPX_RPC_FAILED;
        return;
    }
    sRpcTimer = 0;
    sRpcState = NPX_RPC_WAITING;
    gNetplayExtMailbox.reqSeq++;
}

// Done with the last answer: the channel is free for the next request.
void NpxRpc_Release(void)
{
    sRpcState = NPX_RPC_IDLE;
}

u8 NpxRpc_Poll(void)
{
    if (sRpcState != NPX_RPC_WAITING)
        return sRpcState;
    if (gNetplayExtMailbox.respSeq == gNetplayExtMailbox.reqSeq)
        sRpcState = NPX_RPC_DONE;
    else if (!Npx_IsOnline() || ++sRpcTimer > sRpcTimeout)
        sRpcState = NPX_RPC_FAILED;
    return sRpcState;
}

static void ParseAnswer(struct NpxAnswer *answer)
{
    u32 i, row = 0, col = 0;
    u32 len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);

    memcpy(answer->buf, gNetplayExtMailbox.resp, len);
    answer->buf[len] = 0;
    answer->len = len;
    answer->binOffset = 0;
    answer->rowCount = 0;
    if (len == 0)
        return;
    answer->fields[0][0] = 0;
    for (i = 0; i < len; i++)
    {
        u8 c = answer->buf[i];
        if (c == 0)
        {
            answer->binOffset = i + 1;
            break;
        }
        if (c == '|' || c == '\n')
        {
            answer->buf[i] = 0;
            if (c == '\n')
            {
                answer->colCount[row] = col + 1;
                if (row + 1 >= NPX_MAX_ROWS)
                    break;
                row++;
                col = 0;
            }
            else if (col + 1 < NPX_MAX_COLS)
            {
                col++;
            }
            answer->fields[row][col] = i + 1;
        }
    }
    answer->colCount[row] = col + 1;
    answer->rowCount = row + 1;
}

// Takes the answer that just came in (after NpxRpc_Poll said DONE) for
// Npx_Field and friends. Other request layers (src/online.c) use this.
void Npx_TakeAnswer(void)
{
    ParseAnswer(&sAnswer);
}

u32 Npx_RowCount(void)
{
    return sAnswer.rowCount;
}

const char *Npx_Field(u32 row, u32 col)
{
    if (row >= sAnswer.rowCount || col >= sAnswer.colCount[row])
        return "";
    return (const char *)&sAnswer.buf[sAnswer.fields[row][col]];
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

s32 Npx_Num(u32 row, u32 col)
{
    return AsciiToNum(Npx_Field(row, col));
}

u8 *Npx_CopyField(u8 *dest, u32 row, u32 col, u32 maxChars)
{
    return Npx_AsciiToGame(dest, Npx_Field(row, col), maxChars);
}

const u8 *Npx_Binary(u32 *size)
{
    if (sAnswer.binOffset == 0)
    {
        *size = 0;
        return NULL;
    }
    *size = sAnswer.len - sAnswer.binOffset;
    return &sAnswer.buf[sAnswer.binOffset];
}

const char *Npx_Chosen(u32 col)
{
    return col < NPX_MAX_COLS ? sChosen[col] : "";
}

s32 Npx_ChosenNum(u32 col)
{
    return AsciiToNum(Npx_Chosen(col));
}

// NetplayExt_Request -----------------------------------------------------
//
// VAR_0x8004: NPX_REQ_*. Sends the request, waits for the answer, and runs
// the request's `after`. VAR_RESULT: NETPLAY_EXT_OK, _REFUSED or _FAILED;
// for the last two the reason is in gStringVar4.

#define tReq   data[0]
#define tState data[1]

#define REQ_OTHER 0xFF // tReq for a request from another file (NetplayExt_RunRequest)

static EWRAM_DATA const struct NpxRequest *sOtherRequest = NULL;

static void Task_Request(u8 taskId);

// Like NetplayExt_Request, for a request table kept in another file.
void NetplayExt_RunRequest(const struct NpxRequest *req)
{
    u8 taskId = CreateTask(Task_Request, 80);
    sOtherRequest = req;
    gTasks[taskId].tReq = REQ_OTHER;
    gTasks[taskId].tState = 0;
}

void NetplayExt_Request(void)
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

static void Task_Request(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    const struct NpxRequest *req = tReq == REQ_OTHER ? sOtherRequest : &gNpxRequests[tReq];

    switch (tState)
    {
    case 0:
        if (!Npx_IsOnline())
        {
            StringCopy(gStringVar4, COMPOUND_STRING("You're not online. Pick JOIN ONLINE\non the title menu to play online."));
            FinishRequest(taskId, NETPLAY_EXT_FAILED);
            return;
        }
        // A background request (titles, a replay upload) may still be out.
        if (!NpxRpc_Begin(req->word))
            return;
        if (req->build != NULL)
            req->build();
        if (req->timeout != 0)
            sRpcTimeout = req->timeout;
        NpxRpc_Send();
        tState++;
        break;
    case 1:
        switch (NpxRpc_Poll())
        {
        case NPX_RPC_DONE:
            ParseAnswer(&sAnswer);
            sRpcState = NPX_RPC_IDLE;
            if (gNetplayExtMailbox.status == 0)
            {
                if (req->after != NULL)
                    req->after();
                FinishRequest(taskId, NETPLAY_EXT_OK);
            }
            else
            {
                Npx_AsciiToGame(gStringVar4, (const char *)sAnswer.buf, 120);
                FinishRequest(taskId, NETPLAY_EXT_REFUSED);
            }
            break;
        case NPX_RPC_FAILED:
            sRpcState = NPX_RPC_IDLE;
            StringCopy(gStringVar4, COMPOUND_STRING("The server didn't answer."));
            FinishRequest(taskId, NETPLAY_EXT_FAILED);
            break;
        }
        break;
    }
}

#undef tReq
#undef tState

// VAR_RESULT: rows in the last answer.
void NetplayExt_CountRows(void)
{
    gSpecialVar_Result = Npx_RowCount();
}

// VAR_RESULT: TRUE if new mail came in since the plaza was last opened.
void NetplayExt_HasNewMail(void)
{
    gSpecialVar_Result = sNewMail;
}

void NetplayExt_ClearNewMail(void)
{
    sNewMail = FALSE;
}

bool32 NetplayExt_NewMailWaiting(void)
{
    return sNewMail;
}

// Boards -----------------------------------------------------------------
//
// NetplayExt_ShowBoard: a ranked list from the last answer, in a window with
// the feature's icon beside it. VAR_0x8004: NPX_BOARD_*. Closes on A or B.

#define BOARD_LINE   11
#define BOARD_ROWS   9
#define BOARD_ICON_TAG 0x5E20
#define BOARD_PAL_TAG  0x5E21

struct BoardColumn
{
    u8 field;
    u8 x;
    const u8 *label;
};

struct BoardDef
{
    const u8 *title;
    u8 icon; // PLAZA_* - 1
    u8 firstRow;
    bool8 lastRowIsMe;
    u8 columnCount;
    struct BoardColumn columns[5];
    const u8 *empty;
};

extern const u32 gNpxIconsGfx[];
extern const u16 gNpxIconsPal[];

static const struct BoardDef sBoards[] =
{
    [NPX_BOARD_GUILD] =
    {
        .title = NULL, // the guild's tag and name
        .icon = PLAZA_GUILDS - 1,
        .firstRow = 1,
        .columnCount = 2,
        .columns = {{0, 6, COMPOUND_STRING("MEMBER")}, {1, 120, COMPOUND_STRING("RATING")}},
    },
    [NPX_BOARD_GUILD_LADDER] =
    {
        .title = COMPOUND_STRING("GUILD LADDER"),
        .icon = PLAZA_GUILDS - 1,
        .columnCount = 5,
        .columns = {{0, 6, COMPOUND_STRING("No.")}, {1, 24, COMPOUND_STRING("TAG")},
                    {2, 56, COMPOUND_STRING("GUILD")}, {3, 132, COMPOUND_STRING("MEM.")}, {4, 160, COMPOUND_STRING("SCORE")}},
        .empty = COMPOUND_STRING("No guilds yet. Found the first one!"),
    },
    [NPX_BOARD_DEX] =
    {
        .title = COMPOUND_STRING("POKéDEX RACE"),
        .icon = PLAZA_DEX - 1,
        .lastRowIsMe = TRUE,
        .columnCount = 4,
        .columns = {{0, 6, COMPOUND_STRING("No.")}, {1, 24, COMPOUND_STRING("TRAINER")},
                    {2, 104, COMPOUND_STRING("CAUGHT")}, {3, 148, COMPOUND_STRING("SEEN")}},
        .empty = COMPOUND_STRING("Nobody has entered yet."),
    },
    [NPX_BOARD_TOWER] =
    {
        .title = COMPOUND_STRING("BATTLE TOWER RECORDS"),
        .icon = PLAZA_TOWER - 1,
        .lastRowIsMe = TRUE,
        .columnCount = 3,
        .columns = {{0, 6, COMPOUND_STRING("No.")}, {1, 24, COMPOUND_STRING("TRAINER")}, {2, 110, COMPOUND_STRING("BEST FLOOR")}},
        .empty = COMPOUND_STRING("No floors cleared yet."),
    },
    [NPX_BOARD_DRAFT_POOL] =
    {
        .title = COMPOUND_STRING("TODAY'S DRAFT POOL"),
        .icon = PLAZA_DRAFT - 1,
        .columnCount = 0,
    },
};

static const struct WindowTemplate sBoardWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 5,
    .tilemapTop = 1,
    .width = 24,
    .height = 17,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const struct OamData sOam_Icon =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 0,
};

static const struct SpriteTemplate sBoardIconTemplate =
{
    .tileTag = BOARD_ICON_TAG,
    .paletteTag = BOARD_PAL_TAG,
    .oam = &sOam_Icon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

void NpxFeatures_DrawDraftPool(u8 windowId, u32 y, u32 lineHeight);

static void PrintBoardLine(u8 windowId, const struct BoardDef *def, u32 row, u32 y, u8 fontColor)
{
    u32 c;
    static const u8 sColors[][3] =
    {
        {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY},
        {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED},
        {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE},
    };

    for (c = 0; c < def->columnCount; c++)
    {
        const struct BoardColumn *col = &def->columns[c];
        if (row == 0xFF)
            StringCopy(gStringVar4, col->label);
        else if (def->lastRowIsMe && row == Npx_RowCount() - 1 && c == 0 && Npx_Num(row, 0) == 0)
            StringCopy(gStringVar4, COMPOUND_STRING("-"));
        else
            Npx_CopyField(gStringVar4, row, col->field, 14);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, col->x, y, sColors[fontColor], TEXT_SKIP_DRAW, gStringVar4);
    }
}

static void DrawBoard(u8 windowId, const struct BoardDef *def)
{
    u32 rows = Npx_RowCount(), row, line = 0;
    u32 last = def->lastRowIsMe && rows != 0 ? rows - 1 : rows;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (def->title != NULL)
    {
        AddTextPrinterParameterized(windowId, FONT_NORMAL, def->title, 6, 0, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        // My guild: "[TAG] Name", then leader and rank.
        u8 *str = StringCopy(gStringVar4, COMPOUND_STRING("("));
        str = Npx_CopyField(str, 0, 0, 4);
        str = StringAppend(str, COMPOUND_STRING(") "));
        Npx_CopyField(str, 0, 1, 12);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 6, 0, TEXT_SKIP_DRAW, NULL);
        str = StringCopy(gStringVar4, COMPOUND_STRING("Leader "));
        str = Npx_CopyField(str, 0, 2, 7);
        str = StringAppend(str, COMPOUND_STRING("  Ladder No."));
        str = Npx_CopyField(str, 0, 5, 3);
        str = StringAppend(str, COMPOUND_STRING("  Score "));
        Npx_CopyField(str, 0, 4, 5);
        AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 6, 14, TEXT_SKIP_DRAW, NULL);
        line = 1;
    }
    line++;
    if (def->columnCount == 0)
    {
        NpxFeatures_DrawDraftPool(windowId, BOARD_LINE * line + 4, 16);
    }
    else
    {
        PrintBoardLine(windowId, def, 0xFF, BOARD_LINE * line + 4, 2);
        line++;
        for (row = def->firstRow; row < last && row - def->firstRow < BOARD_ROWS; row++)
            PrintBoardLine(windowId, def, row, BOARD_LINE * line++ + 4, 0);
        if (last <= def->firstRow && def->empty != NULL)
            AddTextPrinterParameterized(windowId, FONT_SMALL, def->empty, 6, BOARD_LINE * line + 4, TEXT_SKIP_DRAW, NULL);
        if (def->lastRowIsMe && rows != 0)
            PrintBoardLine(windowId, def, rows - 1, BOARD_LINE * 11 + 4, 1);
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

#define tWindowId data[0]
#define tSpriteId data[1]

static void Task_Board(u8 taskId);

static u8 CreateFeatureIcon(u8 icon, s16 x, s16 y)
{
    struct SpriteSheet sheet = {(const u8 *)gNpxIconsGfx + icon * 0x200, 0x200, BOARD_ICON_TAG};
    struct SpritePalette pal = {gNpxIconsPal, BOARD_PAL_TAG};

    FreeSpriteTilesByTag(BOARD_ICON_TAG);
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&pal);
    return CreateSprite(&sBoardIconTemplate, x, y, 0);
}

static void DestroyFeatureIcon(u8 spriteId)
{
    if (spriteId < MAX_SPRITES)
        DestroySprite(&gSprites[spriteId]);
    FreeSpriteTilesByTag(BOARD_ICON_TAG);
    FreeSpritePaletteByTag(BOARD_PAL_TAG);
}

void NetplayExt_ShowBoard(void)
{
    const struct BoardDef *def = &sBoards[gSpecialVar_0x8004];
    u8 taskId = CreateTask(Task_Board, 80);
    struct Task *task = &gTasks[taskId];

    task->tWindowId = AddWindow(&sBoardWindowTemplate);
    DrawStdWindowFrame(task->tWindowId, FALSE);
    PutWindowTilemap(task->tWindowId);
    DrawBoard(task->tWindowId, def);
    ScheduleBgCopyTilemapToVram(0);
    task->tSpriteId = CreateFeatureIcon(def->icon, 20, 24);
}

static void Task_Board(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        DestroyFeatureIcon(task->tSpriteId);
        ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
        ClearWindowTilemap(task->tWindowId);
        RemoveWindow(task->tWindowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

#undef tWindowId
#undef tSpriteId

// Pick lists -------------------------------------------------------------
//
// NetplayExt_ChooseRow: a list to pick one row of the last answer (or of a
// list the game makes itself). VAR_0x8004: NPX_LIST_*. VAR_RESULT: the row
// picked, or NPX_CHOSE_NOTHING. The picked row's fields are kept for the
// next request (Npx_Chosen), and some go to gStringVar1-3 for messages.

u32 NpxFeatures_BuildList(u32 list, u8 (*text)[32], u32 max);
void NpxFeatures_OnChosen(u32 list, u32 row);

#define LIST_SHOWN 6

#define tListTaskId data[0]
#define tWindowId   data[1]
#define tList       data[2]
#define tCount      data[3]
#define tSpriteId   data[4]

static void Task_ChooseRow(u8 taskId);

void NetplayExt_ChooseRow(void)
{
    struct ListMenuTemplate template = gMultiuseListMenuTemplate;
    struct WindowTemplate window = {.bg = 0, .tilemapLeft = 6, .tilemapTop = 1, .width = 20, .paletteNum = 15, .baseBlock = 0x008};
    u8 taskId = CreateTask(Task_ChooseRow, 80);
    struct Task *task = &gTasks[taskId];
    u32 i, count = NpxFeatures_BuildList(gSpecialVar_0x8004, sItemText, NPX_MAX_ROWS);

    for (i = 0; i < count; i++)
    {
        sItems[i].name = sItemText[i];
        sItems[i].id = i;
    }
    sItems[count].name = gText_Cancel;
    sItems[count].id = LIST_CANCEL;

    window.height = min(count + 1, LIST_SHOWN) * 2;
    task->tWindowId = AddWindow(&window);
    task->tList = gSpecialVar_0x8004;
    task->tCount = count;
    DrawStdWindowFrame(task->tWindowId, FALSE);

    template.items = sItems;
    template.totalItems = count + 1;
    template.maxShowed = min(count + 1, LIST_SHOWN);
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
    task->tSpriteId = CreateFeatureIcon(gSpecialVar_0x8008 < PLAZA_COUNT ? gSpecialVar_0x8008 : 0, 24, 24);
}

static void Task_ChooseRow(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 input = ListMenu_ProcessInput(task->tListTaskId);

    if (input == LIST_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL)
    {
        gSpecialVar_Result = NPX_CHOSE_NOTHING;
    }
    else
    {
        gSpecialVar_Result = input;
        NpxFeatures_OnChosen(task->tList, input);
    }
    DestroyFeatureIcon(task->tSpriteId);
    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(task->tWindowId, FALSE);
    ClearWindowTilemap(task->tWindowId);
    RemoveWindow(task->tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tListTaskId
#undef tWindowId
#undef tList
#undef tCount
#undef tSpriteId

// Keeps a row of the last answer as the chosen one.
void Npx_ChooseAnswerRow(u32 row)
{
    u32 c;

    for (c = 0; c < NPX_MAX_COLS; c++)
    {
        const char *f = Npx_Field(row, c);
        u32 i;
        for (i = 0; i < NPX_FIELD_LENGTH - 1 && f[i] != 0; i++)
            sChosen[c][i] = f[i];
        sChosen[c][i] = 0;
    }
}

void Npx_SetChosen(u32 col, const char *value)
{
    u32 i;

    for (i = 0; i < NPX_FIELD_LENGTH - 1 && value[i] != 0; i++)
        sChosen[col][i] = value[i];
    sChosen[col][i] = 0;
}

// Title badges -----------------------------------------------------------
//
// Every few seconds the game asks which title each player on the map shows,
// and draws that title's badge above their head (and our own above ours).

#define BADGE_TILE_TAG 0x5E22
#define BADGE_PAL_TAG  0x5E23
#define NPX_BADGE_COUNT 13

extern const u32 gNpxBadgesGfx[];
extern const u16 gNpxBadgesPal[];

#define sSlot data[0]

static void SpriteCB_Badge(struct Sprite *sprite);

static const struct OamData sOam_Badge =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

#define BADGE_ANIM(n) static const union AnimCmd sAnim_Badge##n[] = {ANIMCMD_FRAME(n * 4, 1), ANIMCMD_END}
BADGE_ANIM(0); BADGE_ANIM(1); BADGE_ANIM(2); BADGE_ANIM(3); BADGE_ANIM(4); BADGE_ANIM(5); BADGE_ANIM(6);
BADGE_ANIM(7); BADGE_ANIM(8); BADGE_ANIM(9); BADGE_ANIM(10); BADGE_ANIM(11); BADGE_ANIM(12);
#undef BADGE_ANIM

static const union AnimCmd *const sAnims_Badge[NPX_BADGE_COUNT] =
{
    sAnim_Badge0, sAnim_Badge1, sAnim_Badge2, sAnim_Badge3, sAnim_Badge4, sAnim_Badge5, sAnim_Badge6,
    sAnim_Badge7, sAnim_Badge8, sAnim_Badge9, sAnim_Badge10, sAnim_Badge11, sAnim_Badge12,
};

static const struct SpriteTemplate sBadgeTemplate =
{
    .tileTag = BADGE_TILE_TAG,
    .paletteTag = BADGE_PAL_TAG,
    .oam = &sOam_Badge,
    .anims = sAnims_Badge,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Badge,
};

// After the field reloads, a stale id can point at another slot's new
// badge, so the slot has to match too.
static bool32 BadgeSpriteValid(u8 spriteId, u32 slot)
{
    return spriteId < MAX_SPRITES && gSprites[spriteId].inUse && gSprites[spriteId].template == &sBadgeTemplate
        && gSprites[spriteId].sSlot == slot;
}

// The object event a badge sits on: a remote player's avatar, or ours.
static struct ObjectEvent *BadgeOwner(u32 slot)
{
    u8 objectEventId;

    if (slot == NETPLAY_MAX_REMOTES)
        return &gObjectEvents[gPlayerAvatar.objectEventId];
    if (TryGetObjectEventIdByLocalIdAndMap(LOCALID_NETPLAY_PLAYER_START + slot,
            gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objectEventId))
        return NULL;
    return &gObjectEvents[objectEventId];
}

static void SpriteCB_Badge(struct Sprite *sprite)
{
    struct ObjectEvent *owner = BadgeOwner(sprite->sSlot);
    struct Sprite *ownerSprite;

    if (owner == NULL || !owner->active)
    {
        sprite->invisible = TRUE;
        return;
    }
    ownerSprite = &gSprites[owner->spriteId];
    sprite->x = ownerSprite->x + ownerSprite->x2;
    sprite->y = ownerSprite->y + ownerSprite->y2 - 10;
    sprite->coordOffsetEnabled = ownerSprite->coordOffsetEnabled;
    sprite->invisible = ownerSprite->invisible;
}

static void SetBadge(u32 slot, u8 badgePlusOne)
{
    u8 *spriteId = &sBadgeSprites[slot];

    if (badgePlusOne == 0 || badgePlusOne > NPX_BADGE_COUNT)
    {
        if (BadgeSpriteValid(*spriteId, slot))
            DestroySprite(&gSprites[*spriteId]);
        *spriteId = MAX_SPRITES;
        return;
    }
    if (!BadgeSpriteValid(*spriteId, slot))
    {
        if (GetSpriteTileStartByTag(BADGE_TILE_TAG) == 0xFFFF)
        {
            struct SpriteSheet sheet = {(const u8 *)gNpxBadgesGfx, NPX_BADGE_COUNT * 0x80, BADGE_TILE_TAG};
            LoadSpriteSheet(&sheet);
        }
        if (IndexOfSpritePaletteTag(BADGE_PAL_TAG) == 0xFF)
        {
            struct SpritePalette pal = {gNpxBadgesPal, BADGE_PAL_TAG};
            LoadSpritePalette(&pal);
        }
        *spriteId = CreateSprite(&sBadgeTemplate, 0, 0, 0);
        if (*spriteId == MAX_SPRITES)
            return;
        gSprites[*spriteId].sSlot = slot;
        gSprites[*spriteId].data[1] = 0xFF;
    }
    if (gSprites[*spriteId].data[1] != badgePlusOne)
    {
        gSprites[*spriteId].data[1] = badgePlusOne;
        StartSpriteAnim(&gSprites[*spriteId], badgePlusOne - 1);
    }
    SpriteCB_Badge(&gSprites[*spriteId]);
}

#undef sSlot

void Npx_SetMyBadge(u8 badge)
{
    sMyBadge = badge + 1;
}

static void PollTitles(void)
{
    u32 i;
    bool32 any = FALSE;

    if (sTitlePolling)
    {
        u8 state = NpxRpc_Poll();
        if (state == NPX_RPC_WAITING)
            return;
        sTitlePolling = FALSE;
        sRpcState = NPX_RPC_IDLE;
        if (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0)
        {
            // pid|title|name|tag per line
            const u8 *resp = gNetplayExtMailbox.resp;
            u32 len = gNetplayExtMailbox.respLen, pos = 0;
            for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
                sTitleBadges[i] = 0;
            while (pos < len)
            {
                u32 pid = 0, title = 0, slot;
                while (pos < len && resp[pos] >= '0' && resp[pos] <= '9')
                    pid = pid * 10 + resp[pos++] - '0';
                pos++;
                while (pos < len && resp[pos] >= '0' && resp[pos] <= '9')
                    title = title * 10 + resp[pos++] - '0';
                while (pos < len && resp[pos] != '\n')
                    pos++;
                pos++;
                if (pid == gNetplayMailbox.localId)
                    sMyBadge = title + 1;
                for (slot = 0; slot < NETPLAY_MAX_REMOTES; slot++)
                {
                    if (gNetplayMailbox.remotes[slot].id == pid)
                    {
                        sTitleIds[slot] = pid;
                        sTitleBadges[slot] = title + 1;
                    }
                }
            }
        }
        return;
    }
    if (++sTitlePollTimer < TITLE_POLL_FRAMES || !Npx_IsOnline() || gNetplayMailbox.localId == 0)
        return;
    if (!NpxRpc_Begin("TITLE"))
        return;
    sTitlePollTimer = 0;
    NpxRpc_Word("OF");
    NpxRpc_Num(gNetplayMailbox.localId);
    for (i = 0; i < NETPLAY_MAX_REMOTES; i++)
    {
        if (gNetplayMailbox.remotes[i].id != 0)
        {
            NpxRpc_Num(gNetplayMailbox.remotes[i].id);
            any = TRUE;
        }
    }
    (void)any;
    NpxRpc_Send();
    sTitlePolling = (sRpcState == NPX_RPC_WAITING);
}

static void UpdateBadges(void)
{
    u32 slot;

    for (slot = 0; slot < NETPLAY_MAX_REMOTES; slot++)
    {
        const struct NetplayRemotePlayer *remote = &gNetplayMailbox.remotes[slot];
        bool32 known = remote->id != 0 && remote->id == sTitleIds[slot] && BadgeOwner(slot) != NULL;
        SetBadge(slot, known && Npx_IsOnline() ? sTitleBadges[slot] : 0);
    }
    SetBadge(NETPLAY_MAX_REMOTES, Npx_IsOnline() ? sMyBadge : 0);
}

// Overworld --------------------------------------------------------------

// Called every overworld frame. Returns TRUE if it started a script.
bool32 NetplayExt_OverworldUpdate(void)
{
    if (sTitlePolling || (sRpcState == NPX_RPC_IDLE && !NpxFeatures_IsBusy()))
        PollTitles();
    UpdateBadges();
    NpxFeatures_OverworldUpdate();

    if (gNetplayExtMailbox.notes != sLastNotes)
    {
        sLastNotes = gNetplayExtMailbox.notes;
        sNewMail = TRUE;
        PlaySE(SE_EXP_MAX);
    }

    gNetplayExtMailbox.fieldLocked = ArePlayerFieldControlsLocked();
    if (gNetplayExtMailbox.testOpen == NPX_TEST_SEED)
    {
        gNetplayExtMailbox.testOpen = 0;
        NpxFeatures_SeedTestData();
    }
    else if (gNetplayExtMailbox.testOpen != 0 && !ArePlayerFieldControlsLocked())
    {
        gNetplayExtMailbox.testOpen = 0;
        ScriptContext_SetupScript(NetplayExt_EventScript_Plaza);
        return TRUE;
    }
    return FALSE;
}

// For the ONLINE menu: opens the Online Plaza from the field.
void NetplayExt_OpenPlazaFromMenu(void)
{
    ScriptContext_SetupScript(NetplayExt_EventScript_Plaza);
}
