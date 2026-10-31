#include "global.h"
#include "rovers.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "netplay_ext.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "life.h"
#include "constants/life.h"
#include "constants/songs.h"

// The auction house: put a Pokémon up for 1, 8 or 24 hours and other players
// bid on it. It talks to server/life_auction.py over the Online Plaza's
// request channel (netplay_ext.c), with its own request table. The server
// holds bids, so the game takes a bid out of the player's money when it's
// accepted and gives refunds, sales and won Pokémon back through COLLECT.

#define MAX_MONEY 999999
#define MIN_START 100

struct AuctionListing
{
    u32 id;
    u32 price; // the top bid, or the start price if nobody has bid
    u32 secondsLeft;
    u16 species;
    u16 bids;
    u8 level;
    u8 flags;
    u8 seller[PLAYER_NAME_LENGTH + 1];
};

// Flags in a listing, as the server sends them.
#define LISTING_MINE  1
#define LISTING_TOP   2
#define LISTING_SHINY 4

static EWRAM_DATA struct AuctionListing sListings[LIFE_AUC_LISTINGS] = {0};
static EWRAM_DATA u8 sListingCount = 0;
static EWRAM_DATA u8 sPicked = 0;     // index in sListings
static EWRAM_DATA u32 sAmount = 0;    // the bid, or the start price of a new listing

// The lowest bid a listing takes: the same rule as the server's min_bid.
static u32 MinBid(const struct AuctionListing *l)
{
    u32 step;

    if (l->bids == 0)
        return l->price;
    step = max(100, l->price / 20);
    return min(MAX_MONEY, (l->price + step + 99) / 100 * 100);
}

static void SaveAfterAuction(void)
{
    TrySavingData(SAVE_NORMAL);
}

// Requests ---------------------------------------------------------------

static void BuildList(void)    { NpxRpc_Word("LIST"); }
static void BuildCollect(void) { NpxRpc_Word("COLLECT"); }

static void BuildPost(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005];

    NpxRpc_Word("POST");
    NpxRpc_Num(GetMonData(mon, MON_DATA_SPECIES));
    NpxRpc_Num(GetMonData(mon, MON_DATA_LEVEL));
    NpxRpc_Num(GetMonData(mon, MON_DATA_IS_SHINY));
    NpxRpc_Num(sAmount);
    NpxRpc_Num(gSpecialVar_0x8006);
    NpxRpc_Data(mon, sizeof(*mon));
}

static void BuildBid(void)
{
    NpxRpc_Word("BID");
    NpxRpc_Num(sListings[sPicked].id);
    NpxRpc_Num(sAmount);
}

static void BuildCancel(void)
{
    NpxRpc_Word("CANCEL");
    NpxRpc_Num(sListings[sPicked].id);
}

static void AfterList(void)
{
    u32 i;

    sListingCount = min(Npx_RowCount(), LIFE_AUC_LISTINGS);
    for (i = 0; i < sListingCount; i++)
    {
        struct AuctionListing *l = &sListings[i];
        l->id = Npx_Num(i, 0);
        Npx_CopyField(l->seller, i, 1, PLAYER_NAME_LENGTH);
        l->species = Npx_Num(i, 2);
        if (l->species >= NUM_SPECIES)
            l->species = SPECIES_NONE;
        l->level = Npx_Num(i, 3);
        l->price = Npx_Num(i, 4);
        l->bids = Npx_Num(i, 5);
        l->secondsLeft = Npx_Num(i, 6);
        l->flags = Npx_Num(i, 7);
    }
    // An empty answer is one row with nothing in it.
    if (sListingCount == 1 && sListings[0].id == 0)
        sListingCount = 0;
}

static void AfterPost(void)
{
    StringCopy(gStringVar1, GetSpeciesName(GetMonData(&gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005], MON_DATA_SPECIES)));
    ZeroMonData(&gParties[B_TRAINER_PLAYER][gSpecialVar_0x8005]);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    SaveAfterAuction();
}

static void AfterBid(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, sAmount);
    SaveAfterAuction();
}

static void AfterCancel(void)
{
}

// A Pokémon from the auction: to the party, or the PC if the party is full.
static void ReceiveMon(const u8 *data)
{
    struct Pokemon mon;
    u16 species;

    memcpy(&mon, data, sizeof(mon));
    Rovers_FromListing(&mon); // a roaming trainer's Pokémon
    mon.mail = MAIL_NONE;
    species = GetMonData(&mon, MON_DATA_SPECIES);
    if (!GetMonData(&mon, MON_DATA_IS_EGG))
    {
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
    }
    CalculatePlayerPartyCount();
    if (gPartiesCount[B_TRAINER_PLAYER] < PARTY_SIZE)
    {
        gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]] = mon;
        CalculatePlayerPartyCount();
    }
    else
    {
        CopyMonToPC(&mon);
    }
}

// VAR_0x8005: LIFE_AUC_GOT_*. gStringVar1: the other player, gStringVar2:
// the species, gStringVar3: the money.
static void AfterCollect(void)
{
    static const char *const kinds[] = {"", "WON", "SOLD", "OUTBID", "UNSOLD", "CANCELED"};
    const char *kind = Npx_Field(0, 0);
    u32 i, size, species;
    const u8 *mon;

    gSpecialVar_0x8005 = LIFE_AUC_GOT_NOTHING;
    for (i = 1; i < ARRAY_COUNT(kinds); i++)
    {
        u32 j;
        for (j = 0; kind[j] == kinds[i][j] && kind[j] != 0; j++)
            ;
        if (kind[j] == kinds[i][j])
            gSpecialVar_0x8005 = i;
    }
    if (gSpecialVar_0x8005 == LIFE_AUC_GOT_NOTHING)
        return;
    species = Npx_Num(0, 2);
    StringCopy(gStringVar2, GetSpeciesName(species < NUM_SPECIES ? species : SPECIES_NONE));
    Npx_CopyField(gStringVar1, 0, 3, PLAYER_NAME_LENGTH);
    ConvertIntToDecimalStringN(gStringVar3, Npx_Num(0, 4), STR_CONV_MODE_LEFT_ALIGN, 6);
    AddMoney(&gSaveBlock1Ptr->money, Npx_Num(0, 1));
    mon = Npx_Binary(&size);
    if (mon != NULL && size >= sizeof(struct Pokemon))
        ReceiveMon(mon);
    SaveAfterAuction();
}

static const struct NpxRequest sRequests[LIFE_AUC_REQ_COUNT] =
{
    [LIFE_AUC_REQ_LIST]    = {"AUCTION", BuildList, AfterList},
    [LIFE_AUC_REQ_POST]    = {"AUCTION", BuildPost, AfterPost},
    [LIFE_AUC_REQ_BID]     = {"AUCTION", BuildBid, AfterBid},
    [LIFE_AUC_REQ_CANCEL]  = {"AUCTION", BuildCancel, AfterCancel},
    [LIFE_AUC_REQ_COLLECT] = {"AUCTION", BuildCollect, AfterCollect},
};

// VAR_0x8004: LIFE_AUC_REQ_*. Use with waitstate. VAR_RESULT:
// NETPLAY_EXT_OK, or _REFUSED / _FAILED with the reason in gStringVar4.
void LifeAuction_Request(void)
{
    NetplayExt_RunRequest(&sRequests[gSpecialVar_0x8004]);
}

// The board ---------------------------------------------------------------
//
// A list of the open auctions on the left, the picked one's Pokémon and
// details on the right. A on someone else's auction picks a bid with the
// D-pad; A on your own auction (with no bids yet) offers to take it back.

#define BOARD_ROWS        7
#define BOARD_TOP         18
#define BOARD_LINE_HEIGHT 14
#define DETAIL_X          150
#define HELP_Y            122

enum
{
    MODE_LIST,
    MODE_AMOUNT,
};

static const struct WindowTemplate sBoardWindow =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 28,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static const u8 sTextColor[]  = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sGoodColor[]  = {TEXT_COLOR_WHITE, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN};
static const u8 sMineColor[]  = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sPriceColor[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

#define tWindowId data[0]
#define tCursor   data[1]
#define tScroll   data[2]
#define tMode     data[3]
#define tSpriteId data[4]
#define tSpecies  data[5]
#define tFrames   data[6]

static void Task_Board(u8 taskId);

static void PrintAt(u8 windowId, u8 font, const u8 *str, u32 x, u32 y, const u8 *color)
{
    AddTextPrinterParameterized3(windowId, font, x, y, color, TEXT_SKIP_DRAW, str);
}

static void PrintRight(u8 windowId, u8 font, const u8 *str, u32 right, u32 y, const u8 *color)
{
    PrintAt(windowId, font, str, right - GetStringWidth(font, str, 0), y, color);
}

static void FormatMoney(u8 *dest, u32 amount)
{
    *dest++ = CHAR_CURRENCY;
    ConvertIntToDecimalStringN(dest, amount, STR_CONV_MODE_LEFT_ALIGN, 6);
}

static void FormatTimeLeft(u8 *dest, u32 seconds)
{
    if (seconds == 0)
    {
        StringCopy(dest, COMPOUND_STRING("ENDING"));
        return;
    }
    if (seconds >= 3600)
    {
        dest = ConvertIntToDecimalStringN(dest, seconds / 3600, STR_CONV_MODE_LEFT_ALIGN, 2);
        *dest++ = CHAR_h;
        *dest++ = CHAR_SPACE;
        seconds %= 3600;
        dest = ConvertIntToDecimalStringN(dest, seconds / 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        *dest++ = CHAR_m;
    }
    else
    {
        dest = ConvertIntToDecimalStringN(dest, seconds / 60, STR_CONV_MODE_LEFT_ALIGN, 2);
        *dest++ = CHAR_m;
        *dest++ = CHAR_SPACE;
        dest = ConvertIntToDecimalStringN(dest, seconds % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        *dest++ = CHAR_s;
    }
    *dest = EOS;
}

static u32 MaxBid(void)
{
    return min(GetMoney(&gSaveBlock1Ptr->money), MAX_MONEY);
}

static void ShowIcon(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 species = sListingCount != 0 ? sListings[tCursor].species : SPECIES_NONE;

    if (species == tSpecies)
        return;
    if (tSpriteId != MAX_SPRITES)
    {
        FreeAndDestroyMonIconSprite(&gSprites[tSpriteId]);
        FreeMonIconPalette(tSpecies);
        tSpriteId = MAX_SPRITES;
    }
    tSpecies = species;
    if (species == SPECIES_NONE)
        return;
    LoadMonIconPalette(species);
    tSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, 8 + DETAIL_X + 36, 8 + BOARD_TOP + 10, 0, 0);
    gSprites[tSpriteId].oam.priority = 0;
}

static void DrawDetail(u8 windowId, const struct AuctionListing *l)
{
    u8 line[32];
    u32 y = BOARD_TOP + 28;

    PrintAt(windowId, FONT_NORMAL, GetSpeciesName(l->species), DETAIL_X, y, sTextColor);
    y += 15;
    StringCopy(line, COMPOUND_STRING("Lv. "));
    ConvertIntToDecimalStringN(line + StringLength(line), l->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintAt(windowId, FONT_SMALL, line, DETAIL_X, y, sTextColor);
    if (l->flags & LISTING_SHINY)
        PrintRight(windowId, FONT_SMALL, COMPOUND_STRING("SHINY"), 222, y, sPriceColor);
    y += 11;
    StringCopy(line, COMPOUND_STRING("From "));
    StringAppend(line, l->seller);
    PrintAt(windowId, FONT_SMALL, line, DETAIL_X, y, sTextColor);
    y += 11;
    StringCopy(line, COMPOUND_STRING("Bids: "));
    ConvertIntToDecimalStringN(line + StringLength(line), l->bids, STR_CONV_MODE_LEFT_ALIGN, 4);
    PrintAt(windowId, FONT_SMALL, line, DETAIL_X, y, sTextColor);
    y += 11;
    StringCopy(line, COMPOUND_STRING("Ends in "));
    FormatTimeLeft(line + StringLength(line), l->secondsLeft);
    PrintAt(windowId, FONT_SMALL, line, DETAIL_X, y, sTextColor);
    y += 12;
    if (l->flags & LISTING_MINE)
        PrintAt(windowId, FONT_SMALL, COMPOUND_STRING("YOUR AUCTION"), DETAIL_X, y, sMineColor);
    else if (l->flags & LISTING_TOP)
        PrintAt(windowId, FONT_SMALL, COMPOUND_STRING("YOU'RE WINNING!"), DETAIL_X, y, sGoodColor);
}

static void DrawBoard(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 windowId = tWindowId;
    u8 line[32];
    u32 i;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    PrintAt(windowId, FONT_NORMAL, COMPOUND_STRING("AUCTION HOUSE"), 4, 0, sTextColor);
    FormatMoney(line, GetMoney(&gSaveBlock1Ptr->money));
    PrintRight(windowId, FONT_NORMAL, line, 220, 0, sTextColor);
    FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY), 0, 15, 224, 1);

    for (i = 0; i < BOARD_ROWS && tScroll + i < sListingCount; i++)
    {
        const struct AuctionListing *l = &sListings[tScroll + i];
        u32 y = BOARD_TOP + i * BOARD_LINE_HEIGHT;
        const u8 *color = (l->flags & LISTING_MINE) ? sMineColor : (l->flags & LISTING_TOP) ? sGoodColor : sTextColor;

        if (tScroll + i == tCursor)
            PrintAt(windowId, FONT_NORMAL, gText_SelectorArrow2, 0, y, sTextColor);
        PrintAt(windowId, FONT_NORMAL, GetSpeciesName(l->species), 10, y, color);
        FormatMoney(line, l->price);
        PrintRight(windowId, FONT_NORMAL, line, 140, y, sPriceColor);
    }
    if (sListingCount == 0)
    {
        PrintAt(windowId, FONT_NORMAL, COMPOUND_STRING("No auctions right now."), 10, BOARD_TOP, sTextColor);
    }
    else
    {
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY), DETAIL_X - 6, BOARD_TOP, 1, BOARD_ROWS * BOARD_LINE_HEIGHT);
        DrawDetail(windowId, &sListings[tCursor]);
    }

    FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_LIGHT_GRAY), 0, HELP_Y - 3, 224, 1);
    if (tMode == MODE_AMOUNT)
    {
        StringCopy(line, COMPOUND_STRING("Your bid: "));
        FormatMoney(line + StringLength(line), sAmount);
        PrintAt(windowId, FONT_NORMAL, line, 4, HELP_Y, sPriceColor);
        PrintRight(windowId, FONT_SMALL, COMPOUND_STRING("{UP_ARROW}{DOWN_ARROW} 100  {LEFT_ARROW}{RIGHT_ARROW} 1000  {A_BUTTON} BID"), 220, HELP_Y + 2, sTextColor);
    }
    else if (sListingCount == 0)
    {
        PrintAt(windowId, FONT_SMALL, COMPOUND_STRING("{B_BUTTON} BACK"), 4, HELP_Y + 2, sTextColor);
    }
    else
    {
        const struct AuctionListing *l = &sListings[tCursor];
        if (l->flags & LISTING_MINE)
            StringCopy(line, l->bids == 0 ? COMPOUND_STRING("{A_BUTTON} TAKE BACK") : COMPOUND_STRING("Bids are in!"));
        else if (l->flags & LISTING_TOP)
            StringCopy(line, COMPOUND_STRING("You have the top bid."));
        else
        {
            StringCopy(line, COMPOUND_STRING("{A_BUTTON} BID  from "));
            FormatMoney(line + StringLength(line), MinBid(l));
        }
        PrintAt(windowId, FONT_SMALL, line, 4, HELP_Y + 2, sTextColor);
        PrintRight(windowId, FONT_SMALL, COMPOUND_STRING("{B_BUTTON} BACK"), 220, HELP_Y + 2, sTextColor);
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
    ShowIcon(taskId);
}

// Shows the listings from the last LIFE_AUC_REQ_LIST. Use with waitstate.
// VAR_RESULT: LIFE_AUC_PICK_*.
void LifeAuction_Browse(void)
{
    u8 taskId = CreateTask(Task_Board, 80);
    s16 *data = gTasks[taskId].data;

    tWindowId = AddWindow(&sBoardWindow);
    tCursor = min(sPicked, sListingCount != 0 ? sListingCount - 1 : 0);
    tScroll = tCursor >= BOARD_ROWS ? tCursor - BOARD_ROWS + 1 : 0;
    tMode = MODE_LIST;
    tSpriteId = MAX_SPRITES;
    tSpecies = SPECIES_NONE;
    tFrames = 0;
    DrawStdWindowFrame(tWindowId, FALSE);
    PutWindowTilemap(tWindowId);
    DrawBoard(taskId);
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseBoard(u8 taskId, u16 result)
{
    s16 *data = gTasks[taskId].data;

    if (tSpriteId != MAX_SPRITES)
    {
        FreeAndDestroyMonIconSprite(&gSprites[tSpriteId]);
        FreeMonIconPalette(tSpecies);
    }
    sPicked = tCursor;
    ClearStdWindowAndFrameToTransparent(tWindowId, FALSE);
    ClearWindowTilemap(tWindowId);
    RemoveWindow(tWindowId);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    gSpecialVar_Result = result;
    ScriptContext_Enable();
}

static bool32 ChangeAmount(s32 delta, u32 low, u32 high)
{
    s32 amount = (s32)sAmount + delta;

    amount = max(amount, (s32)low);
    amount = min(amount, (s32)high);
    if ((u32)amount == sAmount)
        return FALSE;
    sAmount = amount;
    return TRUE;
}

static bool32 AmountInput(u32 low, u32 high)
{
    if (JOY_REPEAT(DPAD_UP))
        return ChangeAmount(100, low, high);
    if (JOY_REPEAT(DPAD_DOWN))
        return ChangeAmount(-100, low, high);
    if (JOY_REPEAT(DPAD_RIGHT))
        return ChangeAmount(1000, low, high);
    if (JOY_REPEAT(DPAD_LEFT))
        return ChangeAmount(-1000, low, high);
    return FALSE;
}

static void Task_Board(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    bool32 redraw = FALSE;
    const struct AuctionListing *l = &sListings[tCursor];

    // The clocks count down while the board is open.
    if (++tFrames >= 60)
    {
        u32 i;
        tFrames = 0;
        for (i = 0; i < sListingCount; i++)
        {
            if (sListings[i].secondsLeft != 0)
                sListings[i].secondsLeft--;
        }
        redraw = TRUE;
    }

    if (tMode == MODE_AMOUNT)
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            CloseBoard(taskId, LIFE_AUC_PICK_BID);
            return;
        }
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            tMode = MODE_LIST;
            redraw = TRUE;
        }
        else if (AmountInput(MinBid(l), MaxBid()))
        {
            PlaySE(SE_DEX_SCROLL);
            redraw = TRUE;
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseBoard(taskId, LIFE_AUC_PICK_NONE);
        return;
    }
    else if (JOY_NEW(A_BUTTON) && sListingCount != 0)
    {
        if ((l->flags & LISTING_MINE) && l->bids == 0)
        {
            PlaySE(SE_SELECT);
            CloseBoard(taskId, LIFE_AUC_PICK_CANCEL);
            return;
        }
        if ((l->flags & (LISTING_MINE | LISTING_TOP)) || MinBid(l) > MaxBid() || l->secondsLeft == 0)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            PlaySE(SE_SELECT);
            sAmount = MinBid(l);
            tMode = MODE_AMOUNT;
            redraw = TRUE;
        }
    }
    else if (JOY_REPEAT(DPAD_DOWN) && tCursor + 1 < sListingCount)
    {
        PlaySE(SE_SELECT);
        tCursor++;
        if (tCursor >= tScroll + BOARD_ROWS)
            tScroll++;
        redraw = TRUE;
    }
    else if (JOY_REPEAT(DPAD_UP) && tCursor > 0)
    {
        PlaySE(SE_SELECT);
        tCursor--;
        if (tCursor < tScroll)
            tScroll--;
        redraw = TRUE;
    }
    if (redraw)
        DrawBoard(taskId);
}

#undef tWindowId
#undef tCursor
#undef tScroll
#undef tMode
#undef tSpriteId
#undef tSpecies
#undef tFrames

// Start price for a new listing ---------------------------------------------

#define tWindowId data[0]

static const struct WindowTemplate sPriceWindow =
{
    .bg = 0,
    .tilemapLeft = 19,
    .tilemapTop = 9,
    .width = 10,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x008,
};

static void Task_PickPrice(u8 taskId);

static void DrawPrice(u8 windowId)
{
    u8 line[16];

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    FormatMoney(line, sAmount);
    PrintRight(windowId, FONT_NORMAL, line, 76, 1, sPriceColor);
    PrintAt(windowId, FONT_SMALL, COMPOUND_STRING("{UP_ARROW}{DOWN_ARROW}100 {LEFT_ARROW}{RIGHT_ARROW}1000"), 2, 17, sTextColor);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

// The opening bid for a new listing, over the message box. Use with
// waitstate. VAR_RESULT: TRUE if picked, FALSE on B.
void LifeAuction_PickPrice(void)
{
    u8 taskId = CreateTask(Task_PickPrice, 80);
    s16 *data = gTasks[taskId].data;

    if (sAmount < MIN_START || sAmount > MAX_MONEY)
        sAmount = 1000;
    tWindowId = AddWindow(&sPriceWindow);
    DrawStdWindowFrame(tWindowId, FALSE);
    PutWindowTilemap(tWindowId);
    DrawPrice(tWindowId);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_PickPrice(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = JOY_NEW(A_BUTTON) ? TRUE : FALSE;
        ConvertIntToDecimalStringN(gStringVar2, sAmount, STR_CONV_MODE_LEFT_ALIGN, 6);
        ClearStdWindowAndFrameToTransparent(tWindowId, FALSE);
        ClearWindowTilemap(tWindowId);
        RemoveWindow(tWindowId);
        ScheduleBgCopyTilemapToVram(0);
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (AmountInput(MIN_START, MAX_MONEY))
    {
        PlaySE(SE_DEX_SCROLL);
        DrawPrice(tWindowId);
    }
}

#undef tWindowId

// For messages after LifeAuction_Browse: gStringVar1 the species,
// gStringVar2 the amount.
void LifeAuction_BufferPicked(void)
{
    StringCopy(gStringVar1, GetSpeciesName(sListings[sPicked].species));
    ConvertIntToDecimalStringN(gStringVar2, sAmount, STR_CONV_MODE_LEFT_ALIGN, 6);
}

// For the ONLINE menu.
void LifeAuction_OpenFromMenu(void)
{
    ScriptContext_SetupScript(Life_EventScript_Auction);
}
