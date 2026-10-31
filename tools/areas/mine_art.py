"""Cobblerock Mine's art in Emerald's style: mine-cart rails (straights and
curves), carts, crates, ore piles, ore veins you can chip, timber tunnel
supports and lamp posts.

    python3 tools/areas/mine_art.py

Writes data/tilesets/secondary/mine/ (the cave tileset plus this art) and
tools/areas/mine_tiles.json. The town outside uses the same pieces through
tools/areas/bamboo_art.py (the Rustboro-based tileset, palette 6).
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

CAVE_FLOOR = 0x211        # Granite Cave's floor shows under see-through pixels

PAL = [(115, 197, 164), (41, 33, 33), (90, 57, 33), (148, 98, 57), (205, 156, 98), (74, 74, 90),
       (131, 131, 148), (197, 197, 213), (98, 74, 57), (156, 123, 90), (205, 172, 131), (49, 41, 49),
       (230, 74, 90), (82, 164, 238), (238, 246, 255), (255, 205, 74)]
OUT, WOOD_D, WOOD, WOOD_L, IRON_D, IRON, IRON_L, ROCK_D, ROCK, ROCK_L, COAL, GEM_R, GEM_B, GEM_HI, FLAME = range(1, 16)


# ---------------------------------------------------------------- rails
def rail(kind):
    """Track seen from above: wooden ties under two iron rails."""
    c = Canvas(16, 16)
    if kind in ('h', 'v'):
        for t in range(1, 16, 5):
            c.rect(t, 2, t + 2, 13, WOOD)
            c.rect(t, 2, t, 13, WOOD_L)
            c.rect(t + 2, 2, t + 2, 13, WOOD_D)
        for y in (4, 11):
            c.rect(0, y, 15, y, IRON_L)
            c.rect(0, y + 1, 15, y + 1, IRON_D)
        if kind == 'v':
            c.a = c.a.T.copy()
        return c
    # curves: arcs around one corner of the tile (kind names the two open sides)
    cx = 16 if 'e' in kind else 0
    cy = 16 if 's' in kind else 0
    yy, xx = np.mgrid[0:16, 0:16]
    d = np.hypot(xx + 0.5 - cx, yy + 0.5 - cy)
    ang = np.arctan2(yy + 0.5 - cy, xx + 0.5 - cx)
    ties = (d > 2) & (d < 14) & (np.abs(np.sin(ang * 6)) < 0.3)
    c.a[ties] = WOOD
    for r in (4.5, 11.5):
        c.a[np.abs(d - r) < 0.6] = IRON_L
        c.a[np.abs(d - r - 0.9) < 0.45] = IRON_D
    return c


def cart(full=True):
    c = Canvas(16, 16)
    c.rect(1, 4, 14, 12, IRON)
    c.rect(1, 4, 14, 5, IRON_L)
    c.rect(1, 11, 14, 12, IRON_D)
    for x in (4, 11):
        c.rect(x, 4, x, 12, IRON_D)
    if full:
        c.disc(5, 4, 3, COAL)
        c.disc(10, 3.5, 3, ROCK)
        c.px(5, 2, GEM_B)
        c.px(11, 2, GEM_R)
        c.px(8, 3, GEM_HI)
    for x in (3, 12):
        c.disc(x + 0.5, 13.5, 2, IRON_D)
        c.px(x, 13, IRON_L)
    c.outline(OUT)
    return c


def crate():
    c = Canvas(16, 16)
    c.rect(1, 2, 14, 14, WOOD)
    c.rect(1, 2, 14, 3, WOOD_L)
    c.rect(1, 13, 14, 14, WOOD_D)
    c.line(2, 4, 13, 12, WOOD_D)
    c.line(2, 12, 13, 4, WOOD_D)
    c.rect(1, 8, 14, 8, WOOD_D)
    c.outline(OUT)
    return c


def ore_pile():
    c = Canvas(32, 16)
    yy, xx = np.mgrid[0:16, 0:32]
    mound = np.hypot((xx + 0.5 - 16) / 15, (yy + 0.5 - 15) / 12) < 1
    c.a[mound] = ROCK
    c.a[mound & (yy < 9)] = ROCK_L
    c.a[mound & (yy > 12)] = ROCK_D
    for x, y, col in ((8, 10, COAL), (13, 7, COAL), (20, 9, COAL), (24, 12, COAL), (16, 11, GEM_R), (11, 13, GEM_B),
                      (22, 6, GEM_B)):
        c.rect(x, y, x + 1, y + 1, col)
    c.px(16, 10, GEM_HI)
    c.outline(OUT)
    return c


def ore_vein(mined=False):
    """A knot of rock with gems in it (a chipped one has none left)."""
    c = Canvas(16, 16)
    c.disc(8, 9, 7, ROCK)
    c.disc(6.5, 7, 4, ROCK_L)
    c.disc(9, 11, 3, ROCK_D)
    if mined:
        c.line(5, 6, 10, 11, ROCK_D)
        c.line(10, 5, 6, 10, ROCK_D)
    else:
        for x, y, col in ((5, 6, GEM_R), (10, 8, GEM_B), (6, 11, GEM_B), (11, 12, GEM_R)):
            c.rect(x, y, x + 1, y + 1, col)
            c.px(x, y, GEM_HI)
        c.px(12, 4, GEM_HI)
        c.px(13, 3, FLAME)
    c.outline(OUT)
    return c


def lamp():
    c = Canvas(16, 32)
    c.rect(6, 6, 8, 31, WOOD)
    c.rect(6, 6, 6, 31, WOOD_L)
    c.rect(8, 6, 8, 31, WOOD_D)
    c.rect(4, 28, 10, 31, WOOD_D)
    c.rect(6, 5, 13, 6, WOOD_D)                       # the arm
    c.rect(10, 7, 14, 13, IRON_D)                    # the lantern
    c.rect(11, 8, 13, 12, FLAME)
    c.px(12, 9, GEM_HI)
    c.rect(11, 6, 13, 6, IRON)
    c.outline(OUT)
    return c


def support():
    """A timber frame shoring up a tunnel; you walk through the middle."""
    c = Canvas(48, 32)
    for x in (2, 41):
        c.rect(x, 4, x + 4, 31, WOOD)
        c.rect(x, 4, x, 31, WOOD_L)
        c.rect(x + 4, 4, x + 4, 31, WOOD_D)
    c.rect(0, 0, 47, 5, WOOD)
    c.rect(0, 0, 47, 1, WOOD_L)
    c.rect(0, 5, 47, 5, WOOD_D)
    for x in (7, 37):                                # braces
        c.line(x, 6, x + (4 if x < 20 else -4), 10, WOOD_D, 2)
    c.rect(22, 6, 25, 9, IRON_D)                     # a hanging lamp
    c.rect(23, 7, 24, 8, FLAME)
    c.outline(OUT)
    return c


def objects(pal):
    return {
        'rail_h': (lambda: rail('h'), pal, {'walk': lambda r, c: True}),
        'rail_v': (lambda: rail('v'), pal, {'walk': lambda r, c: True}),
        'rail_ne': (lambda: rail('ne'), pal, {'walk': lambda r, c: True}),
        'rail_nw': (lambda: rail('nw'), pal, {'walk': lambda r, c: True}),
        'rail_se': (lambda: rail('se'), pal, {'walk': lambda r, c: True}),
        'rail_sw': (lambda: rail('sw'), pal, {'walk': lambda r, c: True}),
        'cart': (cart, pal, {}),
        'cart_empty': (lambda: cart(False), pal, {}),
        'crate': (crate, pal, {}),
        'ore_pile': (ore_pile, pal, {}),
        'ore_vein': (ore_vein, pal, {}),
        'ore_vein_mined': (lambda: ore_vein(True), pal, {}),
        'mine_lamp': (lamp, pal, {'over': lambda r, c: r == 0}),
        'support': (support, pal, {'over': lambda r, c: r == 0, 'walk': lambda r, c: r == 0 or c == 1}),
    }


def main():
    pack('data/tilesets/secondary/cave', 'data/tilesets/secondary/mine', objects(12), {12: PAL}, CAVE_FLOOR,
         f'{ROOT}/tools/areas/mine_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
