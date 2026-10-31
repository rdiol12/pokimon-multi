#include "global.h"
#include "treasure.h"
#include "bg.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "menu.h"
#include "netplay.h"
#include "palette.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "trainer_level.h"
#include "window.h"
#include "world.h"
#include "constants/items.h"

// Treasure hunts (docs/adventure.md): the old treasure hunter in Dewford
// Town hands out one treasure map a day, the same one to everybody. It
// shows the spot inked on parchment with an X (drawn by
// tools/adventure/make_treasure_maps.py) and a riddle that names the place.
// Pressing A on the X digs up the treasure. Treasures found go on the
// server's TREASURE HUNTERS board.

#define sSave (gSaveBlock3Ptr->adventure)

#define XP_PER_TREASURE 25

#define TREASURE_NONE    0 // no map for today yet
#define TREASURE_HUNTING 1
#define TREASURE_FOUND   2

// The map window: 7x5 metatiles of map, framed, above the message box.
#define MAP_WINDOW_PAL 13

struct TreasureSpot
{
    u16 map;
    u8 x, y;
    const u8 *riddle;
    const u32 *gfx;
    const u16 *pal;
};

static const u32 sMap0[] = INCGFX_U32("graphics/adventure/treasure_maps/0.png", ".4bpp");
static const u16 sMap0Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/0.png", ".gbapal");
static const u32 sMap1[] = INCGFX_U32("graphics/adventure/treasure_maps/1.png", ".4bpp");
static const u16 sMap1Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/1.png", ".gbapal");
static const u32 sMap2[] = INCGFX_U32("graphics/adventure/treasure_maps/2.png", ".4bpp");
static const u16 sMap2Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/2.png", ".gbapal");
static const u32 sMap3[] = INCGFX_U32("graphics/adventure/treasure_maps/3.png", ".4bpp");
static const u16 sMap3Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/3.png", ".gbapal");
static const u32 sMap4[] = INCGFX_U32("graphics/adventure/treasure_maps/4.png", ".4bpp");
static const u16 sMap4Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/4.png", ".gbapal");
static const u32 sMap5[] = INCGFX_U32("graphics/adventure/treasure_maps/5.png", ".4bpp");
static const u16 sMap5Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/5.png", ".gbapal");
static const u32 sMap6[] = INCGFX_U32("graphics/adventure/treasure_maps/6.png", ".4bpp");
static const u16 sMap6Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/6.png", ".gbapal");
static const u32 sMap7[] = INCGFX_U32("graphics/adventure/treasure_maps/7.png", ".4bpp");
static const u16 sMap7Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/7.png", ".gbapal");
static const u32 sMap8[] = INCGFX_U32("graphics/adventure/treasure_maps/8.png", ".4bpp");
static const u16 sMap8Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/8.png", ".gbapal");
static const u32 sMap9[] = INCGFX_U32("graphics/adventure/treasure_maps/9.png", ".4bpp");
static const u16 sMap9Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/9.png", ".gbapal");
static const u32 sMap10[] = INCGFX_U32("graphics/adventure/treasure_maps/10.png", ".4bpp");
static const u16 sMap10Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/10.png", ".gbapal");
static const u32 sMap11[] = INCGFX_U32("graphics/adventure/treasure_maps/11.png", ".4bpp");
static const u16 sMap11Pal[] = INCGFX_U16("graphics/adventure/treasure_maps/11.png", ".gbapal");

// Same order as SPOTS in tools/adventure/make_treasure_maps.py.
static const struct TreasureSpot sTreasureSpots[] =
{
    {MAP_OLDALE_TOWN, 10, 10, COMPOUND_STRING("Among the flowers by the town's sign,\nnorth of the POKéMON CENTER."), sMap0, sMap0Pal},
    {MAP_PETALBURG_CITY, 16, 11, COMPOUND_STRING("Just below the sign of the city's\nGYM, beside the sandy path."), sMap1, sMap1Pal},
    {MAP_ROUTE104, 19, 53, COMPOUND_STRING("On the grass east of the old sailor's\ncottage, above the sand."), sMap2, sMap2Pal},
    {MAP_DEWFORD_TOWN, 12, 15, COMPOUND_STRING("On the sand right by the sign of\nthe island's GYM."), sMap3, sMap3Pal},
    {MAP_SLATEPORT_CITY, 20, 32, COMPOUND_STRING("On the green strip by the stone road,\nwest of the shipyard's fence."), sMap4, sMap4Pal},
    {MAP_ROUTE113, 30, 8, COMPOUND_STRING("In the ash a few steps south of\nthe GLASS WORKSHOP's sign."), sMap5, sMap5Pal},
    {MAP_MOSSDEEP_CITY, 33, 22, COMPOUND_STRING("On the sandy path west of the\nhouse with the red roof."), sMap6, sMap6Pal},
    {MAP_ROUTE119, 9, 35, COMPOUND_STRING("South of the WEATHER INSTITUTE's\nsign, between sand and tall grass."), sMap7, sMap7Pal},
    {MAP_FALLARBOR_TOWN, 13, 10, COMPOUND_STRING("On the sandy road in front of the\nPOKéMON CENTER, where paths cross."), sMap8, sMap8Pal},
    {MAP_ROUTE123, 24, 9, COMPOUND_STRING("Inside the BERRY MASTER's fenced\ngarden, by its grassy path."), sMap9, sMap9Pal},
    {MAP_LILYCOVE_CITY, 22, 27, COMPOUND_STRING("In the flower bed below the CONTEST\nHALL, just above the cliff."), sMap10, sMap10Pal},
    {MAP_ROUTE116, 40, 11, COMPOUND_STRING("South of the sign by the TUNNELER's\nrest house."), sMap11, sMap11Pal},
};

// What a chest can hold; the day picks one.
static const u16 sTreasures[] =
{
    ITEM_NUGGET, ITEM_BIG_PEARL, ITEM_STAR_PIECE, ITEM_RARE_CANDY,
    ITEM_PP_UP, ITEM_ABILITY_CAPSULE, ITEM_BOTTLE_CAP, ITEM_COMET_SHARD,
    ITEM_HEART_SCALE, ITEM_MOON_STONE, ITEM_SUN_STONE, ITEM_BIG_NUGGET,
    ITEM_RELIC_GOLD, ITEM_RARE_BONE, ITEM_PEARL_STRING, ITEM_GOLD_BOTTLE_CAP,
};

static EWRAM_DATA u8 sMapWindowId = 0;
static EWRAM_DATA bool8 sMapOpen = FALSE;

static const struct WindowTemplate sMapWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 8,
    .tilemapTop = 3,
    .width = 14,
    .height = 10,
    .paletteNum = MAP_WINDOW_PAL,
    .baseBlock = 0x008,
};

static u16 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

// Everybody hunts the same spot on the same day; the order of the spots
// is shuffled so neighbouring days aren't neighbouring towns.
static u32 TodaysSpot(void)
{
    return (Today() * 5 + 3) % ARRAY_COUNT(sTreasureSpots);
}

static void NewDayCheck(void)
{
    if (sSave.treasureDay != Today())
        sSave.treasureState = TREASURE_NONE;
}

// Script: VAR_RESULT = TREASURE_TALK_*. Hands out today's map if there's
// none yet.
void Treasure_Talk(void)
{
    NewDayCheck();
    if (sSave.treasureState == TREASURE_FOUND)
    {
        gSpecialVar_Result = TREASURE_TALK_DONE;
        return;
    }
    if (sSave.treasureState == TREASURE_NONE)
    {
        sSave.treasureDay = Today();
        sSave.treasureSpot = TodaysSpot();
        sSave.treasureState = TREASURE_HUNTING;
        gSpecialVar_Result = sSave.treasuresFound == 0 ? TREASURE_TALK_FIRST : TREASURE_TALK_NEW;
        return;
    }
    gSpecialVar_Result = TREASURE_TALK_STILL;
}

// Script: VAR_RESULT = whether there's a map to look at today.
void Treasure_HasMap(void)
{
    NewDayCheck();
    gSpecialVar_Result = sSave.treasureState == TREASURE_HUNTING;
}

// Script: opens today's map above the message box and puts its riddle in
// STR_VAR_1. The script shows the riddle, then calls Treasure_CloseMap.
void Treasure_OpenMap(void)
{
    const struct TreasureSpot *spot = &sTreasureSpots[sSave.treasureSpot % ARRAY_COUNT(sTreasureSpots)];

    StringCopy(gStringVar1, spot->riddle);
    if (sMapOpen)
        return;
    sMapOpen = TRUE;
    sMapWindowId = AddWindow(&sMapWindowTemplate);
    LoadPalette(spot->pal, BG_PLTT_ID(MAP_WINDOW_PAL), PLTT_SIZE_4BPP);
    DrawStdWindowFrame(sMapWindowId, FALSE);
    BlitBitmapToWindow(sMapWindowId, (const u8 *)spot->gfx, 0, 0, 112, 80);
    PutWindowTilemap(sMapWindowId);
    CopyWindowToVram(sMapWindowId, COPYWIN_FULL);
}

void Treasure_CloseMap(void)
{
    if (!sMapOpen)
        return;
    sMapOpen = FALSE;
    ClearStdWindowAndFrameToTransparent(sMapWindowId, FALSE);
    CopyWindowToVram(sMapWindowId, COPYWIN_FULL);
    RemoveWindow(sMapWindowId);
}

static bool32 IsTreasureAt(s16 x, s16 y)
{
    const struct TreasureSpot *spot = &sTreasureSpots[sSave.treasureSpot % ARRAY_COUNT(sTreasureSpots)];

    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(spot->map)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(spot->map)
        && x - MAP_OFFSET == spot->x && y - MAP_OFFSET == spot->y;
}

// The A button: on the X, or facing it, digs.
bool32 Treasure_TryInteract(s16 x, s16 y)
{
    s16 px, py;

    if (sSave.treasureState != TREASURE_HUNTING)
        return FALSE;
    PlayerGetDestCoords(&px, &py);
    if (!IsTreasureAt(x, y) && !IsTreasureAt(px, py))
        return FALSE;
    NewDayCheck();
    if (sSave.treasureState != TREASURE_HUNTING)
    {
        ScriptContext_SetupScript(Adventure_EventScript_TreasureTooLate);
        return TRUE;
    }
    ScriptContext_SetupScript(Adventure_EventScript_TreasureDig);
    return TRUE;
}

// Script: digs up today's treasure. VAR_0x8000 = the item, for giveitem.
void Treasure_Dig(void)
{
    sSave.treasureState = TREASURE_FOUND;
    sSave.treasuresFound++;
    gSpecialVar_0x8000 = sTreasures[(sSave.treasureDay * 7 + sSave.treasureSpot) % ARRAY_COUNT(sTreasures)];
    TrainerLevel_AddXp(XP_PER_TREASURE);
    if (Netplay_IsOnline())
        World_SubmitScore(WORLD_BOARD_TREASURE, sSave.treasuresFound, sSave.treasureSpot);
    ConvertIntToDecimalStringN(gStringVar2, sSave.treasuresFound, STR_CONV_MODE_LEFT_ALIGN, 5);
}

// Screenshots (src/adventure.c): today's map is this spot, still to dig.
void Treasure_Showcase(u32 spot)
{
    sSave.treasureDay = Today();
    sSave.treasureSpot = spot % ARRAY_COUNT(sTreasureSpots);
    sSave.treasureState = TREASURE_HUNTING;
}
