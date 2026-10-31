"""The Verdant River's tileset: Fortree's jungle tiles (only the ones Routes
119 and 120 and Fortree use; the rest are cleared to make room) plus new
pixel art in Emerald's style: rapids that carry you along as you surf
(each direction), wooden sluice levers, stilt huts with thatched roofs,
dock planks, canoes, totems and the overgrown river temple.

    python3 tools/areas/jungle_art.py

Writes data/tilesets/secondary/jungle/ and tools/areas/jungle_tiles.json.
"""
import json, os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001            # General's ground shows under see-through pixels
CURRENT = {'e': 0x50, 'w': 0x51, 'n': 0x52, 's': 0x53}

# Palette 12: river water and wood.
PAL = [(115, 197, 164), (33, 41, 57), (41, 90, 172), (74, 131, 213), (115, 172, 238), (189, 230, 255),
       (255, 255, 255), (90, 57, 33), (139, 90, 49), (189, 139, 82), (230, 189, 123), (205, 57, 41),
       (246, 197, 57), (57, 123, 65), (98, 172, 82), (49, 49, 49)]
OUT, W_D, W, W_L, FOAM, WHITE, WOOD_D, WOOD, WOOD_L, WOOD_LL, RED, GOLD, MOSS_D, MOSS, IRON = range(1, 16)

# Palette 7: thatch, temple stone and carvings.
PAL_STONE = [(115, 197, 164), (41, 41, 41), (156, 123, 57), (205, 172, 90), (238, 213, 139), (90, 98, 90),
             (139, 148, 131), (189, 197, 180), (230, 238, 222), (41, 98, 49), (82, 156, 74), (139, 205, 106),
             (205, 74, 57), (57, 164, 189), (238, 205, 74), (106, 74, 49)]
S_OUT, THATCH_D, THATCH, THATCH_L, STONE_D, STONE, STONE_L, STONE_LL, VINE_D, VINE, VINE_L, PAINT_R, PAINT_B, \
    PAINT_Y, POLE = range(1, 16)


# ---------------------------------------------------------------- the river
def rapids(d):
    """Fast water seen from above: streaks and foam chevrons pointing the
    way the current runs."""
    c = Canvas(16, 16)
    c.a[:] = W
    r = np.random.RandomState({'e': 1, 'w': 2, 'n': 3, 's': 4}[d])
    for _ in range(6):                               # streaks along the flow
        x, y = r.randint(0, 16), r.randint(0, 16)
        for i in range(5):
            c.px((x + i) % 16, y, W_L)
    for i, (x0, y0) in enumerate(((3, 4), (10, 11))):  # foam chevrons
        for k in range(4):
            c.px(x0 - k, y0 - k, FOAM)
            c.px(x0 - k, y0 + k, FOAM)
            c.px(x0 - k + 1, y0 - k, WHITE)
        c.px(x0 + 1, y0, WHITE)
    for _ in range(5):
        c.px(r.randint(0, 16), r.randint(0, 16), W_D)
    a = c.a                                          # drawn pointing east; turn it
    c.a = {'e': a, 'w': a[:, ::-1], 's': a.T.copy(), 'n': a.T[::-1, :].copy()}[d]
    return c


def lever(on):
    """A sluice lever on a post: up, then pulled down."""
    c = Canvas(16, 32)
    c.rect(5, 12, 10, 30, WOOD)
    c.rect(5, 12, 6, 30, WOOD_L)
    c.rect(4, 28, 11, 31, WOOD_D)
    c.rect(3, 14, 12, 18, IRON)
    if on:
        c.line(8, 16, 14, 26, IRON, 2)
        c.disc(14, 27, 2, GOLD)
    else:
        c.line(8, 16, 13, 4, IRON, 2)
        c.disc(13, 3, 2, RED)
    c.outline(OUT)
    return c


def dock():
    """Dock planks over the water; you can walk on them."""
    c = Canvas(16, 16)
    c.a[:] = WOOD
    for y in (0, 5, 10, 15):
        c.rect(0, y, 15, y, WOOD_D)
    for y in (1, 6, 11):
        c.rect(0, y, 15, y, WOOD_L)
    for x, y in ((2, 3), (13, 3), (2, 8), (13, 8), (2, 13), (13, 13)):
        c.px(x, y, IRON)
    return c


def canoe():
    """A dugout canoe pulled up on the bank, 2 blocks by 1."""
    c = Canvas(32, 16)
    yy, xx = np.mgrid[0:16, 0:32]
    hull = ((xx + 0.5 - 16) / 15) ** 2 + ((yy + 0.5 - 8) / 5.5) ** 2 <= 1
    c.a[hull] = WOOD
    inner = ((xx + 0.5 - 16) / 12) ** 2 + ((yy + 0.5 - 8) / 3) ** 2 <= 1
    c.a[inner] = WOOD_D
    c.a[hull & (yy < 5)] = WOOD_L
    c.rect(20, 6, 26, 7, WOOD_LL)                     # the paddle
    c.rect(26, 5, 29, 8, WOOD_L)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- the village
def hut():
    """A stilt hut, 3 blocks by 3: a thatched roof over a plank room on
    poles, a ladder up to its door."""
    c = Canvas(48, 48)
    for i in range(18):                               # the roof
        half = 6 + i * 18 // 17
        c.rect(24 - half, 2 + i, 23 + half, 2 + i, THATCH if i % 4 else THATCH_D)
    c.rect(1, 18, 46, 19, THATCH_D)
    for x in range(3, 46, 3):
        c.px(x, 20, THATCH_D)
    c.rect(6, 20, 41, 34, POLE)                       # the walls
    for x in range(6, 42, 5):
        c.rect(x, 20, x, 34, THATCH_D)
    c.rect(19, 23, 28, 34, S_OUT)                     # the doorway
    c.rect(8, 24, 14, 29, THATCH_D)                   # a window
    c.rect(9, 25, 13, 28, S_OUT)
    c.rect(4, 34, 43, 36, THATCH_D)                   # the floor
    for x in (7, 22, 25, 40):                          # the stilts
        c.rect(x, 37, x + 1, 47, POLE)
    for y in range(37, 47, 3):                         # the ladder
        c.rect(20, y, 27, y, THATCH)
    c.outline(S_OUT)
    return c


def totem():
    c = Canvas(16, 32)
    c.rect(3, 2, 12, 31, POLE)
    c.rect(3, 2, 12, 10, PAINT_R)
    c.rect(5, 5, 6, 6, S_OUT)
    c.rect(9, 5, 10, 6, S_OUT)
    c.rect(3, 11, 12, 19, PAINT_B)
    c.rect(5, 14, 10, 15, PAINT_Y)
    c.rect(3, 20, 12, 28, PAINT_Y)
    c.rect(6, 22, 9, 25, S_OUT)
    c.rect(0, 9, 15, 10, PAINT_R)                     # wings
    c.outline(S_OUT)
    return c


def temple():
    """The river temple, 5 blocks by 4: a stepped stone shrine under vines,
    a dark doorway in the middle of its base."""
    c = Canvas(80, 64)
    for i, (x0, x1, y0, y1) in enumerate(((22, 57, 2, 15), (12, 67, 16, 33), (2, 77, 34, 63))):
        c.rect(x0, y0, x1, y1, STONE)
        c.rect(x0, y0, x1, y0 + 2, STONE_L)
        c.rect(x0, y1 - 1, x1, y1, STONE_D)
        for x in range(x0 + 4, x1 - 2, 8):
            c.rect(x, y0 + 4, x + 3, y1 - 3, STONE_D if (x // 8) % 2 else STONE_L)
    c.rect(30, 6, 49, 12, STONE_D)                    # carved eye of the river
    c.disc(40, 9, 3.5, PAINT_B)
    c.rect(32, 40, 47, 63, S_OUT)                     # the doorway
    c.rect(30, 38, 49, 40, STONE_LL)
    r = np.random.RandomState(9)
    for _ in range(9):                                 # hanging vines
        x = r.randint(4, 76)
        if 30 <= x <= 49:
            continue
        y0 = r.randint(2, 36)
        for y in range(y0, min(63, y0 + r.randint(6, 18))):
            c.px(x + (y // 3) % 2, y, VINE if y % 4 else VINE_L)
        c.px(x - 1, y0 + 3, VINE_D)
        c.px(x + 2, y0 + 6, VINE_D)
    c.outline(S_OUT)
    return c


def over_top(n):
    return lambda r, c: r < n


OBJECTS = {
    'lever': (lambda: lever(False), 12, {'over': over_top(1)}),
    'lever_on': (lambda: lever(True), 12, {'over': over_top(1)}),
    'dock': (dock, 12, {'walk': lambda r, c: True}),
    'canoe': (canoe, 12, {}),
    'hut': (hut, 7, {'over': over_top(1), 'door': lambda r, c: (r, c) == (2, 1)}),
    'totem': (totem, 7, {'over': over_top(1)}),
    'temple': (temple, 7, {'over': over_top(2), 'door': lambda r, c: (r, c) == (3, 2)}),
}
for _d, _b in CURRENT.items():
    OBJECTS[f'rapids_{_d}'] = (lambda d=_d: rapids(d), 12, {'behavior': lambda r, c, b=_b: b})


def corpus_metatiles():
    L = {l['id']: l for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts'] if 'id' in l}
    keep = set()
    for lid in ('LAYOUT_ROUTE119', 'LAYOUT_ROUTE120', 'LAYOUT_FORTREE_CITY'):
        keep |= set((np.fromfile(f"{ROOT}/{L[lid]['blockdata_filepath']}", '<u2') & 0x3FF).tolist())
    return keep


def main():
    pack('data/tilesets/secondary/fortree', 'data/tilesets/secondary/jungle', OBJECTS, {7: PAL_STONE, 12: PAL},
         GROUND, f'{ROOT}/tools/areas/jungle_tiles.json', fill_free=True, keep=corpus_metatiles())


if __name__ == '__main__':
    main()
