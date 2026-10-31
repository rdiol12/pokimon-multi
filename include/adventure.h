#ifndef GUARD_ADVENTURE_H
#define GUARD_ADVENTURE_H

#include "constants/adventure.h"

// Fourth-batch game features (docs/adventure.md). The world features'
// hooks (src/world.c) call these; each feature has its own file.

void Adventure_Update(void);
void Adventure_OnCatch(void);
void Adventure_OnTrainerBattleEnd(void);

#endif // GUARD_ADVENTURE_H
