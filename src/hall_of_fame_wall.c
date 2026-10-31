#include "global.h"
#include "hall_of_fame_wall.h"
#include "event_data.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "netplay_ext.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

// The Hall of Fame wall (docs/adventure.md). Entering the Hall of Fame
// keeps the team in the save, and it goes up on the server's wall as soon
// as the game is online (server/adventure_hof.py, through the Online
// Plaza's request channel). The curator in the Pokémon League and
// R > ADVENTURE > HALL OF FAME WALL show the wall: the newest teams first,
// with the selected team's Pokémon along the bottom.

#define sSave (gSaveBlock3Ptr->adventure)

#define WALL_ROWS        16 // as many as the server sends
#define WALL_SHOWN       4
#define WALL_LINE_HEIGHT 14
#define WALL_LIST_TOP    16
#define WALL_TEAM_TOP    76
#define WALL_ICON_Y      (WALL_TEAM_TOP + 22)
#define WALL_LEVEL_Y     (WALL_TEAM_TOP + 40)
#define WALL_FOOT_Y      (WALL_TEAM_TOP + 54)
#define WALL_SLOT_WIDTH  36
#define RETRY_FRAMES     300

enum
{
    RPC_NONE,
    RPC_ADD,
    RPC_LIST,
};

enum
{
    WALL_SENDING,
    WALL_ASKING,
    WALL_WAITING,
    WALL_SHOWING,
    WALL_MESSAGE, // offline or no answer: a message until A or B
};

struct HofWallRow
{
    u16 number;
    u8 name[PLAYER_NAME_LENGTH + 1];
    u8 date[11];
    struct HofWallMon team[PARTY_SIZE];
};

struct HofWall
{
    struct HofWallRow rows[WALL_ROWS];
    u16 count;
    u16 total;
    u16 mine;
    u16 cursor;
    u16 scroll;
    u8 windowId;
    u8 state;
    u8 iconIds[PARTY_SIZE];
    const u8 *message;
};

static EWRAM_DATA u8 sRpc = 0;
static EWRAM_DATA u16 sRetryTimer = 0;
static EWRAM_DATA struct HofWall *sWall = NULL;

static const struct WindowTemplate sWallWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

// Called when a new team enters the Hall of Fame (not when the PC shows it).
void HallOfFameWall_OnEnter(void)
{
    u32 i, n = 0;

    memset(sSave.hofTeam, 0, sizeof(sSave.hofTeam));
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);

        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;
        sSave.hofTeam[n].species = species;
        sSave.hofTeam[n].level = GetMonData(mon, MON_DATA_LEVEL);
        sSave.hofTeam[n].shiny = GetMonData(mon, MON_DATA_IS_SHINY);
        n++;
    }
    sSave.hofPending = (n != 0);
    sRetryTimer = RETRY_FRAMES; // send it as soon as the game is online
}

// The request channel ----------------------------------------------------

static bool32 StartRpc(u8 rpc)
{
    if (!Npx_IsOnline() || !NpxRpc_Begin("HOF"))
        return FALSE;
    if (rpc == RPC_ADD)
    {
        NpxRpc_Word("ADD");
        NpxRpc_Data(sSave.hofTeam, sizeof(sSave.hofTeam));
    }
    else
    {
        NpxRpc_Word("LIST");
    }
    NpxRpc_Send();
    sRpc = rpc;
    return TRUE;
}

static u32 ReadNum(const u8 *buf, u32 *pos, u32 len)
{
    u32 n = 0;

    while (*pos < len && buf[*pos] >= '0' && buf[*pos] <= '9')
        n = n * 10 + buf[(*pos)++] - '0';
    return n;
}

// Copies ASCII up to the next '|' or newline as game text.
static void ReadText(const u8 *buf, u32 *pos, u32 len, u8 *dest, u32 maxChars)
{
    char ascii[24];
    u32 n = 0;

    while (*pos < len && buf[*pos] != '|' && buf[*pos] != '\n')
    {
        if (n < ARRAY_COUNT(ascii) - 1 && n < maxChars)
            ascii[n++] = buf[*pos];
        (*pos)++;
    }
    ascii[n] = 0;
    Npx_AsciiToGame(dest, ascii, maxChars);
}

static void SkipPast(const u8 *buf, u32 *pos, u32 len, u8 c)
{
    while (*pos < len && buf[*pos] != c)
        (*pos)++;
    if (*pos < len)
        (*pos)++;
}

// LIST: total|mine, then number|name|date|species.level.shiny|... per row.
static void ParseList(struct HofWall *wall)
{
    const u8 *buf = gNetplayExtMailbox.resp;
    u32 len = min(gNetplayExtMailbox.respLen, NETPLAY_EXT_RESP_SIZE);
    u32 pos = 0, i;

    wall->total = ReadNum(buf, &pos, len);
    SkipPast(buf, &pos, len, '|');
    wall->mine = ReadNum(buf, &pos, len);
    SkipPast(buf, &pos, len, '\n');
    wall->count = 0;
    while (pos < len && wall->count < WALL_ROWS)
    {
        struct HofWallRow *row = &wall->rows[wall->count++];

        memset(row->team, 0, sizeof(row->team));
        row->number = ReadNum(buf, &pos, len);
        SkipPast(buf, &pos, len, '|');
        ReadText(buf, &pos, len, row->name, PLAYER_NAME_LENGTH);
        SkipPast(buf, &pos, len, '|');
        ReadText(buf, &pos, len, row->date, 10);
        for (i = 0; i < PARTY_SIZE && pos < len && buf[pos] == '|'; i++)
        {
            pos++;
            row->team[i].species = ReadNum(buf, &pos, len);
            SkipPast(buf, &pos, len, '.');
            row->team[i].level = ReadNum(buf, &pos, len);
            SkipPast(buf, &pos, len, '.');
            row->team[i].shiny = ReadNum(buf, &pos, len);
            if (row->team[i].species >= NUM_SPECIES)
                row->team[i].species = SPECIES_NONE;
        }
        SkipPast(buf, &pos, len, '\n');
    }
}

// Polls the request out. Returns its NPX_RPC_* state; a finished request
// is handled and the channel freed.
static u8 PollRpc(void)
{
    u8 state = NpxRpc_Poll();
    bool32 ok = (state == NPX_RPC_DONE && gNetplayExtMailbox.status == 0);

    if (state == NPX_RPC_WAITING)
        return state;
    if (ok && sRpc == RPC_ADD)
    {
        const u8 *buf = gNetplayExtMailbox.resp;
        u32 pos = 0;

        sSave.hofEntry = ReadNum(buf, &pos, gNetplayExtMailbox.respLen);
        sSave.hofPending = FALSE;
    }
    else if (ok && sRpc == RPC_LIST && sWall != NULL)
    {
        ParseList(sWall);
    }
    if (state == NPX_RPC_DONE && !ok)
        state = NPX_RPC_FAILED;
    NpxRpc_Release();
    sRpc = RPC_NONE;
    return state;
}

// Called once per overworld frame: sends a team that entered offline.
void HallOfFameWall_Update(void)
{
    if (sWall != NULL)
        return; // the wall does its own requests
    if (sRpc != RPC_NONE)
    {
        PollRpc();
        return;
    }
    if (!sSave.hofPending || !Npx_IsOnline() || ++sRetryTimer < RETRY_FRAMES)
        return;
    sRetryTimer = 0;
    StartRpc(RPC_ADD);
}

// The wall -------------------------------------------------------------

static void DestroyIcons(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sWall->iconIds[i] != SPRITE_NONE)
            FreeAndDestroyMonIconSprite(&gSprites[sWall->iconIds[i]]);
        sWall->iconIds[i] = SPRITE_NONE;
    }
    FreeMonIconPalettes();
}

static void DrawTeam(const struct HofWallRow *row)
{
    u32 i;

    DestroyIcons();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        const struct HofWallMon *mon = &row->team[i];
        s16 x = 8 + 4 + WALL_SLOT_WIDTH / 2 + i * WALL_SLOT_WIDTH;
        u32 width;

        if (mon->species == SPECIES_NONE)
            continue;
        LoadMonIconPalette(mon->species);
        sWall->iconIds[i] = CreateMonIcon(mon->species, SpriteCB_MonIcon, x, 8 + WALL_ICON_Y, 0, 0);
        gSprites[sWall->iconIds[i]].oam.priority = 0;

        StringCopy(gStringVar1, mon->shiny ? COMPOUND_STRING("{STAR}Lv") : COMPOUND_STRING("Lv"));
        ConvertIntToDecimalStringN(gStringVar2, mon->level, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar1, gStringVar2);
        width = GetStringWidth(FONT_SMALL, gStringVar1, 0);
        AddTextPrinterParameterized(sWall->windowId, FONT_SMALL, gStringVar1,
                                    x - 8 - width / 2, WALL_LEVEL_Y, TEXT_SKIP_DRAW, NULL);
    }
}

static void DrawWall(void)
{
    u32 i, y;

    FillWindowPixelBuffer(sWall->windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, COMPOUND_STRING("HALL OF FAME WALL"), 4, 0, TEXT_SKIP_DRAW, NULL);
    if (sWall->state != WALL_SHOWING)
    {
        AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, sWall->message, 10, WALL_LIST_TOP + 4, TEXT_SKIP_DRAW, NULL);
        DestroyIcons();
        CopyWindowToVram(sWall->windowId, COPYWIN_FULL);
        return;
    }

    ConvertIntToDecimalStringN(gStringVar1, sWall->total, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("{STR_VAR_1} teams"));
    AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, gStringVar4,
                                220 - GetStringWidth(FONT_NORMAL, gStringVar4, 0), 0, TEXT_SKIP_DRAW, NULL);
    if (sWall->count == 0)
    {
        AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL,
                                    COMPOUND_STRING("No one has entered the\nHALL OF FAME yet."),
                                    10, WALL_LIST_TOP + 4, TEXT_SKIP_DRAW, NULL);
        DestroyIcons();
        CopyWindowToVram(sWall->windowId, COPYWIN_FULL);
        return;
    }
    for (i = 0; i < WALL_SHOWN && sWall->scroll + i < sWall->count; i++)
    {
        const struct HofWallRow *row = &sWall->rows[sWall->scroll + i];

        y = WALL_LIST_TOP + i * WALL_LINE_HEIGHT;
        if (sWall->scroll + i == sWall->cursor)
            AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, gText_SelectorArrow2, 0, y, TEXT_SKIP_DRAW, NULL);
        ConvertIntToDecimalStringN(gStringVar1, row->number, STR_CONV_MODE_LEFT_ALIGN, 5);
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("No. {STR_VAR_1}"));
        AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, gStringVar4, 10, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, row->name, 70, y, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(sWall->windowId, FONT_NORMAL, row->date, 144, y, TEXT_SKIP_DRAW, NULL);
    }
    FillWindowPixelRect(sWall->windowId, PIXEL_FILL(3), 4, WALL_TEAM_TOP - 4, 216, 1);
    DrawTeam(&sWall->rows[sWall->cursor]);

    ConvertIntToDecimalStringN(gStringVar1, sWall->mine, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Your teams on the wall: {STR_VAR_1}"));
    AddTextPrinterParameterized(sWall->windowId, FONT_SMALL, gStringVar4, 4, WALL_FOOT_Y, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sWall->windowId, COPYWIN_FULL);
}

static void SetMessage(const u8 *message)
{
    sWall->state = WALL_MESSAGE;
    sWall->message = message;
    DrawWall();
}

static void CloseWall(u8 taskId)
{
    DestroyIcons();
    ClearStdWindowAndFrameToTransparent(sWall->windowId, FALSE);
    ClearWindowTilemap(sWall->windowId);
    RemoveWindow(sWall->windowId);
    ScheduleBgCopyTilemapToVram(0);
    FREE_AND_SET_NULL(sWall);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Wall(u8 taskId)
{
    u16 old;

    switch (sWall->state)
    {
    case WALL_SENDING: // our own new team goes up first
        if (sRpc == RPC_NONE && !sSave.hofPending)
            sWall->state = WALL_ASKING;
        else if (sRpc == RPC_NONE)
            StartRpc(RPC_ADD);
        else if (PollRpc() != NPX_RPC_WAITING)
            sWall->state = WALL_ASKING; // shown either way; it's tried again later
        break;
    case WALL_ASKING:
        if (!Npx_IsOnline())
            SetMessage(COMPOUND_STRING("You're not online. Pick JOIN\nONLINE on the title menu to\nsee the wall."));
        else if (StartRpc(RPC_LIST))
            sWall->state = WALL_WAITING;
        break;
    case WALL_WAITING:
        switch (PollRpc())
        {
        case NPX_RPC_DONE:
            sWall->state = WALL_SHOWING;
            DrawWall();
            break;
        case NPX_RPC_FAILED:
            SetMessage(COMPOUND_STRING("The server didn't answer."));
            break;
        }
        break;
    case WALL_SHOWING:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            CloseWall(taskId);
            return;
        }
        old = sWall->cursor;
        if (JOY_REPEAT(DPAD_DOWN) && sWall->cursor + 1 < sWall->count)
            sWall->cursor++;
        else if (JOY_REPEAT(DPAD_UP) && sWall->cursor > 0)
            sWall->cursor--;
        if (sWall->cursor != old)
        {
            if (sWall->cursor < sWall->scroll)
                sWall->scroll = sWall->cursor;
            else if (sWall->cursor >= sWall->scroll + WALL_SHOWN)
                sWall->scroll = sWall->cursor - WALL_SHOWN + 1;
            PlaySE(SE_SELECT);
            DrawWall();
        }
        break;
    case WALL_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            CloseWall(taskId);
        }
        break;
    }
}

// From a script with `callnative HallOfFameWall_Show, waitstate=1`.
void HallOfFameWall_Show(void)
{
    u32 i;

    sWall = AllocZeroed(sizeof(*sWall));
    for (i = 0; i < PARTY_SIZE; i++)
        sWall->iconIds[i] = SPRITE_NONE;
    sWall->state = WALL_SENDING;
    sWall->message = COMPOUND_STRING("Asking the server…");
    sWall->windowId = AddWindow(&sWallWindowTemplate);
    DrawStdWindowFrame(sWall->windowId, FALSE);
    PutWindowTilemap(sWall->windowId);
    DrawWall();
    ScheduleBgCopyTilemapToVram(0);
    CreateTask(Task_Wall, 80);
}
