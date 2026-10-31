#ifndef GUARD_INJURY_H
#define GUARD_INJURY_H

#include "constants/injury.h"

// Stats
s32 Injury_ModifyStat(struct BoxPokemon *boxMon, u32 stat, s32 value);

// Battle hooks (only the player's own Pokémon are affected).
void Injury_OnCriticalHit(u32 battler);
void Injury_OnBattlerFainted(u32 battler);

// Field
void Injury_OnWalk128Steps(void);
bool32 Injury_TryStartNotify(void);

// Sprites: draws lost parts, eye patch and scar onto a decompressed 64x64 pic.
void Injury_DrawOnPic(struct BoxPokemon *boxMon, u32 species, void *pic, const u16 *palette, bool32 isFrontPic, u32 frames);

// Summary screen: writes the injury memo to dest. Returns FALSE if the Pokémon is unhurt.
bool32 Injury_BufferSummaryMemo(struct BoxPokemon *boxMon, u8 *dest);

// Script natives (Pokémon Center clinic)
void Injury_ClinicFindNext(void);
void Injury_ClinicTreat(void);
bool32 Injury_PartyHasInjury(void);
void Injury_ScriptPartyHasInjury(void);

// Debug / tests
void Injury_Set(struct Pokemon *mon, u32 part, u32 percent, u32 lostMask, u32 flags);

#endif // GUARD_INJURY_H
