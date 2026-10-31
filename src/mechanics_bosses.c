#include "global.h"
#include "battle.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "event_data.h"
#include "item.h"
#include "mechanics.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/species.h"

// Cave bosses: a big Pokémon waits in five caves. It Terastallizes on its
// first turn, or Dynamaxes and stays Dynamaxed for the whole battle. Beat it
// for its treasure, or catch it; either way it's gone for good.

struct CaveBoss
{
    u16 species;
    u8 level;
    u8 gimmick;  // GIMMICK_TERA or GIMMICK_DYNAMAX
    u8 teraType;
    u16 heldItem;
    u16 treasure; // dropped when it's beaten
    u16 flag;
    const u8 *intro;
};

static const struct CaveBoss sBosses[BOSS_COUNT] =
{
    [BOSS_GRANITE] =
    {
        SPECIES_SABLEYE, 22, GIMMICK_TERA, TYPE_FAIRY, ITEM_NONE, ITEM_DUSK_STONE, FLAG_MECH_BOSS_GRANITE,
        COMPOUND_STRING("A SABLEYE glitters in the dark…\n"
                        "Its gems shine pink! It's going to\l"
                        "TERASTALLIZE!"),
    },
    [BOSS_METEOR] =
    {
        SPECIES_LUNATONE, 33, GIMMICK_DYNAMAX, TYPE_MYSTERY, ITEM_NONE, ITEM_MOON_STONE, FLAG_MECH_BOSS_METEOR,
        COMPOUND_STRING("A LUNATONE hums with red energy…\n"
                        "It's swelling up! It's going to\l"
                        "DYNAMAX!"),
    },
    [BOSS_SHOAL] =
    {
        SPECIES_GLALIE, 42, GIMMICK_TERA, TYPE_GHOST, ITEM_NONE, ITEM_SPELL_TAG, FLAG_MECH_BOSS_SHOAL,
        COMPOUND_STRING("A GLALIE stares out of the ice…\n"
                        "A ghostly crown forms on its head!\l"
                        "It's going to TERASTALLIZE!"),
    },
    [BOSS_SEAFLOOR] =
    {
        SPECIES_SHARPEDO, 45, GIMMICK_DYNAMAX, TYPE_MYSTERY, ITEM_NONE, ITEM_DEEP_SEA_TOOTH, FLAG_MECH_BOSS_SEAFLOOR,
        COMPOUND_STRING("A SHARPEDO circles in the dark…\n"
                        "Red light pours out of it! It's\l"
                        "going to DYNAMAX!"),
    },
    [BOSS_VICTORY] =
    {
        SPECIES_FLYGON, 54, GIMMICK_TERA, TYPE_FIRE, ITEM_NONE, ITEM_ABILITY_PATCH, FLAG_MECH_BOSS_VICTORY,
        COMPOUND_STRING("A FLYGON blocks the road, wings\n"
                        "blazing! It's going to\l"
                        "TERASTALLIZE!"),
    },
};

static EWRAM_DATA u8 sBoss = 0; // BOSS_* + 1 while its battle is set up

bool32 Mechanics_IsBossHere(u8 boss)
{
    return !FlagGet(sBosses[boss].flag);
}

// Script: VAR_0x8004 = BOSS_*. Sets up the wild battle (dowildbattle
// next); gStringVar4 = its intro, VAR_0x8006 = its species.
void Mechanics_SetupBoss(void)
{
    const struct CaveBoss *boss = &sBosses[gSpecialVar_0x8004];
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 data;

    sBoss = gSpecialVar_0x8004 + 1;
    gSpecialVar_0x8006 = boss->species;
    StringCopy(gStringVar4, boss->intro);
    ZeroEnemyPartyMons();
    CreateMonWithIVs(mon, boss->species, boss->level, Random32(), OTID_STRUCT_PLAYER_ID, 31);
    GiveMonInitialMoveset(mon);
    data = boss->gimmick == GIMMICK_TERA ? boss->teraType : TYPE_MYSTERY;
    SetMonData(mon, MON_DATA_TERA_TYPE, &data);
    data = boss->gimmick == GIMMICK_DYNAMAX ? MAX_DYNAMAX_LEVEL : BLOCK_AI_DYNAMAX;
    SetMonData(mon, MON_DATA_DYNAMAX_LEVEL, &data);
    if (boss->heldItem != ITEM_NONE)
        SetMonData(mon, MON_DATA_HELD_ITEM, &boss->heldItem);
}

static const struct CaveBoss *GetBattleBoss(u32 battler)
{
    if (sBoss == 0 || GetBattlerSide(battler) != B_SIDE_OPPONENT || (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
     || gBattleMons[battler].species != sBosses[sBoss - 1].species)
        return NULL;
    return &sBosses[sBoss - 1];
}

// Before the turn order is set, when the boss uses a move: its gimmick,
// from its first turn on.
void Mechanics_OnBossMoveChosen(u32 battler)
{
    const struct CaveBoss *boss = GetBattleBoss(battler);

    if (boss == NULL)
        return;
    if (GetActiveGimmick(battler) == GIMMICK_DYNAMAX)
    {
        gBattleStruct->dynamax.dynamaxTurns[battler] = DYNAMAX_TURNS_COUNT; // it never shrinks back
    }
    else if (GetActiveGimmick(battler) == GIMMICK_NONE && !HasTrainerUsedGimmick(battler, boss->gimmick))
    {
        gBattleStruct->gimmick.usableGimmick[battler] = boss->gimmick;
        gBattleStruct->gimmick.toActivate |= 1u << battler;
        if (boss->gimmick == GIMMICK_DYNAMAX)
            gBattleStruct->dynamax.baseMoves[battler] = gChosenMoveByBattler[battler];
    }
}

// Script: after the battle. VAR_RESULT = 0 if it's still there, 1 if it
// was beaten (VAR_0x8005 = its treasure), 2 if it was caught.
void Mechanics_EndBoss(void)
{
    const struct CaveBoss *boss = &sBosses[gSpecialVar_0x8004];

    sBoss = 0;
    gSpecialVar_0x8005 = boss->treasure;
    if (gBattleOutcome == B_OUTCOME_WON)
        gSpecialVar_Result = 1;
    else if (gBattleOutcome == B_OUTCOME_CAUGHT)
        gSpecialVar_Result = 2;
    else
        gSpecialVar_Result = 0;
    if (gSpecialVar_Result != 0)
        FlagSet(boss->flag);
}
