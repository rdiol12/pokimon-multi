# Pokimon extended GBA

Pokimon Multi runs on our own fork of [mGBA](https://mgba.io) that adds hardware a real
GBA never had. The game is still an ordinary GBA ROM built with the ARM toolchain, so
releases stay a `.bps` patch against the player's own ROM. The fork is upstream mGBA at
the commit in `MGBA_COMMIT` plus the patches in `patches/`, under mGBA's MPL-2.0 licence.
It contains no Nintendo code or data.

Why: the GBA's 256 KiB of EWRAM was 96% full, and the 16 palettes and 128 sprite slots
were blocking features. See the plan doc linked from the project thread
"Port the game off the GBA".

## Building

```
emulator/build.sh headless   # libmgba.so + mgba-headless: make check, e2e tests
emulator/build.sh sdl        # desktop player (SDL)
emulator/build.sh qt         # desktop player (Qt)
emulator/build.sh wasm       # mgba.js + mgba.wasm for the phone app (needs emcc)
```

Output goes to `build/emulator/`. `make check` uses it with
`make check ROMTEST=build/emulator/headless/mgba-headless`.

## Hardware

### Detection registers

| Address      | Access | Value                                                        |
| ------------ | ------ | ------------------------------------------------------------ |
| `0x04FFF7F0` | read   | `0x504B` ("PK") on our hardware                              |
| `0x04FFF7F2` | read   | hardware version, currently `1`                              |
| `0x04FFF7F4` | write  | `0x4558` ("EX") unlocks extended EWRAM until the next reset  |
| `0x04FFF7F4` | read   | bit 0 set once extended EWRAM is unlocked                    |

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

## Planned (later versions)

More sprite and background palettes, more OAM slots without the per-line sprite limit,
more VRAM, a CPU overclock, a ROM larger than 32 MiB and a larger save.
