#ifndef GUARD_CONSTANTS_PASTIME_H
#define GUARD_CONSTANTS_PASTIME_H

// Minigames and side activities (docs/pastimes.md).

// Row of the R menu (data/scripts/world/menu.inc) that opens R > PASTIMES.
#define WORLD_MENU_PASTIMES 20

// R > PASTIMES submenu (data/scripts/pastime/menu.inc).
#define PASTIME_MENU_LOST_FOUND 0
#define PASTIME_MENU_CLOSE      31

// NPCs and objects the activities place on maps from code (src/pastime.c)
// use these local ids, so they never clash with a map's own objects.
#define LOCALID_PASTIME_FIRST 170
#define LOCALID_PASTIME_LAST  189

// Lost and found (src/pastime_lost.c).
#define LOST_FOUND_COUNT 8
#define LOST_FOUND_MISSING  0
#define LOST_FOUND_CARRYING 1
#define LOST_FOUND_RETURNED 2

#endif // GUARD_CONSTANTS_PASTIME_H
