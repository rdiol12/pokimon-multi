#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "evolution_scene.h"
#include "item.h"
#include "main.h"
#include "mechanics.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "script.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/species.h"

// Evolution quests: Pokémon that only evolve when traded (Kadabra,
// Machoke, Onix holding a Metal Coat, Karrablast and Shelmet…) can evolve
// without a trade. The sage in a Mauville City house sets the steps: hold
// the right item if there is one, a strong bond (friendship), and a trial
// battle won with the Pokémon in the lead and still standing at the end.

static EWRAM_DATA u8 sSlot = 0;

// The trade evolution of `species`, if any; *item = the item it has to hold.
static u16 GetTradeEvolution(u16 species, u16 *item)
{
    const struct Evolution *evos = GetSpeciesEvolutions(species);
    u32 i, j;

    *item = ITEM_NONE;
    if (evos == NULL)
        return SPECIES_NONE;
    for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
    {
        if (evos[i].method != EVO_TRADE)
            continue;
        for (j = 0; evos[i].params != NULL && evos[i].params[j].condition != CONDITIONS_END; j++)
        {
            if (evos[i].params[j].condition == IF_HOLD_ITEM)
                *item = evos[i].params[j].arg1;
        }
        return evos[i].targetSpecies;
    }
    return SPECIES_NONE;
}

// The trade partner Karrablast and Shelmet need; NULL for everyone else.
static struct Pokemon *GetTradePartner(struct Pokemon *partner, u16 species)
{
    if (species == SPECIES_KARRABLAST)
        CreateMon(partner, SPECIES_SHELMET, 5, 0, OTID_STRUCT_PRESET(0));
    else if (species == SPECIES_SHELMET)
        CreateMon(partner, SPECIES_KARRABLAST, 5, 0, OTID_STRUCT_PRESET(0));
    else
        return NULL;
    return partner;
}

// Script: VAR_0x8004 = the party slot chosen. VAR_RESULT = EVO_QUEST_*;
// gStringVar1 = its nickname, gStringVar2 = the item it needs to hold,
// gStringVar3 = what it will become.
void Mechanics_CheckEvoQuest(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u16 item, target;

    sSlot = gSpecialVar_0x8004;
    gSpecialVar_Result = EVO_QUEST_NONE;
    GetMonNickname(mon, gStringVar1);
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return;
    target = GetTradeEvolution(species, &item);
    if (target == SPECIES_NONE)
        return;
    StringCopy(gStringVar3, GetSpeciesName(target));
    if (item != ITEM_NONE && GetMonData(mon, MON_DATA_HELD_ITEM) != item)
    {
        CopyItemName(item, gStringVar2);
        gSpecialVar_Result = EVO_QUEST_NEEDS_ITEM;
    }
    else if (GetMonData(mon, MON_DATA_FRIENDSHIP) < EVO_QUEST_FRIENDSHIP)
    {
        gSpecialVar_Result = EVO_QUEST_NEEDS_BOND;
    }
    else
    {
        gSpecialVar_Result = EVO_QUEST_READY;
    }
}

// Script: the trial. Puts the Pokémon in the lead and sets up a battle
// against two Pokémon at its level. Follow with Mechanics_StartFacilityBattle.
void Mechanics_SetupEvoTrial(void)
{
    u8 level = GetMonData(&gParties[B_TRAINER_PLAYER][sSlot], MON_DATA_LEVEL);
    u16 trainerId = Mechanics_PickFacilityTrainer(level);

    if (sSlot != 0)
    {
        struct Pokemon lead = gParties[B_TRAINER_PLAYER][0];

        gParties[B_TRAINER_PLAYER][0] = gParties[B_TRAINER_PLAYER][sSlot];
        gParties[B_TRAINER_PLAYER][sSlot] = lead;
        sSlot = 0;
    }
    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    Mechanics_FillFacilityParty(trainerId, 2, level);
}

// Script: after the trial. VAR_RESULT = TRUE if it was won with the
// Pokémon still standing.
void Mechanics_EvoTrialResult(void)
{
    gSpecialVar_Result = gBattleOutcome == B_OUTCOME_WON
                      && GetMonData(&gParties[B_TRAINER_PLAYER][sSlot], MON_DATA_HP) != 0;
}

// Script (callnative, then waitstate): the evolution.
void Mechanics_EvolveForQuest(struct ScriptContext *ctx)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][sSlot];
    struct Pokemon partner;
    struct Pokemon *tradePartner = GetTradePartner(&partner, GetMonData(mon, MON_DATA_SPECIES));
    bool32 canStopEvo = FALSE;
    u16 target = GetEvolutionTargetSpecies(mon, EVO_MODE_TRADE, ITEM_NONE, tradePartner, &canStopEvo, CHECK_EVO);

    if (target == SPECIES_NONE)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    ctx->waitAfterCallNative = TRUE;
    gSpecialVar_Result = TRUE;
    GetEvolutionTargetSpecies(mon, EVO_MODE_TRADE, ITEM_NONE, tradePartner, &canStopEvo, DO_EVO);
    gCB2_AfterEvolution = CB2_ReturnToFieldContinueScript;
    BeginEvolutionScene(mon, target, FALSE, sSlot);
}
