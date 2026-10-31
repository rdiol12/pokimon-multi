#include "global.h"
#include "save.h"
#include "save_ext.h"

// Layout of the extended save memory (PKX_SAVE): two slots of SAVE_EXT_SLOT_SIZE.
// Each save goes to slot gSaveCounter % 2 (the same rotation as the flash save slots),
// data first and the header last, so a slot only looks valid once it's complete.
// The header ties the slot to its flash save: same save counter and trainer ID.

#define SAVE_EXT_MAGIC 0x53584B50 // "PKXS"

struct SaveExtHeader
{
    u32 magic;
    u32 counter;
    u32 trainerId;
    u32 size;
    u32 checksum;
    u32 version;
    u32 unused[2];
};

STATIC_ASSERT(sizeof(struct SaveExtHeader) == SAVE_EXT_HEADER_SIZE, SaveExtHeaderSize);
STATIC_ASSERT(sizeof(struct SaveBlockExt) <= SAVE_EXT_MAX_SIZE, SaveBlockExtFreeSpace);
STATIC_ASSERT(2 * SAVE_EXT_SLOT_SIZE <= PKX_SAVE_SIZE, SaveExtSlotsFit);

ALIGNED(4) EWRAM_EXT_DATA struct SaveBlockExt gSaveBlockExt;

static inline struct SaveExtHeader *SlotHeader(u32 slot)
{
    return (struct SaveExtHeader *)(PKX_SAVE + slot * SAVE_EXT_SLOT_SIZE);
}

static inline void *SlotData(u32 slot)
{
    return (void *)(PKX_SAVE + slot * SAVE_EXT_SLOT_SIZE + SAVE_EXT_HEADER_SIZE);
}

// The BIOS copy functions refuse sources below EWRAM, so reads from the extended save
// use a plain loop.
static void CopyFromExtSave(const void *src, void *dst, u32 size)
{
    const vu32 *from = src;
    u32 *to = dst;
    u32 i;
    for (i = 0; i < size / 4; i++)
        to[i] = from[i];
}

static u32 TrainerId(void)
{
    const u8 *id = gSaveBlock2Ptr->playerTrainerId;
    return id[0] | (id[1] << 8) | (id[2] << 16) | ((u32)id[3] << 24);
}

static u32 Checksum(const void *data, u32 size)
{
    const u32 *words = data;
    u32 i, sum = 0;
    for (i = 0; i < size / 4; i++)
        sum = (sum << 1 | sum >> 31) + words[i];
    return sum;
}

bool32 SaveExt_IsAvailable(void)
{
    return *(vu16 *)REG_ADDR_PKX_ID == PKX_ID && *(vu16 *)REG_ADDR_PKX_VERSION >= 3;
}

void SaveExt_Clear(void)
{
    CpuFill32(0, &gSaveBlockExt, sizeof(gSaveBlockExt));
    gSaveBlockExt.version = SAVE_EXT_VERSION;
}

// Call after gSaveCounter is set for the save being written.
void SaveExt_Write(void)
{
    u32 slot = gSaveCounter % 2;
    struct SaveExtHeader header = {0};

    if (!SaveExt_IsAvailable())
        return;
    gSaveBlockExt.version = SAVE_EXT_VERSION;
    // Invalidate first, so a save cut short never pairs old data with a new header
    SlotHeader(slot)->magic = 0;
    // CPU copies only: DMA can't read the extended region (writing to it is fine)
    CpuCopy32(&gSaveBlockExt, SlotData(slot), sizeof(gSaveBlockExt));
    header.magic = SAVE_EXT_MAGIC;
    header.counter = gSaveCounter;
    header.trainerId = TrainerId();
    header.size = sizeof(gSaveBlockExt);
    header.checksum = Checksum(&gSaveBlockExt, sizeof(gSaveBlockExt));
    header.version = SAVE_EXT_VERSION;
    CpuCopy32(&header, SlotHeader(slot), sizeof(header));
}

static bool32 IsSlotValid(u32 slot)
{
    struct SaveExtHeader header;
    CopyFromExtSave(SlotHeader(slot), &header, sizeof(header));
    if (header.magic != SAVE_EXT_MAGIC || header.trainerId != TrainerId())
        return FALSE;
    if (header.size > SAVE_EXT_MAX_SIZE || header.size % 4 != 0 || header.counter > gSaveCounter)
        return FALSE;
    return Checksum(SlotData(slot), header.size) == header.checksum;
}

// Call after the flash save loaded. Picks the slot written with this save, or else the
// newest older one (a link save rewrites in place), or else starts empty.
void SaveExt_Load(void)
{
    s32 best = -1;
    u32 slot, size;

    SaveExt_Clear();
    if (!SaveExt_IsAvailable() || gSaveFileStatus != SAVE_STATUS_OK)
        return;
    for (slot = 0; slot < 2; slot++)
    {
        if (!IsSlotValid(slot))
            continue;
        if (best < 0 || SlotHeader(slot)->counter > SlotHeader(best)->counter)
            best = slot;
    }
    if (best < 0)
        return;
    size = min(SlotHeader(best)->size, sizeof(gSaveBlockExt));
    CopyFromExtSave(SlotData(best), &gSaveBlockExt, size);
}
