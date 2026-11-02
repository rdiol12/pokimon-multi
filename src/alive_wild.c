#include "global.h"
#include "alive.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "data.h"
#include "follower_helper.h"
#include "constants/map_types.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "pokemon.h"
#include "random.h"
#include "save_ext.h"
#include "wild_encounter.h"
#include "wild_encounter_ow.h"
#include "world.h"
#include "constants/battle.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"

// Wild Pokémon you can see (docs/alive.md). The expansion's overworld
// encounters (src/wild_encounter_ow.c) put them on the map; this adds
// packs of social species and Pokémon that sleep at night, and keeps the
// old random encounters for the tests and anyone who prefers them.

#define OWE_FIRST_LOCALID (LOCALID_OW_ENCOUNTER_END - OWE_SPAWNS_MAX + 1)

static EWRAM_DATA struct
{
    u16 packSpecies;   // the leader of a pack still filling up
    u8 packLevel;
    u8 packLeft;       // more to come
    s16 packX, packY;  // the leader's tile
    u8 packTileWater;  // the pack is on water
    u8 packNow;        // the spawn being made now joins the pack
    u8 asleep[OWE_SPAWNS_MAX];
} sWild = {0};

// Species that live in groups; they turn up two or three at a time.
static const u16 sPackSpecies[] =
{
    SPECIES_POOCHYENA, SPECIES_ZIGZAGOON, SPECIES_WURMPLE, SPECIES_TAILLOW, SPECIES_WINGULL,
    SPECIES_ZUBAT, SPECIES_NINCADA, SPECIES_SURSKIT, SPECIES_LOTAD, SPECIES_SHROOMISH,
    SPECIES_WHISMUR, SPECIES_SKITTY, SPECIES_ODDISH, SPECIES_ARON, SPECIES_SPOINK,
    SPECIES_SWABLU, SPECIES_SPINDA, SPECIES_NUMEL, SPECIES_DODUO, SPECIES_GOLDEEN,
    SPECIES_MAGIKARP, SPECIES_TENTACOOL, SPECIES_CARVANHA, SPECIES_LUVDISC, SPECIES_STARLY,
    SPECIES_BIDOOF, SPECIES_PIDGEY, SPECIES_RATTATA, SPECIES_SENTRET, SPECIES_LEDYBA,
    SPECIES_HOPPIP, SPECIES_PIKIPEK, SPECIES_YUNGOOS, SPECIES_PIDOVE, SPECIES_LILLIPUP,
    SPECIES_ROOKIDEE, SPECIES_SKWOVET, SPECIES_WOOLOO, SPECIES_LECHONK, SPECIES_TAROUNTULA,
    SPECIES_VOLBEAT, SPECIES_ILLUMISE, SPECIES_PLUSLE, SPECIES_MINUN, SPECIES_PSYDUCK,
    SPECIES_GEODUDE, SPECIES_MARILL, SPECIES_AZURILL, SPECIES_SANDSHREW, SPECIES_TRAPINCH,
};

// Out and about at night: they never doze off.
static const u16 sNightOwls[] =
{
    SPECIES_HOOTHOOT, SPECIES_NOCTOWL, SPECIES_ZUBAT, SPECIES_GOLBAT, SPECIES_CROBAT, SPECIES_DUSKULL,
    SPECIES_SHUPPET, SPECIES_SABLEYE, SPECIES_GASTLY, SPECIES_HAUNTER, SPECIES_MURKROW, SPECIES_UMBREON,
    SPECIES_VOLBEAT, SPECIES_ILLUMISE, SPECIES_SPINARAK, SPECIES_ARIADOS, SPECIES_MISDREAVUS,
    SPECIES_SEEDOT, SPECIES_NUZLEAF, SPECIES_POOCHYENA, SPECIES_MIGHTYENA, SPECIES_CHINCHOU,
    SPECIES_LANTURN, SPECIES_LUNATONE, SPECIES_ZIGZAGOON, SPECIES_LINOONE, SPECIES_MEOWTH,
    SPECIES_SNEASEL, SPECIES_HOUNDOUR, SPECIES_KRICKETOT, SPECIES_KRICKETUNE,
};

static bool32 InList(const u16 *list, u32 count, u16 species)
{
    u32 i;
    for (i = 0; i < count; i++)
    {
        if (list[i] == species)
            return TRUE;
    }
    return FALSE;
}

bool32 Alive_ClassicEncounters(void)
{
    // The automated tests count on the old encounters, unless a screenshot
    // scene asked for the living world.
    if (Alive_IsQuiet())
        return TRUE;
    return gSaveBlockExt.alive.classic != 0;
}

void Alive_SetClassicEncounters(bool32 classic)
{
    gSaveBlockExt.alive.classic = classic ? 1 : 0;
}

// --- Packs ---------------------------------------------------------------

static bool32 IsWater(s32 x, s32 y)
{
    return MetatileBehavior_IsWaterWildEncounter(MapGridGetMetatileBehaviorAt(x, y));
}

bool32 Alive_WildTakePackTile(s32 *outX, s32 *outY)
{
    static const s8 sOffsets[][2] =
    {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {2, 0}, {-2, 0}, {0, 2}, {0, -2},
    };
    u32 i, start;

    sWild.packNow = FALSE;
    if (sWild.packLeft == 0)
        return FALSE;
    sWild.packLeft--;
    start = Random() % ARRAY_COUNT(sOffsets);
    for (i = 0; i < ARRAY_COUNT(sOffsets); i++)
    {
        const s8 *o = sOffsets[(start + i) % ARRAY_COUNT(sOffsets)];
        s32 x = sWild.packX + o[0], y = sWild.packY + o[1];
        u32 behavior = MapGridGetMetatileBehaviorAt(x, y);

        if (!AreCoordsInsidePlayerMap(x, y) || MapGridGetCollisionAt(x, y) != 0)
            continue;
        if (GetObjectEventIdByPosition(x, y, 0) != OBJECT_EVENTS_COUNT || Alive_PlayerDistance(x, y) < 2)
            continue;
        if (sWild.packTileWater ? !IsWater(x, y) : !MetatileBehavior_IsLandWildEncounter(behavior))
            continue;
        *outX = x;
        *outY = y;
        sWild.packNow = TRUE;
        return TRUE;
    }
    sWild.packLeft = 0;
    return FALSE;
}

bool32 Alive_WildIsPackSpawn(void)
{
    return sWild.packNow;
}

bool32 Alive_WildCreatePackMon(void)
{
    u32 level = sWild.packLevel;
    // The leader is the strongest.
    if (level > 3 && (Random() & 1))
        level--;
    CreateWildMon(sWild.packSpecies, level);
    return TRUE;
}

// --- Sleeping ------------------------------------------------------------

static s32 SlotOf(struct ObjectEvent *owe)
{
    if (owe == NULL || owe->localId < OWE_FIRST_LOCALID || owe->localId > LOCALID_OW_ENCOUNTER_END)
        return -1;
    return owe->localId - OWE_FIRST_LOCALID;
}

bool32 Alive_WildIsAsleep(struct ObjectEvent *owe)
{
    s32 slot = SlotOf(owe);
    return slot >= 0 && sWild.asleep[slot];
}

static void FallAsleep(struct ObjectEvent *owe)
{
    s32 slot = SlotOf(owe);
    if (slot < 0)
        return;
    sWild.asleep[slot] = TRUE;
    SetTrainerMovementType(owe, MOVEMENT_TYPE_FACE_DOWN);
    ObjectEventTurn(owe, DIR_SOUTH);
    Alive_ShowZzz(owe);
}

static void WakeUp(struct ObjectEvent *owe)
{
    s32 slot = SlotOf(owe);
    if (slot < 0 || !sWild.asleep[slot])
        return;
    sWild.asleep[slot] = FALSE;
    SetTrainerMovementType(owe, OWE_GetMovementTypeFromSpecies(OW_SPECIES(owe)));
    Alive_Emote(owe, FOLLOWER_EMOTION_SURPRISE);
}

void Alive_WildOnSpawned(struct ObjectEvent *owe, s32 x, s32 y)
{
    u16 species = OW_SPECIES(owe);
    s32 slot = SlotOf(owe);
    bool32 joined = sWild.packNow;

    sWild.packNow = FALSE;
    if (slot >= 0)
        sWild.asleep[slot] = FALSE;

    // A new leader: maybe others follow.
    if (!joined && sWild.packLeft == 0 && InList(sPackSpecies, ARRAY_COUNT(sPackSpecies), species)
     && (Random() % 100 < 40 || gAliveShowcase.scene == ALIVE_SHOW_PACK))
    {
        sWild.packSpecies = species;
        sWild.packLevel = owe->trainerRange_berryTreeId & 0x7F;
        sWild.packLeft = 1 + (Random() % 3 == 0);
        sWild.packX = x;
        sWild.packY = y;
        sWild.packTileWater = IsWater(x, y);
        if (gAliveShowcase.scene == ALIVE_SHOW_PACK)
        {
            sWild.packLeft = 2;
            gAliveShowcase.scene = ALIVE_SHOW_NONE;
        }
    }

    // At night most day-loving Pokémon are asleep where they lie. Nocturnal
    // ones (and anything in a cave) are wide awake.
    if (Alive_IsNight() && gMapHeader.mapType != MAP_TYPE_UNDERGROUND && !IsWater(x, y)
     && !InList(sNightOwls, ARRAY_COUNT(sNightOwls), species)
     && DayNight_GetSpeciesTag(species) != DAYNIGHT_NIGHT && Random() % 100 < 60)
        FallAsleep(owe);
}

void Alive_WildOnBattleStart(struct ObjectEvent *owe)
{
    if (Alive_WildIsAsleep(owe))
    {
        u32 status = STATUS1_SLEEP_TURN(2 + Random() % 3);
        SetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_STATUS, &status);
    }
}

void AliveWild_OnMapTransition(void)
{
    memset(&sWild, 0, sizeof(sWild));
}

void AliveWild_Update(void)
{
    u32 i;
    bool32 night = Alive_IsNight();

    for (i = 0; i < OWE_SPAWNS_MAX; i++)
    {
        struct ObjectEvent *owe;
        if (!sWild.asleep[i])
            continue;
        owe = Alive_Obj(OWE_FIRST_LOCALID + i);
        if (owe == NULL)
        {
            sWild.asleep[i] = FALSE;
            continue;
        }
        // Morning came, or someone stepped right up to it.
        if (!night || (Alive_PlayerDistance(owe->currentCoords.x, owe->currentCoords.y) <= 1 && Random() % 240 == 0))
            WakeUp(owe);
        else
            Alive_ShowZzz(owe); // the "Z" goes when the sprite reloads
    }
}
