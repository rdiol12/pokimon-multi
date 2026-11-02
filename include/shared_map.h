#ifndef GUARD_SHARED_MAP_H
#define GUARD_SHARED_MAP_H

#include "netplay.h"

// Shared wild spawns and map channels (docs/shared_map.md).
//
// Online, the visible wild Pokémon this game makes (src/wild_encounter_ow.c)
// are reported to the server, and everyone else's on the same map and
// channel are drawn here too. The first player to battle one claims it.
// A busy map is split into channels; START > ONLINE > CHANNEL moves you.
//
// The bridge (tools/netplay_bridge/netplay_world.lua, NetplayShared) finds
// gSharedMapMailbox through gWorldMailbox.shared.

#define SHARED_MAP_VERSION        1
#define SHARED_OWN_SPAWNS         4  // OWE_SPAWNS_MAX
#define SHARED_REMOTE_SPAWNS      8
#define SHARED_CHANNEL_ROWS       8
#define LOCALID_SHARED_WILD_FIRST 241 // 241-248, between the Berry Blender players and the generated OWEs
#define LOCALID_SHARED_WILD_LAST  (LOCALID_SHARED_WILD_FIRST + SHARED_REMOTE_SPAWNS - 1)

// SharedOwnSpawn.flags and SharedRemoteSpawn.flags
#define SHARED_SPAWN_RARE   (1 << 0)
#define SHARED_SPAWN_ASLEEP (1 << 1)
#define SHARED_SPAWN_WATER  (1 << 2)

// SharedOwnSpawn.claim (written by the bridge)
#define SHARED_CLAIM_FREE   0
#define SHARED_CLAIM_OTHER  1 // someone else is battling it
#define SHARED_CLAIM_TAKEN  2 // someone else caught or beat it: remove it

enum SharedMapRequest
{
    SHARED_REQ_NONE,
    SHARED_REQ_CLAIM,     // owner, uid
    SHARED_REQ_END,       // owner, uid, arg: SHARED_RESULT_*
    SHARED_REQ_CHANNELS,  // list the channels of this map
    SHARED_REQ_JOIN,      // arg: channel
};

#define SHARED_RESULT_CAUGHT 1
#define SHARED_RESULT_BEATEN 2
#define SHARED_RESULT_RAN    3

#define SHARED_CLAIMED_OK    1
#define SHARED_CLAIMED_TAKEN 2
#define SHARED_CLAIMED_GONE  3

struct SharedOwnSpawn
{
    /*0x0*/ u16 uid;    // 0: empty
    /*0x2*/ u16 gfx;    // graphics id: species, shiny and female (OBJ_EVENT_MON...)
    /*0x4*/ s16 x;      // map coordinates, no MAP_OFFSET
    /*0x6*/ s16 y;
    /*0x8*/ u8 level;
    /*0x9*/ u8 flags;   // SHARED_SPAWN_*
    /*0xA*/ u8 facing;
    /*0xB*/ u8 claim;   // SHARED_CLAIM_*, by the bridge
}; // size 0xC

struct SharedRemoteSpawn
{
    /*0x00*/ u16 owner; // player id, 0: empty
    /*0x02*/ u16 uid;
    /*0x04*/ u16 gfx;
    /*0x06*/ s16 x;
    /*0x08*/ s16 y;
    /*0x0A*/ u8 level;
    /*0x0B*/ u8 flags;
    /*0x0C*/ u8 facing;
    /*0x0D*/ u8 claimed; // someone is battling it
    /*0x0E*/ u8 mapGroup;
    /*0x0F*/ u8 mapNum;
    /*0x10*/ u8 claimer[NETPLAY_NAME_LENGTH];
}; // size 0x18

struct SharedChannelRow
{
    /*0x0*/ u8 channel;
    /*0x1*/ u8 players;
    /*0x2*/ u8 friends;
    /*0x3*/ u8 pad;
    /*0x4*/ u8 friendName[NETPLAY_NAME_LENGTH]; // the first friend there
}; // size 0xC

struct SharedMapMailbox
{
    /*0x000*/ char magic[8];   // "PKMNSHR1"
    /*0x008*/ u8 version;
    /*0x009*/ u8 reqSeq;       // bumped by the ROM when the request below is new
    /*0x00A*/ u8 reqType;      // SHARED_REQ_*
    /*0x00B*/ u8 reqArg;
    /*0x00C*/ u16 reqOwner;
    /*0x00E*/ u16 reqUid;
    /*0x010*/ u8 reqAck;       // set to reqSeq by the bridge once sent
    /*0x011*/ u8 claimSeq;     // bumped by the bridge when a claim is answered
    /*0x012*/ u8 claimResult;  // SHARED_CLAIMED_*
    /*0x013*/ u8 channel;      // this player's channel on this map (0: not told, so 1)
    /*0x014*/ u8 channelCount; // channels on this map
    /*0x015*/ u8 channelSeq;   // bumped by the bridge when the two above change
    /*0x016*/ u8 listSeq;      // bumped by the bridge when rows[] or joinResult is new
    /*0x017*/ u8 listCount;
    /*0x018*/ u8 claimer[NETPLAY_NAME_LENGTH]; // who has it (SHARED_CLAIMED_TAKEN)
    /*0x020*/ u8 rareSeq;      // bumped by the bridge: a rare one just appeared here
    /*0x021*/ u8 joinResult;   // 1 moved, 2 full
    /*0x022*/ u16 localId;     // the bridge's copy of this player's id
    /*0x024*/ u16 rareOwner;
    /*0x026*/ u16 rareUid;
    /*0x028*/ struct SharedOwnSpawn own[SHARED_OWN_SPAWNS];
    /*0x058*/ struct SharedRemoteSpawn remote[SHARED_REMOTE_SPAWNS];
    /*0x118*/ struct SharedChannelRow rows[SHARED_CHANNEL_ROWS];
    /*0x178*/ u8 ownClaimer[SHARED_OWN_SPAWNS][NETPLAY_NAME_LENGTH]; // who is battling own[i]
}; // size 0x198

extern struct SharedMapMailbox gSharedMapMailbox;

// Screenshot scenes (tools/shared_map/shots.lua) set these.
struct SharedMapShowcase
{
    u8 shinyNext; // the next spawn this game makes is shiny
    u8 noSpawns;  // this game makes none (only sees others')
    u8 pad[2];
};
extern struct SharedMapShowcase gSharedMapShowcase;

struct ObjectEvent;

void SharedMap_Init(void);
void SharedMap_OverworldUpdate(void);
void SharedMap_OnMapTransition(void);
bool32 SharedMap_HoldLocalSpawns(void);
bool32 SharedMap_IsRemoteSpawn(const struct ObjectEvent *obj);
bool32 SharedMap_IsOwnHeld(const struct ObjectEvent *owe);
void SharedMap_OnWildBattleEnd(void);
bool32 SharedMap_IsWatchableBattle(void);
void SharedMap_GetWatchableOpponent(u8 *dest);
void ShowSharedMapChannels(void);

extern const u8 SharedMap_EventScript_Remote[];

#endif // GUARD_SHARED_MAP_H
