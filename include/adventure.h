#ifndef GUARD_ADVENTURE_H
#define GUARD_ADVENTURE_H

#include "constants/adventure.h"

// Fourth-batch game features (docs/adventure.md). The world features'
// hooks (src/world.c) call these; each feature has its own file.

void Adventure_Update(void);
void Adventure_OnCatch(u16 species);
bool32 Adventure_TryInteract(s16 x, s16 y);
enum Species Adventure_ModifyWildSpecies(enum Species species);
void Adventure_OnTrainerBattleEnd(void);

#endif // GUARD_ADVENTURE_H
