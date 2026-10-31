#include "global.h"
#include "dungeon.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "item.h"
#include "netplay.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "trainer_level.h"
#include "world.h"
#include "constants/items.h"
#include "constants/map_types.h"
#include "constants/species.h"

// The Rift Below (docs/adventure.md): a cave under Route 114 that is dug
// out anew every time the player climbs down a ladder. Each floor is a few
// rooms joined by tunnels, with the way out, item balls, a ladder to the
// next floor, and wild Pokémon that get stronger and rarer the deeper it
// goes. The deepest floor reached goes on the server's DEEPEST DIVERS board.

#define sSave (gSaveBlock3Ptr->adventure)

// The floor is planned on a 16x16 grid of 2x2-metatile cells, so walls are
// always at least two metatiles thick and the cave edges always fit.
#define CELLS       16
#define MAP_SIZE    (CELLS * 2)
#define MAX_ROOMS   8
#define MAX_ITEMS   4
#define XP_PER_FLOOR 15

// The start room around the arrival point and the way out below it.
#define START_X     16
#define START_Y     27
#define EXIT_Y      29

// Metatiles (gTileset_Cave), with collision and elevation as in Altering Cave.
#define FLOOR       (0x3000 | 0x211)
#define FLOOR_ROCKY (0x3000 | 0x201)
#define SOLID       (0x0400 | 0x211)
#define WALL(id)    (0x0400 | (id))
#define BOULDER     (0x3400 | 0x266)
#define LADDER      (0x0400 | 0x23F)
#define EXIT_STEP   (0x0000 | 0x23C)
#define EXIT_LEFT   (0x0400 | 0x258)
#define EXIT_MID    (0x0400 | 0x259)
#define EXIT_RIGHT  (0x0400 | 0x25A)

struct Room
{
    u8 x, y, w, h; // in cells
};

struct DungeonPlan
{
    u32 rng;
    u8 floor;
    u8 numRooms;
    struct Room rooms[MAX_ROOMS];
    bool8 open[CELLS][CELLS];
    u8 ladderX, ladderY;      // metatiles
    u8 itemX[MAX_ITEMS], itemY[MAX_ITEMS];
    u16 items[MAX_ITEMS];
    u8 numItems;
};

static EWRAM_DATA struct DungeonPlan sPlan = {0};

// The plan comes from the floor's seed alone, so drawing it again after a
// battle or on Continue gives the same cave.
static u32 PlanRandom(void)
{
    sPlan.rng = sPlan.rng * 1103515245 + 24691;
    return sPlan.rng >> 16;
}

static u32 PlanRange(u32 n)
{
    return PlanRandom() % n;
}

static void Dig(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 x, y;

    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            if (x > 0 && y > 0 && x < CELLS - 1 && y < CELLS - 1)
                sPlan.open[y][x] = TRUE;
}

static bool32 RoomFits(const struct Room *room)
{
    u32 i;

    for (i = 0; i < sPlan.numRooms; i++)
    {
        const struct Room *o = &sPlan.rooms[i];
        // One cell of rock between rooms.
        if (room->x <= o->x + o->w && o->x <= room->x + room->w
         && room->y <= o->y + o->h && o->y <= room->y + room->h)
            return FALSE;
    }
    return TRUE;
}

// An L-shaped tunnel, one cell wide, between the middles of two rooms.
static void Tunnel(const struct Room *a, const struct Room *b)
{
    u32 ax = a->x + a->w / 2, ay = a->y + a->h / 2;
    u32 bx = b->x + b->w / 2, by = b->y + b->h / 2;

    if (PlanRange(2))
    {
        Dig(min(ax, bx), ay, max(ax, bx), ay);
        Dig(bx, min(ay, by), bx, max(ay, by));
    }
    else
    {
        Dig(ax, min(ay, by), ax, max(ay, by));
        Dig(min(ax, bx), by, max(ax, bx), by);
    }
}

static const u16 sItemsShallow[] = {
    ITEM_SUPER_POTION, ITEM_GREAT_BALL, ITEM_ETHER, ITEM_SUPER_REPEL,
    ITEM_ESCAPE_ROPE, ITEM_HARD_STONE, ITEM_REVIVE, ITEM_FULL_HEAL,
};
static const u16 sItemsMiddle[] = {
    ITEM_HYPER_POTION, ITEM_ULTRA_BALL, ITEM_REVIVE, ITEM_MAX_REPEL,
    ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE, ITEM_LEAF_STONE,
    ITEM_MOON_STONE, ITEM_EVERSTONE, ITEM_ELIXIR, ITEM_STAR_PIECE,
};
static const u16 sItemsDeep[] = {
    ITEM_MAX_POTION, ITEM_FULL_RESTORE, ITEM_MAX_REVIVE, ITEM_RARE_CANDY,
    ITEM_PP_UP, ITEM_NUGGET, ITEM_ULTRA_BALL, ITEM_MAX_ELIXIR,
    ITEM_DUSK_STONE, ITEM_SHINY_STONE, ITEM_DAWN_STONE, ITEM_STAR_PIECE,
};
// Every fifth floor one ball holds a prize.
static const u16 sItemsPrize[] = {
    ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_BIG_NUGGET, ITEM_PP_MAX, ITEM_MASTER_BALL,
};

static u16 PickItem(u32 slot)
{
    u32 floor = sPlan.floor;

    if (slot == 0 && floor % 5 == 0)
    {
        // A Master Ball only from B25F down, one in five.
        u32 n = floor >= 25 ? ARRAY_COUNT(sItemsPrize) : ARRAY_COUNT(sItemsPrize) - 1;
        return sItemsPrize[PlanRange(n)];
    }
    if (floor >= 8)
        return sItemsDeep[PlanRange(ARRAY_COUNT(sItemsDeep))];
    if (floor >= 4)
        return sItemsMiddle[PlanRange(ARRAY_COUNT(sItemsMiddle))];
    return sItemsShallow[PlanRange(ARRAY_COUNT(sItemsShallow))];
}

static bool32 IsFloorAt(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= MAP_SIZE || y >= MAP_SIZE)
        return FALSE;
    return sPlan.open[y / 2][x / 2];
}

// A free spot in a room, away from its edges, for a ball or the ladder.
static bool32 PickSpot(const struct Room *room, u8 *outX, u8 *outY)
{
    u32 tries, i;

    for (tries = 0; tries < 20; tries++)
    {
        u32 x = room->x * 2 + 1 + PlanRange(room->w * 2 - 2);
        u32 y = room->y * 2 + 1 + PlanRange(room->h * 2 - 2);
        bool32 taken = (x == sPlan.ladderX && y == sPlan.ladderY)
                    || (x >= START_X - 2 && x <= START_X + 1 && y >= START_Y - 2);

        for (i = 0; i < sPlan.numItems; i++)
            if (abs((s32)x - sPlan.itemX[i]) + abs((s32)y - sPlan.itemY[i]) < 2)
                taken = TRUE;
        if (!taken)
        {
            *outX = x;
            *outY = y;
            return TRUE;
        }
    }
    return FALSE;
}

static void Plan(void)
{
    struct Room room;
    u32 i, tries, want, far, farDist;

    memset(&sPlan, 0, sizeof(sPlan));
    sPlan.rng = sSave.dungeonSeed;
    sPlan.floor = max(sSave.dungeonFloor, 1);
    sPlan.ladderX = sPlan.ladderY = 0xFF;

    // The start room, with the way out at its bottom.
    room.x = START_X / 2 - 1;
    room.y = CELLS - 4;
    room.w = 2;
    room.h = 3;
    sPlan.rooms[sPlan.numRooms++] = room;
    Dig(room.x, room.y, room.x + room.w - 1, room.y + room.h - 1);

    want = min(4 + sPlan.floor / 3, MAX_ROOMS);
    for (tries = 0; tries < 200 && sPlan.numRooms < want; tries++)
    {
        room.w = 2 + PlanRange(3);
        room.h = 2 + PlanRange(3);
        room.x = 1 + PlanRange(CELLS - 2 - room.w);
        room.y = 1 + PlanRange(CELLS - 2 - room.h);
        if (!RoomFits(&room))
            continue;
        Dig(room.x, room.y, room.x + room.w - 1, room.y + room.h - 1);
        // Join it to the room before, so every room is reachable.
        Tunnel(&sPlan.rooms[sPlan.numRooms - 1], &room);
        sPlan.rooms[sPlan.numRooms++] = room;
    }
    // A loop or two, so the floor isn't one long corridor.
    if (sPlan.numRooms > 3)
        Tunnel(&sPlan.rooms[0], &sPlan.rooms[2 + PlanRange(sPlan.numRooms - 2)]);

    // The ladder goes in the room farthest from the start.
    far = 0;
    farDist = 0;
    for (i = 1; i < sPlan.numRooms; i++)
    {
        u32 d = abs((s32)sPlan.rooms[i].x - (s32)sPlan.rooms[0].x) + abs((s32)sPlan.rooms[i].y - (s32)sPlan.rooms[0].y);
        if (d > farDist)
        {
            farDist = d;
            far = i;
        }
    }
    PickSpot(&sPlan.rooms[far], &sPlan.ladderX, &sPlan.ladderY);

    // Item balls: two near the top, one more every third floor.
    want = min(2 + sPlan.floor / 3, MAX_ITEMS);
    for (i = 0; i < want * 4 && sPlan.numItems < want; i++)
    {
        const struct Room *r = &sPlan.rooms[1 + PlanRange(sPlan.numRooms - 1)];
        u32 n = sPlan.numItems;
        if (PickSpot(r, &sPlan.itemX[n], &sPlan.itemY[n]))
        {
            sPlan.items[n] = PickItem(n);
            sPlan.numItems++;
        }
    }
}

static bool32 IsSpotTaken(s32 x, s32 y)
{
    u32 i;

    if (x == sPlan.ladderX && y == sPlan.ladderY)
        return TRUE;
    for (i = 0; i < sPlan.numItems; i++)
        if (x == sPlan.itemX[i] && y == sPlan.itemY[i])
            return TRUE;
    return FALSE;
}

// The wall piece for a rock metatile, from where the open cave is around it.
static u16 WallAt(s32 x, s32 y)
{
    bool32 up = IsFloorAt(x, y - 1), down = IsFloorAt(x, y + 1);
    bool32 left = IsFloorAt(x - 1, y), right = IsFloorAt(x + 1, y);

    if (down)
        return left ? WALL(0x218) : right ? WALL(0x21A) : WALL(0x219);
    if (up)
        return left ? WALL(0x220) : right ? WALL(0x222) : WALL(0x223);
    if (right)
        return WALL(0x212);
    if (left)
        return WALL(0x210);
    if (IsFloorAt(x + 1, y + 1))
        return WALL(0x21B);
    if (IsFloorAt(x - 1, y + 1))
        return WALL(0x21C);
    if (IsFloorAt(x + 1, y - 1) || IsFloorAt(x - 1, y - 1))
        return WALL(0x223);
    return SOLID;
}

static void Draw(void)
{
    s32 x, y;
    u32 i;

    for (y = 0; y < MAP_SIZE; y++)
    {
        for (x = 0; x < MAP_SIZE; x++)
        {
            u16 metatile;
            if (!IsFloorAt(x, y))
                metatile = WallAt(x, y);
            else if (PlanRange(6) == 0)
                metatile = FLOOR_ROCKY;
            else
                metatile = FLOOR;
            MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, metatile);
        }
    }

    // A few boulders, never next to each other, so no tunnel gets blocked.
    for (i = 0; i < 6; i++)
    {
        const struct Room *r = &sPlan.rooms[1 + PlanRange(sPlan.numRooms - 1)];
        if (r->w >= 3 && r->h >= 3)
        {
            x = r->x * 2 + 1 + PlanRange(r->w * 2 - 2);
            y = r->y * 2 + 1 + PlanRange(r->h * 2 - 2);
            if ((x + y) % 2 == 0 && !IsSpotTaken(x, y))
                MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, BOULDER);
        }
    }

    if (sPlan.ladderX != 0xFF)
        MapGridSetMetatileEntryAt(sPlan.ladderX + MAP_OFFSET, sPlan.ladderY + MAP_OFFSET, LADDER);
    MapGridSetMetatileEntryAt(START_X + MAP_OFFSET, EXIT_Y + MAP_OFFSET, EXIT_STEP);
    MapGridSetMetatileEntryAt(START_X - 1 + MAP_OFFSET, EXIT_Y + 1 + MAP_OFFSET, EXIT_LEFT);
    MapGridSetMetatileEntryAt(START_X + MAP_OFFSET, EXIT_Y + 1 + MAP_OFFSET, EXIT_MID);
    MapGridSetMetatileEntryAt(START_X + 1 + MAP_OFFSET, EXIT_Y + 1 + MAP_OFFSET, EXIT_RIGHT);
}

static bool32 InDungeon(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ADVENTURE_DUNGEON)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ADVENTURE_DUNGEON);
}

// Map script, ON_TRANSITION: puts the floor's item balls in place.
void Dungeon_SetupObjects(void)
{
    u32 i;

    Plan();
    for (i = 0; i < MAX_ITEMS; i++)
    {
        if (i < sPlan.numItems)
            SetObjEventTemplateCoords(i + 1, sPlan.itemX[i], sPlan.itemY[i]);
        else
            FlagSet(FLAG_TEMP_11 + i);
    }
}

// Map script, ON_LOAD and ON_RESUME: digs out the floor.
void Dungeon_DrawMap(void)
{
    Plan();
    Draw();
}

// Script (the guide on Route 114): starts a new dive at B1F.
void Dungeon_Enter(void)
{
    sSave.dungeonFloor = 1;
    sSave.dungeonSeed = Random32();
    sSave.dungeonRuns++;
}

// Script: the deepest floor so far, in STR_VAR_1 and VAR_RESULT.
void Dungeon_GetBest(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sSave.dungeonBest, STR_CONV_MODE_LEFT_ALIGN, 3);
    gSpecialVar_Result = sSave.dungeonBest;
}

// Script: the next floor down, in STR_VAR_1.
void Dungeon_GetNextFloor(void)
{
    ConvertIntToDecimalStringN(gStringVar1, min(sSave.dungeonFloor + 1, DUNGEON_MAX_FLOOR), STR_CONV_MODE_LEFT_ALIGN, 3);
}

// Script: down the ladder to a new floor.
void Dungeon_Descend(void)
{
    if (sSave.dungeonFloor < DUNGEON_MAX_FLOOR)
        sSave.dungeonFloor++;
    sSave.dungeonSeed = Random32();
    if (sSave.dungeonFloor > sSave.dungeonBest)
    {
        sSave.dungeonBest = sSave.dungeonFloor;
        if (Netplay_IsOnline())
            World_SubmitScore(WORLD_BOARD_DUNGEON, sSave.dungeonBest, sSave.dungeonRuns);
    }
    TrainerLevel_AddXp(XP_PER_FLOOR);
}

// Script, item balls: VAR_0x8000 = the item, VAR_0x8001 = how many.
void Dungeon_GetItem(void)
{
    u32 slot = gSpecialVar_LastTalked - 1;

    Plan();
    gSpecialVar_0x8000 = slot < sPlan.numItems ? sPlan.items[slot] : ITEM_POTION;
    gSpecialVar_0x8001 = 1;
}

// The A button facing the ladder.
bool32 Dungeon_TryInteract(s16 x, s16 y)
{
    if (!InDungeon())
        return FALSE;
    Plan();
    if (x - MAP_OFFSET != sPlan.ladderX || y - MAP_OFFSET != sPlan.ladderY)
        return FALSE;
    ScriptContext_SetupScript(AdventureDungeon_EventScript_Ladder);
    return TRUE;
}

// Wild Pokémon: who lives on each band of floors. The map's own table gives
// the slot odds and B1F's levels.
static const u16 sWildShallow[] = {
    SPECIES_ZUBAT, SPECIES_GEODUDE, SPECIES_ARON, SPECIES_WHISMUR,
    SPECIES_MAKUHITA, SPECIES_NOSEPASS, SPECIES_SABLEYE, SPECIES_MAWILE,
};
static const u16 sWildMiddle[] = {
    SPECIES_GOLBAT, SPECIES_GRAVELER, SPECIES_LAIRON, SPECIES_LOUDRED,
    SPECIES_SABLEYE, SPECIES_MAWILE, SPECIES_LUNATONE, SPECIES_SOLROCK,
    SPECIES_BALTOY, SPECIES_ONIX, SPECIES_SNORUNT, SPECIES_TRAPINCH,
};
static const u16 sWildDeep[] = {
    SPECIES_GOLBAT, SPECIES_GRAVELER, SPECIES_LAIRON, SPECIES_CLAYDOL,
    SPECIES_BAGON, SPECIES_BELDUM, SPECIES_ONIX, SPECIES_GLALIE,
    SPECIES_HARIYAMA, SPECIES_EXPLOUD, SPECIES_SHELGON, SPECIES_GIBLE,
};
// From B15F a few strong ones turn up.
static const u16 sWildAbyss[] = {
    SPECIES_CROBAT, SPECIES_GOLEM, SPECIES_AGGRON, SPECIES_STEELIX,
    SPECIES_SHELGON, SPECIES_METANG, SPECIES_GABITE, SPECIES_CLAYDOL,
    SPECIES_FLYGON, SPECIES_EXPLOUD, SPECIES_HARIYAMA, SPECIES_DUSCLOPS,
};

enum Species Dungeon_ModifyWildSpecies(enum Species species)
{
    u32 floor = sSave.dungeonFloor;

    if (!InDungeon())
        return species;
    if (floor >= 15)
        return sWildAbyss[Random() % ARRAY_COUNT(sWildAbyss)];
    if (floor >= 8)
        return sWildDeep[Random() % ARRAY_COUNT(sWildDeep)];
    if (floor >= 4)
        return sWildMiddle[Random() % ARRAY_COUNT(sWildMiddle)];
    return sWildShallow[Random() % ARRAY_COUNT(sWildShallow)];
}

// Two levels more per floor, up to 75.
u8 Dungeon_ModifyWildLevel(u8 level)
{
    if (!InDungeon() || sSave.dungeonFloor <= 1)
        return level;
    return min(level + (sSave.dungeonFloor - 1) * 2, 75);
}

// Screenshots (src/adventure.c): the next floor drawn is this one, always
// the same cave; says where its ladder is.
void Dungeon_Showcase(u32 floor, u8 *ladderX, u8 *ladderY)
{
    sSave.dungeonFloor = floor;
    sSave.dungeonSeed = floor * 7919 + 1;
    if (floor > sSave.dungeonBest)
        sSave.dungeonBest = floor;
    Plan();
    *ladderX = sPlan.ladderX;
    *ladderY = sPlan.ladderY;
}
