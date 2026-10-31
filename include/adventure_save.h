#ifndef GUARD_ADVENTURE_SAVE_H
#define GUARD_ADVENTURE_SAVE_H

// Saved state of the fourth-batch game features (docs/adventure.md). Lives
// at the end of SaveBlock3.

struct AdventureSaveData
{
    // Trainer levels.
    u32 trainerXp;
    u8 outfit;          // OUTFIT_*
    u8 trainerLevel;    // last level shown to the player, for level-up notices
    u16 lastPvpWins;    // game stats already counted for XP
    u16 lastPvpBattles;
    u16 lastTrades;
};

#endif // GUARD_ADVENTURE_SAVE_H
