#include "global.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "netplay_ext.h"
#include "online.h"
#include "overworld.h"
#include "constants/maps.h"

// Guild halls, clan wars and guild base raids (server/online_guilds.py).

// VAR_RESULT: TRUE if the player is in a guild hall now.
void Online_InGuildHall(void)
{
    gSpecialVar_Result = gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ONLINE_GUILD_HALL)
                      && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ONLINE_GUILD_HALL);
}

// The hall's door leads back to where the player stands now.
void Online_SetGuildHallReturn(void)
{
    s16 x, y;

    PlayerGetDestCoords(&x, &y);
    SetDynamicWarpWithCoords(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
                             WARP_ID_NONE, x - MAP_OFFSET, y - MAP_OFFSET);
}

// RAID DEFEND's build: the player's team, as a secret base record.
void Online_AddTeamRecord(void)
{
    NetplayExt_AddTeamRecord();
}

// After RAID GET: the base's team is ready to battle. VAR_0x8005: FALSE if
// it didn't arrive whole.
void Online_StoreRaidTeam(void)
{
    gSpecialVar_0x8005 = NetplayExt_StoreBattleTeam();
}
