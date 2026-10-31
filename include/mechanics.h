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

// Battle rule sets (src/mechanics_battle.c). Set before the battle starts;
// a link battle needs the same rule set on both games. The facility battles
// here clear it when the battle ends; anyone else calls ClearBattleRules.
void Mechanics_SetBattleRules(u8 ruleSet);
void Mechanics_ClearBattleRules(void);
bool32 Mechanics_IsBagBanned(void);
const u8 *Mechanics_GetRuleName(u8 ruleSet);

// Battles against Battle Frontier trainers with a party made in C.
u8 Mechanics_GetPartyBattleLevel(void);
u16 Mechanics_PickFacilityTrainer(u8 level);
void Mechanics_FillFacilityParty(u16 trainerId, u8 count, u8 level);
void Mechanics_StartFacilityBattle(void);

#endif // GUARD_MECHANICS_H
