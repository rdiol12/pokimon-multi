"""The Prism Caverns' tileset: the cave tiles of Victory Road and Granite
Cave with the rock turned a cold violet, plus new pixel art in Emerald's
style: crystal clusters and formations, glittering shards on the floor, the
four resonance crystals (ruby, sapphire, emerald and topaz, each dull or
lit), the glowing teleport pads of each colour (dormant or awake), the
crystal seal before the heart chamber, the heart crystal itself and the
geologists' camp.

    python3 tools/areas/crystal_art.py

Writes data/tilesets/secondary/crystal/ and tools/areas/crystal_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

CAVE_FLOOR = 0x201        # the caves' floor shows under see-through pixels
MB_AQUA_HIDEOUT_WARP = 0x67   # a teleport pad: step on it and you vanish in a flash

# The cave's palette 6 (its brown rock) turned to violet stone. Entries 8-11
# (the lights) stay as they were.
PAL_ROCK = [(255, 131, 123), (222, 222, 255), (189, 189, 238), (156, 156, 213), (123, 123, 189), (98, 90, 164),
            (74, 65, 131), (49, 41, 90), (255, 255, 255), (255, 189, 197), (255, 65, 106), (180, 0, 0),
            (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0)]

# Palettes 7, 8 and 10 (the lake banks and pebbles) in the same violet.
PAL_BANK = [(115, 197, 164)] + PAL_ROCK[1:8] + [(0, 0, 0)] * 8
PAL_SHORE = [(115, 197, 164), (255, 255, 255), (230, 230, 255), (205, 205, 238), (172, 164, 222), (139, 131, 197),
             (106, 98, 164), (74, 65, 123), (255, 255, 255), (123, 205, 255), (57, 123, 255), (0, 41, 255),
             (255, 189, 197), (255, 65, 106), (180, 0, 0), (49, 8, 49)]
PAL_PEBBLE = [(0, 0, 0), (205, 205, 238), (172, 172, 222), (139, 131, 197), (106, 98, 164), (255, 115, 65),
              (230, 115, 65), (180, 98, 90), (164, 82, 82), (205, 49, 0), (148, 49, 0), (246, 24, 24), (246, 148, 0),
              (0, 0, 0), (0, 0, 0), (0, 0, 0)]

# Palette 12: the crystals in their four colours, glints and pedestal stone.
PAL = [(115, 197, 164), (33, 25, 57), (90, 74, 156), (148, 131, 222), (205, 197, 255), (255, 255, 255),
       (189, 41, 82), (255, 123, 156), (41, 98, 205), (115, 197, 255), (32, 148, 98), (123, 230, 156),
       (205, 148, 24), (255, 222, 90), (74, 74, 98), (131, 131, 156)]
OUT, PURP_D, PURP, PURP_L, WHITE, RED_D, RED_L, BLUE_D, BLUE_L, GRN_D, GRN_L, GOLD_D, GOLD_L, STONE_D, STONE = range(1, 16)
COLOURS = {'red': (RED_D, RED_L), 'blue': (BLUE_D, BLUE_L), 'green': (GRN_D, GRN_L), 'gold': (GOLD_D, GOLD_L),
           'white': (PURP, WHITE)}


def prism(c, x, y, w, h, dark, light, glint=True):
    """One crystal point standing up: a hexagonal column with a pointed tip,
    lit from the upper left (light face left, dark face right)."""
    tip = max(2, w // 2)
    for i in range(h):
        yy = y + i
        if i < tip:                               # the point narrows to the top
            half = (w * (i + 1)) // (2 * tip)
        else:
            half = w // 2
        cx = x + w // 2
        c.rect(cx - half, yy, cx - 1, yy, light)
        c.rect(cx, yy, cx + half - 1 + (w % 2), yy, dark)
    if glint:
        c.px(x + w // 2 - 2, y + tip + 1, WHITE)
        c.px(x + w // 2 - 2, y + tip + 2, WHITE)


# ---------------------------------------------------------------- crystals
def cluster():
    """A cluster of violet crystals growing from a rock, one block wide, two tall."""
    c = Canvas(16, 32)
    c.disc(8, 27, 6.5, STONE)
    c.rect(2, 26, 13, 30, STONE)
    c.rect(2, 29, 13, 30, STONE_D)
    prism(c, 5, 4, 6, 24, PURP, PURP_L)
    prism(c, 1, 13, 5, 15, PURP_D, PURP)
    prism(c, 10, 11, 5, 17, PURP, PURP_L)
    c.outline(OUT)
    return c


def formation():
    """A great crystal formation, two blocks by two."""
    c = Canvas(32, 32)
    c.disc(16, 26, 13, STONE)
    c.rect(3, 27, 28, 30, STONE_D)
    prism(c, 12, 1, 8, 27, PURP, PURP_L)
    prism(c, 5, 9, 7, 19, PURP_D, PURP)
    prism(c, 20, 6, 7, 22, PURP, PURP_L)
    prism(c, 1, 17, 5, 11, PURP, PURP_L, glint=False)
    prism(c, 26, 16, 5, 12, PURP_D, PURP, glint=False)
    c.outline(OUT)
    return c


def small():
    """A low crystal clump in the way, one block."""
    c = Canvas(16, 16)
    c.disc(8, 12, 6, STONE)
    c.rect(2, 12, 13, 14, STONE_D)
    prism(c, 6, 1, 5, 12, PURP, PURP_L)
    prism(c, 2, 6, 4, 8, PURP_D, PURP, glint=False)
    prism(c, 10, 5, 4, 9, PURP, PURP_L, glint=False)
    c.outline(OUT)
    return c


def shards():
    """Crystal splinters glittering on the floor; you can walk over them."""
    c = Canvas(16, 16)
    r = np.random.RandomState(11)
    for _ in range(5):
        x, y = r.randint(1, 13), r.randint(2, 14)
        c.rect(x, y, x + 1, y, PURP_L)
        c.px(x + 1, y - 1, WHITE)
        c.px(x, y + 1, PURP_D)
    return c


def geode():
    """A split geode on the floor, purple inside."""
    c = Canvas(16, 16)
    c.disc(8, 9, 6, STONE)
    c.disc(8, 9, 4, PURP_D)
    c.disc(8, 9, 2.5, PURP_L)
    c.px(7, 8, WHITE)
    c.outline(OUT)
    return c


def resonance(colour, lit):
    """A resonance crystal on its stone pedestal: dull grey until you touch
    it, then shining in its colour with a ring of light."""
    dark, light = COLOURS[colour] if lit else (STONE_D, STONE)
    c = Canvas(16, 32)
    if lit:
        for x, y in ((1, 6), (14, 4), (0, 15), (15, 13), (2, 1), (12, 0)):
            c.px(x, y, light)
            c.px(x, y + 1, WHITE)
    c.rect(2, 25, 13, 30, STONE)
    c.rect(2, 25, 13, 26, WHITE if lit else STONE)
    c.rect(2, 29, 13, 30, STONE_D)
    c.rect(4, 22, 11, 24, STONE_D)
    prism(c, 4, 3, 8, 20, dark, light, glint=lit)
    if lit:
        c.rect(6, 12, 6, 15, WHITE)
    c.outline(OUT)
    return c


def pad(colour, awake):
    """A teleport pad: a hexagonal crystal plate set in the floor. Awake, it
    glows in its colour; dormant, only its rim shows the colour, the plate is
    dull grey and nothing happens."""
    dark, light = COLOURS[colour][0], COLOURS[colour][1] if awake else STONE
    c = Canvas(16, 16)
    rows = [(5, 10), (3, 12), (2, 13), (1, 14), (1, 14), (1, 14), (1, 14), (1, 14), (1, 14), (1, 14), (1, 14),
            (2, 13), (3, 12), (5, 10)]
    for i, (x0, x1) in enumerate(rows):
        c.rect(x0, i + 1, x1, i + 1, dark)
    for i, (x0, x1) in enumerate(rows[2:-2]):
        c.rect(x0 + 2, i + 3, x1 - 2, i + 3, light)
    c.disc(8, 8, 2.6, WHITE if awake else STONE_D)
    if colour == 'white' and awake:                # the secret pad shows every colour
        for (x, y), col in zip(((5, 5), (10, 5), (5, 11), (10, 11)), (RED_L, BLUE_L, GRN_L, GOLD_L)):
            c.rect(x, y, x + 1, y + 1, col)
    c.outline(OUT)
    return c


def seal():
    """The crystal seal across the heart chamber's mouth: a wall of pink and
    white crystal, three blocks wide and two tall."""
    c = Canvas(48, 32)
    c.rect(0, 26, 47, 31, STONE_D)
    for x, h, d, l in ((0, 22, RED_D, RED_L), (6, 30, PURP, PURP_L), (13, 26, RED_D, RED_L), (20, 31, PURP, WHITE),
                       (28, 27, RED_D, RED_L), (35, 30, PURP, PURP_L), (41, 23, RED_D, RED_L)):
        prism(c, x, 31 - h, 7, h, d, l)
    c.outline(OUT)
    return c


def heart():
    """The heart crystal: a great pink jewel on a stone dais, two blocks by two."""
    c = Canvas(32, 32)
    c.rect(2, 25, 29, 31, STONE)
    c.rect(2, 25, 29, 26, WHITE)
    c.rect(2, 30, 29, 31, STONE_D)
    pts = [(16, 1), (27, 9), (16, 24), (5, 9)]      # a cut diamond
    yy, xx = np.mgrid[0:32, 0:32]
    inside = np.ones((32, 32), bool)
    for (x0, y0), (x1, y1) in zip(pts, pts[1:] + pts[:1]):
        inside &= (xx + 0.5 - x0) * (y1 - y0) - (yy + 0.5 - y0) * (x1 - x0) <= 0
    c.a[inside] = RED_L
    c.a[inside & (xx >= 16)] = RED_D
    c.a[inside & (yy < 9)] = WHITE
    c.a[inside & (yy < 9) & (xx >= 16)] = RED_L
    c.line(5, 9, 27, 9, RED_D)
    c.line(11, 3, 16, 24, RED_D)
    c.line(21, 3, 16, 24, RED_L)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- the camp
def tent():
    """The geologists' tent, two blocks by two, its flap open to the south."""
    c = Canvas(32, 32)
    for i in range(24):
        half = 3 + i * 12 // 23
        c.rect(16 - half, 4 + i, 15, 4 + i, GOLD_L)
        c.rect(16, 4 + i, 15 + half, 4 + i, GOLD_D)
    c.rect(12, 16, 19, 27, OUT)                       # the open flap
    c.line(12, 16, 16, 9, OUT)
    c.line(19, 16, 16, 9, OUT)
    c.rect(15, 1, 16, 4, STONE_D)                     # the pole
    c.rect(3, 28, 28, 28, STONE_D)
    c.outline(OUT)
    return c


def lamp():
    """A camp lantern on a tripod."""
    c = Canvas(16, 32)
    c.line(8, 14, 3, 30, STONE_D)
    c.line(8, 14, 13, 30, STONE_D)
    c.rect(7, 14, 8, 30, STONE)
    c.rect(5, 5, 10, 13, STONE)
    c.rect(6, 7, 9, 11, GOLD_L)
    c.rect(7, 8, 8, 9, WHITE)
    c.rect(6, 3, 9, 4, STONE_D)
    c.outline(OUT)
    return c


def crate():
    """A sample crate full of crystals."""
    c = Canvas(16, 16)
    c.rect(1, 6, 14, 15, GOLD_D)
    c.rect(1, 6, 14, 7, GOLD_L)
    c.rect(1, 11, 14, 11, OUT)
    prism(c, 3, 1, 4, 6, PURP, PURP_L, glint=False)
    prism(c, 8, 0, 4, 7, BLUE_D, BLUE_L, glint=False)
    c.outline(OUT)
    return c


def ceiling():
    """The rock above the halls, darker than the floor so the halls read
    clearly (palette 6, the violet rock)."""
    c = Canvas(16, 16)
    c.a[:] = 6
    r = np.random.RandomState(5)
    for _ in range(10):
        x, y = r.randint(0, 15), r.randint(0, 16)
        c.rect(x, y, x + 1, y, 7)
        c.px(x, y - 1 if y else y, 5)
    return c


def over_top(r, c):
    return r == 0


OBJECTS = {
    'cluster': (cluster, 12, {'over': over_top}),
    'formation': (formation, 12, {'over': over_top}),
    'small': (small, 12, {}),
    'shards': (shards, 12, {'walk': lambda r, c: True}),
    'geode': (geode, 12, {}),
    'seal': (seal, 12, {'over': over_top}),
    'heart': (heart, 12, {'over': over_top}),
    'tent': (tent, 12, {'over': over_top}),
    'lamp': (lamp, 12, {'over': over_top}),
    'crate': (crate, 12, {}),
    'ceiling': (ceiling, 6, {}),
}
for _col in ('red', 'blue', 'green', 'gold'):
    OBJECTS[f'res_{_col}'] = (lambda col=_col: resonance(col, False), 12, {'over': over_top})
    OBJECTS[f'res_{_col}_lit'] = (lambda col=_col: resonance(col, True), 12, {'over': over_top})
for _col in ('red', 'blue', 'green', 'gold', 'white'):
    OBJECTS[f'pad_{_col}'] = (lambda col=_col: pad(col, True), 12, {'behavior': lambda r, c: MB_AQUA_HIDEOUT_WARP})
    OBJECTS[f'pad_{_col}_dull'] = (lambda col=_col: pad(col, False), 12, {'walk': lambda r, c: True})


def main():
    pack('data/tilesets/secondary/cave', 'data/tilesets/secondary/crystal', OBJECTS, {6: PAL_ROCK, 7: PAL_SHORE, 8: PAL_PEBBLE, 10: PAL_BANK, 12: PAL}, CAVE_FLOOR,
         f'{ROOT}/tools/areas/crystal_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
