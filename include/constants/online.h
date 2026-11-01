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
#define ONL_REQ_BET_LIST    51
#define ONL_REQ_BET_SHOW    52 // the battle picked from BET_LIST
#define ONL_REQ_BET_PLACE   53 // VAR_0x8005: coins; the side picked from BET_SHOW
#define ONL_REQ_BET_COLLECT 54
#define ONL_REQ_BET_BOARD   55
#define ONL_REQ_DOUBLES_BOARD 56
#define ONL_REQ_TEAM_VIEW   57
#define ONL_REQ_TEAM_WHO    58
#define ONL_REQ_TEAM_ASK    59 // the player picked from TEAM_WHO
#define ONL_REQ_TEAM_YES    60
#define ONL_REQ_TEAM_NO     61
#define ONL_REQ_TEAM_LEAVE  62
#define ONL_REQ_TEAM_BOARD  63
#define ONL_REQ_GHALL_VIEW    64
#define ONL_REQ_GHALL_MEMBERS 65
#define ONL_REQ_GHALL_CHEST   66
#define ONL_REQ_GHALL_PUT     67 // VAR_0x8005 item, VAR_0x8006 count
#define ONL_REQ_GHALL_TAKE    68 // the item picked from GHALL_CHEST
#define ONL_REQ_WAR_BOARD     69
#define ONL_REQ_WAR_CLAIM     70
#define ONL_REQ_RAID_DEFEND   71
#define ONL_REQ_RAID_LIST     72
#define ONL_REQ_RAID_GET      73 // the base picked from RAID_LIST
#define ONL_REQ_RAID_RESULT   74 // VAR_0x8005: TRUE if the raid won
#define ONL_REQ_RACE_LIST     75
#define ONL_REQ_RACE_JOIN     76 // the course picked from RACE_LIST
#define ONL_REQ_RACE_BOARD    77 // the course picked from RACE_LIST
#define ONL_REQ_RACE_FINISH   78 // VAR_0x8005 course, VAR_0x8006 time in frames
#define ONL_REQ_RACE_QUIT     79
#define ONL_REQ_HIDE_LIST     80
#define ONL_REQ_HIDE_JOIN     81 // the arena picked from HIDE_LIST
#define ONL_REQ_HIDE_GO       82
#define ONL_REQ_HIDE_TAG      83 // VAR_0x8007: the player talked to
#define ONL_REQ_HIDE_END      84
#define ONL_REQ_HIDE_QUIT     85
#define ONL_REQ_HIDE_BOARD    86
#define ONL_REQ_ECO_AREA      87
#define ONL_REQ_ECO_MOVES     88
#define ONL_REQ_RESEARCH_BOARD 89 // VAR_0x8005: species the player completed
#define ONL_REQ_COUNT         90

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
#define ONLINE_HUB_BETTING  9
#define ONLINE_HUB_DOUBLES  10
#define ONLINE_HUB_TEAMS    11
#define ONLINE_HUB_GUILD    12
#define ONLINE_HUB_RACING   13
#define ONLINE_HUB_HIDE     14
#define ONLINE_HUB_WILDLIFE 15
#define ONLINE_HUB_RESEARCH 16
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

// VAR_0x800B of TEAM_VIEW.
#define TEAM_NONE    0
#define TEAM_INVITED 1 // someone asked the player; gStringVar1 is who
#define TEAM_ASKED   2 // the player asked someone
#define TEAM_TEAMED  3 // on a team with gStringVar1

// VAR_0x800B of GHALL_VIEW. gStringVar1: the guild's tag, gStringVar2: its name.
#define GUILD_NONE   0
#define GUILD_MEMBER 1
#define GUILD_LEADER 2

// Mount racing (src/online_racing.c).
#define RACE_COURSES 3
#define RACE_LANES   4 // racers in a heat
#define RACE_GATES   4 // the last is the finish

#endif // GUARD_CONSTANTS_ONLINE_H
