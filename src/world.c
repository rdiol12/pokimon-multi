#include "global.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "script.h"
#include "main.h"
#include "overworld.h"
#include "rtc.h"
#include "world.h"

// Entry points the overworld calls for the world features, and the showcase
// scenes the screenshot driver uses (tools/world_shots/).

// The screenshot driver writes a scene here once the player is in the
// overworld; the next overworld frame sets the clock and warps there.
struct WorldShowcase
{
    u8 pending;
    u8 hour;
    u8 mapGroup;
    u8 mapNum;
    s16 x;
    s16 y;
    u8 scene; // extra setup, see RunShowcase
};

EWRAM_DATA struct WorldShowcase gWorldShowcase = {0};

void World_OnMapTransition(void)
{
    DayNight_UpdateFlags();
    RoamingWeather_ApplyToCurrentMap();
}

static void RunShowcase(void)
{
    gWorldShowcase.pending = FALSE;
    RtcCalcLocalTimeOffset(0, gWorldShowcase.hour, 0, 0);
    SetWarpDestination(gWorldShowcase.mapGroup, gWorldShowcase.mapNum, WARP_ID_NONE, gWorldShowcase.x, gWorldShowcase.y);
    DoWarp();
}

bool32 World_OverworldUpdate(void)
{
    if (gWorldShowcase.pending && !ArePlayerFieldControlsLocked())
    {
        RunShowcase();
        return TRUE;
    }
    RoamingWeather_Update();
    return FALSE;
}

enum Species World_ModifyWildSpecies(enum Species species, u8 level)
{
    return RoamingWeather_ModifyWildSpecies(species, level);
}
