#include "global.h"
#include "save.h"
#include "save_ext.h"
#include "test/test.h"

static void SetTrainerId(u32 id)
{
    gSaveBlock2Ptr->playerTrainerId[0] = id;
    gSaveBlock2Ptr->playerTrainerId[1] = id >> 8;
    gSaveBlock2Ptr->playerTrainerId[2] = id >> 16;
    gSaveBlock2Ptr->playerTrainerId[3] = id >> 24;
}

static void SaveWithMarker(u32 counter, u32 marker)
{
    gSaveCounter = counter;
    SaveExt_Clear();
    gSaveBlockExt.reserved[0] = marker;
    SaveExt_Write();
}

static u32 LoadMarker(u32 counter)
{
    gSaveCounter = counter;
    gSaveFileStatus = SAVE_STATUS_OK;
    SaveExt_Load();
    return gSaveBlockExt.reserved[0];
}

TEST("Extended save is available on the extended GBA")
{
    EXPECT(SaveExt_IsAvailable());
}

TEST("Extended save round-trips with the save it was written with")
{
    SetTrainerId(0x12345678);
    SaveWithMarker(10, 0xCAFE0010);
    SaveWithMarker(11, 0xCAFE0011);
    EXPECT_EQ(LoadMarker(11), 0xCAFE0011);
    EXPECT_EQ(gSaveBlockExt.version, SAVE_EXT_VERSION);
}

TEST("Extended save falls back to the older slot when the flash save is older")
{
    SetTrainerId(0x12345678);
    SaveWithMarker(20, 0xCAFE0020);
    SaveWithMarker(21, 0xCAFE0021); // flash write failed, counter went back to 20
    EXPECT_EQ(LoadMarker(20), 0xCAFE0020);
}

TEST("Extended save ignores a corrupted slot")
{
    SetTrainerId(0x12345678);
    SaveWithMarker(30, 0xCAFE0030);
    SaveWithMarker(31, 0xCAFE0031);
    *(vu32 *)(PKX_SAVE + (31 % 2) * SAVE_EXT_SLOT_SIZE + SAVE_EXT_HEADER_SIZE + 4) ^= 1;
    EXPECT_EQ(LoadMarker(31), 0xCAFE0030);
}

TEST("Extended save from another file is not loaded")
{
    SetTrainerId(0x12345678);
    SaveWithMarker(40, 0xCAFE0040);
    SaveWithMarker(41, 0xCAFE0041);
    SetTrainerId(0x0BADF00D);
    EXPECT_EQ(LoadMarker(41), 0);
}

TEST("Extended save is empty when the flash save didn't load")
{
    SetTrainerId(0x12345678);
    SaveWithMarker(50, 0xCAFE0050);
    gSaveCounter = 50;
    gSaveFileStatus = SAVE_STATUS_EMPTY;
    SaveExt_Load();
    EXPECT_EQ(gSaveBlockExt.reserved[0], 0);
}
