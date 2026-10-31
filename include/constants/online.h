#ifndef GUARD_CONSTANTS_ONLINE_H
#define GUARD_CONSTANTS_ONLINE_H

// Sixth batch of online features (docs/online.md), shared between C and
// event scripts.

// VAR_RESULT of Online_Request.
#define ONLINE_OK      0
#define ONLINE_REFUSED 1 // the server said no; its reason is in gStringVar4
#define ONLINE_FAILED  2 // offline or no answer; the reason is in gStringVar4

// VAR_RESULT of Online_Choose when nothing was picked.
#define ONLINE_CHOSE_NOTHING 0xFF

// Online_Request, VAR_0x8004. Arguments, where a request takes any, are in
// VAR_0x8005 onwards (see gOnlineRequests in src/online_requests.c).
#define ONL_REQ_GOAL_VIEW   0
#define ONL_REQ_GOAL_CLAIM  1
#define ONL_REQ_SIGHT_BOARD 2
#define ONL_REQ_FISHT_BOARD 3
#define ONL_REQ_FISHT_CLAIM 4
#define ONL_REQ_COFISH_WHO  5
#define ONL_REQ_COFISH_ASK  6 // the player picked from COFISH_WHO
#define ONL_REQ_COFISH_JOIN 7
#define ONL_REQ_COFISH_NO   8
#define ONL_REQ_COFISH_WAIT 9
#define ONL_REQ_COFISH_REEL 10 // VAR_0x8005: hits
#define ONL_REQ_COFISH_RESULT 11
#define ONL_REQ_COFISH_BOARD 12
#define ONL_REQ_COUNT       13

// The hub's rows (VAR_RESULT of its menu).
#define ONLINE_HUB_GOALS    0
#define ONLINE_HUB_SIGHTINGS 1
#define ONLINE_HUB_FISHING  2
#define ONLINE_HUB_COFISH   3
#define ONLINE_HUB_CLOSE    127

// VAR_0x800B of the co-op fishing requests.
#define COFISH_WAITING  0
#define COFISH_CAUGHT   1
#define COFISH_GOT_AWAY 2
#define COFISH_DECLINED 3

#endif // GUARD_CONSTANTS_ONLINE_H
