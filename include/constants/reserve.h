#ifndef GUARD_CONSTANTS_RESERVE_H
#define GUARD_CONSTANTS_RESERVE_H

// The Haven Reserve (docs/reserve.md): a guarded meadow west of Verdanturf
// where trainers leave Pokémon they don't want. The left Pokémon roam
// inside it, shared by every player through the server
// (server/community_reserve.py), and anyone can walk in and battle or
// catch them.

// Pokémon roaming the reserve at once: objects 1-8 of MAP_HAVEN_RESERVE,
// drawn with VAR_OBJ_GFX_ID_0..7 and hidden with FLAG_TEMP_11..18.
#define RESERVE_SHOWN        8
#define RESERVE_FIRST_LOCALID 1
#define FLAG_RESERVE_HIDE_1  FLAG_TEMP_11

// Pokémon left while offline (kept in the save, SaveBlock3 reserve).
#define RESERVE_LOCAL_MAX    6
// Strays: wild Pokémon that wander in on their own, new ones each day.
#define RESERVE_STRAYS       3

// Requests (gCommunityMailbox.reqType, include/constants/community.h).
#define COMMUNITY_REQ_RESERVE_LIST   75 // who roams the reserve now
#define COMMUNITY_REQ_RESERVE_LEAVE  76 // leave a Pokémon there
#define COMMUNITY_REQ_RESERVE_CLAIM  77 // start a battle with one
#define COMMUNITY_REQ_RESERVE_RESULT 78 // how that battle ended
#define COMMUNITY_REQ_RESERVE_LOG    79 // the notice board

// Answer statuses (after COMMUNITY_OK / COMMUNITY_OFFLINE).
#define RESERVE_GONE 2 // someone else got to it first / it left
#define RESERVE_FULL 3 // the reserve is full

// What a roaming Pokémon is (VAR_RESULT after Reserve_DescribeRoamer).
#define RESERVE_KIND_SHARED 0 // left by a trainer, on the server
#define RESERVE_KIND_MINE   1 // you left it here (online or offline)
#define RESERVE_KIND_LOCAL  2 // you left it while offline
#define RESERVE_KIND_STRAY  3 // wandered in on its own
#define RESERVE_KIND_NONE   4

// How a battle with one ended (VAR_RESULT after Reserve_AfterBattle), and
// what the server is told (COMMUNITY_REQ_RESERVE_RESULT a1).
#define RESERVE_END_STILL_HERE 0 // ran away or lost: it stays
#define RESERVE_END_CAUGHT     1
#define RESERVE_END_KNOCKED_OUT 2

// Reserve_CanLeave answers (VAR_RESULT).
#define RESERVE_LEAVE_OK        0
#define RESERVE_LEAVE_EGG       1
#define RESERVE_LEAVE_LAST      2 // the last Pokémon that can battle
#define RESERVE_LEAVE_LOCAL_FULL 3

#endif // GUARD_CONSTANTS_RESERVE_H
