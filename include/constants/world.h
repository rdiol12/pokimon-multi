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

#endif // GUARD_CONSTANTS_WORLD_H
