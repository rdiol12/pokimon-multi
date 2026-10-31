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


# --- Photo safari (src/life_photo.c) -----------------------------------------
# One corner of the camera's viewfinder (the other three are flips of it).
FINDER_KEYS = ".owg"
FINDER_PAL = [(0, 0, 0), (0x30, 0x30, 0x38), (0xF8, 0xF8, 0xF8), (0xC8, 0xC8, 0xD0)]
FINDER = [
    "oooooooooooo....",
    "owwwwwwwwwwo....",
    "owggggggggoo....",
    "owgoooooooo.....",
    "owgo............",
    "owgo............",
    "owgo............",
    "owgo............",
    "owgo............",
    "owgo............",
    "owoo............",
    "ooo.............",
    "................",
    "................",
    "................",
    "................",
]
png("photo_finder", FINDER_PAL, FINDER, FINDER_KEYS)


# --- Team Eclipse (src/life_eclipse.c) ---------------------------------------
# The postgame villains. Their battle pictures are Emerald's Team Aqua
# grunts, a Magma Admin's hooded outfit and the Hex Maniac redressed in
# Eclipse colours: midnight violet with gold, and the Aqua "A" on the
# bandanas replaced with a gold crescent moon.
TRAINERS = os.path.join(here, "..", "..", "graphics", "trainers", "front_pics")
GOLD, GOLD_LIGHT, GOLD_DARK = (230, 189, 74), (255, 230, 139), (172, 123, 41)

# A crescent moon, 4x5, opening to the right.
CRESCENT = [
    ".GH.",
    "GH..",
    "G...",
    "GD..",
    ".GD.",
]


def recolor(base, name, colors, logo=None, crescent_at=None, keys=None):
    img = Image.open(os.path.join(TRAINERS, base + ".png"))
    pal = img.getpalette()[:48]
    for index, rgb in colors.items():
        pal[index * 3:index * 3 + 3] = list(rgb)
    img.putpalette(pal)
    px = img.load()
    if logo:
        # Paint the old emblem over with the cloth around it.
        (x0, y0, x1, y1), old, cloth = logo
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                if px[x, y] in old:
                    px[x, y] = cloth
    if crescent_at:
        cx, cy = crescent_at
        for dy, row in enumerate(CRESCENT):
            for dx, ch in enumerate(row):
                if ch != ".":
                    px[cx + dx, cy + dy] = keys[ch]
    img.save(os.path.join(out, name + ".png"))


VIOLET = {5: (164, 139, 222), 6: (115, 90, 189), 7: (65, 49, 123), 8: (41, 33, 57)}
# Grunts: index 11 is free for the gold, 9 (light grey) is its highlight.
recolor("aqua_grunt_m", "eclipse_grunt_m", {**VIOLET, 11: GOLD},
        logo=((28, 8, 33, 14), (14, 5), 6), crescent_at=(29, 9), keys={"G": 11, "H": 9, "D": 7})
recolor("aqua_grunt_f", "eclipse_grunt_f", {**VIOLET, 11: GOLD, 10: (90, 74, 131), 12: (148, 131, 197), 13: (115, 98, 164)},
        logo=((34, 5, 39, 11), (14, 5), 6), crescent_at=(35, 6), keys={"G": 11, "H": 9, "D": 7})
# The admin: a violet hood, the Magma emblem on the chest swapped for the
# crescent (indices 7 and 9 are free for the gold).
recolor("magma_admin", "eclipse_admin", {10: (57, 41, 98), 12: (139, 106, 205), 13: (98, 74, 156),
                                         5: (98, 90, 123), 6: (65, 57, 90), 8: (41, 33, 57), 7: GOLD, 9: GOLD_LIGHT},
        logo=((32, 22, 38, 27), (15,), 10), crescent_at=(34, 23), keys={"G": 7, "H": 9, "D": 8})
# The leader: a midnight robe and hat with a gold crescent on it.
recolor("hex_maniac", "eclipse_leader", {5: (90, 98, 164), 6: (57, 65, 131), 7: (41, 41, 90), 8: (24, 24, 49),
                                         12: (164, 156, 197), 13: (213, 205, 238), 11: GOLD},
        crescent_at=(34, 8), keys={"G": 11, "H": 14, "D": 7})

# Overworld: one palette for every Eclipse member (graphics/object_events'
# npc_4 with the blues and pinks turned violet and the shirt stripes gold).
with open(os.path.join(here, "..", "..", "graphics", "object_events", "palettes", "npc_4.pal")) as f:
    lines = f.read().split("\n")
ow = [tuple(int(v) for v in line.split()) for line in lines[3:19]]
ow[5], ow[6] = (148, 115, 205), (74, 57, 131)
ow[8], ow[9], ow[10] = (205, 164, 238), (139, 82, 197), (65, 32, 106)
ow[11] = GOLD
with open(os.path.join(out, "eclipse_ow.pal"), "w") as f:
    f.write("JASC-PAL\r\n0100\r\n16\r\n" + "".join("%d %d %d\r\n" % c for c in ow))
