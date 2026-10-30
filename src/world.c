#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "field_screen_effect.h"
#include "script.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "rtc.h"
#include "world.h"

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
    u16 giveItem; // 20 of these go in the bag
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
    if (gWorldShowcase.giveItem != ITEM_NONE)
    {
        AddBagItem(gWorldShowcase.giveItem, 20);
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
    Mount_Update();
    Achievements_Update();
    return Hardcore_Update();
}

bool32 World_TryOpenMenu(void)
{
    ScriptContext_SetupScript(World_EventScript_Menu);
    return TRUE;
}

enum Species World_ModifyWildSpecies(enum Species species, u8 level)
{
    return RoamingWeather_ModifyWildSpecies(species, level);
}

// Back from a wild battle, before the field reloads.
void World_OnWildBattleEnd(void)
{
    struct Pokemon *wild = &gEnemyParty[0];

    if (gBattleOutcome == B_OUTCOME_CAUGHT)
        Achievements_OnCatch(wild);
    Hardcore_OnBattleEnd(TRUE);
}

void World_OnTrainerBattleEnd(void)
{
    Hardcore_OnBattleEnd(FALSE);
}

// Until the online part sends it to the hardcore ladder.
void World_OnHardcoreProgress(void)
{
}
