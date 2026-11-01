# Pokimon extended GBA

Pokimon Multi runs on our own fork of [mGBA](https://mgba.io) that adds hardware a real
GBA never had. The game is still an ordinary GBA ROM built with the ARM toolchain, so
releases stay a `.bps` patch against the player's own ROM. The fork is upstream mGBA at
the commit in `MGBA_COMMIT` plus the patches in `patches/`, under mGBA's MPL-2.0 licence.
It contains no Nintendo code or data.

Why: the GBA's 256 KiB of EWRAM was 96% full, and the 16 sprite palettes and 128 sprite
slots were blocking features. See the plan doc linked from the project thread
"Port the game off the GBA".

## Building

```
emulator/build.sh headless   # libmgba.so + mgba-headless: make check, e2e tests
emulator/build.sh sdl        # desktop player (SDL)
emulator/build.sh qt         # desktop player (Qt)
emulator/build.sh wasm       # mgba.js + mgba.wasm for the phone app (needs emcc)
```

Output goes to `build/emulator/`. `make check` uses `build/emulator/headless/mgba-headless`
automatically once it exists (it needs libelf for ELF loading), and the netplay e2e
tests link the harness against `build/emulator/headless/libmgba.so`.

## Hardware

### Detection registers

| Address      | Access | Value                                                        |
| ------------ | ------ | ------------------------------------------------------------ |
| `0x04FFF7F0` | read   | `0x504B` ("PK") on our hardware                              |
| `0x04FFF7F2` | read   | hardware version, currently `5`                              |
| `0x04FFF7F4` | write  | `0x4558` ("EX") unlocks extended EWRAM until the next reset  |
| `0x04FFF7F4` | write  | `0x4F41` turns extended sprites on, `0x4F40` turns them off  |
| `0x04FFF7F4` | write  | `0x4300` + n (n = 1 to 8) runs the CPU n times faster        |
| `0x04FFF7F4` | read   | bit 0: extended EWRAM unlocked; bit 1: extended sprites on; bits 8–11: CPU multiplier |
| `0x04FFF7F6` | both   | background palette banks: 2 bits per background (BG0 in bits 0–1), 0 = normal |

These sit next to mGBA's own debug registers (`0x04FFF600`–`0x04FFF780`), in I/O space a
real GBA leaves unused, so writes there are harmless elsewhere.

### Extended EWRAM (version 1)

- A real GBA repeats its 256 KiB of EWRAM across the whole `0x02000000`–`0x02FFFFFF`
  block. Once unlocked, that block is 16 MiB of real RAM instead.
- `0x02000000`–`0x0203FFFF` is unchanged, so savestates keep EWRAM at the same offset and
  the netplay bridge and phone app find the mailboxes where they always did.
- `0x02040000`–`0x02FFFFFF` is the new RAM. Savestate files store it as extra data (only up
  to the last non-zero word), so loading an older state still works.
- A reset locks it again and clears all 16 MiB.

### In the game

- `src/crt0.s` unlocks extended EWRAM as its first instruction, before the BIOS RAM reset,
  so the reset clears all of it.
- `AgbMain` checks the detection registers and stops on a red screen when they are
  missing, because on a stock emulator the new addresses would silently overwrite the
  first 256 KiB.
- `EWRAM_EXT_DATA` (include/gba/defines.h) places zero-initialised data in the new RAM.
  Never use it for anything the netplay bridge scans for: mailboxes stay `EWRAM_DATA`.
- Initialised data (`EWRAM_INIT`) can't go there yet. Use `EWRAM_EXT_DATA` and set the
  values at runtime.

### Extended sprites (version 2)

Region `0x01000000` (unused on a real GBA) holds extra video memory:

| Address      | Size         | Contents                                                  |
| ------------ | ------------ | --------------------------------------------------------- |
| `0x01000000` | `0x600`      | sprite palettes 16..63 (16 colors each, same format as PLTT) |
| `0x01001000` | 512 × 8      | OAM entries; entries 0..127 here are ignored               |
| `0x01002000` | 512          | one attribute byte per OAM entry 0..511                   |

- While extended sprites are on, the renderer draws OAM entries 0..127 from the normal
  OAM and entries 128..511 from `0x01001400` on. Lower numbers still draw on top.
- Attribute bits 0-1 are the entry's palette bank: a 16-color sprite uses palette
  `bank * 16 + paletteNum`. Bank 0 is the normal 16 palettes, so old code is unaffected.
- Rotation/scaling matrices stay the 32 in the normal OAM.
- There is no per-scanline sprite limit while extended sprites are on.
- 256-color sprites ignore the bank. Background palettes are not extended.
- Only the software renderer draws them (the default, and what the phone app uses), not
  the OpenGL renderer. Savestates keep all of it.

In the game:

- `InitPkxObj` (src/sprite.c) turns extended sprites on at boot when the version is at
  least 2. Without it everything behaves like before (64 sprites, 16 palettes).
- `MAX_SPRITES` is 240 (sprite ids are u8 and `SPRITE_NONE` is 0xFF). Sprites go into the normal OAM first (up to `gOamLimit`, 64) and
  overflow into entries 128..511.
- Sprite palette slots 16..63 are handed out only once the normal 16 are full.
  `OBJ_PLTT_ID(slot)` works for every slot: the palette buffers are 0x300 colors longer
  and `TransferPlttBuffer` copies the extra ones to `0x01000000`.
- A sprite's palette slot is `GetSpritePaletteSlot(sprite)` and is set with
  `SetSpritePaletteSlot(sprite, slot)`. Don't read or write `sprite->oam.paletteNum`
  directly, because it only holds the low 4 bits.
- Fades, time-of-day blending and weather apply to the extra palettes whenever they
  apply to all 16 normal sprite palettes.

### Extended save (version 3)

- `0x01100000`–`0x011FFFFF` is 1 MiB of battery-backed RAM: read and write it like SRAM,
  8, 16 or 32 bits at a time. It is only there for flash-save games (Emerald is one).
- It lives in the same `.sav` file: 128 KiB of flash, then the RTC data, then the 1 MiB at
  file offset `0x21000`. The file only grows once the game first writes there, and older
  saves just read as zeros. Writes reach the file shortly after the game stops writing,
  like flash saves.
- Exporting or importing a save (the phone app's save backup) includes it.
- DMA and the BIOS copy functions (`CpuSet`, `CpuFastSet`) can't read from it, like any
  address below EWRAM on a real GBA. Writing to it with them is fine. Read it with a plain loop.

In the game, `gSaveBlockExt` (include/save_ext.h, in extended EWRAM) is saved there with
every save and loaded with the save. To save new data, add a field at the end of
`struct SaveBlockExt`; saves made before the field existed load it as zero. Two slots of
512 KiB alternate like the flash save slots, and a header ties each slot to its flash save
(save counter, trainer ID, checksum). If the newest one doesn't check out, the older one is used.

### CPU overclock (version 4)

- Write `0x4300 | n` to `0x04FFF7F4` to run the CPU n times faster (n = 1 to 8). Video,
  sound, timers and DMA keep their normal speed, so only game code gets more time per frame.
  Bits 8–11 of the control register read back n. A reset sets it back to 1, and savestates
  keep it.
- The game sets 4× at boot (`PKX_OVERCLOCK` in include/gba/defines.h). In the three-player
  online test that cut the frames where the game slowed down from 214 to 95 for the
  watching player; 8× gave no more.
- `gLagFrames` (src/main.c) counts frames the main loop missed, for measuring.

### Bigger ROM (version 4)

- A ROM file can be up to 64 MiB. The first 32 MiB work as on a GBA: at `0x08000000`, with
  mirrors at `0x0A000000` and `0x0C000000`. The game relies on the `0x0A000000` mirror, because
  pokeemerald-expansion tags script functions with it.
- In a ROM larger than 32 MiB, the bytes after the first 32 MiB show at `0x0C000000`–`0x0DFFFFFF`
  instead of the mirror there.
- In the game, mark big read-only data `ROM2_DATA` (include/gba/defines.h). The linker puts it at
  `0x0C000000` and right after the first 32 MiB in the file. Nothing uses it yet, so the ROM
  stays 32 MiB and the .bps patch stays the same size until something does.

### More sprite VRAM (version 5)

- `0x01010000`–`0x01027FFF` holds three more 32 KiB banks of sprite tiles (1024 tiles each).
  Bits 2–3 of a sprite's attribute byte (`0x01002000` + entry) pick the bank it draws from;
  0 is the normal OBJ VRAM. Write the tiles with the CPU or DMA; read them back with the CPU.
- In the game, sprite tile numbers run from 0 to 4095: 0–1023 are normal VRAM, then the banks.
  `AllocSpriteTiles` and `LoadSpriteSheet` fill normal VRAM first and only spill into the banks
  when it's full, so nothing changes until a scene runs out. A block never crosses a bank
  boundary. Sheet sprites follow their sheet's bank, other sprites keep theirs in
  `sprite->tileBank`. Code that writes tiles by hand should use `SpriteTileAddr(tileNum)`
  instead of `OBJ_VRAM0 + tileNum * 32`.

### More background palettes (version 5)

- `0x01003000`–`0x010035FF` holds 48 more background palettes: banks 1–3 of 16 palettes each.
- The register at `0x04FFF7F6` picks a bank for each background layer (2 bits per layer). A layer
  on bank b looks up its tiles' palette n in bank b instead of the normal palette n. Only
  text-mode backgrounds use it (the field, menus and battles are all text mode).
- In the game: `LoadExtBgPalette(src, bank, n, size)` and `SetBgPaletteBank(bg, bank)`
  (include/palette.h). They live in the palette buffers at `EXT_BG_PLTT_ID(bank, n)` and fade,
  tint, follow weather and time of day together with normal BG palette n. `ResetBgs` puts every
  layer back on the normal palettes, so a new screen always starts clean.

## Planned (later versions)

Nothing yet. Ask in the project if something else hits a GBA limit.
