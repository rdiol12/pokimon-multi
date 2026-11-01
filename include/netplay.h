#ifndef GUARD_NETPLAY_H
#define GUARD_NETPLAY_H

#include "link.h"
#include "pokemon.h"
#include "constants/netplay.h"

struct ObjectEvent;

// Online multiplayer. The ROM talks to the outside world only through
// gNetplayMailbox, which tools/netplay_bridge/netplay_bridge.lua copies to
// and from the netplay server every frame. See docs/netplay.md.

#define NETPLAY_MAILBOX_VERSION 4
// Other players drawn on the overworld. The GBA's 16 objects allowed 8; the
// extended GBA allows 24. The first 8 slots are remotes[] where they always
// were, the rest moreRemotes[] at the end (mailbox version 4). Use
// Netplay_GetRemoteSlot() rather than indexing either array.
#define NETPLAY_MAX_REMOTES     24
#define NETPLAY_MAILBOX_REMOTES 8
#define NETPLAY_NAME_LENGTH     8
#define NETPLAY_RING_SIZE       16
#define NETPLAY_HOST_LENGTH     40
#define NETPLAY_LADDER_SIZE     10
#define NETPLAY_MAX_FRIENDS     10

// Object event local ids used for other players on the overworld (above the
// map's own objects, which stay below OBJECT_EVENT_TEMPLATES_COUNT).
#define LOCALID_NETPLAY_PLAYER_START 70
#define LOCALID_NETPLAY_PLAYER_END   (LOCALID_NETPLAY_PLAYER_START + NETPLAY_MAX_REMOTES - 1)

enum NetplayBridgeState
{
    NETPLAY_OFFLINE,
    NETPLAY_CONNECTING,
    NETPLAY_ONLINE,
};

enum NetplayRequestType
{
    NETPLAY_REQ_NONE,
    NETPLAY_REQ_CHALLENGE,
    NETPLAY_REQ_TRADE,
    NETPLAY_REQ_ACCEPT,
    NETPLAY_REQ_DECLINE,
    NETPLAY_REQ_RESULT,
    NETPLAY_REQ_LEAVE,
    NETPLAY_REQ_DEAL,
    NETPLAY_REQ_LADDER,
    NETPLAY_REQ_EMOTE,
    NETPLAY_REQ_FRIENDS,     // fetch the friend list
    NETPLAY_REQ_FRIEND_ADD,  // targetId: player to add
    NETPLAY_REQ_FRIEND_DEL,  // arg: index in the last friend list
    NETPLAY_REQ_SEASON,      // fetch the current season
    NETPLAY_REQ_CLAIM,       // claim the oldest unclaimed season reward
    NETPLAY_REQ_MARKET,      // arg: page of the market board
    NETPLAY_REQ_MARKET_SELL, // list market.mon for market.sellPrice
    NETPLAY_REQ_MARKET_BUY,  // arg: index on the current page
    NETPLAY_REQ_MARKET_CANCEL, // arg: index on the current page
    NETPLAY_REQ_MARKET_COLLECT, // money from sales while away
    NETPLAY_REQ_LIVE,        // list live PvP battles
    NETPLAY_REQ_WATCH,       // arg: index in the live list
    NETPLAY_REQ_UNWATCH,
    NETPLAY_REQ_COOP,        // target: the player to team up with
    NETPLAY_REQ_DOUBLE,      // target: the player to challenge to a ranked double battle
    NETPLAY_REQ_MULTI,       // target: a player of the team to challenge to a 2v2 team battle
};

enum NetplayEventType
{
    NETPLAY_EV_NONE,
    NETPLAY_EV_INVITE,
    NETPLAY_EV_START,
    NETPLAY_EV_DECLINED,
    NETPLAY_EV_ENDED,
    NETPLAY_EV_PENDING,
    NETPLAY_EV_EMOTE, // peerId did emote `kind`
};

enum NetplayResult
{
    NETPLAY_RESULT_NONE,
    NETPLAY_RESULT_WON,
    NETPLAY_RESULT_LOST,
    NETPLAY_RESULT_DRAW,
};

enum NetplayPacketKind
{
    NETPLAY_PACKET_NONE,
    NETPLAY_PACKET_DATA,
    NETPLAY_PACKET_HANDSHAKE,
    NETPLAY_PACKET_HANDSHAKE_ACK,
    NETPLAY_PACKET_DEAL_OFFER,  // filler1: chunk index, filler2: offer revision; cmd: 16 bytes of the offer
    NETPLAY_PACKET_DEAL_ANSWER, // cmd: accepted, our revision, their revision we accepted
    NETPLAY_PACKET_DEAL_COMMIT, // cmd: our revision, their revision
};

struct NetplayRemotePlayer
{
    /*0x00*/ u16 id; // 0 = empty slot
    /*0x02*/ u8 mapGroup;
    /*0x03*/ u8 mapNum;
    /*0x04*/ s16 x;
    /*0x06*/ s16 y;
    /*0x08*/ u8 facing;
    /*0x09*/ u8 elevation;
    /*0x0A*/ u16 graphicsId;
    /*0x0C*/ u8 name[NETPLAY_NAME_LENGTH];
}; // size 0x14

struct NetplayRequest
{
    /*0x00*/ u8 seq;
    /*0x01*/ u8 type;
    /*0x02*/ u16 targetId;
    /*0x04*/ u16 sessionId;
    /*0x06*/ u16 arg;
};

struct NetplayEvent
{
    /*0x00*/ u8 seq;
    /*0x01*/ u8 type;
    /*0x02*/ u16 peerId;
    /*0x04*/ u16 sessionId;
    /*0x06*/ u8 kind;
    /*0x07*/ u8 role;
};

struct NetplayPacket
{
    /*0x00*/ u8 kind;
    /*0x01*/ u8 filler1;
    /*0x02*/ u16 filler2;
    /*0x04*/ u16 cmd[CMD_LENGTH];
}; // size 0x14

struct NetplayRing
{
    u8 head;
    u8 tail;
    u16 filler;
    struct NetplayPacket packets[NETPLAY_RING_SIZE];
};

struct NetplayLadderEntry
{
    /*0x00*/ u8 name[NETPLAY_NAME_LENGTH];
    /*0x08*/ u16 rating;
    /*0x0A*/ u16 wins;
    /*0x0C*/ u16 losses;
    /*0x0E*/ u16 draws;
}; // size 0x10

// The PvP ladder, written by the bridge after a NETPLAY_REQ_LADDER.
struct NetplayLadder
{
    /*0x00*/ u8 seq; // bumped by the bridge when the ladder below is new
    /*0x01*/ u8 count;
    /*0x02*/ u16 myRank; // 0: no rated battles yet
    /*0x04*/ struct NetplayLadderEntry me;
    /*0x14*/ struct NetplayLadderEntry top[NETPLAY_LADDER_SIZE];
}; // size 0xB4

struct NetplayFriend
{
    /*0x00*/ u8 name[NETPLAY_NAME_LENGTH];
    /*0x08*/ u16 playerId; // 0: offline
    /*0x0A*/ u8 mapGroup;
    /*0x0B*/ u8 mapNum;
    /*0x0C*/ s16 x;
    /*0x0E*/ s16 y;
    /*0x10*/ u16 graphicsId;
    /*0x12*/ u8 busy; // in a battle, trade or deal
    /*0x13*/ u8 filler;
}; // size 0x14

// Written by the bridge after NETPLAY_REQ_FRIENDS / NETPLAY_REQ_FRIEND_ADD.
struct NetplayFriends
{
    /*0x00*/ u8 seq;       // bumped when the list below is new
    /*0x01*/ u8 count;
    /*0x02*/ u8 addSeq;    // bumped when the server answers a FRIEND_ADD
    /*0x03*/ u8 addResult; // NETPLAY_FRIEND_ADD_*
    /*0x04*/ struct NetplayFriend list[NETPLAY_MAX_FRIENDS];
}; // size 0xCC

#define NETPLAY_NO_TIER 0xFF

// Written by the bridge after NETPLAY_REQ_SEASON / NETPLAY_REQ_CLAIM.
struct NetplaySeason
{
    /*0x00*/ u8 seq;         // bumped when the fields below are new
    /*0x01*/ u8 rewardTier;  // unclaimed reward, or NETPLAY_NO_TIER
    /*0x02*/ u16 season;
    /*0x04*/ u16 daysLeft;
    /*0x06*/ u16 rating;
    /*0x08*/ u8 rated;       // played a rated battle this season
    /*0x09*/ u8 claimSeq;    // bumped when the server answers a claim
    /*0x0A*/ u8 claimedTier; // the reward just claimed, or NETPLAY_NO_TIER
    /*0x0B*/ u8 filler;
    /*0x0C*/ u16 rewardSeason;
    /*0x0E*/ u16 filler2;
}; // size 0x10

#define NETPLAY_MARKET_PAGE_SIZE 5

struct NetplayMarketListing
{
    /*0x00*/ u16 species;
    /*0x02*/ u8 level;
    /*0x03*/ u8 mine;
    /*0x04*/ u32 price;
    /*0x08*/ u8 seller[PLAYER_NAME_LENGTH + 1];
}; // size 0x10

// The market board (Poké Marts). The bridge publishes a page with seq; the
// answer to a sell, buy, cancel or collect comes with answerSeq.
struct NetplayMarket
{
    /*0x00*/ u8 seq;
    /*0x01*/ u8 count;
    /*0x02*/ u8 page;
    /*0x03*/ u8 pages;
    /*0x04*/ u8 answerSeq;
    /*0x05*/ u8 answer;      // NETPLAY_MARKET_*
    /*0x06*/ u16 sales;      // collect: listings sold while away
    /*0x08*/ u32 money;      // buy: price paid; collect: money earned
    /*0x0C*/ struct NetplayMarketListing listings[NETPLAY_MARKET_PAGE_SIZE];
    /*0x5C*/ u32 sellPrice;  // written by the ROM before a sell
    /*0x60*/ u16 sellSpecies;
    /*0x62*/ u8 sellLevel;
    /*0x63*/ u8 filler;
    /*0x64*/ struct Pokemon mon; // sell: written by the ROM; buy/cancel: by the bridge
}; // size 0xC8

#define NETPLAY_BATTLE_TEXT_LENGTH 64
#define NETPLAY_MAX_LIVE_BATTLES   5

struct NetplayBattleSide
{
    /*0x00*/ u16 species;
    /*0x02*/ u8 level;
    /*0x03*/ u8 alive;     // party Pokémon that can still fight
    /*0x04*/ u16 hp;
    /*0x06*/ u16 maxHP;
    /*0x08*/ u8 partySize;
    /*0x09*/ u8 filler[3];
}; // size 0x0C

// A PvP battle as spectators see it: the active Pokémon of the host (side 0)
// and the guest (side 1), and the battle's latest message.
struct NetplayBattleReport
{
    /*0x00*/ u8 seq;  // bumped on each change
    /*0x01*/ u8 filler[3];
    /*0x04*/ struct NetplayBattleSide sides[2];
    /*0x1C*/ u8 text[NETPLAY_BATTLE_TEXT_LENGTH]; // game charset, EOS-terminated
}; // size 0x5C

struct NetplayLiveBattle
{
    /*0x00*/ u16 sessionId;
    /*0x02*/ u8 host[PLAYER_NAME_LENGTH + 1];
    /*0x0A*/ u8 guest[PLAYER_NAME_LENGTH + 1];
    /*0x12*/ u16 filler;
}; // size 0x14

// Spectating: the live battle list, and the battle being watched.
struct NetplayWatch
{
    /*0x00*/ u8 listSeq;
    /*0x01*/ u8 count;
    /*0x02*/ u8 ended;   // the watched battle is over
    /*0x03*/ u8 filler;
    /*0x04*/ struct NetplayLiveBattle list[NETPLAY_MAX_LIVE_BATTLES];
    /*0x68*/ struct NetplayBattleReport view;
}; // size 0xC4

struct NetplayMailbox
{
    /*0x000*/ char magic[8];
    /*0x008*/ u8 version;
    /*0x009*/ u8 bridgeState;
    /*0x00A*/ u16 localId;
    /*0x00C*/ u8 name[NETPLAY_NAME_LENGTH];
    /*0x014*/ u16 trainerId;
    /*0x016*/ u8 mapGroup;
    /*0x017*/ u8 mapNum;
    /*0x018*/ s16 x;
    /*0x01A*/ s16 y;
    /*0x01C*/ u8 facing;
    /*0x01D*/ u8 elevation;
    /*0x01E*/ u16 graphicsId;
    /*0x020*/ struct NetplayRemotePlayer remotes[NETPLAY_MAILBOX_REMOTES];
    /*0x0C0*/ struct NetplayRequest request;
    /*0x0C8*/ struct NetplayEvent event;
    /*0x0D0*/ u8 eventAck;
    /*0x0D1*/ u8 testBoot;   // see Netplay_TryTestBoot
    /*0x0D2*/ u8 joinOnline; // the bridge connects only once this is set
    /*0x0D3*/ u8 filler;
    /*0x0D4*/ struct NetplayRing tx;
    /*0x218*/ struct NetplayRing rx;
    /*0x35C*/ char serverHost[NETPLAY_HOST_LENGTH]; // ASCII, NUL-terminated
    /*0x384*/ u16 serverPort;
    /*0x386*/ u16 filler2;
    /*0x388*/ struct NetplayLadder ladder;
    /*0x43C*/ struct NetplayFriends friends;
    /*0x508*/ struct NetplaySeason season;
    /*0x518*/ struct NetplayMarket market;
    /*0x5E0*/ struct NetplayBattleReport battleReport; // written by the host of a PvP battle
    /*0x63C*/ struct NetplayWatch watch;
    /*0x700*/ struct NetplayRemotePlayer moreRemotes[NETPLAY_MAX_REMOTES - NETPLAY_MAILBOX_REMOTES];
}; // size 0x840

extern struct NetplayMailbox gNetplayMailbox;

static inline struct NetplayRemotePlayer *Netplay_GetRemoteSlot(u32 slot)
{
    if (slot < NETPLAY_MAILBOX_REMOTES)
        return &gNetplayMailbox.remotes[slot];
    return &gNetplayMailbox.moreRemotes[slot - NETPLAY_MAILBOX_REMOTES];
}

void Netplay_Init(void);
void Netplay_Update(void);
bool32 Netplay_OverworldUpdate(void);
bool32 Netplay_IsNetplayObjectEvent(const struct ObjectEvent *objectEvent);
u32 Netplay_GetPeerTrainerLook(void);
const u8 *Netplay_GetTalkScript(u8 objectEventId);
bool32 Netplay_InSession(void);
void Netplay_OnBattleEnd(u8 battleOutcome);
void Netplay_EndSession(void);
bool32 Netplay_TryTestBoot(u8 taskId);
bool32 Netplay_IsTestBoot(void);

// Link transport used by link.c instead of the serial port while a session is active.
bool32 Netplay_IsLinkActive(void);
u8 Netplay_GetLinkRole(void);
u8 Netplay_GetLinkPlayerCount(void);
void Netplay_LinkReset(void);
bool32 Netplay_LinkHandshake(void);
void Netplay_LinkTransfer(void);

void Netplay_JoinOnline(void);
bool32 Netplay_HasJoined(void);
void ShowNetplayLadder(void);
bool32 Netplay_TryOpenEmoteMenu(void);
void ShowNetplayFriends(void);
void ShowNetplaySeason(void);
void ShowNetplayWatch(void);
bool32 Netplay_IsCoopBattle(void);
bool32 Netplay_MarketAvailable(void);
bool32 Netplay_IsOnline(void);

#endif // GUARD_NETPLAY_H
