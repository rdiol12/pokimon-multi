#include "global.h"
#include "palette.h"
#include "constants/rgb.h"
#include "sprite.h"
#include "test/test.h"

// Extended GBA v5: sprite tile banks and background palette banks (emulator/README.md)

static void ResetTiles(void)
{
    InitPkxObj();
    gReservedSpriteTileCount = 0;
    AllocSpriteTiles(0);
}

TEST("Extended GBA v5 sprite tile banks are available")
{
    InitPkxObj();
    EXPECT(gPkxObjTileBanks);
}

TEST("Sprite tiles spill into the extra banks once normal VRAM is full")
{
    ResetTiles();
    EXPECT_EQ(AllocSpriteTiles(TOTAL_OBJ_TILE_COUNT), 0);
    EXPECT_EQ(AllocSpriteTiles(64), TOTAL_OBJ_TILE_COUNT);
    EXPECT_EQ(AllocSpriteTiles(64), TOTAL_OBJ_TILE_COUNT + 64);
    AllocSpriteTiles(0);
}

TEST("A block of sprite tiles never crosses into the next bank")
{
    ResetTiles();
    EXPECT_EQ(AllocSpriteTiles(TOTAL_OBJ_TILE_COUNT), 0);
    EXPECT_EQ(AllocSpriteTiles(1000), TOTAL_OBJ_TILE_COUNT);
    EXPECT_EQ(AllocSpriteTiles(100), 2 * TOTAL_OBJ_TILE_COUNT);
    EXPECT(CanAllocSpriteTiles(24));
    EXPECT_EQ(AllocSpriteTiles(24), TOTAL_OBJ_TILE_COUNT + 1000);
    AllocSpriteTiles(0);
}

TEST("Sprite tiles fail only when every bank is full")
{
    u32 bank;
    ResetTiles();
    for (bank = 0; bank <= PKX_OBJ_TILE_BANKS; bank++)
        EXPECT_EQ(AllocSpriteTiles(TOTAL_OBJ_TILE_COUNT), bank * TOTAL_OBJ_TILE_COUNT);
    EXPECT(!CanAllocSpriteTiles(1));
    EXPECT_EQ(AllocSpriteTiles(1), -1);
    AllocSpriteTiles(0);
}

TEST("Sprite tile numbers past normal VRAM point into the extra banks")
{
    EXPECT_EQ((u32)SpriteTileAddr(0), OBJ_VRAM0);
    EXPECT_EQ((u32)SpriteTileAddr(TOTAL_OBJ_TILE_COUNT), PKX_OBJ_TILES);
    EXPECT_EQ((u32)SpriteTileAddr(2 * TOTAL_OBJ_TILE_COUNT + 1), PKX_OBJ_TILES + TOTAL_OBJ_TILE_COUNT * TILE_SIZE_4BPP + TILE_SIZE_4BPP);
}

TEST("Extra sprite tile banks keep what is written to them")
{
    vu32 *tiles = (vu32 *)SpriteTileAddr(3 * TOTAL_OBJ_TILE_COUNT + 5);
    *tiles = 0x12345678;
    EXPECT_EQ(*tiles, 0x12345678);
    *tiles = 0;
}

TEST("Background palette banks are set per background")
{
    EXPECT(HasExtBgPalettes());
    ResetExtBgPalettes();
    SetBgPaletteBank(1, 2);
    SetBgPaletteBank(3, 1);
    EXPECT_EQ(GetBgPaletteBank(0), 0);
    EXPECT_EQ(GetBgPaletteBank(1), 2);
    EXPECT_EQ(GetBgPaletteBank(3), 1);
    EXPECT_EQ(*(vu16 *)REG_ADDR_PKX_BG_PALETTE_BANKS, (2 << 2) | (1 << 6));
    ResetExtBgPalettes();
    EXPECT_EQ(*(vu16 *)REG_ADDR_PKX_BG_PALETTE_BANKS, 0);
}

TEST("Extended background palettes load, transfer and fade with their BG palette")
{
    static const u16 sReds[16] = { [0 ... 15] = RGB_RED };
    ResetExtBgPalettes();
    LoadExtBgPalette(sReds, 2, 3, sizeof(sReds));
    EXPECT_EQ(GetExtBgPalettesInUse(), (u64)1 << (16 + 3));
    EXPECT_EQ(gPlttBufferFaded[EXT_BG_PLTT_ID(2, 3) + 1], RGB_RED);

    TransferPlttBuffer();
    EXPECT_EQ(*(vu16 *)(PKX_BG_PLTT + (16 + 3) * PLTT_SIZE_4BPP + 2), RGB_RED);

    // Fading BG palette 3 fades extended palette 3 of every bank too
    BlendPalettesFine(1 << 3, gPlttBufferUnfaded, gPlttBufferFaded, 16, RGB_BLACK);
    EXPECT_EQ(gPlttBufferFaded[EXT_BG_PLTT_ID(2, 3) + 1], RGB_BLACK);
    // ...but not when a different BG palette fades
    CpuCopy16(sReds, &gPlttBufferFaded[EXT_BG_PLTT_ID(2, 3)], sizeof(sReds));
    BlendPalettesFine(1 << 4, gPlttBufferUnfaded, gPlttBufferFaded, 16, RGB_BLACK);
    EXPECT_EQ(gPlttBufferFaded[EXT_BG_PLTT_ID(2, 3) + 1], RGB_RED);
    ResetExtBgPalettes();
}
