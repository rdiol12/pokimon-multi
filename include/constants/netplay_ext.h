#ifndef GUARD_CONSTANTS_NETPLAY_EXT_H
#define GUARD_CONSTANTS_NETPLAY_EXT_H

// Second batch of online features, shared between C and event scripts.
// See docs/netplay_ext.md.

// VAR_RESULT of NetplayExt_Request.
#define NETPLAY_EXT_OK       0
#define NETPLAY_EXT_REFUSED  1 // the server said no; its reason is in gStringVar4
#define NETPLAY_EXT_FAILED   2 // offline or no answer; the reason is in gStringVar4

// VAR_RESULT of NetplayExt_OpenPlaza: the feature picked.
#define PLAZA_EXIT     0
#define PLAZA_GUILDS   1
#define PLAZA_WONDER   2
#define PLAZA_BASES    3
#define PLAZA_TOWER    4
#define PLAZA_BOUNTIES 5
#define PLAZA_TITLES   6
#define PLAZA_REPLAYS  7
#define PLAZA_DRAFT    8
#define PLAZA_MAIL     9
#define PLAZA_DEX      10
#define PLAZA_COUNT    10

// VAR_0x8004 of NetplayExt_Request.
#define NPX_REQ_GUILD_INFO     0
#define NPX_REQ_GUILD_LIST     1
#define NPX_REQ_GUILD_CREATE   2
#define NPX_REQ_GUILD_JOIN     3
#define NPX_REQ_GUILD_LEAVE    4
#define NPX_REQ_TITLE_LIST     5
#define NPX_REQ_TITLE_SET      6
#define NPX_REQ_DEX_SUBMIT     7
#define NPX_REQ_DEX_BOARD      8
#define NPX_REQ_WONDER_CHECK   9
#define NPX_REQ_WONDER_SEND    10 // VAR_0x8005: party slot
#define NPX_REQ_BOUNTY_LIST    11
#define NPX_REQ_BOUNTY_POST    12 // VAR_0x8005: species, VAR_0x8006: reward / 100
#define NPX_REQ_BOUNTY_FILL    13 // VAR_0x8005: party slot; the chosen row is the bounty
#define NPX_REQ_BOUNTY_COLLECT 14
#define NPX_REQ_BOUNTY_CANCEL  15
#define NPX_REQ_MAIL_WHO       16
#define NPX_REQ_MAIL_SEND      17 // chosen row: recipient; VAR_0x8005 phrase, 0x8006 money / 100, 0x8007 item
#define NPX_REQ_MAIL_INBOX     18
#define NPX_REQ_MAIL_OPEN      19 // chosen row: the mail
#define NPX_REQ_BASE_PUT       20
#define NPX_REQ_BASE_LIST      21
#define NPX_REQ_BASE_GET       22 // chosen row: the base
#define NPX_REQ_TOWER_STATE    23
#define NPX_REQ_TOWER_START    24
#define NPX_REQ_TOWER_NEXT     25
#define NPX_REQ_TOWER_WON      26
#define NPX_REQ_TOWER_LOST     27
#define NPX_REQ_TOWER_BOARD    28
#define NPX_REQ_DRAFT_POOL     29
#define NPX_REQ_DRAFT_QUEUE    30
#define NPX_REQ_DRAFT_LEAVE    31
#define NPX_REQ_REPLAY_LIST    32
#define NPX_REQ_COUNT          33

// VAR_0x8004 of NetplayExt_ShowBoard.
#define NPX_BOARD_GUILD        0 // GUILD_INFO: my guild and its members
#define NPX_BOARD_GUILD_LADDER 1 // GUILD_LIST
#define NPX_BOARD_DEX          2 // DEX_BOARD
#define NPX_BOARD_TOWER        3 // TOWER_BOARD
#define NPX_BOARD_DRAFT_POOL   4 // DRAFT_POOL

// VAR_0x8004 of NetplayExt_ChooseRow: what the rows of the last answer are.
#define NPX_LIST_GUILDS    0
#define NPX_LIST_TITLES    1
#define NPX_LIST_BOUNTIES  2
#define NPX_LIST_PLAYERS   3
#define NPX_LIST_INBOX     4
#define NPX_LIST_BASES     5
#define NPX_LIST_REPLAYS   6
#define NPX_LIST_DRAFT     7
#define NPX_LIST_PHRASES   8
#define NPX_LIST_GIFT_ITEM 9
#define NPX_LIST_SPECIES   10

#define NPX_CHOSE_NOTHING 0xFF

#endif // GUARD_CONSTANTS_NETPLAY_EXT_H
