"""Whisperstem Valley's tileset: Rustboro's secondary tileset (the one Route
104 and Petalburg Woods use) plus new pixel art in Emerald's style: bamboo
groves that tile in any size, a teahouse, stone lanterns and a shrine gate.

    python3 tools/areas/bamboo_art.py

Writes data/tilesets/secondary/bamboo/ and tools/areas/bamboo_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001            # General's grass shows under see-through pixels

# Palette 12 (Rustboro leaves 6 and 12 free).
PAL = [(115, 197, 164), (33, 49, 57), (41, 98, 57), (82, 156, 74), (148, 205, 98), (205, 238, 148),
       (24, 106, 65), (57, 156, 90), (98, 57, 41), (156, 98, 57), (213, 156, 98), (57, 65, 98),
       (106, 115, 156), (238, 230, 205), (205, 57, 49), (164, 164, 156)]
OUT, BAM_D, BAM, BAM_L, BAM_HI, LEAF_D, LEAF, WOOD_D, WOOD, WOOD_L, ROOF_D, ROOF, PAPER, RED, STONE = range(1, 16)


# ---------------------------------------------------------------- bamboo
# A grove is drawn from 2x1 pieces: a leafy top row, any number of middle
# rows (the stalks repeat every 16 pixels, so they tile), and a bottom row.
FRONT = [(1, 3), (11, 9), (22, 14), (28, 4)]       # (x, node height) of the near stalks
BACK = [(7, 11), (17, 2), (26, 8)]


def stalk(c, x, node, y0, y1, back=False, w=4):
    if back:
        c.rect(x, y0, x + 1, y1, BAM_D)
        return
    c.rect(x, y0, x + w - 1, y1, BAM)
    c.rect(x, y0, x, y1, BAM_L)
    c.rect(x + w - 1, y0, x + w - 1, y1, BAM_D)
    if y0 <= node <= y1:
        c.rect(x, node, x + w - 1, node, BAM_D)
        if node + 1 <= y1:
            c.rect(x, node + 1, x + w - 1, node + 1, BAM_HI)


def leaves(c, x, y, flip=False):
    """A spray of three thin bamboo leaves."""
    d = -1 if flip else 1
    for k, (dx, dy) in enumerate(((5, -2), (6, 1), (4, 3))):
        c.line(x, y, x + d * dx, y + dy, LEAF if k % 2 else LEAF_D, 1)
        c.px(x + d * (dx - 1), y + dy, LEAF)


def bamboo(part, parted=False):
    c = Canvas(32, 16)
    front = [(x, n) for x, n in FRONT if not (parted and 9 <= x <= 22)]
    back = list(BACK)
    if parted:                                    # the near stalks lean apart: a way through
        front += [(8, 6), (21, 12)]
    y0 = 6 if part == 'top' else 0
    y1 = 12 if part == 'bottom' else 15
    c.rect(0, 4 if part == 'top' else 0, 31, y1, LEAF_D)   # the dark grove behind the stalks
    for x, n in back:
        stalk(c, x, n, y0 + 2 if part == 'top' else y0, y1, back=True)
    for x, n in front:
        stalk(c, x, n, y0, y1)
    if part == 'mid':
        leaves(c, 5, 7)
        leaves(c, 26, 12, flip=True)
        if not parted:
            leaves(c, 16, 4)
    if part == 'top':
        yy, xx = np.mgrid[0:16, 0:32]
        for cx, cy, r in ((4, 5, 6), (13, 4, 7), (22, 4, 7), (30, 6, 6), (9, 8, 5), (25, 9, 4), (17, 9, 4)):
            blob = np.hypot(xx + 0.5 - cx, (yy + 0.5 - cy) * 1.3) < r
            c.a[blob] = LEAF
            c.a[blob & (np.hypot(xx + 0.5 - cx + 2, (yy + 0.5 - cy + 2) * 1.3) < r - 3)] = BAM_L
            c.a[blob & (yy + 0.5 > cy + r * 0.45)] = LEAF_D
    if part == 'bottom':
        for x, n in front:
            c.rect(x - 1, 13, x + 4, 14, BAM_D)          # the root knuckle
        for x in (9, 20):
            if not parted:
                c.rect(x, 11, x + 1, 14, BAM_L)          # a new shoot
                c.px(x, 10, BAM)
    c.outline(OUT, where=np.ones_like(c.a, bool) if part == 'bottom' else None)
    if part != 'top':
        c.a[0, :] = np.where(c.a[0, :] == OUT, 0, c.a[0, :])
    if part == 'mid':
        c.a[15, :] = np.where(c.a[15, :] == OUT, 0, c.a[15, :])
    return c


# ---------------------------------------------------------------- buildings
def teahouse():
    """A small wooden teahouse: a slate roof with upturned eaves, paper walls
    and a red curtain over the door (block row 3, column 2)."""
    c = Canvas(80, 64)
    # walls and porch
    c.rect(6, 30, 73, 59, WOOD)
    c.rect(4, 58, 75, 63, WOOD_D)
    c.rect(4, 58, 75, 58, WOOD_L)
    for x in (6, 22, 36, 43, 57, 72):
        c.rect(x, 30, x + 1, 58, WOOD_D)
    for x0 in (9, 60):                              # paper screens
        c.rect(x0, 34, x0 + 10, 54, PAPER)
        for gx in range(x0 + 3, x0 + 10, 4):
            c.rect(gx, 34, gx, 54, WOOD_L)
        for gy in range(38, 54, 5):
            c.rect(x0, gy, x0 + 10, gy, WOOD_L)
    c.rect(24, 34, 33, 46, PAPER)                   # little windows by the door
    c.rect(46, 34, 55, 46, PAPER)
    c.rect(28, 34, 28, 46, WOOD_L)
    c.rect(50, 34, 50, 46, WOOD_L)
    # the doorway and its curtain
    c.rect(33, 44, 46, 58, OUT)
    c.rect(34, 45, 45, 58, WOOD_D)
    c.rect(33, 42, 46, 50, RED)
    c.rect(39, 42, 40, 50, OUT)
    c.rect(33, 42, 46, 42, OUT)
    # roof
    for y in range(4, 31):
        t = (y - 4) / 26
        half = 22 + t * 18
        lift = 2 if y > 26 else 0
        c.rect(int(40 - half - lift), y, int(39 + half + lift), y, ROOF if (y // 4) % 2 else ROOF_D)
    for x in range(4, 76, 6):
        c.rect(x, 22, x + 1, 30, ROOF_D)
    c.rect(14, 2, 65, 5, ROOF_D)                    # ridge
    c.rect(14, 2, 65, 2, ROOF)
    c.rect(10, 0, 15, 4, ROOF_D)                    # upturned ridge ends
    c.rect(64, 0, 69, 4, ROOF_D)
    c.rect(0, 27, 4, 30, ROOF_D)                    # upturned eaves
    c.rect(75, 27, 79, 30, ROOF_D)
    c.rect(36, 9, 43, 16, WOOD_L)                   # a name board
    c.rect(37, 10, 42, 15, PAPER)
    c.rect(39, 11, 40, 14, OUT)
    c.outline(OUT)
    return c


def lantern():
    c = Canvas(16, 32)
    c.rect(3, 28, 12, 31, STONE)                    # base
    c.rect(3, 28, 12, 28, PAPER)
    c.rect(6, 18, 9, 28, STONE)                     # post
    c.rect(6, 18, 6, 28, PAPER)
    c.rect(9, 18, 9, 28, ROOF)
    c.rect(2, 16, 13, 18, STONE)
    c.rect(3, 9, 12, 16, STONE)                     # the light box
    c.rect(5, 10, 10, 15, BAM_HI)
    c.rect(7, 10, 8, 15, PAPER)
    c.rect(3, 9, 3, 16, PAPER)
    c.rect(12, 9, 12, 16, ROOF)
    for y in range(3, 9):                           # cap
        half = 2 + (y - 3) * 1.2
        c.rect(int(8 - half), y, int(7 + half), y, STONE if y < 7 else ROOF)
    c.rect(7, 1, 8, 3, STONE)
    c.outline(OUT)
    return c


def gate():
    """A red wooden shrine gate; the path runs through the middle column."""
    c = Canvas(48, 48)
    for x in (6, 37):
        c.rect(x, 10, x + 4, 47, RED)
        c.rect(x, 10, x, 47, (RED))
        c.rect(x + 4, 10, x + 4, 47, WOOD_D)
        c.rect(x - 1, 43, x + 5, 47, OUT)
        c.rect(x, 44, x + 4, 46, STONE)
    c.rect(0, 2, 47, 6, OUT)                        # the top beam, its ends lifted
    c.rect(1, 3, 46, 5, ROOF_D)
    c.rect(0, 0, 3, 3, OUT)
    c.rect(44, 0, 47, 3, OUT)
    c.rect(2, 7, 45, 10, RED)
    c.rect(2, 10, 45, 10, WOOD_D)
    c.rect(4, 16, 43, 18, RED)                      # the tie beam
    c.rect(4, 18, 43, 18, WOOD_D)
    c.rect(20, 10, 27, 16, WOOD_D)                  # the name plaque
    c.rect(21, 11, 26, 15, PAPER)
    c.rect(23, 12, 24, 14, OUT)
    c.outline(OUT)
    return c


def stepping_stone():
    c = Canvas(16, 16)
    c.disc(8, 8.5, 6, STONE)
    c.disc(7, 7.5, 3.5, PAPER)
    c.disc(7, 7.5, 2.5, STONE)
    c.outline(ROOF)
    return c


PATH_COL = lambda r, c: c == 1
OBJECTS = {
    'bamboo_top': (lambda: bamboo('top'), 12, {}),
    'bamboo_mid': (lambda: bamboo('mid'), 12, {}),
    'bamboo_bottom': (lambda: bamboo('bottom'), 12, {}),
    # the same grove with its stalks parted: you can slip through (drawn over you)
    'bamboo_top_parted': (lambda: bamboo('top', True), 12, {'walk': lambda r, c: True, 'over': lambda r, c: True}),
    'bamboo_mid_parted': (lambda: bamboo('mid', True), 12, {'walk': lambda r, c: True, 'over': lambda r, c: True}),
    'bamboo_bottom_parted': (lambda: bamboo('bottom', True), 12, {'walk': lambda r, c: True}),
    'teahouse': (teahouse, 12, {'door': lambda r, c: (r, c) == (3, 2)}),
    'lantern': (lantern, 12, {'over': lambda r, c: r == 0}),
    'gate': (gate, 12, {'walk': PATH_COL, 'over': lambda r, c: r < 2}),
    'stepping_stone': (stepping_stone, 12, {'walk': lambda r, c: True}),
}


def main():
    pack('data/tilesets/secondary/rustboro', 'data/tilesets/secondary/bamboo', OBJECTS, {12: PAL}, GROUND,
         f'{ROOT}/tools/areas/bamboo_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
