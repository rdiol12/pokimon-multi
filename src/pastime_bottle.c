#include "global.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "menu.h"
#include "palette.h"
#include "pastime.h"
#include "rtc.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "window.h"
#include "world.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/rgb.h"

// Bottle treasure maps: once a day a bottle washes up on one of
// BOTTLE_BEACHES beaches, bobbing at the water's edge. Inside is a letter
// from Captain Marlow, who buried his treasures around Hoenn's seas, and a
// torn piece of one of his BOTTLE_MAPS maps. With all BOTTLE_MAP_PIECES
// pieces a map is whole: its riddle names the place, and A on the X digs
// up the treasure. R > PASTIMES > BOTTLE MAPS lists the maps.

extern const u8 Bottle_EventScript_Bottle[];
extern const u8 Bottle_EventScript_Dig[];

struct BottleSpot
{
    u16 map;
    u8 x, y;
};

static const struct BottleSpot sBeaches[BOTTLE_BEACHES] =
{
    {MAP_ROUTE104, 14, 64},
    {MAP_ROUTE109, 13, 19},
    {MAP_DEWFORD_TOWN, 10, 2},
    {MAP_SLATEPORT_CITY, 35, 57},
    {MAP_LILYCOVE_CITY, 64, 23},
};

// The maps: 16 x 12 cells. '~' sea, ':' sand, '#' land, 'T' a tree, 'r' a
// rock, 'X' the treasure. Piece k is the quarter (k % 2, k / 2).
#define MAP_COLS 16
#define MAP_ROWS 12

struct TreasureMap
{
    const u8 *name;
    const u8 *riddle;
    struct BottleSpot spot;
    u16 item;
    const char *cells;
};

static const struct TreasureMap sMaps[BOTTLE_MAPS] =
{
    {
        COMPOUND_STRING("ROUTE 109"),
        COMPOUND_STRING("South of SLATEPORT the sand runs\nto the sea. Dig where it meets\lthe water on the east side!"),
        {MAP_ROUTE109, 32, 10}, ITEM_PEARL_STRING,
        "################"
        "###T####T#######"
        "#######:::::####"
        "##T###::::::::##"
        "#####:::::::::::"
        "####::::::X:::~~"
        "###::::::::::~~~"
        "##::::::::~~~~~~"
        "#:::::~~~~~~~~~~"
        "::::~~~~~~~~~~~~"
        "::~~~~~~~~~~~~~~"
        "~~~~~~~~~~~~~~~~",
    },
    {
        COMPOUND_STRING("ROUTE 132"),
        COMPOUND_STRING("Out where the currents run east\nfrom PACIFIDLOG lies a lone isle.\lDig on its sandy shore!"),
        {MAP_ROUTE132, 46, 24}, ITEM_BIG_NUGGET,
        "~~~~~~~~~~~~~~~~"
        "~~~~~~~~~~~r~~~~"
        "~~~~~~~~~~~~~~~~"
        "~~~~~:::::~~~~~~"
        "~~~~:##T##:~~~~~"
        "~~~:####T##:~~~~"
        "~~~:#######X~~~~"
        "~~~~:#####:~~~~~"
        "~~~~~:::::~~~~~~"
        "~r~~~~~~~~~~~~~~"
        "~~~~~~~~~~~~r~~~"
        "~~~~~~~~~~~~~~~~",
    },
    {
        COMPOUND_STRING("ROUTE 134"),
        COMPOUND_STRING("Between PACIFIDLOG and SLATEPORT\nthe water runs fast past a strip\lof sand. Dig there, by the rocks!"),
        {MAP_ROUTE134, 44, 16}, ITEM_COMET_SHARD,
        "~~~~~~~~~~~~~~~~"
        "~~~~~~~~~~~~~~~~"
        "~~~~:::::~~~~~~~"
        "~~~:##T##:~~~~~~"
        "~~~:#####X~~~~~~"
        "~~~~:::::~~~r~~~"
        "~~~~~~~~~~:::::~"
        "~~r~~~~~~:##T##:"
        "~~~~~~~~~~:::::~"
        "~~~~~~~~~~~~~~~~"
        "~~~~~r~~~~~~~~~~"
        "~~~~~~~~~~~~~~~~",
    },
};

// Captain Marlow's letters, one with each piece.
static const u8 *const sLetters[BOTTLE_MAPS * BOTTLE_MAP_PIECES] =
{
    COMPOUND_STRING("“To whoever finds this: I am Captain\nMarlow. I've hidden my treasures\lwhere no one will look.”"),
    COMPOUND_STRING("“My crew wanted to spend it all. So I\ntore the maps up and gave them to\lthe sea. Let the sea decide!”"),
    COMPOUND_STRING("“The sand south of the port is soft.\nA good place to hide a necklace\lof pearls, I thought.”"),
    COMPOUND_STRING("“That's the first map whole, if the\nsea has been kind. Dig, friend!”"),
    COMPOUND_STRING("“The currents near PACIFIDLOG nearly\ntook my ship. I found an island\lthere that no chart shows.”"),
    COMPOUND_STRING("“My WAILMER pulled the boat ashore.\nI buried a nugget the size of my\lfist in the sand.”"),
    COMPOUND_STRING("“If you're reading this, you're as\nstubborn as I am. Good.”"),
    COMPOUND_STRING("“Two maps whole! My old crew never\ngot this far.”"),
    COMPOUND_STRING("“The last treasure is the best: a\nshard of a comet that fell the\lnight my daughter was born.”"),
    COMPOUND_STRING("“I hid it near home, between the\ntwo ports, where the water runs\lfast.”"),
    COMPOUND_STRING("“I'm too old to sail now. I watch\nthe sea from my porch and think\lof the friend who will find it.”"),
    COMPOUND_STRING("“The last piece! Go, and may the\nsea be calm for you.  MARLOW”"),
};

#define sSave (gSaveBlock3Ptr->pastime)

#define ALL_PIECES ((1 << (BOTTLE_MAPS * BOTTLE_MAP_PIECES)) - 1)

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

static bool32 IsHere(const struct BottleSpot *spot)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(spot->map)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(spot->map);
}

static const struct BottleSpot *TodaysBeach(void)
{
    if (sSave.bottlePieces == ALL_PIECES || sSave.bottleDay == Today() + 1)
        return NULL;
    return &sBeaches[(Today() * 3) % BOTTLE_BEACHES];
}

static u32 CountBits(u32 bits)
{
    u32 n = 0;

    for (; bits != 0; bits >>= 1)
        n += bits & 1;
    return n;
}

static u32 PiecesOf(u32 map)
{
    return (sSave.bottlePieces >> (map * BOTTLE_MAP_PIECES)) & ((1 << BOTTLE_MAP_PIECES) - 1);
}

static bool32 IsMapWhole(u32 map)
{
    return PiecesOf(map) == (1 << BOTTLE_MAP_PIECES) - 1;
}

// The bottle on the beach -------------------------------------------------------------------

#define TAG_BOTTLE 0x5A76

static const u32 sBottleGfx[] = INCBIN_U32("graphics/pastime/bottle.4bpp");
static const u16 sBottlePal[] = INCBIN_U16("graphics/pastime/bottle.gbapal");
static const struct SpriteSheet sBottleSheet = {sBottleGfx, sizeof(sBottleGfx), TAG_BOTTLE};
static const struct SpritePalette sBottlePalette = {sBottlePal, TAG_BOTTLE};

static const struct OamData sBottleOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2,
};

static void SpriteCB_Bottle(struct Sprite *sprite);

static const struct SpriteTemplate sBottleTemplate =
{
    .tileTag = TAG_BOTTLE,
    .paletteTag = TAG_BOTTLE,
    .oam = &sBottleOam,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Bottle,
};

static EWRAM_DATA u8 sBottleSpriteId = 0;

// Bobs on the waves; hidden when far off screen (a sprite's coordinates
// would wrap round).
static void SpriteCB_Bottle(struct Sprite *sprite)
{
    s32 x = sprite->x + gSpriteCoordOffsetX, y = sprite->y + gSpriteCoordOffsetY;

    sprite->data[0]++;
    sprite->y2 = (sprite->data[0] / 16) % 2;
    sprite->invisible = x < -16 || x > DISPLAY_WIDTH + 16 || y < -16 || y > DISPLAY_HEIGHT + 16;
}

static bool32 IsBottleSpriteUp(void)
{
    return gSprites[sBottleSpriteId].inUse && gSprites[sBottleSpriteId].callback == SpriteCB_Bottle;
}

// Pastime_Update: puts the bottle on today's beach (or takes it away).
void Bottle_Update(void)
{
    const struct BottleSpot *beach = TodaysBeach();
    s16 x, y;

    if (beach == NULL || !IsHere(beach))
    {
        if (IsBottleSpriteUp())
            DestroySprite(&gSprites[sBottleSpriteId]);
        return;
    }
    if (IsBottleSpriteUp())
        return;
    if (GetSpriteTileStartByTag(TAG_BOTTLE) == 0xFFFF)
        LoadSpriteSheet(&sBottleSheet);
    if (IndexOfSpritePaletteTag(TAG_BOTTLE) == 0xFF)
        LoadSpritePalette(&sBottlePalette);
    x = beach->x + MAP_OFFSET;
    y = beach->y + MAP_OFFSET;
    SetSpritePosToOffsetMapCoords(&x, &y, 8, 8);
    sBottleSpriteId = CreateSprite(&sBottleTemplate, x, y, 0xFF);
    if (sBottleSpriteId == MAX_SPRITES)
        return;
    gSprites[sBottleSpriteId].coordOffsetEnabled = TRUE;
    SpriteCB_Bottle(&gSprites[sBottleSpriteId]);
}

static bool32 IsAt(const struct BottleSpot *spot, s16 x, s16 y)
{
    s16 px, py;

    if (spot == NULL || !IsHere(spot))
        return FALSE;
    PlayerGetDestCoords(&px, &py);
    return (x - MAP_OFFSET == spot->x && y - MAP_OFFSET == spot->y)
        || (px - MAP_OFFSET == spot->x && py - MAP_OFFSET == spot->y);
}

// The A button: facing (or standing on) the bottle picks it up; on a whole
// map's X, digs.
bool32 Bottle_TryInteract(s16 x, s16 y)
{
    u32 i;

    if (IsAt(TodaysBeach(), x, y))
    {
        ScriptContext_SetupScript(Bottle_EventScript_Bottle);
        return TRUE;
    }
    for (i = 0; i < BOTTLE_MAPS; i++)
    {
        if (IsMapWhole(i) && !(sSave.bottleTreasures & (1 << i)) && IsAt(&sMaps[i].spot, x, y))
        {
            gSpecialVar_0x8004 = i;
            ScriptContext_SetupScript(Bottle_EventScript_Dig);
            return TRUE;
        }
    }
    return FALSE;
}

// The map picture -----------------------------------------------------------------------

// Only the standard window colors: a message box (which shows next to the
// map) loads them again.
#define COLOR_MISSING TEXT_COLOR_LIGHT_GRAY
#define COLOR_INK     TEXT_COLOR_DARK_GRAY
#define COLOR_SEA     TEXT_COLOR_LIGHT_BLUE
#define COLOR_WAVE    TEXT_COLOR_WHITE
#define COLOR_SAND    TEXT_COLOR_WHITE
#define COLOR_LAND    TEXT_COLOR_LIGHT_GREEN
#define COLOR_TREE    TEXT_COLOR_GREEN

#define MAP_WIN_X 19 // tiles
#define MAP_WIN_Y 1
#define MAP_WIN_W 10
#define MAP_WIN_H 8
#define MAP_BASE_BLOCK 0xD0
#define MAP_CELL 5 // pixels
#define MAP_GRID_Y 2

static EWRAM_DATA bool8 sMapShown = FALSE;
static EWRAM_DATA u8 sMapWindowId = 0;

static void DrawMap(u32 map)
{
    const char *cells = sMaps[map].cells;
    u32 pieces = PiecesOf(map), i;

    FillWindowPixelBuffer(sMapWindowId, PIXEL_FILL(COLOR_MISSING));
    for (i = 0; i < MAP_COLS * MAP_ROWS; i++)
    {
        u32 col = i % MAP_COLS, row = i / MAP_COLS;
        u32 piece = (row / (MAP_ROWS / 2)) * 2 + col / (MAP_COLS / 2);
        u32 x = col * MAP_CELL, y = MAP_GRID_Y + row * MAP_CELL;

        if (!(pieces & (1 << piece)))
            continue;
        switch (cells[i])
        {
        case '~':
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_SEA), x, y, MAP_CELL, MAP_CELL);
            if ((col + row * 3) % 7 == 0)
                FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_WAVE), x + 1, y + 2, 3, 1);
            break;
        case ':':
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_SAND), x, y, MAP_CELL, MAP_CELL);
            break;
        case 'T':
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_LAND), x, y, MAP_CELL, MAP_CELL);
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_TREE), x + 1, y, 3, 3);
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_INK), x + 2, y + 3, 1, 2);
            break;
        case 'r':
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_SEA), x, y, MAP_CELL, MAP_CELL);
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_INK), x + 1, y + 1, 3, 3);
            break;
        case 'X':
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_SAND), x, y, MAP_CELL, MAP_CELL);
            break;
        default:
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_LAND), x, y, MAP_CELL, MAP_CELL);
            break;
        }
    }
    // The X, bigger than a cell, drawn last.
    for (i = 0; i < MAP_COLS * MAP_ROWS; i++)
    {
        u32 col = i % MAP_COLS, row = i / MAP_COLS, j;
        u32 piece = (row / (MAP_ROWS / 2)) * 2 + col / (MAP_COLS / 2);
        s32 x = col * MAP_CELL - 1, y = MAP_GRID_Y + row * MAP_CELL - 1;

        if (cells[i] != 'X' || !(pieces & (1 << piece)))
            continue;
        for (j = 0; j < 7; j++)
        {
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(TEXT_COLOR_RED), x + j, y + j, 2, 1);
            FillWindowPixelRect(sMapWindowId, PIXEL_FILL(TEXT_COLOR_RED), x + 6 - j, y + j, 2, 1);
        }
    }
    // The tears between the pieces.
    FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_INK), MAP_COLS / 2 * MAP_CELL - 1, 0, 1, MAP_WIN_H * 8);
    FillWindowPixelRect(sMapWindowId, PIXEL_FILL(COLOR_INK), 0, MAP_GRID_Y + MAP_ROWS / 2 * MAP_CELL - 1, MAP_WIN_W * 8, 1);
    CopyWindowToVram(sMapWindowId, COPYWIN_GFX);
}

// Script: shows map VAR_0x8004 (as far as its pieces go) next to a message.
void Bottle_ShowMap(void)
{
    struct WindowTemplate template = CreateWindowTemplate(0, MAP_WIN_X, MAP_WIN_Y, MAP_WIN_W, MAP_WIN_H, 15, MAP_BASE_BLOCK);

    Bottle_HideMap();
    sMapWindowId = AddWindow(&template);
    PutWindowTilemap(sMapWindowId);
    SetStandardWindowBorderStyle(sMapWindowId, TRUE);
    DrawMap(gSpecialVar_0x8004);
    CopyWindowToVram(sMapWindowId, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
    sMapShown = TRUE;
}

void Bottle_HideMap(void)
{
    if (!sMapShown)
        return;
    ClearStdWindowAndFrameToTransparent(sMapWindowId, TRUE);
    RemoveWindow(sMapWindowId);
    sMapShown = FALSE;
}

// Scripts ------------------------------------------------------------------------------

// Script: opens today's bottle. gPastimeText = the letter; VAR_0x8004 = the
// map the piece is of; gStringVar1 = the map's place; gStringVar2 = its
// pieces found; VAR_RESULT = TRUE if the map is now whole (gStringVar3 =
// its riddle).
void Bottle_Open(void)
{
    u32 piece = CountBits(sSave.bottlePieces), map = piece / BOTTLE_MAP_PIECES;

    sSave.bottleDay = Today() + 1;
    sSave.bottlePieces |= 1 << piece;
    StringCopy(gPastimeText, sLetters[piece]);
    gSpecialVar_0x8004 = map;
    StringCopy(gStringVar1, sMaps[map].name);
    ConvertIntToDecimalStringN(gStringVar2, CountBits(PiecesOf(map)), STR_CONV_MODE_LEFT_ALIGN, 1);
    gSpecialVar_Result = IsMapWhole(map);
    StringCopy(gStringVar3, sMaps[map].riddle);
}

// Script: digs up map VAR_0x8004's treasure. VAR_0x8005 = the item.
void Bottle_Dig(void)
{
    sSave.bottleTreasures |= 1 << gSpecialVar_0x8004;
    gSpecialVar_0x8005 = sMaps[gSpecialVar_0x8004].item;
}

// R > PASTIMES > BOTTLE MAPS ----------------------------------------------------------------

static void GetLine(u16 index, u8 *dest)
{
    if (index < BOTTLE_MAPS)
    {
        dest = StringCopy(dest, COMPOUND_STRING("Map "));
        dest = ConvertIntToDecimalStringN(dest, index + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 40}"));
        if (PiecesOf(index) == 0)
        {
            StringCopy(dest, COMPOUND_STRING("-"));
            return;
        }
        dest = StringCopy(dest, IsMapWhole(index) ? sMaps[index].name : COMPOUND_STRING("???"));
        dest = StringCopy(dest, COMPOUND_STRING("{CLEAR_TO 112}"));
        if (sSave.bottleTreasures & (1 << index))
        {
            StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}DUG UP"));
            return;
        }
        dest = ConvertIntToDecimalStringN(dest, CountBits(PiecesOf(index)), STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringCopy(dest, COMPOUND_STRING("/"));
        ConvertIntToDecimalStringN(dest, BOTTLE_MAP_PIECES, STR_CONV_MODE_LEFT_ALIGN, 1);
        return;
    }
    dest = StringCopy(dest, COMPOUND_STRING("Bottle today{CLEAR_TO 112}"));
    if (sSave.bottlePieces == ALL_PIECES)
        StringCopy(dest, COMPOUND_STRING("No more"));
    else if (sSave.bottleDay == Today() + 1)
        StringCopy(dest, COMPOUND_STRING("Found"));
    else
        StringCopy(dest, COMPOUND_STRING("{COLOR GREEN}{SHADOW LIGHT_GREEN}On a beach"));
}

static void GetDetail(u16 index, u8 *dest)
{
    if (index < BOTTLE_MAPS && IsMapWhole(index) && !(sSave.bottleTreasures & (1 << index)))
    {
        // The riddle, on one line.
        const u8 *riddle = sMaps[index].riddle;
        u32 i;
        for (i = 0; riddle[i] != EOS && riddle[i] != CHAR_NEWLINE; i++)
            dest[i] = riddle[i];
        dest[i] = EOS;
        return;
    }
    StringCopy(dest, COMPOUND_STRING("Bottles wash up on the beaches"));
}

void Bottle_ShowList(void)
{
    WorldBoard_Show(COMPOUND_STRING("BOTTLE MAPS"), BOTTLE_MAPS + 1, GetLine, GetDetail);
}
