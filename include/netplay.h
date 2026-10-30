#ifndef GUARD_NETPLAY_H
#define GUARD_NETPLAY_H

#include "link.h"
#include "constants/netplay.h"

struct ObjectEvent;

// Online multiplayer. The ROM talks to the outside world only through
// gNetplayMailbox, which tools/netplay_bridge/netplay_bridge.lua copies to
// and from the netplay server every frame. See docs/netplay.md.

#define NETPLAY_MAILBOX_VERSION 2
#define NETPLAY_MAX_REMOTES     8
#define NETPLAY_NAME_LENGTH     8
#define NETPLAY_RING_SIZE       16
#define NETPLAY_HOST_LENGTH     40
#define NETPLAY_LADDER_SIZE     10

// Object event local ids used for other players on the overworld.
#define LOCALID_NETPLAY_PLAYER_START 228
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
    /*0x020*/ struct NetplayRemotePlayer remotes[NETPLAY_MAX_REMOTES];
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
}; // size 0x43C

extern struct NetplayMailbox gNetplayMailbox;

void Netplay_Init(void);
void Netplay_Update(void);
bool32 Netplay_OverworldUpdate(void);
bool32 Netplay_IsNetplayObjectEvent(const struct ObjectEvent *objectEvent);
const u8 *Netplay_GetTalkScript(u8 objectEventId);
bool32 Netplay_InSession(void);
void Netplay_OnBattleEnd(u8 battleOutcome);
void Netplay_EndSession(void);
bool32 Netplay_TryTestBoot(u8 taskId);

// Link transport used by link.c instead of the serial port while a session is active.
bool32 Netplay_IsLinkActive(void);
u8 Netplay_GetLinkRole(void);
void Netplay_LinkReset(void);
bool32 Netplay_LinkHandshake(void);
void Netplay_LinkTransfer(void);

void Netplay_JoinOnline(void);
bool32 Netplay_HasJoined(void);
void ShowNetplayLadder(void);
bool32 Netplay_TryOpenEmoteMenu(void);

#endif // GUARD_NETPLAY_H
