#include "global.h"
#include "battle.h"
#include "battle_frontier.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle_util.h"
#include "event_data.h"
#include "list_menu.h"
#include "malloc.h"
#include "frontier_util.h"
#include "main.h"
#include "mechanics.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_trainers.h"

// Battles the mechanics set up in C: rule sets, against Battle Frontier
// trainers whose party is made here. They play like Battle Tower battles
// (no EXP, no prize money, no whiting out); the script gets the outcome in
// VAR_RESULT afterwards.

static EWRAM_DATA u8 sRuleSet = RULESET_NONE;
static EWRAM_DATA u8 sSavedLvlMode = 0;

// Rule sets ---------------------------------------------------------------

void Mechanics_SetBattleRules(u8 ruleSet)
{
    Mechanics_ClearBattleRules();
    sRuleSet = ruleSet;
    switch (ruleSet)
    {
    case RULESET_INVERSE:
        FlagSet(FLAG_MECH_RULE_INVERSE);
        break;
    case RULESET_TRICK_ROOM:
        SetStartingStatus(STARTING_STATUS_TRICK_ROOM);
        break;
    case RULESET_NO_ITEMS:
        SetStartingStatus(STARTING_STATUS_MAGIC_ROOM);
        break;
    case RULESET_WONDER_ROOM:
        SetStartingStatus(STARTING_STATUS_WONDER_ROOM);
        break;
    case RULESET_MISTY:
        SetStartingStatus(STARTING_STATUS_MISTY_TERRAIN);
        break;
    }
}

void Mechanics_ClearBattleRules(void)
{
    if (sRuleSet != RULESET_NONE)
        ResetStartingStatuses();
    FlagClear(FLAG_MECH_RULE_INVERSE);
    sRuleSet = RULESET_NONE;
}

bool32 Mechanics_IsBagBanned(void)
{
    return sRuleSet == RULESET_NO_ITEMS;
}

// Facility battles --------------------------------------------------------

// The average level of the player's three strongest Pokémon (counting a
// missing one as level 5), so one strong Pokémon can't carry a weak team.
u8 Mechanics_GetPartyBattleLevel(void)
{
    u8 top[FRONTIER_PARTY_SIZE] = {5, 5, 5};
    u32 i, j, sum = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u8 level;

        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
        for (j = 0; j < FRONTIER_PARTY_SIZE; j++)
        {
            if (level > top[j])
            {
                u8 tmp = top[j];
                top[j] = level;
                level = tmp;
            }
        }
    }
    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        sum += top[i];
    return sum / FRONTIER_PARTY_SIZE;
}

// A Battle Frontier trainer about as strong as a party of this level.
u16 Mechanics_PickFacilityTrainer(u8 level)
{
    s32 id = level * 3 - 40 + (Random() % 40);

    return min(max(id, 0), FRONTIER_TRAINERS_COUNT - 1);
}

// Fills the opponent's party with `count` of the trainer's Battle Frontier
// Pokémon at `level`, no two alike.
void Mechanics_FillFacilityParty(u16 trainerId, u8 count, u8 level)
{
    const u16 *monSet;
    u16 chosen[PARTY_SIZE];
    u32 setSize, i = 0, j, tries = 0;
    u32 otId = Random32();
    u32 data;

    gFacilityTrainers = gBattleFrontierTrainers;
    gFacilityTrainerMons = gBattleFrontierMons;
    monSet = gFacilityTrainers[trainerId].monSet;
    for (setSize = 0; monSet[setSize] != 0xFFFF; setSize++)
        ;
    ZeroEnemyPartyMons();
    while (i < count && tries++ < 500)
    {
        u16 monId = monSet[Random() % setSize];

        for (j = 0; j < i; j++)
        {
            if (gFacilityTrainerMons[chosen[j]].species == gFacilityTrainerMons[monId].species
             || (gFacilityTrainerMons[monId].heldItem != ITEM_NONE
              && gFacilityTrainerMons[chosen[j]].heldItem == gFacilityTrainerMons[monId].heldItem))
                break;
        }
        if (j != i)
            continue;
        chosen[i] = monId;
        CreateFacilityMon(&gFacilityTrainerMons[monId], level, GetFrontierTrainerFixedIvs(trainerId), otId, 0,
                          &gParties[B_TRAINER_OPPONENT_A][i]);
        // Frontier sets don't say whether a Pokémon may Dynamax or
        // Terastallize, and the AI takes "not set" as yes (see netplay.c).
        data = BLOCK_AI_DYNAMAX;
        SetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_DYNAMAX_LEVEL, &data);
        data = TYPE_MYSTERY;
        SetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_TERA_TYPE, &data);
        i++;
    }
}

static void CB2_EndFacilityBattle(void)
{
    Mechanics_ClearBattleRules();
    gSaveBlock2Ptr->frontier.lvlMode = sSavedLvlMode;
    gSpecialVar_Result = gBattleOutcome;
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_StartFacilityBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndFacilityBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// Starts a battle against TRAINER_BATTLE_PARAM.opponentA (a Battle Frontier
// trainer), whose party is already in gParties[B_TRAINER_OPPONENT_A].
// Script: follow with waitstate.
void Mechanics_StartFacilityBattle(void)
{
    sSavedLvlMode = gSaveBlock2Ptr->frontier.lvlMode;
    gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_OPEN;
    gFacilityTrainers = gBattleFrontierTrainers;
    gFacilityTrainerMons = gBattleFrontierMons;
    gBattleScripting.specialTrainerBattleType = FACILITY_BATTLE_TOWER;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER;
    CreateTask(Task_StartFacilityBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_B_TOWER));
}

// The Slateport Battle Tent's referee ---------------------------------------

static const u8 *const sRuleNames[RULESET_COUNT + 1] =
{
    [RULESET_INVERSE]     = COMPOUND_STRING("INVERSE"),
    [RULESET_TRICK_ROOM]  = COMPOUND_STRING("TRICK ROOM"),
    [RULESET_NO_ITEMS]    = COMPOUND_STRING("NO ITEMS"),
    [RULESET_WONDER_ROOM] = COMPOUND_STRING("WONDER ROOM"),
    [RULESET_MISTY]       = COMPOUND_STRING("MISTY FIELD"),
};

static const u8 *const sRuleHelp[RULESET_COUNT + 1] =
{
    [RULESET_INVERSE] = COMPOUND_STRING(
        "INVERSE rules turn the type chart\n"
        "around. Super effective becomes not\l"
        "very effective, and the other way.\p"
        "Immunities become weaknesses, too!\n"
        "FIRE burns right through WATER here."),
    [RULESET_TRICK_ROOM] = COMPOUND_STRING(
        "TRICK ROOM rules twist the battlefield\n"
        "for the whole battle.\p"
        "The slowest POKéMON move first!"),
    [RULESET_NO_ITEMS] = COMPOUND_STRING(
        "NO ITEMS rules put a MAGIC ROOM over\n"
        "the whole battle.\p"
        "Held items don't work, and nobody\n"
        "may use the BAG."),
    [RULESET_WONDER_ROOM] = COMPOUND_STRING(
        "WONDER ROOM rules last the whole\n"
        "battle.\p"
        "Every POKéMON's DEFENSE and SP. DEF\n"
        "are swapped!"),
    [RULESET_MISTY] = COMPOUND_STRING(
        "MISTY FIELD rules cover the ground in\n"
        "mist for the whole battle.\p"
        "POKéMON on the ground can't get status\n"
        "conditions, and DRAGON moves are weak."),
};

const u8 *Mechanics_GetRuleName(u8 ruleSet)
{
    return ruleSet <= RULESET_COUNT && sRuleNames[ruleSet] ? sRuleNames[ruleSet] : COMPOUND_STRING("");
}

// Script: VAR_0x8005 = RULESET_*. gStringVar4 = how it works.
void Mechanics_GetRuleHelp(void)
{
    StringCopy(gStringVar4, sRuleHelp[gSpecialVar_0x8005]);
}

// Script: pushes one dynmulti row per rule set, beaten ones marked.
void Mechanics_PushRuleRows(void)
{
    u32 i;

    for (i = 1; i <= RULESET_COUNT; i++)
    {
        // Like dynmultipush: the menu frees the row when it closes.
        struct ListMenuItem item = {.name = Alloc(100), .id = i};
        u8 *end = StringCopy((u8 *)item.name, sRuleNames[i]);

        if (gSaveBlock3Ptr->mechanics.rulesWon & (1 << i))
            StringCopy(end, COMPOUND_STRING(" {COLOR GREEN}WON"));
        MultichoiceDynamic_PushElement(item);
    }
}

// Script: VAR_0x8005 = RULESET_*. Sets up the battle; gStringVar1 = the
// opponent's name. Follow with Mechanics_StartFacilityBattle.
void Mechanics_SetupRulesBattle(void)
{
    u8 level = Mechanics_GetPartyBattleLevel();
    u16 trainerId = Mechanics_PickFacilityTrainer(level);

    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    Mechanics_FillFacilityParty(trainerId, FRONTIER_PARTY_SIZE, level);
    StringCopy(gStringVar1, gBattleFrontierTrainers[trainerId].trainerName);
    StringCopy(gStringVar2, sRuleNames[gSpecialVar_0x8005]);
    Mechanics_SetBattleRules(gSpecialVar_0x8005);
}

// Script: after the battle. VAR_RESULT = TRUE if the player won; and
// VAR_0x8006 = TRUE if that finished every rule set for the first time.
void Mechanics_RulesAfterBattle(void)
{
    u32 all = ((1 << (RULESET_COUNT + 1)) - 2);
    bool32 hadAll = (gSaveBlock3Ptr->mechanics.rulesWon & all) == all;

    gSpecialVar_Result = (gBattleOutcome == B_OUTCOME_WON);
    gSpecialVar_0x8006 = FALSE;
    StringCopy(gStringVar2, sRuleNames[gSpecialVar_0x8005]);
    if (!gSpecialVar_Result)
        return;
    gSaveBlock3Ptr->mechanics.rulesWon |= 1 << gSpecialVar_0x8005;
    gSpecialVar_0x8006 = !hadAll && (gSaveBlock3Ptr->mechanics.rulesWon & all) == all;
}
