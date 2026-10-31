#include "global.h"
#include "event_data.h"
#include "rtc.h"
#include "world.h"
#include "constants/flags.h"
#include "constants/map_types.h"
#include "constants/species.h"
#include "fieldmap.h"
#include "wild_encounter.h"
#include "pokemon.h"
#include "overworld.h"
#include "netplay.h"
#include "world_mailbox.h"
#include "palette.h"
#include "tilesets.h"

// Day/night: night encounter tables live in src/data/wild_encounters.json
// (the *_Night tables), and the overworld tint is the expansion's DNS. This
// file shows and hides the visitors that only come out at night, and says
// which wild Pokémon are awake at this hour.

static const u16 sNightOnlyObjectFlags[] =
{
    FLAG_HIDE_OLDALE_NIGHT_STORYTELLER,
    FLAG_HIDE_FALLARBOR_NIGHT_STARGAZER,
    FLAG_HIDE_LILYCOVE_NIGHT_MARKET,
};

void DayNight_UpdateFlags(void)
{
    bool32 night = (GetTimeOfDay() == TIME_NIGHT);
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sNightOnlyObjectFlags); i++)
    {
        if (night)
            FlagClear(sNightOnlyObjectFlags[i]);
        else
            FlagSet(sNightOnlyObjectFlags[i]);
    }
}

// Which Pokémon come out when. Every wild Pokémon is out by day, by night,
// or at both times (everything not listed below). Morning and day count as
// day, night as night, and evening is twilight: both kinds are out. The
// tags only apply outdoors (towns, cities, routes, the sea); caves,
// buildings and the sea floor are as dark at noon as at midnight. The
// chosen slot is rerolled when its Pokémon is asleep at this hour (see
// DayNight_PickSlot), so a table never runs empty: if nobody in it is
// awake, the first roll stands.

static const u16 sNightSpecies[] =
{
    SPECIES_HOOTHOOT, SPECIES_NOCTOWL, SPECIES_ROWLET,
    SPECIES_SPINARAK, SPECIES_ARIADOS, SPECIES_VENONAT, SPECIES_VENOMOTH,
    SPECIES_ODDISH, SPECIES_GLOOM, SPECIES_KRICKETOT, SPECIES_KRICKETUNE,
    SPECIES_VOLBEAT, SPECIES_ILLUMISE, SPECIES_CASCOON, SPECIES_DUSTOX,
    SPECIES_MORELULL, SPECIES_SHIINOTIC,
    SPECIES_ZUBAT, SPECIES_GOLBAT, SPECIES_WOOBAT, SPECIES_NOIBAT, SPECIES_NOIVERN,
    SPECIES_GASTLY, SPECIES_HAUNTER, SPECIES_MISDREAVUS, SPECIES_MISMAGIUS,
    SPECIES_DUSKULL, SPECIES_DUSCLOPS, SPECIES_SHUPPET, SPECIES_BANETTE,
    SPECIES_DRIFLOON, SPECIES_DRIFBLIM, SPECIES_LITWICK, SPECIES_PHANTUMP,
    SPECIES_TREVENANT, SPECIES_PUMPKABOO, SPECIES_YAMASK, SPECIES_COFAGRIGUS,
    SPECIES_RUNERIGUS, SPECIES_SINISTEA, SPECIES_POLTEAGEIST, SPECIES_GREAVARD,
    SPECIES_SPIRITOMB, SPECIES_FROSLASS,
    SPECIES_MURKROW, SPECIES_SNEASEL, SPECIES_HOUNDOUR, SPECIES_ZORUA,
    SPECIES_MEOWTH, SPECIES_PERSIAN, SPECIES_CLEFAIRY, SPECIES_LUNATONE,
    SPECIES_CHINCHOU, SPECIES_LANTURN, SPECIES_FINNEON, SPECIES_LUMINEON,
};

static const u16 sDaySpecies[] =
{
    SPECIES_PIDGEY, SPECIES_PIDGEOTTO, SPECIES_SPEAROW, SPECIES_FEAROW,
    SPECIES_TAILLOW, SPECIES_SWELLOW, SPECIES_FLETCHLING, SPECIES_ROOKIDEE,
    SPECIES_RUFFLET, SPECIES_BRAVIARY, SPECIES_VULLABY, SPECIES_TRUMBEAK,
    SPECIES_STARAPTOR, SPECIES_SWABLU, SPECIES_ALTARIA, SPECIES_DODUO,
    SPECIES_DODRIO, SPECIES_NATU, SPECIES_XATU, SPECIES_HAWLUCHA,
    SPECIES_WINGULL, SPECIES_PELIPPER, SPECIES_CRAMORANT,
    SPECIES_CATERPIE, SPECIES_METAPOD, SPECIES_WEEDLE, SPECIES_KAKUNA,
    SPECIES_LEDYBA, SPECIES_COMBEE, SPECIES_SILCOON, SPECIES_SEWADDLE,
    SPECIES_SWADLOON, SPECIES_YANMA, SPECIES_YANMEGA, SPECIES_SCYTHER,
    SPECIES_HOPPIP, SPECIES_SUNKERN, SPECIES_BOUNSWEET, SPECIES_STEENEE,
    SPECIES_TROPIUS, SPECIES_SOLROCK, SPECIES_LARVESTA,
    SPECIES_SENTRET, SPECIES_PONYTA, SPECIES_RAPIDASH, SPECIES_TAUROS,
    SPECIES_MILTANK, SPECIES_SKIDDO, SPECIES_MAREEP, SPECIES_LITLEO,
    SPECIES_GROWLITHE, SPECIES_SNUBBULL, SPECIES_PAWMI, SPECIES_PAWMO,
};

static bool32 InList(const u16 *list, u32 count, u32 species)
{
    u32 i;
    for (i = 0; i < count; i++)
    {
        if (list[i] == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 TagsApplyHere(void)
{
    switch (gMapHeader.mapType)
    {
    case MAP_TYPE_TOWN:
    case MAP_TYPE_CITY:
    case MAP_TYPE_ROUTE:
    case MAP_TYPE_OCEAN_ROUTE:
        // Mt. Pyre's misty slopes keep their ghosts all day.
        return !(gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_MT_PYRE_EXTERIOR)
              && (gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_MT_PYRE_EXTERIOR)
               || gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_MT_PYRE_SUMMIT)));
    default:
        return FALSE;
    }
}

enum DayNightTag DayNight_GetSpeciesTag(u32 species)
{
    species = GET_BASE_SPECIES_ID(species);
    if (InList(sNightSpecies, ARRAY_COUNT(sNightSpecies), species))
        return DAYNIGHT_NIGHT;
    if (InList(sDaySpecies, ARRAY_COUNT(sDaySpecies), species))
        return DAYNIGHT_DAY;
    return DAYNIGHT_BOTH;
}

bool32 DayNight_IsSpeciesOut(u32 species)
{
    enum DayNightTag tag;

    if (!TagsApplyHere())
        return TRUE;
    tag = DayNight_GetSpeciesTag(species);
    switch (GetTimeOfDay())
    {
    case TIME_MORNING:
    case TIME_DAY:
        return tag != DAYNIGHT_NIGHT;
    case TIME_NIGHT:
        return tag != DAYNIGHT_DAY;
    default:
        return TRUE;
    }
}

u32 DayNight_PickSlot(const struct WildPokemon *mons, u32 index, u32 (*choose)(u32 arg), u32 arg)
{
    u32 i;

    for (i = 0; i < 12 && !DayNight_IsSpeciesOut(mons[index].species); i++)
    {
        u32 next = choose(arg);
        if (DayNight_IsSpeciesOut(mons[next].species))
            return next;
    }
    return index;
}

// The server's clock (server/world.py, CLOCK), which the bridge keeps
// ticking in gWorldMailbox. Players online at the same moment get the same
// hour, so they share the same sky, tint and wild Pokemon.
bool32 DayNight_GetServerClock(s32 *hours, s32 *minutes)
{
    u32 minute;

    if (!gWorldMailbox.clockSet || !Netplay_IsOnline())
        return FALSE;
    minute = gWorldMailbox.clockMinute % (24 * 60);
    *hours = minute / 60;
    *minutes = minute % 60;
    return TRUE;
}

// Lit windows. Colours with bit 15 set are blended toward lamplight by the
// time-of-day tint (TimeMixPalettes) instead of darkening. Bit 15 is set on
// the window colours in sDayNightLights in the evening and at night, and
// cleared by day (the morning tint would light them too). The GBA ignores
// bit 15, so nothing else notices.
#define MAX_LIGHTS_PER_TILESET 4

struct DayNightLights
{
    const struct Tileset *tileset;
    u8 colors[MAX_LIGHTS_PER_TILESET]; // palette << 4 | index, 0 = none
};

#include "data/daynight_lights.h"

static void MarkTilesetLights(const struct Tileset *tileset, bool32 lit)
{
    u32 i, j;

    if (tileset == NULL)
        return;
    for (i = 0; i < ARRAY_COUNT(sDayNightLights); i++)
    {
        if (sDayNightLights[i].tileset != tileset)
            continue;
        for (j = 0; j < MAX_LIGHTS_PER_TILESET && sDayNightLights[i].colors[j] != 0; j++)
        {
            u32 color = sDayNightLights[i].colors[j];
            if (lit)
                gPlttBufferUnfaded[color] |= 0x8000;
            else
                gPlttBufferUnfaded[color] &= 0x7FFF;
        }
    }
}

void DayNight_UpdateLights(void)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    bool32 lit;

    if (layout == NULL || layout->isFrlg)
        return;
    lit = MapHasNaturalLight(gMapHeader.mapType)
       && (gTimeOfDay == TIME_EVENING || gTimeOfDay == TIME_NIGHT);
    MarkTilesetLights(layout->primaryTileset, lit);
    MarkTilesetLights(layout->secondaryTileset, lit);
}
