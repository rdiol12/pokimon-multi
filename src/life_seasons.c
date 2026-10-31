#include "global.h"
#include "event_data.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "life.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "tilesets.h"
#include "world.h"
#include "constants/rgb.h"
#include "constants/weather.h"

// Seasons (docs/life.md): the real-world month picks the season. Trees and
// grass on outdoor Hoenn maps are recoloured (cherry blossom in spring,
// red and gold leaves in autumn, snow in winter), winter brings light snow,
// some wild Pokémon only come out in their season, and Deerling and
// Sawsbuck change form with it, as in the main series.

#define SEASON_SWAP_PERCENT 15

struct SeasonMon
{
    u16 species[2];
    u8 level; // evolves into species[1] at this level (0: never)
};

// The last two of each list are rare (1 in 10 of the season's Pokémon).
static const struct SeasonMon sSpringMons[] =
{
    {{SPECIES_DEERLING_SPRING, SPECIES_SAWSBUCK_SPRING}, 34},
    {{SPECIES_CHERUBI, SPECIES_CHERRIM}, 25},
    {{SPECIES_FLABEBE, SPECIES_FLOETTE}, 19},
    {{SPECIES_COMBEE, SPECIES_NONE}, 0},
    {{SPECIES_BUDEW, SPECIES_NONE}, 0},
    {{SPECIES_CUTIEFLY, SPECIES_RIBOMBEE}, 25},
    {{SPECIES_COMFEY, SPECIES_NONE}, 0},
};

static const struct SeasonMon sSummerMons[] =
{
    {{SPECIES_DEERLING_SUMMER, SPECIES_SAWSBUCK_SUMMER}, 34},
    {{SPECIES_SUNKERN, SPECIES_NONE}, 0},
    {{SPECIES_VOLBEAT, SPECIES_NONE}, 0},
    {{SPECIES_ILLUMISE, SPECIES_NONE}, 0},
    {{SPECIES_DEWPIDER, SPECIES_ARAQUANID}, 22},
    {{SPECIES_HERACROSS, SPECIES_NONE}, 0},
    {{SPECIES_YANMA, SPECIES_NONE}, 0},
};

static const struct SeasonMon sAutumnMons[] =
{
    {{SPECIES_DEERLING_AUTUMN, SPECIES_SAWSBUCK_AUTUMN}, 34},
    {{SPECIES_SEWADDLE, SPECIES_SWADLOON}, 20},
    {{SPECIES_PUMPKABOO, SPECIES_NONE}, 0},
    {{SPECIES_PHANTUMP, SPECIES_NONE}, 0},
    {{SPECIES_FOONGUS, SPECIES_AMOONGUSS}, 39},
    {{SPECIES_APPLIN, SPECIES_NONE}, 0},
    {{SPECIES_ZORUA, SPECIES_ZOROARK}, 30},
};

static const struct SeasonMon sWinterMons[] =
{
    {{SPECIES_DEERLING_WINTER, SPECIES_SAWSBUCK_WINTER}, 34},
    {{SPECIES_SNOVER, SPECIES_ABOMASNOW}, 40},
    {{SPECIES_CUBCHOO, SPECIES_BEARTIC}, 37},
    {{SPECIES_BERGMITE, SPECIES_AVALUGG}, 37},
    {{SPECIES_SNOM, SPECIES_NONE}, 0},
    {{SPECIES_DELIBIRD, SPECIES_NONE}, 0},
    {{SPECIES_FRIGIBAX, SPECIES_ARCTIBAX}, 35},
};

static const struct
{
    const struct SeasonMon *mons;
    u8 count;
    const u8 *name;
    const u8 *toast;
} sSeasons[SEASON_COUNT] =
{
    [SEASON_SPRING] = {sSpringMons, ARRAY_COUNT(sSpringMons), COMPOUND_STRING("Spring"), COMPOUND_STRING("Blossoms on the trees!")},
    [SEASON_SUMMER] = {sSummerMons, ARRAY_COUNT(sSummerMons), COMPOUND_STRING("Summer"), COMPOUND_STRING("Bug Pokémon are buzzing.")},
    [SEASON_AUTUMN] = {sAutumnMons, ARRAY_COUNT(sAutumnMons), COMPOUND_STRING("Autumn"), COMPOUND_STRING("The leaves are turning.")},
    [SEASON_WINTER] = {sWinterMons, ARRAY_COUNT(sWinterMons), COMPOUND_STRING("Winter"), COMPOUND_STRING("Snow is falling in Hoenn!")},
};

// The general tileset's palette 2 holds the tree greens (colours 1-4) and
// the grass teals (12-15). Summer keeps Emerald's own colours.
#define TREE_PAL   2
static const u16 sTreeColors[SEASON_COUNT][4] =
{
    [SEASON_SPRING] = {RGB2GBA(255, 222, 238), RGB2GBA(246, 172, 205), RGB2GBA(213, 115, 164), RGB2GBA(115, 57, 82)},
    [SEASON_AUTUMN] = {RGB2GBA(255, 213, 106), RGB2GBA(238, 148, 57), RGB2GBA(197, 82, 41), RGB2GBA(98, 41, 24)},
    [SEASON_WINTER] = {RGB2GBA(255, 255, 255), RGB2GBA(213, 230, 238), RGB2GBA(98, 139, 123), RGB2GBA(49, 74, 57)},
};
static const u16 sGrassColors[SEASON_COUNT][4] =
{
    [SEASON_SPRING] = {RGB2GBA(172, 222, 189), RGB2GBA(123, 205, 156), RGB2GBA(74, 189, 123), RGB2GBA(32, 164, 98)},
    [SEASON_AUTUMN] = {RGB2GBA(213, 213, 156), RGB2GBA(189, 189, 115), RGB2GBA(164, 164, 82), RGB2GBA(123, 131, 57)},
    [SEASON_WINTER] = {RGB2GBA(246, 246, 255), RGB2GBA(222, 230, 246), RGB2GBA(189, 205, 230), RGB2GBA(148, 172, 205)},
};

static EWRAM_DATA u16 sToastDelay = 0;

u32 Seasons_Get(void)
{
    enum Month month;

    if (gLifeShowcase.season != 0)
        return gLifeShowcase.season - 1;
    month = GetMonth();
    if (month >= MONTH_MAR && month <= MONTH_MAY)
        return SEASON_SPRING;
    if (month >= MONTH_JUN && month <= MONTH_AUG)
        return SEASON_SUMMER;
    if (month >= MONTH_SEP && month <= MONTH_NOV)
        return SEASON_AUTUMN;
    return SEASON_WINTER;
}

const u8 *Seasons_GetName(void)
{
    return sSeasons[Seasons_Get()].name;
}

static bool32 IsSeasonalMap(void)
{
    return IsMapTypeOutdoors(gMapHeader.mapType)
        && gMapHeader.mapLayout != NULL
        && gMapHeader.mapLayout->primaryTileset == &gTileset_General;
}

// After the primary tileset's palettes are loaded (fieldmap.c).
void Seasons_TintPrimaryPalettes(bool32 skipFaded)
{
    u32 season;
    u32 i, offset = BG_PLTT_ID(TREE_PAL);

    if (!IsSeasonalMap())
        return;
    season = Seasons_Get();
    if (season == SEASON_SUMMER)
        return;
    for (i = 0; i < 4; i++)
    {
        gPlttBufferUnfaded[offset + 1 + i] = sTreeColors[season][i];
        gPlttBufferUnfaded[offset + 12 + i] = sGrassColors[season][i];
        if (!skipFaded)
        {
            gPlttBufferFaded[offset + 1 + i] = sTreeColors[season][i];
            gPlttBufferFaded[offset + 12 + i] = sGrassColors[season][i];
        }
    }
}

// Maps with clear skies get light snow in winter; story and special
// weather (ash, sand, fog, rain fronts) stays.
static bool32 ShouldSnow(void)
{
    if (Seasons_Get() != SEASON_WINTER || !IsMapTypeOutdoors(gMapHeader.mapType))
        return FALSE;
    switch (GetSavedWeather())
    {
    case WEATHER_NONE:
    case WEATHER_SUNNY:
    case WEATHER_SUNNY_CLOUDS:
        return TRUE;
    }
    return FALSE;
}

// Deerling and Sawsbuck in the party take the season's form.
static void UpdateSeasonForms(void)
{
    static const u16 sDeerling[SEASON_COUNT] = {SPECIES_DEERLING_SPRING, SPECIES_DEERLING_SUMMER, SPECIES_DEERLING_AUTUMN, SPECIES_DEERLING_WINTER};
    static const u16 sSawsbuck[SEASON_COUNT] = {SPECIES_SAWSBUCK_SPRING, SPECIES_SAWSBUCK_SUMMER, SPECIES_SAWSBUCK_AUTUMN, SPECIES_SAWSBUCK_WINTER};
    u32 season = Seasons_Get();
    u32 i, j;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        for (j = 0; j < SEASON_COUNT; j++)
        {
            u16 target = SPECIES_NONE;
            if (species == sDeerling[j])
                target = sDeerling[season];
            else if (species == sSawsbuck[j])
                target = sSawsbuck[season];
            if (target != SPECIES_NONE && target != species)
            {
                SetMonData(mon, MON_DATA_SPECIES, &target);
                CalculateMonStats(mon);
            }
        }
    }
}

void Seasons_OnMapTransition(void)
{
    u32 season = Seasons_Get();

    if (ShouldSnow())
        SetSavedWeather(WEATHER_SNOW);
    if (!IsMapTypeOutdoors(gMapHeader.mapType))
        return;
    // The first time the player steps outside in a new season.
    if (gSaveBlock3Ptr->life.season != season + 1)
    {
        if (gSaveBlock3Ptr->life.season != 0)
            sToastDelay = 150; // after the map name pop-up
        gSaveBlock3Ptr->life.season = season + 1;
        UpdateSeasonForms();
    }
}

// Keeps the winter snow when the hourly weather update clears the sky, and
// shows the new season's notice.
void Seasons_Update(void)
{
    if (sToastDelay != 0 && --sToastDelay == 0)
        World_ShowToast(sSeasons[Seasons_Get()].name, sSeasons[Seasons_Get()].toast);
    if (gMain.vblankCounter1 % 64 == 32 && ShouldSnow())
    {
        SetSavedWeather(WEATHER_SNOW);
        SetNextWeather(WEATHER_SNOW);
    }
}

enum Species Seasons_ModifyWildSpecies(enum Species species, u8 level)
{
    const struct SeasonMon *mons, *mon;
    u32 count, roll;

    if (!IsMapTypeOutdoors(gMapHeader.mapType) || Random() % 100 >= SEASON_SWAP_PERCENT)
        return species;
    mons = sSeasons[Seasons_Get()].mons;
    count = sSeasons[Seasons_Get()].count;
    roll = Random() % 10;
    if (roll == 0)
        mon = &mons[count - 2 + (Random() & 1)];
    else
        mon = &mons[Random() % (count - 2)];
    if (mon->level != 0 && level >= mon->level)
        return mon->species[1];
    return mon->species[0];
}
