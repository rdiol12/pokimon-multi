#ifndef GUARD_LIFE_H
#define GUARD_LIFE_H

#include "constants/life.h"

// Fifth batch of features (docs/life.md).

// Called once per overworld frame. Returns TRUE if it took over input.
bool32 Life_OverworldUpdate(void);
// Called after the map header's weather is loaded, before the map's
// ON_TRANSITION script runs.
void Life_OnMapTransition(void);

// Bonding (life_bond.c): opens the bonding screen for party slot
// VAR_0x8004 (a script calls it after ChoosePartyMon).
void LifeBond_Open(void);

// Screenshot showcase settings (life.c), written by tools/life_shots/.
struct LifeShowcase
{
    u8 pending;
    u8 friendship; // the lead's friendship, if not 0
    u8 season;     // SEASON_* + 1 overrides the clock, 0 uses it
    u8 finishJobs; // 1: every Pokémon job is done
};
extern struct LifeShowcase gLifeShowcase;

// Lets this batch swap in another species for a wild encounter.
enum Species Life_ModifyWildSpecies(enum Species species, u8 level);

// Seasons (life_seasons.c).
u32 Seasons_Get(void);
const u8 *Seasons_GetName(void);
void Seasons_TintPrimaryPalettes(bool32 skipFaded);
void Seasons_OnMapTransition(void);
void Seasons_Update(void);
enum Species Seasons_ModifyWildSpecies(enum Species species, u8 level);

// Pokémon jobs (life_jobs.c).
void LifeJobs_Describe(void);
void LifeJobs_HasFreeSlot(void);
void LifeJobs_Send(void);
void LifeJobs_Collect(void);
void LifeJobs_ShowList(void);
void LifeJobs_FinishAll(void);

#endif // GUARD_LIFE_H
