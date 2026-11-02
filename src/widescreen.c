#include "global.h"
#include "widescreen.h"

// See include/widescreen.h and emulator/README.md (extended hardware v6).

COMMON_DATA bool8 gFieldWide = FALSE;
static u8 sWideMapBgs;

bool32 Widescreen_Available(void)
{
    return *(vu16 *)REG_ADDR_PKX_ID == PKX_ID
        && *(vu16 *)REG_ADDR_PKX_VERSION >= 6
        && *(vu16 *)REG_ADDR_PKX_WIDE >= PKX_WIDE_WIDTH;
}

void Widescreen_SetLayers(u32 layers)
{
    if (*(vu16 *)REG_ADDR_PKX_ID == PKX_ID && *(vu16 *)REG_ADDR_PKX_VERSION >= 6)
        *(vu16 *)REG_ADDR_PKX_WIDE = layers;
}

void Widescreen_SetBgWideMap(u32 bg, bool32 wide)
{
    if (wide)
        sWideMapBgs |= 1 << bg;
    else
        sWideMapBgs &= ~(1 << bg);
    if (*(vu16 *)REG_ADDR_PKX_ID == PKX_ID && *(vu16 *)REG_ADDR_PKX_VERSION >= 6)
        *(vu16 *)REG_ADDR_PKX_WIDE_MAPS = sWideMapBgs;
}

bool32 Widescreen_IsBgWideMap(u32 bg)
{
    return (sWideMapBgs >> bg) & 1;
}

void Widescreen_CopyRightHalf(u32 bg, const void *tilemap)
{
    // Writing to the extended video region with DMA is fine (only reading from it isn't)
    DmaCopy32(3, (const u8 *)tilemap + 0x800, PKX_WIDE_MAP(bg), 0x800);
}

void Widescreen_Reset(void)
{
    sWideMapBgs = 0;
    gFieldWide = FALSE;
    if (*(vu16 *)REG_ADDR_PKX_ID == PKX_ID && *(vu16 *)REG_ADDR_PKX_VERSION >= 6)
    {
        *(vu16 *)REG_ADDR_PKX_WIDE = 0;
        *(vu16 *)REG_ADDR_PKX_WIDE_MAPS = 0;
    }
}

s32 Widescreen_FieldMargin(void)
{
    return gFieldWide ? PKX_WIDE_MARGIN : 0;
}
