#include "global.h"
#include "community.h"
#include "event_data.h"
#include "pokemon.h"
#include "script.h"
#include "constants/pokemon.h"

// Gym rematches at your level (data/scripts/community/rematch.inc): after the
// Hall of Fame, each leader offers a double battle with their strongest team
// (TRAINER_X_5), scaled to a level above your best Pokémon. The leaders' own
// Match Call rematches still come first when one is due.

#define REMATCH_LEVEL_BONUS 1

static EWRAM_DATA u16 sTrainerId = 0;
static EWRAM_DATA bool8 sWasDefeated = FALSE;

// Script: VAR_0x8004 = trainer. VAR_RESULT: FALSE when the player can't
// battle a double. Otherwise the next battle against that trainer is scaled,
// and its trainer flag is cleared so the battle runs; Finish puts it back.
void CommunityRematch_Prepare(void)
{
    gSpecialVar_Result = GetMonsStateToDoubles() == PLAYER_HAS_TWO_USABLE_MONS;
    if (!gSpecialVar_Result)
        return;
    sTrainerId = gSpecialVar_0x8004;
    sWasDefeated = FlagGet(TRAINER_FLAGS_START + sTrainerId);
    FlagClear(TRAINER_FLAGS_START + sTrainerId);
    gCommunityTrainerScale.trainerId = sTrainerId;
    gCommunityTrainerScale.partySize = 0;
    gCommunityTrainerScale.levelBonus = REMATCH_LEVEL_BONUS;
    gCommunityTrainerScale.aceSpecies = SPECIES_NONE;
}

// Script: after a won rematch. Leaves the Match Call rematches as they were.
void CommunityRematch_Finish(void)
{
    if (sTrainerId != 0 && !sWasDefeated)
        FlagClear(TRAINER_FLAGS_START + sTrainerId);
    sTrainerId = 0;
}
