#include "global.h"
#include "battle.h"
#include "battle_frontier.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "list_menu.h"
#include "malloc.h"
#include "mechanics.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "constants/battle_frontier.h"
#include "constants/items.h"
#include "constants/moves.h"

// Level-capped tournaments at the Verdanturf Battle Tent: three rounds in a
// row against Battle Frontier trainers. For the tournament, the player's
// Pokémon above the cup's level are brought down to it (stats and all);
// the real party comes back afterwards, untouched.

struct Cup
{
    const u8 *name;
    u8 level;
    bool8 littleCup; // only Pokémon that haven't evolved but still can
    u16 prize;       // the first time the cup is won
};

static const struct Cup sCups[CUP_COUNT] =
{
    [CUP_LITTLE] = {COMPOUND_STRING("LITTLE CUP"), 5, TRUE, ITEM_EVIOLITE},
    [CUP_RISING] = {COMPOUND_STRING("RISING CUP"), 30, FALSE, ITEM_LIFE_ORB},
    [CUP_PRIME]  = {COMPOUND_STRING("PRIME CUP"), 50, FALSE, ITEM_CHOICE_SCARF},
};

// Little Cup opponents: Pokémon that can evolve, with the moves they know
// at level 5 (Battle Frontier sets are all evolved).
static const u16 sLittleCupSpecies[] =
{
    SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE, SPECIES_PICHU, SPECIES_MACHOP,
    SPECIES_GASTLY, SPECIES_ABRA, SPECIES_GEODUDE, SPECIES_ONIX, SPECIES_SCYTHER,
    SPECIES_MAGNEMITE, SPECIES_PORYGON, SPECIES_RIOLU, SPECIES_MUNCHLAX, SPECIES_FLETCHLING,
    SPECIES_TIMBURR, SPECIES_FOONGUS, SPECIES_FERROSEED, SPECIES_DEINO, SPECIES_MUDBRAY,
    SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP, SPECIES_RALTS, SPECIES_ARON,
    SPECIES_WINGULL, SPECIES_CARVANHA, SPECIES_TRAPINCH, SPECIES_SNORUNT, SPECIES_BAGON,
};

// The real party while a level cap is on (not on the heap: battles reset it).
static EWRAM_DATA struct Pokemon sSavedParty[PARTY_SIZE] = {0};
static EWRAM_DATA bool8 sPartySaved = FALSE;
static EWRAM_DATA u8 sCup = 0;

static bool32 IsLittleCupEligible(u16 species)
{
    return GetSpeciesEvolutions(species)[0].method != EVOLUTIONS_END
        && GetSpeciesPreEvolution(species) == SPECIES_NONE;
}

static void CapMonLevel(struct Pokemon *mon, u8 level)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);

    if (GetMonData(mon, MON_DATA_LEVEL) <= level)
        return;
    SetMonData(mon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][level]);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    CalculateMonStats(mon);
}

// Brings the party's Pokémon above `level` down to it until
// Mechanics_RestoreParty. Online PvP can use it for capped battles too.
void Mechanics_ApplyLevelCap(u8 level)
{
    u32 i;

    if (!sPartySaved)
    {
        memcpy(sSavedParty, gParties[B_TRAINER_PLAYER], sizeof(sSavedParty));
        sPartySaved = TRUE;
    }
    for (i = 0; i < PARTY_SIZE; i++)
        CapMonLevel(&gParties[B_TRAINER_PLAYER][i], level);
}

// TRUE while the player is in a tournament (or another capped battle).
bool32 Mechanics_IsCupActive(void)
{
    return sPartySaved;
}

void Mechanics_RestoreParty(void)
{
    if (!sPartySaved)
        return;
    memcpy(gParties[B_TRAINER_PLAYER], sSavedParty, sizeof(sSavedParty));
    sPartySaved = FALSE;
    CalculatePlayerPartyCount();
}

// Script: pushes one dynmulti row per cup, won ones marked.
void Mechanics_PushCupRows(void)
{
    u32 i;

    for (i = 0; i < CUP_COUNT; i++)
    {
        struct ListMenuItem item = {.name = Alloc(100), .id = i + 1};
        u8 *end = StringCopy((u8 *)item.name, sCups[i].name);

        if (gSaveBlock3Ptr->mechanics.cupsWon & (1 << i))
            StringCopy(end, COMPOUND_STRING(" {COLOR GREEN}WON"));
        MultichoiceDynamic_PushElement(item);
    }
}

// Script: VAR_0x8005 = CUP_* + 1. Checks the party and, if it may enter,
// caps its levels. VAR_RESULT = how many Pokémon will fight (0: none
// may); gStringVar1 = the cup, gStringVar2 = its level.
void Mechanics_EnterCup(void)
{
    const struct Cup *cup = &sCups[gSpecialVar_0x8005 - 1];
    struct Pokemon *entrants;
    u32 i, count = 0;

    sCup = gSpecialVar_0x8005 - 1;
    StringCopy(gStringVar1, cup->name);
    ConvertIntToDecimalStringN(gStringVar2, cup->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    entrants = AllocZeroed(sizeof(struct Pokemon) * PARTY_SIZE);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES);

        if (species == SPECIES_NONE || GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_IS_EGG))
            continue;
        if (cup->littleCup && !IsLittleCupEligible(species))
            continue;
        entrants[count++] = gParties[B_TRAINER_PLAYER][i];
    }
    gSpecialVar_Result = count;
    if (count != 0)
    {
        Mechanics_ApplyLevelCap(cup->level);
        // Only the entrants fight; the full party comes back afterwards.
        memcpy(gParties[B_TRAINER_PLAYER], entrants, sizeof(struct Pokemon) * PARTY_SIZE);
        for (i = 0; i < PARTY_SIZE; i++)
            CapMonLevel(&gParties[B_TRAINER_PLAYER][i], cup->level);
        CalculatePlayerPartyCount();
    }
    Free(entrants);
}

static void FillLittleCupParty(void)
{
    u32 i, j;
    u16 chosen[FRONTIER_PARTY_SIZE];

    ZeroEnemyPartyMons();
    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        do
        {
            chosen[i] = sLittleCupSpecies[Random() % ARRAY_COUNT(sLittleCupSpecies)];
            for (j = 0; j < i && chosen[j] != chosen[i]; j++)
                ;
        } while (j != i);
        CreateMonWithIVsPersonality(&gParties[B_TRAINER_OPPONENT_A][i], chosen[i], 5, 0x3FFFFFFF & Random32(), Random32());
        Mechanics_PrepareOpponentMon(&gParties[B_TRAINER_OPPONENT_A][i]);
    }
}

// Script: VAR_0x8006 = round (0-2). Sets up that round's opponent;
// gStringVar3 = their name. Follow with Mechanics_StartFacilityBattle.
void Mechanics_SetupCupRound(void)
{
    const struct Cup *cup = &sCups[sCup];
    // Each round's trainer is a little stronger than the last.
    u16 trainerId = Mechanics_PickFacilityTrainer(cup->level + 10 * gSpecialVar_0x8006 + (cup->littleCup ? 30 : 0));

    TRAINER_BATTLE_PARAM.opponentA = trainerId;
    if (cup->littleCup)
        FillLittleCupParty();
    else
        Mechanics_FillFacilityParty(trainerId, FRONTIER_PARTY_SIZE, cup->level);
    StringCopy(gStringVar3, gBattleFrontierTrainers[trainerId].trainerName);
    StringCopy(gStringVar1, cup->name);
}

// Script: after the tournament (won or lost). Puts the real party back.
// VAR_RESULT = the prize item if the cup was won for the first time,
// else ITEM_NONE.
void Mechanics_LeaveCup(bool32 won)
{
    Mechanics_RestoreParty();
    StringCopy(gStringVar1, sCups[sCup].name);
    gSpecialVar_Result = ITEM_NONE;
    if (!won)
        return;
    if (!(gSaveBlock3Ptr->mechanics.cupsWon & (1 << sCup)))
        gSpecialVar_Result = sCups[sCup].prize;
    gSaveBlock3Ptr->mechanics.cupsWon |= 1 << sCup;
}

void Mechanics_WinCup(void)
{
    Mechanics_LeaveCup(TRUE);
}

void Mechanics_LoseCup(void)
{
    Mechanics_LeaveCup(FALSE);
}

static const u8 *const sCupHelp[CUP_COUNT] =
{
    [CUP_LITTLE] = COMPOUND_STRING(
        "The LITTLE CUP is for POKéMON that\n"
        "haven't evolved yet, but still can.\p"
        "Everyone battles at level 5!"),
    [CUP_RISING] = COMPOUND_STRING(
        "In the RISING CUP, POKéMON above\n"
        "level 30 battle at level 30."),
    [CUP_PRIME] = COMPOUND_STRING(
        "In the PRIME CUP, POKéMON above\n"
        "level 50 battle at level 50."),
};

// Script: VAR_0x8005 = CUP_* + 1. gStringVar4 = the cup's rules.
void Mechanics_GetCupHelp(void)
{
    StringCopy(gStringVar4, sCupHelp[gSpecialVar_0x8005 - 1]);
}
