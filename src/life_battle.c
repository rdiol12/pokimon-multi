#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "life.h"
#include "pokemon.h"
#include "constants/battle.h"
#include "constants/trainers.h"

// Battles against trainers that aren't in gTrainers: Team Eclipse
// (life_eclipse.c) and ghost trainers. While gLifeTrainer is set,
// GetTrainerStructFromId (include/data.h) returns it for the opponent, so
// the battle takes its name, class, picture, AI and prize money from it.
// No trainer flags are touched. Call from a script with waitstate;
// VAR_RESULT is TRUE if the player won (battle_setup.c).

EWRAM_DATA const struct Trainer *gLifeTrainer = NULL;

void LifeBattle_Start(const struct Trainer *trainer, const u8 *defeatText)
{
    gLifeTrainer = trainer;
    memset(&gTrainerBattleParameter, 0, sizeof(gTrainerBattleParameter));
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_NONE;
    TRAINER_BATTLE_PARAM.defeatTextA = (u8 *)defeatText;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    if (trainer->battleType == TRAINER_BATTLE_TYPE_DOUBLES)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], trainer);
    BattleSetup_StartLifeTrainerBattle();
}
