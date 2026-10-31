#ifndef GUARD_FIELD_ABILITY_H
#define GUARD_FIELD_ABILITY_H

// Pokémon abilities and types that work outside battle (docs/field_abilities.md).
// The world hooks (src/world.c) and a few engine spots call these.

// Flags FLAG_UNUSED_0x940-0x95F: one per world spot (ice block, generator,
// dig spot, boulder) in sFieldSpots, in order. Local ids 150-169.
#define FLAG_FIELD_ABILITY_FIRST 0x940
#define LOCALID_FIELD_ABILITY_FIRST 150

void FieldAbility_Update(void);
bool32 FieldAbility_TryInteract(s16 x, s16 y);

// Dark caves: TRUE if a generator lit this map for good.
bool32 FieldAbility_IsCaveLit(void);
// Dark caves: TRUE if the lead's flame (Fire type, Flame Body, Illuminate...)
// lights the way like Flash.
bool32 FieldAbility_HasLantern(void);
// Surfing at running speed with a Swift Swim / Water Veil lead.
bool32 FieldAbility_SurfsFast(void);
// Extra berries a berry tree gives with the current lead (Harvest, Grass).
u32 FieldAbility_ExtraBerries(void);
// Called on every step; TRUE if it started a script (Healer, Natural Cure).
bool32 FieldAbility_OnStep(void);
// Wild battles: Frisk, Pickpocket and Magician pocket the foe's held item.
void FieldAbility_OnBattleStart(void);
void FieldAbility_OnWildBattleEnd(void);
// Daily reset: the dig spots fill up again.
void FieldAbility_ResetDaily(void);

// Script natives.
void FieldAbility_FindStrongMon(void);
void FieldAbility_FindHelper(void);
void FieldAbility_Solve(void);

#endif // GUARD_FIELD_ABILITY_H
