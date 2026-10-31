#ifndef GUARD_NETPLAY_EXT_H
#define GUARD_NETPLAY_EXT_H

#include "list_menu.h"
#include "constants/netplay_ext.h"

struct SecretBase;

// Second batch of online features: guilds, Wonder Trade, shared secret
// bases, the online Battle Tower, bounties, titles and badges, battle
// replays, draft battles, mail and gifts, and the Pokédex race.
//
// They all talk to the server through gNetplayExtMailbox, one request at a
// time: the ROM writes a request line (ASCII) and optional binary data, the
// bridge passes it on and writes the answer back. See docs/netplay_ext.md.

#define NETPLAY_EXT_VERSION     1
#define NETPLAY_EXT_TEXT_LENGTH 64
#define NETPLAY_EXT_DATA_SIZE   1024
#define NETPLAY_EXT_RESP_SIZE   1536

struct NetplayExtMailbox
{
    /*0x000*/ char magic[8]; // "PKMNEXT1"
    /*0x008*/ u8 version;
    /*0x009*/ u8 reqSeq;  // ROM: bumped once the request below is ready
    /*0x00A*/ u8 respSeq; // bridge: set to reqSeq once the answer below is ready
    /*0x00B*/ u8 status;  // bridge: 0 ok, 1 refused, 2 bad request
    /*0x00C*/ u16 reqLen;
    /*0x00E*/ u16 respLen;
    /*0x010*/ char reqText[NETPLAY_EXT_TEXT_LENGTH]; // ASCII, NUL-terminated
    /*0x050*/ u8 reqData[NETPLAY_EXT_DATA_SIZE];
    /*0x450*/ u8 resp[NETPLAY_EXT_RESP_SIZE];
    /*0xA50*/ u8 notes;    // bridge: bumped when new mail arrives
    /*0xA51*/ u8 testOpen;    // test driver: 1 opens the Online Plaza, 0xEE adds test data
    /*0xA52*/ u8 fieldLocked; // ROM: the player can't move (a script or menu is running)
    /*0xA53*/ u8 filler;
}; // size 0xA54

extern struct NetplayExtMailbox gNetplayExtMailbox;

void NetplayExt_Init(void);
bool32 NetplayExt_OverworldUpdate(void);
void NetplayExt_OnLinkBattleEnd(u8 battleOutcome);
const struct SecretBase *NetplayExt_GetBattleBase(void);
void NetplayExt_AddTeamRecord(void);
bool32 NetplayExt_StoreBattleTeam(void);
void NetplayExt_OpenPlazaFromMenu(void);

// Shared with src/netplay_ext_features.c and src/netplay_ext_plaza.c.

enum
{
    NPX_RPC_IDLE,
    NPX_RPC_WAITING,
    NPX_RPC_DONE,
    NPX_RPC_FAILED,
};

#define NPX_MAX_ROWS  24

// Rows for the list screens (the plaza's and the online hub's).
extern u8 gNpxListText[NPX_MAX_ROWS + 1][32];
extern struct ListMenuItem gNpxListItems[NPX_MAX_ROWS + 1];
#define NPX_MAX_COLS  8

bool32 NpxRpc_Begin(const char *word);
void NpxRpc_Word(const char *word);
void NpxRpc_Num(s32 n);
void NpxRpc_Name(const u8 *gameText);
void NpxRpc_Data(const void *data, u32 size);
void NpxRpc_Send(void);
u8 NpxRpc_Poll(void);
void NpxRpc_Release(void);

void Npx_TakeAnswer(void);
void Npx_ChooseAnswerRow(u32 row);
void Npx_SetChosen(u32 col, const char *value);
u32 Npx_RowCount(void);
const char *Npx_Field(u32 row, u32 col);
s32 Npx_Num(u32 row, u32 col);
u8 *Npx_CopyField(u8 *dest, u32 row, u32 col, u32 maxChars);
const u8 *Npx_Binary(u32 *size);
const char *Npx_Chosen(u32 col);
s32 Npx_ChosenNum(u32 col);
u8 *Npx_AsciiToGame(u8 *dest, const char *src, u32 maxChars);
bool32 Npx_IsOnline(void);

struct NpxRequest
{
    const char *word;
    void (*build)(void);   // adds the arguments after the word; may add data
    void (*after)(void);   // runs on an ok answer; buffers what the script shows
    u16 timeout;           // frames, 0: default
};

extern const struct NpxRequest gNpxRequests[NPX_REQ_COUNT];

void NetplayExt_RunRequest(const struct NpxRequest *req);

#endif // GUARD_NETPLAY_EXT_H
