#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "save_ext.h"
#include "constants/reserve.h"
#include "test/test.h"

void Reserve_LeaveOffline(void);

TEST("Haven Reserve keeps up to 40 offline drop-offs with the extended save")
{
    u32 i;

    SaveExt_Clear();
    memset(&gSaveBlock3Ptr->reserve, 0, sizeof(gSaveBlock3Ptr->reserve));
    for (i = 0; i < RESERVE_LOCAL_MAX; i++)
    {
        CreateMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_ZIGZAGOON + i % 4, 5 + i, 0x1000 + i, OTID_STRUCT_PLAYER_ID);
        gSpecialVar_0x8004 = 0;
        Reserve_LeaveOffline();
        EXPECT_EQ(gSpecialVar_Result, RESERVE_LEAVE_OK);
    }
    EXPECT_EQ(gSaveBlock3Ptr->reserve.count, RESERVE_LOCAL_MAX);
    EXPECT_EQ(gSaveBlock3Ptr->reserve.mons[0].level, 5);
    EXPECT_EQ(gSaveBlockExt.reserveMoreMons[RESERVE_LOCAL_MAX - RESERVE_LOCAL_SAVED - 1].level, 5 + RESERVE_LOCAL_MAX - 1);
    EXPECT_EQ(gSaveBlockExt.reserveMoreMons[0].personality, 0x1000 + RESERVE_LOCAL_SAVED);

    Reserve_LeaveOffline();
    EXPECT_EQ(gSpecialVar_Result, RESERVE_LEAVE_LOCAL_FULL);
}
