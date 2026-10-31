#include "global.h"
#include "event_data.h"
#include "field_weather.h"
#include "main.h"
#include "overworld.h"
#include "random.h"
#include "rtc.h"
#include "region_map.h"
#include "string_util.h"
#include "world.h"
#include "constants/map_types.h"
#include "constants/maps.h"
#include "constants/weather.h"

// Roaming weather: three weather fronts (a rainstorm, a sandstorm and a snow
// squall) drift along fixed paths across Hoenn, one map per hour of the
// in-game clock. The position only depends on the clock, so everyone who
// plays at the same time finds the fronts in the same places.
//
// The map under a front gets its heavy weather; the rainstorm also brings
// light rain to the maps either side of it. While a front's weather is on
// the map, part of the wild encounters are swapped for Pokémon that come out
// in that weather (see sRainMons etc.).

#define WEATHER_SWAP_PERCENT 35

struct WeatherFront
{
    const u16 *path; // maps, as MAP_GROUP << 8 | MAP_NUM
    u8 length;
    u8 hourOffset;
    u8 weather;
    u8 edgeWeather; // on the maps next to it, or WEATHER_NONE
};

#define M(map) ((MAP_GROUP(map) << 8) | MAP_NUM(map))

static const u16 sRainPath[] =
{
    M(MAP_ROUTE101), M(MAP_OLDALE_TOWN), M(MAP_ROUTE103), M(MAP_ROUTE110),
    M(MAP_MAUVILLE_CITY), M(MAP_ROUTE118), M(MAP_ROUTE119), M(MAP_FORTREE_CITY),
    M(MAP_ROUTE120), M(MAP_ROUTE121), M(MAP_LILYCOVE_CITY),
};

static const u16 sSandPath[] =
{
    M(MAP_ROUTE117), M(MAP_MAUVILLE_CITY), M(MAP_ROUTE111), M(MAP_ROUTE112),
    M(MAP_ROUTE113), M(MAP_FALLARBOR_TOWN), M(MAP_ROUTE114),
};

static const u16 sSnowPath[] =
{
    M(MAP_ROUTE102), M(MAP_PETALBURG_CITY), M(MAP_ROUTE104), M(MAP_RUSTBORO_CITY),
    M(MAP_ROUTE116), M(MAP_ROUTE115),
};

enum { FRONT_RAIN, FRONT_SAND, FRONT_SNOW, FRONT_COUNT };

static const struct WeatherFront sFronts[FRONT_COUNT] =
{
    [FRONT_RAIN] = {sRainPath, ARRAY_COUNT(sRainPath), 0, WEATHER_RAIN_THUNDERSTORM, WEATHER_RAIN},
    [FRONT_SAND] = {sSandPath, ARRAY_COUNT(sSandPath), 5, WEATHER_SANDSTORM, WEATHER_NONE},
    [FRONT_SNOW] = {sSnowPath, ARRAY_COUNT(sSnowPath), 9, WEATHER_SNOW, WEATHER_NONE},
};

// A family is up to three stages: the species and the level it is found at
// from then on (its official evolution level).
struct WeatherMon
{
    u16 species[3];
    u8 level[3];
};

static const struct WeatherMon sRainMons[] =
{
    {{SPECIES_LOTAD, SPECIES_LOMBRE}, {0, 14}},
    {{SPECIES_WINGULL, SPECIES_PELIPPER}, {0, 25}},
    {{SPECIES_PSYDUCK, SPECIES_GOLDUCK}, {0, 33}},
    {{SPECIES_POLIWAG, SPECIES_POLIWHIRL}, {0, 25}},
    {{SPECIES_SURSKIT, SPECIES_MASQUERAIN}, {0, 22}},
    {{SPECIES_MARILL, SPECIES_AZUMARILL}, {0, 18}},
    {{SPECIES_WOOPER, SPECIES_QUAGSIRE}, {0, 20}},
    {{SPECIES_TYMPOLE, SPECIES_PALPITOAD, SPECIES_SEISMITOAD}, {0, 25, 36}},
    {{SPECIES_GOOMY, SPECIES_SLIGGOO}, {0, 40}},
};

static const struct WeatherMon sSandMons[] =
{
    {{SPECIES_SANDSHREW, SPECIES_SANDSLASH}, {0, 22}},
    {{SPECIES_TRAPINCH, SPECIES_VIBRAVA}, {0, 35}},
    {{SPECIES_CACNEA, SPECIES_CACTURNE}, {0, 32}},
    {{SPECIES_BALTOY, SPECIES_CLAYDOL}, {0, 36}},
    {{SPECIES_HIPPOPOTAS, SPECIES_HIPPOWDON}, {0, 34}},
    {{SPECIES_SILICOBRA, SPECIES_SANDACONDA}, {0, 36}},
    {{SPECIES_DWEBBLE, SPECIES_CRUSTLE}, {0, 34}},
    {{SPECIES_GIBLE, SPECIES_GABITE}, {0, 24}},
    {{SPECIES_LARVITAR, SPECIES_PUPITAR}, {0, 30}},
};

static const struct WeatherMon sSnowMons[] =
{
    {{SPECIES_SNORUNT, SPECIES_GLALIE}, {0, 42}},
    {{SPECIES_SNOVER, SPECIES_ABOMASNOW}, {0, 40}},
    {{SPECIES_SWINUB, SPECIES_PILOSWINE}, {0, 33}},
    {{SPECIES_SPHEAL, SPECIES_SEALEO, SPECIES_WALREIN}, {0, 32, 44}},
    {{SPECIES_CUBCHOO, SPECIES_BEARTIC}, {0, 37}},
    {{SPECIES_VANILLITE, SPECIES_VANILLISH, SPECIES_VANILLUXE}, {0, 35, 47}},
    {{SPECIES_SNOM}, {0}},
    {{SPECIES_SNEASEL}, {0}},
    {{SPECIES_VULPIX_ALOLA}, {0}},
};

static EWRAM_DATA s8 sLastHour = 0;

static u32 HoursNow(void)
{
    RtcCalcLocalTime();
    return (u32)gLocalTime.days * 24 + gLocalTime.hours;
}

// Index into the path; the front walks to the end and back.
static u32 FrontPosition(const struct WeatherFront *front, u32 hours)
{
    u32 cycle = front->length * 2 - 2;
    u32 step = (hours + front->hourOffset) % cycle;
    return step < front->length ? step : cycle - step;
}

static enum OverworldWeather WeatherForMapAt(u16 map, u32 hours)
{
    u32 i;
    s32 pos, j;

    for (i = 0; i < FRONT_COUNT; i++)
    {
        const struct WeatherFront *front = &sFronts[i];
        pos = FrontPosition(front, hours);
        if (front->path[pos] == map)
            return front->weather;
        if (front->edgeWeather == WEATHER_NONE)
            continue;
        for (j = pos - 1; j <= pos + 1; j += 2)
        {
            if (j >= 0 && j < front->length && front->path[j] == map)
                return front->edgeWeather;
        }
    }
    return WEATHER_NONE;
}

enum OverworldWeather RoamingWeather_GetForMap(u8 mapGroup, u8 mapNum)
{
    return WeatherForMapAt((mapGroup << 8) | mapNum, HoursNow());
}

static bool32 CanHaveRoamingWeather(void)
{
    if (!IsMapTypeOutdoors(gMapHeader.mapType))
        return FALSE;
    // Story weather (Groudon/Kyogre, the Route 119/123 cycles) wins.
    switch (GetSavedWeather())
    {
    case WEATHER_ABNORMAL:
    case WEATHER_DROUGHT:
    case WEATHER_DOWNPOUR:
    case WEATHER_UNDERWATER:
    case WEATHER_UNDERWATER_BUBBLES:
    case WEATHER_ROUTE119_CYCLE:
    case WEATHER_ROUTE123_CYCLE:
        return FALSE;
    }
    return TRUE;
}

void RoamingWeather_ApplyToCurrentMap(void)
{
    enum OverworldWeather weather;

    sLastHour = gLocalTime.hours;
    if (!CanHaveRoamingWeather())
        return;
    weather = RoamingWeather_GetForMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (weather != WEATHER_NONE)
        SetSavedWeather(weather);
}

// When the hour turns while the player stands on a map, the front moves on
// and the sky changes without a map transition.
void RoamingWeather_Update(void)
{
    enum OverworldWeather weather;

    if (gMain.vblankCounter1 % 64 != 0)
        return;
    HoursNow();
    if (gLocalTime.hours == sLastHour)
        return;
    sLastHour = gLocalTime.hours;
    if (!IsMapTypeOutdoors(gMapHeader.mapType))
        return;
    weather = RoamingWeather_GetForMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (weather == WEATHER_NONE)
        weather = gMapHeader.weather;
    if (weather != GetSavedWeather() && CanHaveRoamingWeather())
    {
        SetSavedWeather(weather);
        SetNextWeather(weather);
    }
}

static const struct WeatherMon *TableForWeather(u8 weather, u32 *count)
{
    switch (weather)
    {
    case WEATHER_RAIN:
    case WEATHER_RAIN_THUNDERSTORM:
    case WEATHER_DOWNPOUR:
        *count = ARRAY_COUNT(sRainMons);
        return sRainMons;
    case WEATHER_SANDSTORM:
        *count = ARRAY_COUNT(sSandMons);
        return sSandMons;
    case WEATHER_SNOW:
        *count = ARRAY_COUNT(sSnowMons);
        return sSnowMons;
    }
    return NULL;
}

enum Species RoamingWeather_ModifyWildSpecies(enum Species species, u8 level)
{
    const struct WeatherMon *table, *mon;
    u32 count, i, roll;

    if (!IsMapTypeOutdoors(gMapHeader.mapType))
        return species;
    table = TableForWeather(GetSavedWeather(), &count);
    if (table == NULL || Random() % 100 >= WEATHER_SWAP_PERCENT)
        return species;

    // The last two families of each table are the rare ones (1 in 20).
    roll = Random() % 100;
    if (roll < 5)
        mon = &table[count - 2 + (roll & 1)];
    else
        mon = &table[Random() % (count - 2)];

    for (i = 2; i > 0; i--)
    {
        if (mon->species[i] != SPECIES_NONE && level >= mon->level[i])
            return mon->species[i];
    }
    return mon->species[0];
}

// Puts the map under the rainstorm, sandstorm and snow squall into
// STR_VAR_1..3, for the forecaster NPCs.
void RoamingWeather_GetForecast(void)
{
    static u8 *const vars[FRONT_COUNT] = {gStringVar1, gStringVar2, gStringVar3};
    u32 hours = HoursNow();
    u32 i;

    for (i = 0; i < FRONT_COUNT; i++)
    {
        const struct WeatherFront *front = &sFronts[i];
        u16 map = front->path[FrontPosition(front, hours)];
        const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF);

        GetMapName(vars[i], header->regionMapSectionId, 0);
    }
}

// The map the front (0 rainstorm, 1 sandstorm, 2 snow squall) is over
// hoursAhead hours from now, as MAP_GROUP << 8 | MAP_NUM.
u16 RoamingWeather_GetFrontMap(u32 front, u32 hoursAhead)
{
    return sFronts[front].path[FrontPosition(&sFronts[front], HoursNow() + hoursAhead)];
}
