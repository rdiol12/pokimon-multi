#ifndef GUARD_RANCH_H
#define GUARD_RANCH_H

#include "constants/adventure.h"

// Pokémon ranching at the Hoenn Ranch (docs/adventure.md).

u32 Ranch_CountMons(void);
void Ranch_Collect(void);
void Ranch_ShowHaul(void);
void Ranch_SetupMap(void);
void Ranch_TalkToMon(void);
void Ranch_Showcase(const u16 *species, u32 count, u32 hoursAgo);

#endif // GUARD_RANCH_H
