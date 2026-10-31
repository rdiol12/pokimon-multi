#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "field_weather.h"
#include "palette.h"
#include "mechanics.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/field_weather.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "constants/weather.h"

// The postgame storms: after the Hall of Fame, what's left of Groudon's and
// Kyogre's power stirs up a drought on Route 114 and a thunderstorm on Route 118.
// A researcher in the Weather Institute sends the player there; on each
// route an echo of the legendary waits at the heart of the storm. Beat it
// (it can't be caught) and the weather goes back to normal.

struct Storm
{
    u16 species;
    u8 level;
    u8 weather;
    u16 map;
    u16 flag;      // set when it's calmed
    u16 treasure;  // left behind when the echo fades
    const u8 *intro;
};

static const struct Storm sStorms[STORM_COUNT] =
{
    [STORM_GROUDON] =
    {
        SPECIES_GROUDON, 70, WEATHER_DROUGHT, MAP_ROUTE114, FLAG_MECH_STORM_GROUDON, ITEM_HEAT_ROCK,
        COMPOUND_STRING("The ground is baking hot…\n"
                        "GROUDON's echo glares out of the\l"
                        "heat haze!"),
    },
    [STORM_KYOGRE] =
    {
        SPECIES_KYOGRE, 70, WEATHER_RAIN_THUNDERSTORM, MAP_ROUTE118, FLAG_MECH_STORM_KYOGRE, ITEM_DAMP_ROCK,
        COMPOUND_STRING("Thunder rolls over the water…\n"
                        "KYOGRE's echo rises out of the\l"
                        "storm!"),
    },
};

bool32 Mechanics_IsStormActive(u8 storm)
{
    return FlagGet(FLAG_SYS_GAME_CLEAR) && !FlagGet(sStorms[storm].flag);
}

// Mechanics_Update: once the player is on a storm's route, the weather
// turns. Only the current weather changes, never the saved one: a map
// can't start in a drought (Drought_InitAll waits forever on a warp), so
// the storm builds up again each time instead.
void Mechanics_UpdateStorms(void)
{
    u32 i;

    for (i = 0; i < STORM_COUNT; i++)
    {
        if (gSaveBlock1Ptr->location.mapGroup == (sStorms[i].map >> 8)
         && gSaveBlock1Ptr->location.mapNum == (sStorms[i].map & 0xFF)
         && Mechanics_IsStormActive(i) && GetCurrentWeather() != sStorms[i].weather
         // Not while the screen fades in: that fade would never finish.
         && !gPaletteFade.active && gWeatherPtr->palProcessingState == WEATHER_PAL_STATE_IDLE)
            SetNextWeather(sStorms[i].weather);
    }
}

// Script: VAR_0x8004 = STORM_*. Sets up the echo for a wild battle
// (dowildbattle next); gStringVar4 = its intro, VAR_0x8006 = its species.
void Mechanics_SetupStormEcho(void)
{
    const struct Storm *storm = &sStorms[gSpecialVar_0x8004];
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];

    gSpecialVar_0x8006 = storm->species;
    StringCopy(gStringVar4, storm->intro);
    ZeroEnemyPartyMons();
    CreateMonWithIVs(mon, storm->species, storm->level, Random32(), OTID_STRUCT_PRESET(MECH_GUARDIAN_OT_ID), 25);
    GiveMonInitialMoveset(mon);
    Mechanics_PrepareOpponentMon(mon);
}

// Script: after the battle. VAR_RESULT = TRUE if the echo was beaten (the
// storm is calmed; VAR_0x8005 = what it left behind).
void Mechanics_EndStormEcho(void)
{
    const struct Storm *storm = &sStorms[gSpecialVar_0x8004];

    gSpecialVar_Result = (gBattleOutcome == B_OUTCOME_WON);
    gSpecialVar_0x8005 = storm->treasure;
    if (gSpecialVar_Result)
        FlagSet(storm->flag);
}

// Script: VAR_RESULT = how many storms are calmed.
void Mechanics_CountCalmedStorms(void)
{
    u32 i;

    gSpecialVar_Result = 0;
    for (i = 0; i < STORM_COUNT; i++)
    {
        if (FlagGet(sStorms[i].flag))
            gSpecialVar_Result++;
    }
}
