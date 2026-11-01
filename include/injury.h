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

// Sprites. "Looks" are what the sprites show (INJURY_LOOKS_*), 0 = nothing.
u32 Injury_GetLooks(struct BoxPokemon *boxMon);
u32 Injury_GetLooksForSpecies(struct BoxPokemon *boxMon, u32 species);
// Draws lost parts, eye patch and scar onto decompressed 4bpp frames.
void Injury_DrawLooks(u32 looks, void *pic, u32 width, u32 height, const u16 *palette, bool32 isFrontPic, u32 frames);
// Same for a 64x64 battle/summary pic.
void Injury_DrawOnPic(struct BoxPokemon *boxMon, u32 species, void *pic, const u16 *palette, bool32 isFrontPic, u32 frames);
// Icons: copies 32x32 icon frames from src to dest with the marks drawn on.
// Returns FALSE (and copies nothing) if the Pokémon has no marks.
bool32 Injury_DrawOnIconTiles(struct BoxPokemon *boxMon, const u8 *src, u8 *dest, const u16 *palette, u32 frames);
// Gives a CreateMonIcon sprite the marks of boxMon.
void Injury_ApplyToMonIcon(u8 spriteId, struct BoxPokemon *boxMon);
// Overworld: our follower, and other players' followers online.
struct ObjectEvent;
void Injury_ApplyToObjectEvent(struct ObjectEvent *objectEvent);
void Injury_SetObjectEventLooks(struct ObjectEvent *objectEvent, u32 looks);
u32 Injury_GetObjectEventLooks(struct ObjectEvent *objectEvent);

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
