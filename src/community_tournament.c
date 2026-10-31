#include "global.h"
#include "community.h"
#include "event_data.h"
#include "script.h"
#include "string_util.h"
#include "constants/characters.h"

// Online tournaments (server/community_tournament.py): START > ONLINE >
// COMMUNITY > TOURNAMENT shows the cup, signs up and shows the bracket.
// Matches are ordinary online battles; when the server pairs you with
// someone, a message tells you who to find.

extern const u8 Community_EventScript_Tournament[];
extern const u8 Community_EventScript_TourMatchReady[];

// gCommunityMailbox.events[COMMUNITY_EVENT_TOUR], written by the server.
struct TourEvent
{
    u8 seq;
    u8 cup;
    u8 round;
    u8 opponent[8];
    u8 cupName[16];
};

static EWRAM_DATA u8 sSeenSeq = 0;
static EWRAM_DATA u8 sShownCup = 0;
static EWRAM_DATA u8 sShownRound = 0;

void Community_OpenTournament(void)
{
    ScriptContext_SetupScript(Community_EventScript_Tournament);
}

bool32 CommunityTour_OverworldUpdate(void)
{
    const struct TourEvent *event = (const struct TourEvent *)gCommunityMailbox.events[COMMUNITY_EVENT_TOUR];

    if (event->seq == sSeenSeq || ArePlayerFieldControlsLocked())
        return FALSE;
    sSeenSeq = event->seq;
    // Say it once per match, not each time the server repeats it.
    if (event->cup == sShownCup && event->round == sShownRound)
        return FALSE;
    sShownCup = event->cup;
    sShownRound = event->round;
    StringCopyN(gStringVar1, event->opponent, sizeof(event->opponent));
    gStringVar1[sizeof(event->opponent)] = EOS;
    StringCopyN(gStringVar2, event->cupName, sizeof(event->cupName));
    gStringVar2[sizeof(event->cupName)] = EOS;
    ScriptContext_SetupScript(Community_EventScript_TourMatchReady);
    return TRUE;
}
