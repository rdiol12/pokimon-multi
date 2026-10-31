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
void Mechanics_PrepareOpponentMon(struct Pokemon *mon);

// Level caps (src/mechanics_cups.c): brings the party's Pokémon above
// `level` down to it, until RestoreParty puts the real party back.
void Mechanics_ApplyLevelCap(u8 level);
void Mechanics_RestoreParty(void);

// Rare color variants (src/mechanics_colors.c).
u8 Mechanics_GetColorVariant(u32 personality);
const u8 *Mechanics_GetColorVariantName(u8 variant);
const u16 *Mechanics_ApplyColorVariant(const u16 *palette, u32 personality);

// Pokémon personalities (src/mechanics_personality.c).
struct ScriptContext;
struct ObjectEvent;
u8 Mechanics_GetPersona(u32 personality);
const u8 *Mechanics_GetPersonaName(u8 persona);
void Mechanics_AdjustFollowerMoods(struct Pokemon *mon, u8 *weights);
bool32 Mechanics_TryFollowerPersonaLine(struct ScriptContext *ctx, struct Pokemon *mon, struct ObjectEvent *follower);
void Mechanics_UpdatePersonaFollower(void);

// Battle styles (src/mechanics_styles.c).
bool32 Mechanics_CanUseBattleStyles(void);
const u8 *Mechanics_GetStyleName(u8 style);
void Mechanics_ResetStyleChoice(u32 battler);
bool32 Mechanics_CycleStyleChoice(u32 battler, u32 pp);
u8 Mechanics_GetStyleChoice(u32 battler);
u32 Mechanics_GetStyleReturnBits(u32 battler);
void Mechanics_SetBattlerStyle(u32 battler, u8 returnValue);
void Mechanics_ClearBattlerStyle(u32 battler);
void Mechanics_BeforeTurnOrder(void);
u32 Mechanics_GetStylePowerModifier(u32 battler);
s32 Mechanics_GetStyleOrder(u32 battler);
u32 Mechanics_GetStyleExtraPP(u32 battler);
bool32 Mechanics_TryAnnounceBattleStyle(u32 battler);
const u8 *Mechanics_GetBattleMessage(u8 id);
void Mechanics_SetupStylePractice(void);

// Battle commentary (src/mechanics_commentary.c).
void Mechanics_OnBattleString(u16 stringId, u32 battler);
bool32 Mechanics_TryOpeningCommentary(void);
bool32 Mechanics_TryTurnCommentary(void);
const u8 *Mechanics_GetCommentary(u8 line);
bool32 Mechanics_IsCupActive(void);

// Trainer skills (src/mechanics_skills.c).
bool32 Mechanics_HasSkill(u8 skill);
bool32 Mechanics_TryKeepBattleItem(u16 item);
u32 Mechanics_BoostCatchOdds(u32 odds);
u32 Mechanics_BoostPrizeMoney(u32 money);
void Mechanics_OnTrainerBattleEnd(void);
void Mechanics_UpdateSkills(void);

// Starter signature moves (src/mechanics_signature.c).
bool32 Mechanics_CanLearnSignatureMove(u16 species, u16 move);
bool32 Mechanics_IsCatchBlocked(void);
bool32 Mechanics_IsStatueHere(u8 line);

// Cave bosses (src/mechanics_bosses.c).
bool32 Mechanics_IsBossHere(u8 boss);
void Mechanics_OnBossMoveChosen(u32 battler);
void Mechanics_EndStylePractice(void);

#endif // GUARD_MECHANICS_H
