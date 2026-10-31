#ifndef GUARD_CONSTANTS_COMMUNITY_H
#define GUARD_CONSTANTS_COMMUNITY_H

// The community features (docs/community.md). Each feature has its own
// block of request types; the server's handler for each lives in
// server/community_<feature>.py.

// Answers in gCommunityMailbox.respStatus (and VAR_RESULT after
// Community_ScriptRequest). Features add their own from 2 up.
#define COMMUNITY_OK      0
#define COMMUNITY_OFFLINE 1 // not online, or no answer in time

// Requests (gCommunityMailbox.reqType).
#define COMMUNITY_REQ_NONE           0
#define COMMUNITY_REQ_TOUR_STATUS    1  // tournament: what's on and where I stand
#define COMMUNITY_REQ_TOUR_JOIN      2
#define COMMUNITY_REQ_TOUR_LEAVE     3
#define COMMUNITY_REQ_TOUR_BRACKET   4
#define COMMUNITY_REQ_DAYCARE_STATUS 10 // shared daycare
#define COMMUNITY_REQ_DAYCARE_LIST   11
#define COMMUNITY_REQ_DAYCARE_LEAVE  12
#define COMMUNITY_REQ_DAYCARE_EGG    13
#define COMMUNITY_REQ_DAYCARE_TAKE   14
#define COMMUNITY_REQ_BOSS_STATUS    20 // server-wide boss
#define COMMUNITY_REQ_BOSS_DAMAGE    21
#define COMMUNITY_REQ_BOSS_CLAIM     22
#define COMMUNITY_REQ_CARD_PUBLISH   30 // trainer cards
#define COMMUNITY_REQ_CARD_LIST      31
#define COMMUNITY_REQ_CARD_VIEW      32
#define COMMUNITY_REQ_CONTEST_ENTER  40 // online contests
#define COMMUNITY_REQ_CONTEST_BOARD  41
#define COMMUNITY_REQ_CHAT_ROOMS     50 // chat and lobby rooms
#define COMMUNITY_REQ_CHAT_JOIN      51
#define COMMUNITY_REQ_CHAT_SAY       52
#define COMMUNITY_REQ_CHAT_LEAVE     53
#define COMMUNITY_REQ_LEGAL_CHECK    60 // anti-cheat

// Event sections the server writes into without being asked
// (gCommunityMailbox.events).
#define COMMUNITY_EVENT_CHAT  0
#define COMMUNITY_EVENT_BOSS  1
#define COMMUNITY_EVENT_TOUR  2
#define COMMUNITY_EVENT_SPARE 3
#define COMMUNITY_EVENT_COUNT 4

// START > ONLINE > COMMUNITY rows (src/community.c).
#define COMMUNITY_MENU_TOURNAMENT 0
#define COMMUNITY_MENU_DAYCARE    1
#define COMMUNITY_MENU_BOSS       2
#define COMMUNITY_MENU_CARDS      3
#define COMMUNITY_MENU_CONTEST    4
#define COMMUNITY_MENU_CHAT       5
#define COMMUNITY_MENU_LEGAL      6
#define COMMUNITY_MENU_COUNT      7

// Flags and vars the community features use.
#define FLAG_COMMUNITY_RIVAL_1       FLAG_UNUSED_0x1DE // postgame rival, one per spot
#define FLAG_COMMUNITY_RIVAL_2       FLAG_UNUSED_0x1DF
#define FLAG_COMMUNITY_RIVAL_3       FLAG_UNUSED_0x1E0
#define FLAG_COMMUNITY_RIVAL_4       FLAG_UNUSED_0x1E1
#define FLAG_COMMUNITY_RIVAL_MET     FLAG_UNUSED_0x1E2 // the postgame rival has said hello
#define FLAG_COMMUNITY_NO_CATCHING   FLAG_UNUSED_0x1E3 // the server boss can't be caught
#define FLAG_COMMUNITY_SPARE_2       FLAG_UNUSED_0x1AA
#define FLAG_COMMUNITY_SPARE_3       FLAG_UNUSED_0x1AB
#define VAR_COMMUNITY_RIVAL_WINS     VAR_UNUSED_0x40A8
#define VAR_COMMUNITY_SPARE         VAR_UNUSED_0x40B8

#endif // GUARD_CONSTANTS_COMMUNITY_H
