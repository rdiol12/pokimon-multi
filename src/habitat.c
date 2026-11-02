#include "global.h"
#include "habitat.h"
#include "fieldmap.h"
#include "pokemon.h"
#include "random.h"
#include "wild_encounter.h"
#include "constants/maps.h"
#include "constants/wild_encounter.h"

// Wild Pokémon stay in their own areas.
//
// Several systems swap a wild encounter for a Pokémon that isn't on the
// map's own table: roaming weather (src/world_weather.c), the seasons
// (src/life_seasons.c) and online migrants (src/online_eco.c). They used to
// do it anywhere, so a Gible or a Comfey could turn up on Route 101. Now a
// visitor needs a home here: it must share a type with a Pokémon that lives
// on this map, and the routes around Littleroot only ever have their own.

static const u16 sStartingArea[] =
{
    MAP_ROUTE101,
    MAP_ROUTE102,
    MAP_ROUTE103,
    MAP_ROUTE104,
    MAP_PETALBURG_WOODS,
};

static const u8 sSlots[] =
{
    NUM_LAND_MONS_ENCOUNTER_SLOTS,
    NUM_WATER_MONS_ENCOUNTER_SLOTS,
    NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS,
    NUM_FISHING_MONS_ENCOUNTER_SLOTS,
};

static u32 HeaderOf(u32 group, u32 num)
{
    u32 i;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (gWildMonHeaders[i].mapGroup == group && gWildMonHeaders[i].mapNum == num)
            return i;
    }
    return HEADER_NONE;
}

static const struct WildPokemonInfo *Info(u32 header, u32 time, u32 kind)
{
    const struct WildEncounterTypes *t = &gWildMonHeaders[header].encounterTypes[time];

    switch (kind)
    {
    case 0: return t->landMonsInfo;
    case 1: return t->waterMonsInfo;
    case 2: return t->rockSmashMonsInfo;
    default: return t->fishingMonsInfo;
    }
}

// Calls `fn` for every species on the header's tables (all times of day);
// stops when it returns TRUE.
static bool32 ForEachNative(u32 header, bool32 (*fn)(enum Species, void *), void *arg)
{
    u32 time, kind, i;

    if (header == HEADER_NONE)
        return FALSE;
    for (time = 0; time < TIMES_OF_DAY_COUNT; time++)
    {
        for (kind = 0; kind < ARRAY_COUNT(sSlots); kind++)
        {
            const struct WildPokemonInfo *info = Info(header, time, kind);
            if (info == NULL || info->wildPokemon == NULL)
                continue;
            for (i = 0; i < sSlots[kind]; i++)
            {
                enum Species species = info->wildPokemon[i].species;
                if (species != SPECIES_NONE && fn(species, arg))
                    return TRUE;
            }
        }
    }
    return FALSE;
}

static u32 CurrentHeader(void)
{
    return HeaderOf(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

static bool32 IsSpecies(enum Species species, void *arg)
{
    return species == *(enum Species *)arg;
}

bool32 Habitat_IsNative(enum Species species)
{
    return ForEachNative(CurrentHeader(), IsSpecies, &species);
}

bool32 Habitat_IsStartingArea(void)
{
    u32 i, here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;

    for (i = 0; i < ARRAY_COUNT(sStartingArea); i++)
    {
        if (sStartingArea[i] == here)
            return TRUE;
    }
    return FALSE;
}

static bool32 SharesType(enum Species native, void *arg)
{
    enum Species visitor = *(enum Species *)arg;
    u32 a, b;

    for (a = 0; a < 2; a++)
    {
        for (b = 0; b < 2; b++)
        {
            if (GetSpeciesType(visitor, a) == GetSpeciesType(native, b))
                return TRUE;
        }
    }
    return FALSE;
}

bool32 Habitat_VisitorFits(enum Species species)
{
    u32 header;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return FALSE;
    header = CurrentHeader();
    if (header == HEADER_NONE)
        return FALSE;
    if (ForEachNative(header, IsSpecies, &species))
        return TRUE;
    if (Habitat_IsStartingArea())
        return FALSE;
    return ForEachNative(header, SharesType, &species);
}

// Picking a native of a type -----------------------------------------------

#define PICK_MAX 16

struct Pick
{
    enum Type type;
    bool8 preferWater;
    u8 count;
    u8 waterCount;
    u16 species[PICK_MAX];
    u16 water[PICK_MAX];
};

static bool32 HasType(enum Species species, enum Type type)
{
    return GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type;
}

static void Add(u16 *list, u8 *count, enum Species species)
{
    u32 i;
    for (i = 0; i < *count; i++)
    {
        if (list[i] == species)
            return;
    }
    if (*count < PICK_MAX)
        list[(*count)++] = species;
}

static void Collect(struct Pick *p, u32 header, bool32 night)
{
    u32 time, i;

    if (header == HEADER_NONE)
        return;
    // Only the land and water tables: these are the ones seen on the map.
    for (time = 0; time < TIMES_OF_DAY_COUNT; time++)
    {
        u32 kind;
        bool32 nightTable = (time == TIME_NIGHT);
        // Tables with no night version are used all day.
        if (nightTable != night && gWildMonHeaders[header].encounterTypes[TIME_NIGHT].landMonsInfo != NULL)
            continue;
        for (kind = 0; kind < 2; kind++)
        {
            const struct WildPokemonInfo *info = Info(header, time, kind);
            if (info == NULL || info->wildPokemon == NULL)
                continue;
            for (i = 0; i < sSlots[kind]; i++)
            {
                enum Species species = info->wildPokemon[i].species;
                if (species == SPECIES_NONE || !HasType(species, p->type))
                    continue;
                Add(p->species, &p->count, species);
                if (HasType(species, TYPE_WATER))
                    Add(p->water, &p->waterCount, species);
            }
        }
    }
}

enum Species Habitat_PickNativeOfType(enum Type type, bool32 night, bool32 preferWater)
{
    struct Pick p = {.type = type, .preferWater = preferWater};

    Collect(&p, CurrentHeader(), night);
    // A town has no table of its own: the routes next to it count.
    if (p.count == 0 && gMapHeader.connections != NULL)
    {
        s32 i;
        for (i = 0; i < gMapHeader.connections->count; i++)
        {
            const struct MapConnection *c = &gMapHeader.connections->connections[i];
            Collect(&p, HeaderOf(c->mapGroup, c->mapNum), night);
        }
    }
    if (preferWater && p.waterCount != 0)
        return p.water[Random() % p.waterCount];
    if (p.count == 0)
        return SPECIES_NONE;
    return p.species[Random() % p.count];
}
