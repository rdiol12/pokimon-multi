#ifndef GUARD_CONSTANTS_NETPLAY_H
#define GUARD_CONSTANTS_NETPLAY_H

// Kinds of online sessions, shared between C and event scripts.
#define NETPLAY_KIND_NONE   0
#define NETPLAY_KIND_BATTLE 1
#define NETPLAY_KIND_TRADE  2
#define NETPLAY_KIND_DEAL   3 // Pokémon and/or money, for Pokémon and/or money

// Money a player can put into a deal, picked by index from a menu.
#define NETPLAY_DEAL_MONEY_CHOICES 10

// VAR_RESULT of Netplay_WaitForResponse.
#define NETPLAY_RESPONSE_DECLINED 0
#define NETPLAY_RESPONSE_ACCEPTED 1
#define NETPLAY_RESPONSE_CANCELED 2

#endif // GUARD_CONSTANTS_NETPLAY_H
