#ifndef GUARD_MECHANICS_H
#define GUARD_MECHANICS_H

#include "constants/mechanics.h"

// Battle and Pokémon mechanics (docs/mechanics.md). The world features'
// hooks (src/world.c) call these; each feature has its own file.

// Called once per overworld frame.
void Mechanics_Update(void);
// Called when the player presses A facing (x, y), map coordinates with
// MAP_OFFSET. Returns TRUE if it started a script.
bool32 Mechanics_TryInteract(s16 x, s16 y);

#endif // GUARD_MECHANICS_H
