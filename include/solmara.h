#ifndef GUARD_SOLMARA_H
#define GUARD_SOLMARA_H

#include "constants/solmara.h"

// Saved state of Solmara (docs/solmara.md), in the extended save.
struct SolmaraSaveData
{
    u8 flags[SOLMARA_FLAGS_COUNT / 8];
    u32 championTime; // frames played when the player first became Solmara's champion (0: not yet)
    u16 leagueClears;
    u8 filler[26];
};

u32 Solmara_CountBadges(void);
void Solmara_ScaleTrainerParty(struct Pokemon *party, u16 trainerId);
bool32 Solmara_IsTrainerId(u16 trainerId);

#endif // GUARD_SOLMARA_H
