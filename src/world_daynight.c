#include "global.h"
#include "event_data.h"
#include "rtc.h"
#include "world.h"
#include "constants/flags.h"

// Day/night: night encounter tables live in src/data/wild_encounters.json
// (the *_Night tables), and the overworld tint is the expansion's DNS. This
// file shows and hides the visitors that only come out at night.

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
