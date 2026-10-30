#ifndef GUARD_CONSTANTS_WORLD_H
#define GUARD_CONSTANTS_WORLD_H

// R button menu (data/scripts/world/menu.inc).
#define WORLD_MENU_RIDE          0
#define WORLD_MENU_ACHIEVEMENTS  1
#define WORLD_MENU_RECORDS       2
#define WORLD_MENU_PHOTO         3
#define WORLD_MENU_HUNT          4
#define WORLD_MENU_CLOSE         5

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

#endif // GUARD_CONSTANTS_WORLD_H
