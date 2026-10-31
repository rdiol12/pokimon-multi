#ifndef GUARD_ADVENTURE_SAVE_H
#define GUARD_ADVENTURE_SAVE_H

#include "constants/adventure.h"

struct DailyQuestSlot
{
    u16 day;   // RTC day it was taken or turned in
    u8 quest;  // index in the giver's list
    u8 state;
    u16 base;  // the counter when it was taken
    u16 count; // catches or trainers counted since
};

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
    // Daily quests, one per giver.
    struct DailyQuestSlot dailyQuests[DAILY_QUEST_GIVERS];
    // Crafting.
    u16 itemsCrafted;
    // Ranch.
    u16 ranchItemsGathered;
    u32 ranchLastHour;          // the clock (days * 24 + hours) when finds were last counted
    u8 ranchStarted;
    u8 ranchShown[RANCH_SHOWN]; // box positions of the Pokémon in the pasture
    u8 filler;
    // Dungeon.
    u32 dungeonSeed;  // the current floor's layout
    u8 dungeonFloor;  // 1 = B1F
    u8 dungeonBest;
    u16 dungeonRuns;
};

#endif // GUARD_ADVENTURE_SAVE_H
