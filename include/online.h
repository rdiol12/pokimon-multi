#ifndef GUARD_ONLINE_H
#define GUARD_ONLINE_H

#include "constants/online.h"

// Sixth batch of online features: catch goals, sightings, tournaments,
// shops, pageants, quests, mentors, betting, team battles, guild halls and
// wars, races, hide and seek, fashion. They talk to the server over the
// Online Plaza's request channel (src/netplay_ext.c) with their own request
// words; the server answers in one shape for all of them (docs/online.md).

// Overworld and battle hooks (called from src/world.c).
bool32 Online_OverworldUpdate(void);
void Online_OnCatch(void);
void Online_OnWildBattleStart(void);
void Online_OpenHubFromMenu(void);

// Fishing tournaments (src/online_fishing.c).
bool32 Online_TournamentIsOn(void);
bool32 Online_TryCoFishInvite(void);
void Online_TournamentBite(void);

// Mentors (src/online_mentors.c): reports a changed badge count.
void Online_CheckBadges(void);
void Online_AddBadgeCount(void);

// Player-made quests (src/online_pquests.c): the treasure hunt the player
// took. TRUE if it started a script.
bool32 Online_PlayerQuestUpdate(void);
void Online_PlayerQuestRetry(void);

// Guild base raids (src/online_guilds.c).
void Online_AddTeamRecord(void);
void Online_StoreRaidTeam(void);

// Mount racing (src/online_racing.c). Online_RaceUpdate: TRUE if it
// started a script.
bool32 Online_RaceIsOn(void);
bool32 Online_RaceUpdate(void);
struct ObjectEvent;
bool32 Online_RacePassesThrough(const struct ObjectEvent *mover, const struct ObjectEvent *other);

// Hide and seek (src/online_hide.c).
bool32 Online_HideIsOn(void);
bool32 Online_HideUpdate(void);
void Online_AddTalkedPlayer(void);
void Online_AddHideTalents(void);

// No trainers or wild Pokémon while a race or game is on (src/online_racing.c).
void Online_SetQuietRules(bool32 on);
bool32 Online_QuietRulesOn(void);

// The live features' window at the top right (src/online_hud.c).
typedef void (*OnlineHudText)(u8 *dest);
void Online_ShowHud(OnlineHudText text, bool32 whileLocked);
void Online_HideHud(void);
u8 *Online_FormatTime(u8 *dest, u32 frames, bool32 cs);

// Asks for the live features' status within a few frames.
void Online_PollStatusSoon(void);

// Queues a report for the server, sent in the background as
// "OEV <word> <a> <b> ...". Up to ONLINE_REPORT_ARGS numbers.
#define ONLINE_REPORT_ARGS 6
void Online_Report(const char *word, u32 argCount, const s32 *args);

// The last status the server sent ("S|key|..." rows of OSTAT): the field
// (0-based, after the key) of the entry for key, or "" if there is none.
const char *Online_Status(const char *key, u32 field);
s32 Online_StatusNum(const char *key, u32 field);

// Requests (src/online_requests.c).
#define ONL_ARG_CHOSEN 1 // after the numbers: the id (field 1) of the row picked last
#define ONL_ARG_MON    2 // after the numbers: party slot VAR_0x8005 as species level shiny ability-number nickname
#define ONL_ARG_MONEY  4 // after the numbers: the player's money
#define ONL_ARG_NAME   8 // after the numbers: the player's name
#define ONL_ARG_COINS 16 // after the numbers: the player's Game Corner coins

struct OnlineRequest
{
    const char *text;     // the request word(s)
    u8 numbers;           // how many of VAR_0x8005, 0x8006, 0x8007 to send
    u8 flags;             // ONL_ARG_*
    void (*build)(void);  // adds anything else; may add data
    void (*after)(void);  // runs on an ok answer, after the generic handling
};

extern const struct OnlineRequest gOnlineRequests[ONL_REQ_COUNT];

// The answer to the last request, without row 0 and action rows.
u32 Online_DataRows(void);
const char *Online_DataField(u32 row, u32 col);
s32 Online_DataNum(u32 row, u32 col);
u8 *Online_CopyText(u8 *dest, const char *src, u32 maxChars);

// A scrolling list for a task of your own (src/online.c).
struct ListMenuItem;
u8 Online_OpenList(const struct ListMenuItem *items, u32 count, void (*func)(u8 taskId));
void Online_CloseList(u8 taskId);

#endif // GUARD_ONLINE_H
