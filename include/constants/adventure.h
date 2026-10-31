#ifndef GUARD_CONSTANTS_ADVENTURE_H
#define GUARD_CONSTANTS_ADVENTURE_H

// Fourth-batch game features (docs/adventure.md): trainer levels, daily
// quests, crafting, ranching, dungeons, treasure hunts, the online Battle
// Frontier and the Hall of Fame wall.

// R > ADVENTURE submenu (data/scripts/adventure/menu.inc).
#define ADVENTURE_MENU_TRAINER_LEVEL 0
#define ADVENTURE_MENU_OUTFITS       1
#define ADVENTURE_MENU_DAILY_QUESTS  2
#define ADVENTURE_MENU_CRAFTING      3
#define ADVENTURE_MENU_TREASURE_MAP  4
#define ADVENTURE_MENU_FRONTIER      5
#define ADVENTURE_MENU_HALL_OF_FAME  6
#define ADVENTURE_MENU_CLOSE         15

// Flags (FLAG_UNUSED_0x264-0x27C and 0x281-0x28D are this batch's).
#define FLAG_ADVENTURE_CRAFTING FLAG_UNUSED_0x264 // the Rustboro crafter showed how to craft
#define FLAG_HIDE_RANCH_MON_1   FLAG_UNUSED_0x265 // ... _6 = 0x26A: empty spots in the ranch pasture

// Trainer levels (src/trainer_level.c).
#define TRAINER_LEVEL_MAX 30

// Outfits: 0 is the style's own clothes.
#define OUTFIT_NONE      0
#define OUTFIT_AUTUMN    1
#define OUTFIT_STARLIGHT 2
#define OUTFIT_ACE       3
#define OUTFIT_SHADOW    4
#define OUTFIT_CHAMPION  5
#define OUTFIT_COUNT     6
#define OUTFIT_STATES    7 // normal, Mach Bike, Acro Bike, surfing, field move, fishing, watering

// Perks, each unlocked at a trainer level.
#define PERK_PRIZE_MONEY  0 // +10% prize money from trainers
#define PERK_DISCOUNT     1 // 10% off in Poké Marts
#define PERK_HATCHING     2 // Eggs hatch twice as fast
#define PERK_PRIZE_MONEY2 3 // +25% prize money instead
#define PERK_SHINY        4 // one more shiny roll for wild Pokémon
#define PERK_COUNT        5

// Daily quest givers (VAR_0x8004 for DailyQuest_Talk).
#define DAILY_QUEST_PETALBURG 0
#define DAILY_QUEST_SLATEPORT 1
#define DAILY_QUEST_FORTREE   2
#define DAILY_QUEST_GIVERS    3

// DailyQuest_Talk results.
#define DAILY_QUEST_TALK_OFFER     0
#define DAILY_QUEST_TALK_PROGRESS  1
#define DAILY_QUEST_TALK_COMPLETE  2
#define DAILY_QUEST_TALK_COME_BACK 3

// Ranch (src/ranch.c).
#define RANCH_SHOWN 6 // Pokémon wandering the pasture

// Ranch_Collect results.
#define RANCH_TALK_FIRST       0
#define RANCH_TALK_EMPTY       1
#define RANCH_TALK_NOTHING_YET 2
#define RANCH_TALK_HAUL        3

// Treasure_Talk results.
#define TREASURE_TALK_FIRST 0 // the first map ever
#define TREASURE_TALK_NEW   1 // today's map
#define TREASURE_TALK_STILL 2 // still hunting today's
#define TREASURE_TALK_DONE  3 // found today's

// The online Battle Frontier (src/frontier_online.c): Factory, Arena, Pyramid.
#define FRONTIER_ONLINE_FACILITIES 3

// Dungeon, the Rift Below (src/dungeon.c).
#define DUNGEON_MAX_FLOOR 99

#endif // GUARD_CONSTANTS_ADVENTURE_H
