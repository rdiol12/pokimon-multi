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
#define ONL_REQ_SHOP_LIST   13
#define ONL_REQ_SHOP_OPEN   14 // the shop picked from SHOP_LIST
#define ONL_REQ_SHOP_ITEMS  15 // the shop opened last
#define ONL_REQ_SHOP_BUY    16 // VAR_0x8005: item, VAR_0x8006: how many; the shelf picked
#define ONL_REQ_SHOP_MINE   17
#define ONL_REQ_SHOP_PRICES 18 // VAR_0x8005: item, VAR_0x8006: how many, VAR_0x8007: mart price
#define ONL_REQ_SHOP_STOCK  19 // VAR_0x8005: item, VAR_0x8006: how many; the price picked
#define ONL_REQ_SHOP_TAKE   20 // the shelf picked from SHOP_MINE
#define ONL_REQ_SHOP_COLLECT 21
#define ONL_REQ_PAGEANT_ENTER 22 // VAR_0x8005: party slot
#define ONL_REQ_PAGEANT_SHOW  23 // VAR_0x8005: index
#define ONL_REQ_PAGEANT_VOTE  24 // VAR_0x8005: index, VAR_0x8006: entry
#define ONL_REQ_PAGEANT_BOARD 25
#define ONL_REQ_PAGEANT_CLAIM 26
#define ONL_REQ_DUO_VIEW    27
#define ONL_REQ_DUO_WHO     28
#define ONL_REQ_DUO_ASK     29 // the player picked from DUO_WHO
#define ONL_REQ_DUO_YES     30
#define ONL_REQ_DUO_NO      31
#define ONL_REQ_DUO_CLAIM   32
#define ONL_REQ_DUO_LEAVE   33
#define ONL_REQ_MENTOR_VIEW  34 // every MENTOR request also sends the badge count
#define ONL_REQ_MENTOR_FIND  35
#define ONL_REQ_MENTOR_PICK  36 // the mentor picked from MENTOR_FIND
#define ONL_REQ_MENTOR_VOLUNTEER 37
#define ONL_REQ_MENTOR_WHO   38
#define ONL_REQ_MENTOR_CHEER 39 // the apprentice picked from MENTOR_WHO
#define ONL_REQ_MENTOR_GIFTS 40
#define ONL_REQ_MENTOR_LEAVE 41
#define ONL_REQ_PQUEST_LIST  42
#define ONL_REQ_PQUEST_SHOW  43 // the quest picked from PQUEST_LIST
#define ONL_REQ_PQUEST_TAKE  44 // the same
#define ONL_REQ_PQUEST_DROP  45
#define ONL_REQ_PQUEST_HINTS 46
#define ONL_REQ_PQUEST_MAKE  47 // VAR_0x8005: item, VAR_0x8006: how many; the hint picked
#define ONL_REQ_PQUEST_MINE  48
#define ONL_REQ_PQUEST_CANCEL 49 // the quest picked from PQUEST_MINE
#define ONL_REQ_PQUEST_FIND  50
#define ONL_REQ_COUNT       51

// The hub's rows (VAR_RESULT of its menu).
#define ONLINE_HUB_GOALS    0
#define ONLINE_HUB_SIGHTINGS 1
#define ONLINE_HUB_FISHING  2
#define ONLINE_HUB_COFISH   3
#define ONLINE_HUB_SHOPS    4
#define ONLINE_HUB_PAGEANT  5
#define ONLINE_HUB_DUO      6
#define ONLINE_HUB_MENTORS  7
#define ONLINE_HUB_PQUESTS  8
#define ONLINE_HUB_CLOSE    127

// VAR_0x800B of the co-op fishing requests.
#define COFISH_WAITING  0
#define COFISH_CAUGHT   1
#define COFISH_GOT_AWAY 2
#define COFISH_DECLINED 3

// VAR_0x800B of DUO_VIEW.
#define DUO_NONE    0
#define DUO_INVITED 1 // someone asked the player; gStringVar1 is who
#define DUO_ASKED   2 // the player asked someone
#define DUO_ACTIVE  3
#define DUO_REWARD  4 // active, and a reward is waiting

// VAR_0x800B of MENTOR_VIEW.
#define MENTOR_NEW        0 // no mentor, too few badges to be one
#define MENTOR_APPRENTICE 1 // has a mentor (gStringVar1)
#define MENTOR_ON_LIST    2 // a mentor newer trainers can pick
#define MENTOR_OFF_LIST   3 // enough badges, not on the list

#endif // GUARD_CONSTANTS_ONLINE_H
