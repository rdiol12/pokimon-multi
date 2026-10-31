#ifndef GUARD_CONSTANTS_WORLD_H
#define GUARD_CONSTANTS_WORLD_H

// R button menu (data/scripts/world/menu.inc).
#define WORLD_MENU_RIDE          0
#define WORLD_MENU_ACHIEVEMENTS  1
#define WORLD_MENU_RECORDS       2
#define WORLD_MENU_PHOTO         3
#define WORLD_MENU_HUNT          4
#define WORLD_MENU_CLOSE         5
#define WORLD_MENU_QUESTS        6
#define WORLD_MENU_BOARDS        7
#define WORLD_MENU_ADVENTURE     8 // data/scripts/adventure/menu.inc

// Mount_Toggle results.
#define MOUNT_RODE_OFF     0
#define MOUNT_GOT_OFF      1
#define MOUNT_NONE_IN_PARTY 2
#define MOUNT_NOT_HERE     3

// struct WorldSaveData.hardcore (enum HardcoreState).
#define HARDCORE_OFF    0
#define HARDCORE_ON     1
#define HARDCORE_FAILED 2

// Server leaderboards (World_SubmitScore).
#define WORLD_BOARD_FISH     0 // value: size in mm, extra: species
#define WORLD_BOARD_BERRIES  1 // value: berries picked, extra: best harvest
#define WORLD_BOARD_HARDCORE 2
#define WORLD_BOARD_HUNT     3
// 8-15: the fourth-batch features (docs/adventure.md).
#define WORLD_BOARD_DUNGEON  8 // value: deepest floor, extra: dives
#define WORLD_BOARD_TREASURE 9 // value: treasures found, extra: the last spot

// Legendary quests (src/world_legends.c), index into legendStage.
#define LEGEND_RAIKOU    0
#define LEGEND_SUICUNE   1
#define LEGEND_ENTEI     2
#define LEGEND_CRESSELIA 3
#define LEGEND_COUNT     4

// Who tells the player about them (VAR_0x8004 for Legends_Hint).
#define LEGEND_HINT_STORMS 0 // the Oldale weather watcher: Raikou, Suicune
#define LEGEND_HINT_STARS  1 // the Fallarbor stargazer: Entei
#define LEGEND_HINT_MOON   2 // the Lilycove Night Market: Cresselia

// The legendary standing on the map (other players' followers and mounts
// use 200-214).
#define LOCALID_WORLD_LEGEND 215

// House furniture (src/world_house.c). 0 is an empty slot.
#define FURNITURE_NONE      0
#define FURNITURE_PLANT     1
#define FURNITURE_BOOKCASE  2
#define FURNITURE_TABLE     3
#define FURNITURE_STOOL     4
#define FURNITURE_BED       5
#define FURNITURE_TV        6
#define FURNITURE_LAMP      7
#define FURNITURE_AQUARIUM  8
#define FURNITURE_CUSHION   9
#define FURNITURE_TROPHIES  10
#define FURNITURE_TELESCOPE 11
#define FURNITURE_COUNT     12

// House_StartPlacing results (VAR_RESULT).
#define HOUSE_PLACING   0
#define HOUSE_FULL      1

#endif // GUARD_CONSTANTS_WORLD_H
