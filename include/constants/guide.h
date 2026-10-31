#ifndef GUARD_CONSTANTS_GUIDE_H
#define GUARD_CONSTANTS_GUIDE_H

// The guide (docs/guide.md): R > GUIDE lists every new area and activity,
// marks them on a map, and has the social screens (players nearby, invites).

#define WORLD_MENU_GUIDE 24 // its row in the R menu (data/scripts/world/menu.inc)

// Rows of R > GUIDE.
#define GUIDE_MENU_LIST    0
#define GUIDE_MENU_MAP     1
#define GUIDE_MENU_NEARBY  2
#define GUIDE_MENU_INVITE  3
#define GUIDE_MENU_CLOSE   4

// VAR_RESULT of Guide_ShowList: the entry picked, or this.
#define GUIDE_NONE 0xFF

// Online requests (src/guide_social.c): VAR_0x8004 of Online_Request at and
// after this number reads the guide's own table, not gOnlineRequests.
#define ONL_REQ_GUIDE_FIRST  200
#define ONL_REQ_GUIDE_INVITE 200 // tells your friends you're online
#define ONL_REQ_GUIDE_NEARBY 201 // players online near you
#define ONL_REQ_GUIDE_FRIEND 202 // adds the player picked from GUIDE_NEARBY as a friend
#define ONL_REQ_GUIDE_COUNT  3

// How many entries the save can tell apart (struct GuideSaveData).
#define GUIDE_MAX_ENTRIES 64

#endif // GUARD_CONSTANTS_GUIDE_H
