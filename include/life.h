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

#endif // GUARD_LIFE_H
