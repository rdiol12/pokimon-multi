#ifndef GUARD_WORLD_MAILBOX_H
#define GUARD_WORLD_MAILBOX_H

#include "netplay.h"
#include "world_save.h"

// The online side of the world features. Like gNetplayMailbox, the ROM only
// talks to the outside through this block of EWRAM, which
// tools/netplay_bridge/netplay_world.lua syncs with the server's world
// commands (server/world.py). Layout: docs/world.md.

#define WORLD_MAILBOX_VERSION 1
#define WORLD_BOARD_ROWS      10

enum WorldRequestType
{
    WORLD_REQ_NONE,
    WORLD_REQ_SCORE, // board, key, value, extra
    WORLD_REQ_BOARD, // board, key
};

// What another player online looks like, by netplay player id.
struct WorldRemoteLook
{
    /*0x00*/ u16 id; // 0 = empty
    /*0x02*/ u16 followerGfx; // OBJ_EVENT_MON + species (+ shiny/female bits), 0 = none
    /*0x04*/ u16 mountGfx;    // 0 = on foot
    /*0x06*/ u8 houseOpen;
    /*0x07*/ u8 accessory;  // what the follower wears (src/pastime_fashion.c) + 1, 0 = none
    /*0x08*/ u8 decor[WORLD_HOUSE_SLOTS];
}; // size 0x18

struct WorldBoardRow
{
    /*0x00*/ u8 name[NETPLAY_NAME_LENGTH];
    /*0x08*/ u32 value;
    /*0x0C*/ u32 extra;
}; // size 0x10

struct WorldMailbox
{
    /*0x000*/ char magic[8]; // "PKMNWLD1"
    /*0x008*/ u8 version;
    /*0x009*/ u8 reqSeq;     // bumped by the ROM when the request below is new
    /*0x00A*/ u8 reqType;
    /*0x00B*/ u8 reqBoard;
    /*0x00C*/ u16 reqKey;
    /*0x00E*/ u8 reqAck;     // set to reqSeq by the bridge once it is sent
    /*0x00F*/ u8 photoSeq;   // bumped by the ROM: the bridge takes a screenshot
    /*0x010*/ u32 reqValue;
    /*0x014*/ u32 reqExtra;
    /*0x018*/ u16 followerGfx; // this player's look, sent when it changes
    /*0x01A*/ u16 mountGfx;
    /*0x01C*/ u8 decor[WORLD_HOUSE_SLOTS];
    /*0x02C*/ u8 boardSeq;   // bumped by the bridge when the rows below are new
    /*0x02D*/ u8 boardId;
    /*0x02E*/ u8 boardCount;
    /*0x02F*/ u8 boardMyRank;
    /*0x030*/ u16 huntId;    // the server's current Safari hunt, 0 = none known
    /*0x032*/ u8 huntMinutesLeft;
    /*0x033*/ u8 houseOpen;
    /*0x034*/ struct WorldRemoteLook remotes[NETPLAY_MAX_REMOTES];
    /*0x0F4*/ struct WorldBoardRow rows[WORLD_BOARD_ROWS];
    /*0x194*/ u8 accessory;  // this player's follower's accessory + 1, sent with the look
    /*0x195*/ u8 filler[3];
    /*0x198*/ u32 trainerLook; // this player's colours and accessories (include/trainer_look.h)
    /*0x19C*/ u32 remoteTrainerLooks[NETPLAY_MAX_REMOTES]; // by slot, like remotes[]
}; // size 0x1BC

extern struct WorldMailbox gWorldMailbox;

#endif // GUARD_WORLD_MAILBOX_H
