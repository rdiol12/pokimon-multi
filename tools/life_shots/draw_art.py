#!/usr/bin/env python3
"""Draws the fifth batch's original sprites as indexed 16-colour PNGs
(graphics/life/). Colours sit on the GBA's 5-bit grid and follow Emerald's
style: a dark 1px outline, a flat base, one shadow and one highlight tone,
light from the top left. Index 0 is the transparent colour."""
import os
import sys
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, "..", "..", "graphics", "life")


def png(name, palette, rows, keys):
    h, w = len(rows), len(rows[0])
    assert all(len(r) == w for r in rows), name
    img = Image.new("P", (w, h))
    flat = []
    for c in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += c
    img.putpalette(flat)
    index = {ch: i for i, ch in enumerate(keys)}
    img.putdata([index[ch] for row in rows for ch in row])
    os.makedirs(os.path.dirname(os.path.join(out, name)), exist_ok=True)
    img.save(os.path.join(out, name + ".png"))


# --- Bonding (src/life_bond.c) ---------------------------------------------
# One palette for the hand and the hearts.
# . transparent  o outline  w glove white  g glove shadow  G glove dark
# c cuff blue  C cuff shadow  r heart red  R heart shadow  p heart pink
# h highlight  e empty heart  E empty shadow
BOND_KEYS = ".owgGcCrRpheE"
BOND_PAL = [(0x78, 0xC8, 0xF8), (0x30, 0x30, 0x38), (0xF8, 0xF8, 0xF8), (0xC8, 0xC8, 0xD0),
            (0x98, 0x98, 0xA8), (0x50, 0x88, 0xE0), (0x30, 0x58, 0xA8), (0xE8, 0x40, 0x58),
            (0xA8, 0x20, 0x38), (0xF8, 0x90, 0xA0), (0xF8, 0xE0, 0xE8), (0xD8, 0xD0, 0xD0),
            (0xA0, 0x98, 0x98)]
HAND = [
    "....oo..........",
    "...owgo.........",
    "...owgo.........",
    "...owgo.........",
    "...owgoooo......",
    "...owgowgooo....",
    ".oooowgowgowgo..",
    "owwgowwwwgowgoo.",
    "owwwowwwwwwwwgGo",
    ".owwwwwwwwwwwgGo",
    "..owwwwwwwwwggGo",
    "..owwwwwwwwwgGo.",
    "...owwwwwwwggGo.",
    "....occcccccCo..",
    "....occcccccCo..",
    ".....oooooooo...",
]
png("bond_hand", BOND_PAL, HAND, BOND_KEYS)

HEART_BIG = [
    "................",
    "................",
    "..ooo.....ooo...",
    ".ohhpo...oprro..",
    "ohhpprro.orrrRo.",
    "ohprrrrrorrrrRo.",
    "oprrrrrrrrrrrRo.",
    "oprrrrrrrrrrrRo.",
    ".orrrrrrrrrrRo..",
    "..orrrrrrrrRo...",
    "...orrrrrrRo....",
    "....orrrrRo.....",
    ".....orrRo......",
    "......oRo.......",
    ".......o........",
    "................",
]
png("bond_heart", BOND_PAL, HEART_BIG, BOND_KEYS)

# 16x8: a full heart and an empty one, for the meter.
HEARTS_SMALL = [
    ".oo.oo...oo.oo..",
    "ohproRo.oeeoeEo.",
    "oprrrRo.oeeeeEo.",
    "orrrrRo.oeeeeEo.",
    ".orrRo...oeeEo..",
    "..oRo.....oEo...",
    "...o.......o....",
    "................",
]
png("bond_hearts_small", BOND_PAL, HEARTS_SMALL, BOND_KEYS)

if __name__ == "__main__" and len(sys.argv) > 1:
    print("wrote", out)


# --- Underground mining (src/life_mining.c) ----------------------------------
# BG tiles for the dig wall: three rock layers (16x16 blocks, two variants
# each) over packed earth, in the browns of Emerald's cave tilesets.
# . clear (shows the earth below)  o outline  d dark rock  m rock  l light rock
# h highlight  e earth dark  E earth  r earth light  c crack
import random

MINE_KEYS = ".odmlheErc"
MINE_PAL = [(0, 0, 0), (0x38, 0x28, 0x20), (0x68, 0x48, 0x30), (0x98, 0x70, 0x48),
            (0xC8, 0xA0, 0x68), (0xE8, 0xD0, 0x98), (0x50, 0x38, 0x28), (0x78, 0x58, 0x38),
            (0xA0, 0x78, 0x50), (0x28, 0x18, 0x10)]


def boulders(seed, fill, light, dark, outline="o"):
    """A 16x16 block of rounded stones: each stone gets a dark outline, a
    highlight on its top left and shadow on its bottom right."""
    rnd = random.Random(seed)
    grid = [[fill] * 16 for _ in range(16)]
    stones = []
    for cy, cx, r in ((4, 4, 4), (4, 12, 4), (12, 4, 4), (12, 12, 4), (8, 8, 3)):
        stones.append((cy + rnd.randint(-1, 1), cx + rnd.randint(-1, 1), r + rnd.randint(0, 1)))
    for y in range(16):
        for x in range(16):
            for cy, cx, r in stones:
                d = (x - cx) ** 2 + (y - cy) ** 2
                if d <= r * r:
                    if d >= (r - 1) ** 2 + 1:
                        grid[y][x] = outline
                    elif (x - cx) + (y - cy) < -r // 2:
                        grid[y][x] = light
                    elif (x - cx) + (y - cy) > r // 2:
                        grid[y][x] = dark
                    else:
                        grid[y][x] = fill
    return ["".join(r) for r in grid]


def soft(seed):
    """Loose earth: speckled, with a few pebbles."""
    rnd = random.Random(seed)
    rows = []
    for y in range(16):
        row = ""
        for x in range(16):
            v = rnd.random()
            row += "r" if v < 0.18 else ("e" if v < 0.3 else "E")
        rows.append(row)
    rows = [list(r) for r in rows]
    for _ in range(3):
        px, py = rnd.randint(1, 13), rnd.randint(1, 13)
        rows[py][px:px + 2] = "lh"
        rows[py + 1][px:px + 2] = "md"
    return ["".join(r) for r in rows]


def earth():
    """The bottom: dark packed earth (one 8x8 tile, repeated)."""
    rnd = random.Random(7)
    return ["".join("e" if rnd.random() < 0.8 else ("o" if rnd.random() < 0.5 else "E") for _ in range(8)) for _ in range(8)]


# Tile sheet, 8 pixels wide, one 8x8 tile after another: earth, then the
# blocks (soft x2, rock x2, hard x2) cut into four tiles each (TL, TR, BL, BR).
blocks = [soft(1), soft(2),
          boulders(3, "m", "l", "d"), boulders(4, "m", "l", "d"),
          boulders(5, "d", "m", "o"), boulders(6, "d", "m", "o")]
# Hard rock is cracked.
for b in blocks[4:]:
    rows = [list(r) for r in b]
    rnd = random.Random(len(b[0]))
    x = 3
    for y in range(2, 14):
        rows[y][x] = "c"
        x = max(1, min(14, x + rnd.choice((-1, 0, 1))))
    b[:] = ["".join(r) for r in rows]
sheet = earth()
for b in blocks:
    for ty in (0, 8):
        for tx in (0, 8):
            sheet += [row[tx:tx + 8] for row in b[ty:ty + 8]]
png("mine_tiles", MINE_PAL, sheet, MINE_KEYS)

# The cursor: a 16x16 white frame with a dark edge, open in the middle.
CURSOR_KEYS = ".ow"
CURSOR_PAL = [(0, 0, 0), (0x30, 0x30, 0x38), (0xF8, 0xF8, 0xF8)]
CURSOR = [
    "oooooo....oooooo",
    "owwwwo....owwwwo",
    "owoooo....oooowo",
    "owo..........owo",
    "owo..........owo",
    "oo............oo",
    "................",
    "................",
    "................",
    "................",
    "oo............oo",
    "owo..........owo",
    "owo..........owo",
    "owoooo....oooowo",
    "owwwwo....owwwwo",
    "oooooo....oooooo",
]
png("mine_cursor", CURSOR_PAL, CURSOR, CURSOR_KEYS)
