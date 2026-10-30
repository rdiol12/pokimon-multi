#ifndef GUARD_CONSTANTS_NETPLAY_H
#define GUARD_CONSTANTS_NETPLAY_H

// Kinds of online sessions, shared between C and event scripts.
#define NETPLAY_KIND_NONE   0
#define NETPLAY_KIND_BATTLE 1
#define NETPLAY_KIND_TRADE  2
#define NETPLAY_KIND_DEAL   3 // Pokémon and/or money, for Pokémon and/or money

// Not a session: the talk menu's "Add friend".
#define NETPLAY_KIND_FRIEND 10

// VAR_RESULT of Netplay_WaitForFriendAdd, and NetplayFriends.addResult.
#define NETPLAY_FRIEND_ADD_NO_ANSWER 0
#define NETPLAY_FRIEND_ADD_OK        1
#define NETPLAY_FRIEND_ADD_FULL      2

// VAR_RESULT of Netplay_DealScreen.
#define NETPLAY_DEAL_QUIT      0
#define NETPLAY_DEAL_ADD_MON   1
#define NETPLAY_DEAL_DONE      2
#define NETPLAY_DEAL_PEER_LEFT 3

// VAR_RESULT of Netplay_WaitForResponse.
#define NETPLAY_RESPONSE_DECLINED 0
#define NETPLAY_RESPONSE_ACCEPTED 1
#define NETPLAY_RESPONSE_CANCELED 2

#endif // GUARD_CONSTANTS_NETPLAY_H
