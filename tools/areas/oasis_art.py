"""The Mirage Oasis tileset: Route 111's desert (the Mauville tiles it uses;
the city's are cleared to make room) plus new pixel art in Emerald's
style: date palms and their mirage twins, mirage ponds that shimmer in the
heat, striped tents, market stalls, the village well (dry and flowing),
the cracked bed of the dried-up pond, quicksand pits you sink into, jars,
rugs, the spring altar and the oasis gate.

    python3 tools/areas/oasis_art.py

Writes data/tilesets/secondary/oasis/ and tools/areas/oasis_tiles.json.
"""
import json, os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

SAND = 0x251               # Route 111's sand shows under see-through pixels
MB_MT_PYRE_HOLE = 0x0F     # a quicksand pit: step in and you sink to the ruins below
MB_SAND = 0x21             # footprints in the sand

# Palette 12: palms and the ground.
PAL = [(115, 197, 164), (57, 41, 33), (123, 82, 49), (172, 123, 74), (213, 172, 106), (41, 115, 57),
       (82, 172, 74), (148, 213, 98), (189, 156, 98), (230, 205, 139), (156, 115, 74), (255, 255, 238),
       (82, 156, 230), (148, 205, 246), (106, 74, 49), (238, 222, 172)]
OUT, TRUNK_D, TRUNK, TRUNK_L, LEAF_D, LEAF, LEAF_L, SAND_D, SAND_C, MUD, WHITE, WATER, WATER_L, MUD_D, SAND_L = range(1, 16)

# Palette 10: cloth and goods.
PAL_CLOTH = [(115, 197, 164), (57, 41, 33), (172, 41, 49), (230, 82, 74), (255, 148, 115), (41, 74, 164),
             (82, 131, 222), (205, 156, 41), (246, 213, 82), (255, 255, 238), (123, 57, 148), (180, 106, 205),
             (123, 82, 49), (172, 123, 74), (82, 172, 74), (238, 131, 41)]
C_OUT, RED_D, RED, RED_L, BLUE_D, BLUE, GOLD_D, GOLD, C_WHITE, PURP_D, PURP, WOOD_D, WOOD, FRUIT_G, FRUIT_O = range(1, 16)

# Palette 9: stone, water and the mirage's pale shimmer.
PAL_STONE = [(115, 197, 164), (65, 57, 57), (123, 115, 106), (172, 164, 148), (222, 213, 197), (41, 98, 189),
             (82, 156, 230), (156, 213, 255), (255, 255, 255), (205, 238, 255), (180, 230, 205), (222, 246, 230),
             (98, 205, 189), (49, 164, 164), (238, 205, 106), (189, 148, 49)]
S_OUT, STONE_D, STONE, STONE_L, W_D, W, W_L, S_WHITE, HAZE, HAZE_G, HAZE_GL, GEM, GEM_D, S_GOLD, S_GOLD_D = range(1, 16)


# ---------------------------------------------------------------- palms
def palm_shape(c, trunk, trunk_d, leaf_d, leaf, leaf_l, nut, shadow=None):
    """A date palm 2 blocks wide and 3 tall: a leaning ringed trunk and a
    crown of drooping fronds."""
    for y in range(14, 46):
        x = 15 + (46 - y) // 8 - 2
        c.rect(x - 2, y, x + 2, y, trunk)
        c.px(x + 2, y, trunk_d)
        if y % 4 == 0:
            c.rect(x - 2, y, x + 1, y, trunk_d)
    # fronds: arcs drooping from the crown
    cx, cy = 16, 11
    for ang, ln in ((-178, 16), (-150, 15), (-122, 12), (-95, 9), (-58, 12), (-30, 15), (-2, 16), (165, 12),
                    (15, 12), (-80, 8)):
        a = np.radians(ang)
        for i in range(ln):
            t = i / ln
            x = cx + np.cos(a) * i
            y = cy + np.sin(a) * i * 0.8 + t * t * 13
            col = leaf_l if i < ln * 0.35 else leaf if i < ln * 0.75 else leaf_d
            w = 2 if i < ln * 0.7 else 1
            for k in range(w):
                c.px(x, y + k, col)
            c.px(x, y + w, leaf_d)
            if i % 3 == 1 and 2 < i < ln - 2:
                c.px(x + 1, y + w + 1, leaf)
                c.px(x - 1, y + w + 1, leaf_d)
    if nut:
        for x, y in ((14, 14), (17, 15), (15, 16)):
            c.disc(x + 0.5, y + 0.5, 1.4, nut)
    c.outline(OUT if trunk == TRUNK else 0)


def palm():
    c = Canvas(32, 48)
    palm_shape(c, TRUNK, TRUNK_D, LEAF_D, LEAF, LEAF_L, MUD_D)
    c.outline(OUT)
    yy, xx = np.mgrid[0:48, 0:32]                       # the shadow on the sand, under everything
    sh = (((xx + 0.5 - 19) / 10) ** 2 + ((yy + 0.5 - 45) / 2.6) ** 2 <= 1) & (c.a == 0)
    c.a[sh] = SAND_D
    return c


def palm_mirage():
    """The same palm as a heat-haze mirage: pale, no shadow, and every
    third line missing so it seems to waver."""
    c = Canvas(32, 48)
    palm_shape(c, HAZE_G, HAZE_G, HAZE_G, HAZE_GL, S_WHITE, None)
    c.a[c.a == OUT] = 0
    for y in range(0, 48, 3):
        c.a[y, :] = 0
    for y in range(1, 48, 6):
        c.a[y] = np.roll(c.a[y], 1)
    return c


def pond_mirage():
    """A shimmering pool of water that isn't there, 3 blocks by 2."""
    c = Canvas(48, 32)
    c.disc(24, 16, 15, HAZE)
    yy, xx = np.mgrid[0:32, 0:48]
    ell = ((xx + 0.5 - 24) / 22) ** 2 + ((yy + 0.5 - 16) / 12) ** 2
    c.a[:] = 0
    c.a[ell <= 1] = HAZE
    c.a[ell <= 0.6] = W_L
    for y in range(6, 28, 4):
        c.rect(14 + (y % 8), y, 24 + (y % 8), y, S_WHITE)
    for y in range(0, 32, 3):
        c.a[y, :] = 0
    return c


# ---------------------------------------------------------------- the village
def tent(main, light):
    """A striped canvas tent, 3 blocks by 3, its door flap at the bottom middle."""
    c = Canvas(48, 48)
    for i in range(36):
        half = 6 + i * 17 // 35
        y = 8 + i
        c.rect(24 - half, y, 23 + half, y, main)
        for x in range(24 - half, 24 + half):
            if ((x - 24) * 36 // max(half, 1)) // 9 % 2 == 0:
                c.px(x, y, light)
    c.rect(6, 44, 41, 47, WOOD_D)
    c.rect(18, 30, 29, 47, C_OUT)                      # the door
    c.rect(19, 31, 28, 47, WOOD_D)
    c.line(18, 30, 23, 22, C_OUT)
    c.line(29, 30, 24, 22, C_OUT)
    c.rect(23, 1, 24, 8, WOOD)                         # pole and pennant
    c.rect(25, 1, 30, 4, GOLD)
    c.outline(C_OUT)
    return c


def stall():
    """A market stall, 3 blocks by 2: a striped awning over a counter of fruit and jars."""
    c = Canvas(48, 32)
    for x in range(1, 47):
        c.rect(x, 1, x, 9, RED if (x // 6) % 2 else C_WHITE)
    for x in range(1, 47, 6):
        c.disc(x + 3, 10, 3, RED if (x // 6) % 2 else C_WHITE)
    c.rect(2, 10, 3, 28, WOOD_D)
    c.rect(44, 10, 45, 28, WOOD_D)
    c.rect(1, 20, 46, 29, WOOD)
    c.rect(1, 20, 46, 21, GOLD)
    for x, col in ((7, FRUIT_O), (11, FRUIT_O), (9, FRUIT_O), (19, FRUIT_G), (23, FRUIT_G), (21, RED)):
        c.disc(x, 17 if x in (9, 21) else 18.5, 2.2, col)
    c.rect(30, 12, 34, 19, BLUE)
    c.rect(31, 11, 33, 12, BLUE_D)
    c.rect(37, 13, 41, 19, PURP)
    c.rect(38, 12, 40, 13, PURP_D)
    c.outline(C_OUT)
    return c


def rug():
    """A woven rug laid on the sand, 2 blocks by 1; you can walk on it."""
    c = Canvas(32, 16)
    c.rect(1, 2, 30, 13, RED_D)
    c.rect(3, 4, 28, 11, RED)
    c.rect(6, 6, 25, 9, GOLD)
    for x in range(8, 24, 4):
        c.rect(x, 7, x + 1, 8, BLUE_D)
    for x in range(1, 31, 2):
        c.px(x, 1, GOLD_D)
        c.px(x, 14, GOLD_D)
    return c


def jar():
    c = Canvas(16, 16)
    c.disc(8, 10, 5.5, WOOD)
    c.rect(5, 2, 10, 5, WOOD)
    c.rect(4, 2, 11, 3, WOOD_D)
    c.rect(4, 9, 12, 10, FRUIT_O)
    c.px(6, 8, GOLD)
    c.outline(C_OUT)
    return c


def well(full):
    """The village well, 2 blocks by 2: a stone ring under a little roof,
    dark and empty, or brimming with spring water."""
    c = Canvas(32, 32)
    c.disc(16, 22, 12, STONE)
    c.disc(16, 22, 12, STONE_D, 11)
    for a in range(0, 360, 30):
        r = np.radians(a)
        c.px(16 + np.cos(r) * 11, 22 + np.sin(r) * 11 * 0.8, STONE_L)
    c.disc(16, 21, 8, W if full else S_OUT)
    if full:
        c.disc(16, 21, 6, W_L)
        c.rect(12, 18, 15, 18, S_WHITE)
        c.rect(17, 22, 19, 22, S_WHITE)
    c.rect(4, 2, 5, 18, STONE_D)
    c.rect(26, 2, 27, 18, STONE_D)
    c.rect(2, 1, 29, 4, S_GOLD_D)
    c.rect(2, 1, 29, 2, S_GOLD)
    c.rect(15, 5, 16, 12, STONE_D)                       # the rope
    c.outline(S_OUT)
    return c


def dry_bed():
    """The cracked mud of the dried-up pond; you can walk on it."""
    c = Canvas(16, 16)
    c.a[:] = MUD
    r = np.random.RandomState(4)
    for _ in range(3):
        x, y = r.randint(0, 16), r.randint(0, 16)
        for _ in range(7):
            nx, ny = x + r.randint(-1, 3), y + r.randint(-2, 3)
            c.line(x, y, nx, ny, MUD_D)
            x, y = nx % 16, ny % 16
    for _ in range(6):
        c.px(r.randint(0, 16), r.randint(0, 16), SAND_D)
    return c


def pit():
    """A quicksand pit: a swirl of loose sand sinking into the dark."""
    c = Canvas(16, 16)
    c.disc(8, 8, 7.5, SAND_D)
    c.disc(8, 8, 5.5, MUD)
    c.disc(8, 8, 3, MUD_D)
    c.disc(8, 8, 1.5, OUT)
    for t in np.linspace(0, 4 * np.pi, 40):
        rr = 1.5 + t * 0.45
        c.px(8 + np.cos(t) * rr, 8 + np.sin(t) * rr, SAND_C)
    return c


def altar(stone):
    """The spring altar in the ruins, 2 blocks by 2: a carved plinth that
    once held the blue Spring Stone."""
    c = Canvas(32, 32)
    c.rect(3, 14, 28, 30, STONE)
    c.rect(3, 14, 28, 16, STONE_L)
    c.rect(3, 28, 28, 30, STONE_D)
    for x in range(6, 26, 5):
        c.rect(x, 20, x + 2, 24, STONE_D)
    c.rect(10, 10, 21, 14, STONE_D)
    if stone:
        c.disc(16, 7, 5.5, GEM_D)
        c.disc(15, 6, 4, GEM)
        c.rect(13, 3, 14, 4, S_WHITE)
    c.outline(S_OUT)
    return c


def gate():
    """The oasis gate: two sandstone pillars under a lintel, 4 blocks by 3;
    you walk through the middle."""
    c = Canvas(64, 48)
    for x0 in (2, 50):
        c.rect(x0, 8, x0 + 11, 47, STONE_L)
        c.rect(x0 + 9, 8, x0 + 11, 47, STONE)
        for y in range(14, 46, 8):
            c.rect(x0, y, x0 + 11, y, STONE)
    c.rect(0, 2, 63, 12, S_GOLD)
    c.rect(0, 2, 63, 4, STONE_L)
    c.rect(0, 11, 63, 12, S_GOLD_D)
    for x in range(8, 58, 8):
        c.disc(x, 7, 1.8, GEM)
    c.outline(S_OUT)
    return c


def pillar(broken):
    """A sandstone column of the sunken court, 1 block by 2, or its stump."""
    c = Canvas(16, 32)
    top = 14 if broken else 1
    c.rect(3, top, 12, 29, STONE_L)
    c.rect(10, top, 12, 29, STONE)
    for y in range(top + 4, 28, 5):
        c.rect(3, y, 12, y, STONE)
    c.rect(1, 28, 14, 31, STONE)
    c.rect(1, 28, 14, 28, STONE_L)
    if broken:
        for x, y in ((3, 13), (5, 12), (8, 13), (11, 12)):
            c.px(x, y, STONE_L)
    else:
        c.rect(1, 1, 14, 4, S_GOLD)
        c.rect(1, 1, 14, 1, STONE_L)
    c.outline(S_OUT)
    if broken:
        c.a[:16] = 0
        c.a[:13, :] = 0
    return c


def rubble():
    c = Canvas(16, 16)
    for x, y, r in ((5, 10, 3.5), (11, 11, 3), (8, 6, 2.5)):
        c.disc(x, y, r, STONE_L)
        c.disc(x + 1, y + 1, r - 1.5, STONE)
    c.outline(S_OUT)
    return c


def stairs():
    """Sandstone steps cut into the cliff, leading up to the dunes."""
    c = Canvas(16, 16)
    c.a[:] = STONE_D
    for i, y in enumerate(range(1, 16, 4)):
        c.rect(1, y, 14, y + 2, STONE_L if i % 2 else S_WHITE)
        c.rect(1, y + 3, 14, y + 3, STONE)
    c.rect(0, 0, 0, 15, S_OUT)
    c.rect(15, 0, 15, 15, S_OUT)
    return c


def haze():
    """A wall of shimmering heat haze that hides the way to the real oasis."""
    c = Canvas(16, 16)
    for y in range(16):
        for x in range(16):
            v = (x * 3 + y * 5 + (y // 2) * 7) % 11
            if v < 8:
                c.a[y, x] = HAZE if v < 4 else HAZE_GL if v < 6 else S_WHITE
    return c


def over_top(n):
    return lambda r, c: r < n


OBJECTS = {
    'palm': (palm, 12, {'over': over_top(2)}),
    'palm_mirage': (palm_mirage, 9, {'over': over_top(2), 'walk': lambda r, c: True}),
    'pond_mirage': (pond_mirage, 9, {'walk': lambda r, c: True}),
    'tent_red': (lambda: tent(RED, RED_L), 10, {'over': over_top(1), 'door': lambda r, c: (r, c) == (2, 1)}),
    'tent_blue': (lambda: tent(BLUE_D, BLUE), 10, {'over': over_top(1), 'door': lambda r, c: (r, c) == (2, 1)}),
    'stall': (stall, 10, {'over': over_top(1), 'counter': lambda r, c: r == 1}),
    'rug': (rug, 10, {'walk': lambda r, c: True}),
    'jar': (jar, 10, {}),
    'well_dry': (lambda: well(False), 9, {'over': over_top(1)}),
    'well_full': (lambda: well(True), 9, {'over': over_top(1)}),
    'dry_bed': (dry_bed, 12, {'walk': lambda r, c: True}),
    'pit': (pit, 12, {'behavior': lambda r, c: MB_MT_PYRE_HOLE}),
    'altar': (lambda: altar(True), 9, {'over': over_top(1)}),
    'altar_empty': (lambda: altar(False), 9, {'over': over_top(1)}),
    'gate': (gate, 9, {'over': over_top(1), 'walk': lambda r, c: c in (1, 2)}),
    'pillar': (lambda: pillar(False), 9, {'over': over_top(1)}),
    'pillar_broken': (lambda: pillar(True), 9, {}),
    'rubble': (rubble, 9, {}),
    'stairs': (stairs, 9, {'door': lambda r, c: True}),
    'haze': (haze, 9, {}),
}


def route111_metatiles():
    L = {l['id']: l for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts'] if 'id' in l}
    b = np.fromfile(f"{ROOT}/{L['LAYOUT_ROUTE111']['blockdata_filepath']}", '<u2') & 0x3FF
    return set(b.tolist())


def main():
    pack('data/tilesets/secondary/mauville', 'data/tilesets/secondary/oasis', OBJECTS,
         {9: PAL_STONE, 10: PAL_CLOTH, 12: PAL}, SAND, f'{ROOT}/tools/areas/oasis_tiles.json', fill_free=True,
         keep=route111_metatiles())


if __name__ == '__main__':
    main()
