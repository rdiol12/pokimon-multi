#include "global.h"
#include "event_data.h"
#include "netplay_ext.h"
#include "online.h"

// Mentors (server/online_mentors.py): the server needs the player's badge
// count, sent with every MENTOR request and reported when it changes.

static EWRAM_DATA u8 sReportedBadges = 0; // count + 1; 0: not reported yet

static u32 CountBadges(void)
{
    u32 i, n = 0;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (FlagGet(FLAG_BADGE01_GET + i))
            n++;
    }
    return n;
}

// A request's build: the badge count goes last.
void Online_AddBadgeCount(void)
{
    NpxRpc_Num(CountBadges());
}

void Online_CheckBadges(void)
{
    s32 n = CountBadges();

    if (sReportedBadges == n + 1)
        return;
    sReportedBadges = n + 1;
    Online_Report("BADGES", 1, &n);
}
