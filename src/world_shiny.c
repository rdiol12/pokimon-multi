#include "global.h"
#include "achievements.h"
#include "battle.h"
#include "pokemon.h"
#include "string_util.h"
#include "world.h"
#include "constants/battle.h"

// Shiny chaining (the Let's Go catch combo): defeating or catching the same
// species in a row builds a chain, and the next wild Pokémon of that species
// gets extra shiny rolls (+4 from 11, +8 from 21, +12 from 31, on top of the
// Shiny Charm). Meeting another species, or running, breaks the chain. The
// chain is saved, and a notice shows it after each battle.

static EWRAM_DATA bool8 sNextWildIsChained = FALSE;

// Called with the species about to be created for a wild encounter.
void ShinyChain_OnWildSpecies(enum Species species)
{
    sNextWildIsChained = (species == gSaveBlock3Ptr->world.chainSpecies);
}

// Extra shiny rolls for the Pokémon being created (pokemon.c).
u32 ShinyChain_ExtraRolls(void)
{
    if (!sNextWildIsChained)
        return 0;
    sNextWildIsChained = FALSE;
    return ShinyChain_BonusFor(gSaveBlock3Ptr->world.chainCount);
}

void ShinyChain_OnWildBattleEnd(void)
{
    struct WorldSaveData *world = &gSaveBlock3Ptr->world;
    u16 species = gBattleResults.lastOpponentSpecies;

    sNextWildIsChained = FALSE;
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_CAUGHT)
    {
        if (world->chainCount >= 2)
            World_ShowToast(COMPOUND_STRING("CHAIN BROKEN"), COMPOUND_STRING("Running away ends a chain."));
        world->chainCount = 0;
        return;
    }
    if (species == world->chainSpecies && world->chainCount < 9999)
    {
        world->chainCount++;
    }
    else
    {
        world->chainSpecies = species;
        world->chainCount = 1;
    }
    if (world->chainCount >= 20)
        Achievements_Unlock(ACH_CHAIN_20);
    if (world->chainCount >= 2)
    {
        StringCopy(gStringVar1, GetSpeciesName(species));
        ConvertIntToDecimalStringN(gStringVar2, world->chainCount, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringExpandPlaceholders(gStringVar3, COMPOUND_STRING("{STR_VAR_1} CHAIN x{STR_VAR_2}"));
        ConvertIntToDecimalStringN(gStringVar1, ShinyChain_BonusFor(world->chainCount), STR_CONV_MODE_LEFT_ALIGN, 2);
        StringExpandPlaceholders(gStringVar4, COMPOUND_STRING("Extra shiny rolls: +{STR_VAR_1}"));
        World_ShowToast(gStringVar3, gStringVar4);
    }
}

u32 ShinyChain_BonusFor(u32 count)
{
    if (count > 30)
        return 12;
    if (count > 20)
        return 8;
    if (count > 10)
        return 4;
    return 0;
}
