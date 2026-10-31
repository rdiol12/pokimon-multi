#ifndef GUARD_GUIDE_H
#define GUARD_GUIDE_H

#include "constants/guide.h"
#include "guide_save.h"
#include "online.h"

// The guide (docs/guide.md, src/guide*.c).

enum GuideStatus
{
    GUIDE_LOCKED,  // needs more badges
    GUIDE_NEW,     // open, and the player hasn't been there or looked at it yet
    GUIDE_TODO,    // been there, its quest isn't finished
    GUIDE_KNOWN,   // been there; nothing to finish (a repeatable activity)
    GUIDE_DONE,    // quest finished
};

#define GUIDE_CHAMPION 9 // GuideEntry.badges: needs the League beaten

struct GuideEntry
{
    const u8 *name;
    const u8 *where;        // how to get there, for the detail line
    u16 mapsec;             // where its marker goes on the guide map
    u16 map;                // MAP_*: going there counts as having seen it (0: none)
    u16 startedFlag;        // set once the player has met it (0: none)
    u16 doneFlag;           // set once its quest is done (0: never done)
    bool8 (*isDone)(void);  // instead of doneFlag
    u8 badges;              // badges it needs, or GUIDE_CHAMPION
};

extern const struct GuideEntry gGuideEntries[];
extern const u8 gGuideEntryCount;

enum GuideStatus Guide_GetStatus(u32 entry);
void Guide_MarkSeen(u32 entry);
const u8 *Guide_GetNeedsText(u32 entry);
u32 Guide_CountStatus(enum GuideStatus status);

// Called every overworld frame from World_OverworldUpdate. TRUE if it
// started a script.
bool32 Guide_OverworldUpdate(void);

// The title menu's INVITE FRIENDS: join online, then tell your friends.
void Guide_JoinOnlineAndInvite(void);

// Screens (src/guide_screen.c), opened with callnative ..., waitstate=1.
void Guide_ShowList(void);  // VAR_RESULT: the entry picked for the map, or GUIDE_NONE
void Guide_ShowMap(void);   // VAR_0x8004: the entry to start on, or GUIDE_NONE

// Online requests of the guide (src/guide_social.c).
extern const struct OnlineRequest gGuideRequests[ONL_REQ_GUIDE_COUNT];

#endif // GUARD_GUIDE_H
