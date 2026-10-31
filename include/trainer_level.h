#ifndef GUARD_TRAINER_LEVEL_H
#define GUARD_TRAINER_LEVEL_H

#include "constants/adventure.h"

// Trainer levels (docs/adventure.md): XP earned while playing online
// unlocks outfits and perks.

enum PlayerStyle;

// XP for things done while online; ignored offline. Shows a notice on a
// new level.
void TrainerLevel_AddXp(u32 xp);
u32 TrainerLevel_GetLevel(void);
void TrainerLevel_Update(void);
void TrainerLevel_OnCatch(void);
void TrainerLevel_OnTrainerWin(void);

// For the screenshot showcase (src/adventure.c).
void TrainerLevel_SetXp(u32 xp);
void TrainerLevel_SetOutfit(u32 outfit);

// Perks.
bool32 TrainerLevel_HasPerk(u32 perk);
u32 TrainerLevel_BoostPrizeMoney(u32 money);
u32 TrainerLevel_DiscountPrice(u32 price);
u32 TrainerLevel_ShinyRolls(void);

// Outfits (field_player_avatar.c draws the player with them).
const u16 *TrainerLevel_GetOutfitGfxIds(enum PlayerStyle style);
bool32 TrainerLevel_IsOutfitSurfingGfx(u16 graphicsId);
u8 TrainerLevel_GetOutfitGfxGender(u16 graphicsId);

// Screens, opened from scripts with `callnative ..., waitstate=1`.
void TrainerLevel_ShowCard(void);
void TrainerLevel_ShowOutfitMenu(void);

#endif // GUARD_TRAINER_LEVEL_H
