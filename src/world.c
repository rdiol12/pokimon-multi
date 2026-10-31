#include "global.h"
#include "adventure.h"
#include "achievements.h"
#include "battle.h"
#include "berry.h"
#include "event_data.h"
#include "item.h"
#include "field_screen_effect.h"
#include "script.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "rtc.h"
#include "field_weather.h"
#include "constants/weather.h"
#include "world.h"
#include "online.h"

extern const u8 World_EventScript_Menu[];

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
    u8 lead;     // 1: give giveSpecies and make it the party lead
    u8 shiny;
    u16 giveSpecies;
    u8 giveLevel;
    u16 giveItem; // 20 of these go in the bag; a rod is also registered to SELECT
    u8 ripen;     // 1: the Route 102 Oran Berry tree is ready to pick
    u8 storm;     // 1: pick the hour the thunderstorm is over the map, 2: any rain
};

EWRAM_DATA struct WorldShowcase gWorldShowcase = {0};

void World_OnMapTransition(void)
{
    DayNight_UpdateFlags();
    RoamingWeather_ApplyToCurrentMap();
}

static void RunShowcase(void)
{
    u32 hour;

    gWorldShowcase.pending = FALSE;
    if (gWorldShowcase.giveItem != ITEM_NONE)
    {
        AddBagItem(gWorldShowcase.giveItem, 20);
        if (gWorldShowcase.giveItem == ITEM_OLD_ROD || gWorldShowcase.giveItem == ITEM_GOOD_ROD || gWorldShowcase.giveItem == ITEM_SUPER_ROD)
            gSaveBlock1Ptr->registeredItem = gWorldShowcase.giveItem;
        if (GetItemPocket(gWorldShowcase.giveItem) == POCKET_POKE_BALLS)
            gLastThrownBall = gBallToDisplay = gWorldShowcase.giveItem; // R throws it in battle
        gWorldShowcase.giveItem = ITEM_NONE;
    }
    if (gWorldShowcase.giveSpecies != SPECIES_NONE)
    {
        struct Pokemon mon;
        CreateMonWithIVs(&mon, gWorldShowcase.giveSpecies, gWorldShowcase.giveLevel,
            GetMonPersonality(gWorldShowcase.giveSpecies, MON_GENDER_RANDOM, NATURE_RANDOM, RANDOM_UNOWN_LETTER),
            OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
        if (gWorldShowcase.shiny)
        {
            bool32 shiny = TRUE;
            SetMonData(&mon, MON_DATA_IS_SHINY, &shiny);
        }
        if (gWorldShowcase.lead)
        {
            if (gPlayerPartyCount == PARTY_SIZE)
                gPlayerPartyCount--;
            memmove(&gPlayerParty[1], &gPlayerParty[0], sizeof(struct Pokemon) * gPlayerPartyCount);
            gPlayerParty[0] = mon;
            gPlayerPartyCount++;
        }
        else
        {
            GiveCapturedMonToPlayer(&mon);
        }
        CalculatePlayerPartyCount();
        gWorldShowcase.giveSpecies = SPECIES_NONE;
    }
    if (gWorldShowcase.ripen)
    {
        PlantBerryTree(BERRY_TREE_ROUTE_102_ORAN, ItemIdToBerryType(ITEM_ORAN_BERRY), BERRY_STAGE_BERRIES, TRUE);
        gWorldShowcase.ripen = FALSE;
    }
    RtcCalcLocalTimeOffset(0, gWorldShowcase.hour, 0, 0);
    for (hour = 0; gWorldShowcase.storm && hour < 24; hour++)
    {
        enum OverworldWeather weather;
        RtcCalcLocalTimeOffset(0, hour, 0, 0);
        RtcCalcLocalTime();
        weather = RoamingWeather_GetForMap(gWorldShowcase.mapGroup, gWorldShowcase.mapNum);
        if (weather == WEATHER_RAIN_THUNDERSTORM || (gWorldShowcase.storm == 2 && weather == WEATHER_RAIN))
            break;
    }
    gWorldShowcase.storm = 0;
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
    Mount_Update();
    Legends_Update();
    House_Update();
    WorldOnline_Update();
    Achievements_Update();
    Adventure_Update();
    if (Online_OverworldUpdate())
        return TRUE;
    return Hardcore_Update();
}

// A pressed facing (x, y), map coordinates with MAP_OFFSET.
bool32 World_TryInteract(s16 x, s16 y)
{
    return Legends_TryInteract(x, y) || House_TryInteract(x, y);
}

bool32 World_TryOpenMenu(void)
{
    ScriptContext_SetupScript(World_EventScript_Menu);
    return TRUE;
}

enum Species World_ModifyWildSpecies(enum Species species, u8 level)
{
    species = RoamingWeather_ModifyWildSpecies(species, level);
    ShinyChain_OnWildSpecies(species);
    return species;
}

// Back from a wild battle, before the field reloads.
void World_OnWildBattleEnd(void)
{
    // The enemy party may already be cleared; gBattleResults isn't.
    if (gBattleOutcome == B_OUTCOME_CAUGHT)
    {
        Achievements_OnCatch(gBattleResults.caughtMonSpecies, gBattleResults.shinyWildMon);
        WorldOnline_OnCatch();
        Adventure_OnCatch();
        Online_OnCatch();
    }
    ShinyChain_OnWildBattleEnd();
    Records_OnWildBattleEnd();
    Hardcore_OnBattleEnd(TRUE);
}

void World_OnTrainerBattleEnd(void)
{
    Adventure_OnTrainerBattleEnd();
    Hardcore_OnBattleEnd(FALSE);
}

