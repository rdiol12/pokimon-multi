#ifndef GUARD_COMMUNITY_H
#define GUARD_COMMUNITY_H

#include "constants/community.h"

// The community features: tournaments, the shared daycare, the server boss,
// trainer cards, online contests, chat rooms and legality checks. The ROM
// talks to the server only through this EWRAM block, which
// tools/netplay_bridge/netplay_community.lua copies to and from the server's
// community commands (server/community.py). Layout: docs/community.md.
//
// One request is in flight at a time: the ROM fills req*, bumps reqSeq;
// the bridge sends it and sets reqAck. The answer comes back as text rows
// (rows[0] is a title), four numbers and raw bytes, published by bumping
// respSeq last. Events are sections the server rewrites on its own; the
// first byte of each is a counter the bridge bumps after a write.

#define COMMUNITY_MAILBOX_VERSION 1
#define COMMUNITY_ROWS            12
#define COMMUNITY_ROW_LENGTH      32
#define COMMUNITY_DATA_SIZE       0x100
#define COMMUNITY_EVENT_SIZE      0xC0

struct CommunityMailbox
{
    /*0x000*/ char magic[8]; // "PKMNCOM1"
    /*0x008*/ u8 version;
    /*0x009*/ u8 reqSeq;
    /*0x00A*/ u8 reqType;
    /*0x00B*/ u8 reqAck;
    /*0x00C*/ u8 respSeq;
    /*0x00D*/ u8 respType;
    /*0x00E*/ u8 respStatus;
    /*0x00F*/ u8 rowCount;
    /*0x010*/ u32 reqArgs[4];
    /*0x020*/ u16 reqLen;
    /*0x022*/ u8 partyCount; // for the server's legality checks
    /*0x023*/ u8 monSize;
    /*0x024*/ u32 partyAddr;
    /*0x028*/ u32 respArgs[4];
    /*0x038*/ u16 respLen;
    /*0x03A*/ u16 filler;
    /*0x03C*/ u8 reqData[COMMUNITY_DATA_SIZE];
    /*0x13C*/ u8 respData[COMMUNITY_DATA_SIZE];
    /*0x23C*/ u8 rows[COMMUNITY_ROWS][COMMUNITY_ROW_LENGTH];
    /*0x3BC*/ u8 events[COMMUNITY_EVENT_COUNT][COMMUNITY_EVENT_SIZE];
}; // size 0x6BC

extern struct CommunityMailbox gCommunityMailbox;

// A trainer battle with its party resized and levelled to the player
// (gym rematches, the postgame rival, the postgame aces). Used once by the
// next battle against `trainerId`.
struct CommunityTrainerScale
{
    u16 trainerId;
    u8 partySize; // 0: as defined
    s8 levelBonus; // over the player's strongest Pokémon
    u16 aceSpecies; // if set, added last (after trimming), with its level-up moves
};

extern struct CommunityTrainerScale gCommunityTrainerScale;

// For the screenshot runs (tools/community_shots): a driver fills this in and
// sets `pending`; the next overworld frame applies it.
struct CommunityShowcase
{
    u8 pending;
    u8 gameClear;
    u8 badges;
    u8 level;      // of the party below
    u16 species[PARTY_SIZE]; // replaces the party when species[0] is set
    u16 flags[4];  // set
    u16 vars[2];
    u16 varValues[2];
};

extern struct CommunityShowcase gCommunityShowcase;

bool32 Community_IsOnline(void);
void Community_InitMailbox(void);
void Community_Request(u8 type, u32 a0, u32 a1, u32 a2, u32 a3, const void *data, u16 len);
bool32 Community_HasResponse(u8 type);
// Sends a request, then resumes the script (waitstate) once it's answered;
// VAR_RESULT is the answer's status.
void Community_RequestAndWait(u8 type, u32 a0, u32 a1, u32 a2, const void *data, u16 len);
bool32 Community_OverworldUpdate(void);
void Community_BufferRows(void);
u8 Community_GetPlayerMaxLevel(void);
bool32 Community_OverrideTrainerParty(struct Pokemon *party, u16 trainerId);

// A list window over the response rows (rows[0] is the title). With
// `choose`, A picks a row: VAR_RESULT is its index (from 0) or 0xFF for B.
// Resumes the script when it closes.
void Community_ShowRowList(bool32 choose);

// Feature entry points. Each is defined by its feature's file; a feature
// that isn't built in leaves its row out of the COMMUNITY menu.
void Community_OpenTournament(void) __attribute__((weak));
void Community_OpenDaycare(void) __attribute__((weak));
void Community_OpenBoss(void) __attribute__((weak));
void Community_OpenCards(void) __attribute__((weak));
void Community_OpenContest(void) __attribute__((weak));
void Community_OpenChat(void) __attribute__((weak));
void Community_OpenLegalCheck(void) __attribute__((weak));
// Called each overworld frame by features that watch their event section.
bool32 CommunityChat_OverworldUpdate(void) __attribute__((weak));
bool32 CommunityTour_OverworldUpdate(void) __attribute__((weak));
bool32 CommunityBoss_OverworldUpdate(void) __attribute__((weak));

void ShowCommunityMenu(void);

#endif // GUARD_COMMUNITY_H
