// Changes to the original Hoenn for many players at once (docs/hoenn_multi.md):
// - other players and their Pokémon don't block anyone (HoennMulti_PassesThrough),
// - other players aren't drawn in private or per-player places, or during the
//   player's own cutscenes (HoennMulti_ShowOtherPlayers),
// - friend travel only goes to maps the player has reached (HoennMulti_HasVisitedMap).
#include "global.h"
#include "hoenn_multi.h"
#include "event_object_movement.h"
#include "netplay.h"
#include "online.h"
#include "overworld.h"
#include "region_map.h"
#include "save_ext.h"
#include "script.h"
#include "script_movement.h"
#include "constants/map_types.h"

// Local ids of other players (70-93) and their following Pokémon (94-117).
#define LOCALID_OTHERS_FIRST LOCALID_NETPLAY_PLAYER_START
#define LOCALID_OTHERS_LAST  (LOCALID_NETPLAY_PLAYER_END + NETPLAY_MAX_REMOTES)

STATIC_ASSERT(MAP_GROUPS_COUNT <= HOENN_MULTI_MAP_GROUPS, HoennMultiTooManyMapGroups);

static EWRAM_DATA bool8 sInCutscene = FALSE;

// Places where every player has their own copy (their house, their secret base,
// their run through a gym, the League or a Frontier challenge). Seeing another
// player there looks wrong: they'd stand in your bedroom, walk through gym gates
// that are shut for you, or wander into the Champion's room mid-battle.
static const u16 sPrivateMaps[] =
{
    MAP_INSIDE_OF_TRUCK,
    MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F,
    MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F,
    MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F,
    MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F,
    // Gyms
    MAP_RUSTBORO_CITY_GYM,
    MAP_DEWFORD_TOWN_GYM,
    MAP_MAUVILLE_CITY_GYM,
    MAP_LAVARIDGE_TOWN_GYM_1F,
    MAP_LAVARIDGE_TOWN_GYM_B1F,
    MAP_PETALBURG_CITY_GYM,
    MAP_FORTREE_CITY_GYM,
    MAP_MOSSDEEP_CITY_GYM,
    MAP_SOOTOPOLIS_CITY_GYM_1F,
    MAP_SOOTOPOLIS_CITY_GYM_B1F,
    // The League
    MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM,
    MAP_EVER_GRANDE_CITY_PHOEBES_ROOM,
    MAP_EVER_GRANDE_CITY_GLACIAS_ROOM,
    MAP_EVER_GRANDE_CITY_DRAKES_ROOM,
    MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM,
    MAP_EVER_GRANDE_CITY_HALL1,
    MAP_EVER_GRANDE_CITY_HALL2,
    MAP_EVER_GRANDE_CITY_HALL3,
    MAP_EVER_GRANDE_CITY_HALL4,
    MAP_EVER_GRANDE_CITY_HALL5,
    MAP_EVER_GRANDE_CITY_HALL_OF_FAME,
    // Trick House puzzles
    MAP_ROUTE110_TRICK_HOUSE_CORRIDOR,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE1,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE2,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE3,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE4,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE5,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE6,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE7,
    MAP_ROUTE110_TRICK_HOUSE_PUZZLE8,
    MAP_ROUTE110_TRICK_HOUSE_END,
    // Frontier challenges (the lobbies stay shared)
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_ELEVATOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_PARTNER_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_PRE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_DOME_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PALACE_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_PALACE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_ARENA_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_ARENA_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_FACTORY_PRE_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_FACTORY_BATTLE_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_CORRIDOR,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_THREE_PATH_ROOM,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_NORMAL,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_FINAL,
    MAP_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS,
    MAP_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR,
    MAP_BATTLE_FRONTIER_BATTLE_PYRAMID_TOP,
    MAP_TRAINER_HILL_1F,
    MAP_TRAINER_HILL_2F,
    MAP_TRAINER_HILL_3F,
    MAP_TRAINER_HILL_4F,
    MAP_TRAINER_HILL_ROOF,
    MAP_TRAINER_HILL_ELEVATOR,
};

static bool32 IsSecretBaseMap(u32 mapGroup, u32 mapNum)
{
    return mapGroup == MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1)
        && mapNum >= MAP_NUM(MAP_SECRET_BASE_RED_CAVE1)
        && mapNum <= MAP_NUM(MAP_SECRET_BASE_SHRUB4);
}

static bool32 IsPrivateMap(u32 mapGroup, u32 mapNum)
{
    u32 i;
    u16 map = (mapGroup << 8) | mapNum;

    if (IsSecretBaseMap(mapGroup, mapNum))
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sPrivateMaps); i++)
    {
        if (sPrivateMaps[i] == map)
            return TRUE;
    }
    return FALSE;
}

// A cutscene is a script that moves people around. Once one starts, other
// players stay hidden until the script ends, so nobody stands in the middle
// of the scene or pops in between two of its steps. Their objects also free
// up slots for the scene's own people.
static void UpdateCutscene(void)
{
    if (!ScriptContext_IsEnabled())
        sInCutscene = FALSE;
    else if (!ScriptMovement_IsAllObjectMovementFinished())
        sInCutscene = TRUE;
}

void HoennMulti_SetVisited(u8 mapGroup, u8 mapNum)
{
    if (mapGroup < HOENN_MULTI_MAP_GROUPS && mapNum < HOENN_MULTI_MAPS_PER_GROUP)
        gSaveBlockExt.visitedMaps[mapGroup][mapNum / 8] |= 1 << (mapNum % 8);
}

static void MarkVisited(void)
{
    // Hide-and-seek and races take players to their arena and back; that
    // doesn't count as having found the place.
    if (Online_HideIsOn() || Online_RaceIsOn())
        return;
    HoennMulti_SetVisited(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

void HoennMulti_OverworldUpdate(void)
{
    UpdateCutscene();
    MarkVisited();
}

bool32 HoennMulti_ShowOtherPlayers(void)
{
    if (sInCutscene)
        return FALSE;
    return !IsPrivateMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

bool32 HoennMulti_PassesThrough(const struct ObjectEvent *mover, const struct ObjectEvent *other)
{
    return (other->localId >= LOCALID_OTHERS_FIRST && other->localId <= LOCALID_OTHERS_LAST)
        || (mover->localId >= LOCALID_OTHERS_FIRST && mover->localId <= LOCALID_OTHERS_LAST);
}

bool32 HoennMulti_HasVisitedMap(u8 mapGroup, u8 mapNum)
{
    const struct MapHeader *header;

    if (mapGroup < HOENN_MULTI_MAP_GROUPS && mapNum < HOENN_MULTI_MAPS_PER_GROUP
     && (gSaveBlockExt.visitedMaps[mapGroup][mapNum / 8] & (1 << (mapNum % 8))))
        return TRUE;
    // Saves from before this existed: towns and cities the player can fly to.
    header = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    return RegionMap_CanFlyToMapsec(header->regionMapSectionId);
}
