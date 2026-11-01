#ifndef GUARD_GBA_DEFINES_H
#define GUARD_GBA_DEFINES_H

#include <stddef.h>

#define TRUE   1
#define FALSE  0

#define IWRAM_DATA __attribute__((section(".bss")))
#define EWRAM_DATA __attribute__((section(".sbss")))
#define IWRAM_INIT __attribute__((section(".iwram")))
#define EWRAM_INIT __attribute__((section(".ewram")))
// Zero-initialised data in the 16 MiB extended EWRAM of our emulator fork (emulator/README.md).
// Never use it for anything the netplay bridge scans for (mailboxes): those stay in EWRAM_DATA,
// inside the first 256 KiB, where the bridge and the phone app look.
#define EWRAM_EXT_DATA __attribute__((section(".pkx_ewram")))
#define COMMON_DATA __attribute__((section("common_data")))
#define UNUSED __attribute__((unused))
#define USED __attribute__((used))
#define KEEP_SECTION __attribute__((section(".text.consts")))
#define DEPRECATED(msg) __attribute__((deprecated(msg)))

#define ARM_FUNC __attribute__((target("arm")))

#if MODERN
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

#define ALIGNED(n) __attribute__((aligned(n)))
#define PACKED __attribute__((packed))
#define TRANSPARENT __attribute__ ((__transparent_union__))
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define NONNULL __attribute__((__nonnull__))

#define SOUND_INFO_PTR (*(struct SoundInfo **)0x3007FF0)
#define INTR_CHECK     (*(u16 *)0x3007FF8)
#define INTR_VECTOR    (*(void **)0x3007FFC)

#define ROM_START 0x8000000
#define ROM_END 0xA000000

#define EWRAM_START 0x02000000
#define EWRAM_END   (EWRAM_START + 0x40000)
#define EWRAM_EXT_START EWRAM_END
#define EWRAM_EXT_END   (EWRAM_START + 0x1000000)

// Pokimon extended hardware registers (emulator/README.md)
#define REG_ADDR_PKX_ID      0x04FFF7F0
#define REG_ADDR_PKX_VERSION 0x04FFF7F2
#define REG_ADDR_PKX_CONTROL 0x04FFF7F4
#define PKX_ID               0x504B
#define PKX_UNLOCK_EWRAM     0x4558
#define PKX_CONTROL_EWRAM    0x0001
// Version 2: extra sprite palettes and sprite slots, in the extended video region
#define PKX_ENABLE_OBJ       0x4F41
#define PKX_DISABLE_OBJ      0x4F40
#define PKX_CONTROL_OBJ      0x0002
#define PKX_VIDEO            0x1000000
#define PKX_OBJ_PLTT         (PKX_VIDEO + 0x0000) // sprite palettes 16..63
#define PKX_OAM              (PKX_VIDEO + 0x1000) // 512 OAM entries; 0..127 are ignored (the normal OAM is used)
#define PKX_OAM_ATTR         (PKX_VIDEO + 0x2000) // one byte per OAM entry 0..511: bits 0-1 = palette bank
#define PKX_OBJ_PALETTES     64
#define PKX_OAM_ENTRIES      512
// Version 5: three more 32 KiB banks of sprite tiles. Attribute bits 2-3 pick the bank
// (0 = normal OBJ VRAM). Write to them with the CPU or DMA; read them with the CPU.
#define PKX_OBJ_TILES        (PKX_VIDEO + 0x10000)
#define PKX_OBJ_TILE_BANKS   3
#define PKX_OBJ_TILE_COUNT_ALL (1024 * (PKX_OBJ_TILE_BANKS + 1))
// Version 5: 48 more background palettes, and a register that picks a palette bank for
// each background (2 bits per BG; 0 = the normal 16 palettes, 1-3 = PKX_BG_PLTT banks).
// Text-mode backgrounds only.
#define PKX_BG_PLTT          (PKX_VIDEO + 0x3000)
#define PKX_EXT_BG_PALETTES  48
#define REG_ADDR_PKX_BG_PALETTE_BANKS 0x4FFF7F6
// Version 3: 1 MiB of battery-backed RAM saved with the flash save. CPU access only (no DMA).
#define PKX_SAVE             (PKX_VIDEO + 0x100000)
#define PKX_SAVE_SIZE        0x100000
// Version 4: CPU overclock. Write PKX_SET_OVERCLOCK | n (n = 1..8) to the control register;
// control bits 8-11 read back the current multiplier.
#define PKX_SET_OVERCLOCK    0x4300
#ifndef PKX_OVERCLOCK
#define PKX_OVERCLOCK        4 // the game's multiplier
#endif
// Version 4: ROMs up to 64 MiB. The first 32 MiB stay at ROM_START (and its 0x0A000000 mirror,
// which script function tags use); the rest shows at PKX_ROM2. Put big read-only data there with
// ROM2_DATA. Only valid on extended hardware v4.
#define PKX_ROM2             0x0C000000
#define PKX_ROM2_SIZE        0x2000000
#define ROM2_DATA            __attribute__((section(".rom2")))
#define IWRAM_START 0x03000000
#define IWRAM_END   (IWRAM_START + 0x8000)

#define PLTT          0x5000000
#define BG_PLTT       PLTT
#define BG_PLTT_SIZE  0x200
#define OBJ_PLTT      (PLTT + BG_PLTT_SIZE)
#define OBJ_PLTT_SIZE 0x200
#define PLTT_SIZE     (BG_PLTT_SIZE + OBJ_PLTT_SIZE)

#define VRAM      0x6000000
#define VRAM_SIZE 0x18000

#define BG_VRAM           VRAM
#define BG_VRAM_SIZE      0x10000
#define BG_CHAR_SIZE      0x4000
#define BG_SCREEN_SIZE    0x800
#define BG_CHAR_ADDR(n)   (BG_VRAM + (BG_CHAR_SIZE * (n)))
#define BG_SCREEN_ADDR(n) (BG_VRAM + (BG_SCREEN_SIZE * (n)))

#define BG_TILE_H_FLIP(n) (0x400 + (n))
#define BG_TILE_V_FLIP(n) (0x800 + (n))

#define NUM_BACKGROUNDS 4

// text-mode BG
#define OBJ_VRAM0      (VRAM + 0x10000)
#define OBJ_VRAM0_SIZE 0x8000

// bitmap-mode BG
#define OBJ_VRAM1      (VRAM + 0x14000)
#define OBJ_VRAM1_SIZE 0x4000

#define OAM      0x7000000
#define OAM_SIZE 0x400

#define ROM_HEADER_SIZE   0xC0

// Dimensions of a tile in pixels
#define TILE_WIDTH  8
#define TILE_HEIGHT 8

// Dimensions of the GBA screen in pixels
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 160

// Dimensions of the GBA screen in tiles
#define DISPLAY_TILE_WIDTH  (DISPLAY_WIDTH / TILE_WIDTH)
#define DISPLAY_TILE_HEIGHT (DISPLAY_HEIGHT / TILE_HEIGHT)

// Size of different tile formats in bytes
#define TILE_SIZE(bpp) ((bpp) * TILE_WIDTH * TILE_HEIGHT / 8)
#define TILE_SIZE_1BPP TILE_SIZE(1) // 8
#define TILE_SIZE_4BPP TILE_SIZE(4) // 32
#define TILE_SIZE_8BPP TILE_SIZE(8) // 64

#define TILE_OFFSET_4BPP(n) ((n) * TILE_SIZE_4BPP)
#define TILE_OFFSET_8BPP(n) ((n) * TILE_SIZE_8BPP)

#define TOTAL_OBJ_TILE_COUNT 1024

#define PLTT_SIZEOF(n) ((n) * sizeof(u16))
#define PLTT_SIZE_4BPP PLTT_SIZEOF(16)
#define PLTT_SIZE_8BPP PLTT_SIZEOF(256)

#define PLTT_OFFSET_4BPP(n) ((n) * PLTT_SIZE_4BPP)

#endif // GUARD_GBA_DEFINES_H
