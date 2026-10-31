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
#define ONL_REQ_COUNT       2

// The hub's rows (VAR_RESULT of its menu).
#define ONLINE_HUB_GOALS    0
#define ONLINE_HUB_CLOSE    127

#endif // GUARD_CONSTANTS_ONLINE_H
