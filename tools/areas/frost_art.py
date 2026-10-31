"""Mt. Frostcrown's tileset: Fallarbor's secondary tileset (the rock mesas,
cliffs and trails of Routes 114 and 115) with its rock palette turned to
snow and ice, plus new pixel art in Emerald's style: a sled run you slide
down (straights and turns), log cabins and a lodge with snowy roofs,
snowmen, a campfire, lantern posts, a summit cairn and snowdrifts.

General's trees and grass stay as they are; the game shows them in their
winter colours on these maps whatever the season (src/life_seasons.c).

    python3 tools/areas/frost_art.py

Writes data/tilesets/secondary/frost/ and tools/areas/frost_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x241            # Route 114's trail floor, snow in this palette
MB_SLIDE = {'e': 0x44, 'w': 0x45, 'n': 0x46, 's': 0x47}

# Fallarbor's palette 11 (the rock) with sand turned to snow and the
# shadows to cold blue.
PAL_SNOW = [(24, 41, 82), (246, 246, 255), (214, 226, 246), (214, 226, 246), (156, 172, 213), (123, 139, 180),
            (90, 106, 156), (65, 74, 123), (41, 49, 90), (246, 246, 255), (214, 226, 246), (180, 197, 230),
            (156, 172, 213), (115, 131, 180), (82, 98, 148), (115, 197, 164)]

# Fallarbor's palette 9 (the mountain mass west of the trail, rock and
# scrub) turned to snow-dusted dark rock.
PAL_ROCK = [(0, 0, 0), (230, 238, 255), (172, 189, 222), (115, 131, 172), (57, 65, 98), (205, 213, 230),
            (156, 164, 197), (106, 115, 148), (41, 41, 65), (189, 197, 222), (148, 156, 189), (123, 131, 164),
            (98, 106, 139), (82, 90, 123), (115, 115, 139), (82, 82, 106)]

# Palette 12: wood, snow, ice and warm light.
PAL = [(115, 197, 164), (41, 41, 65), (90, 57, 41), (148, 98, 57), (197, 139, 82), (255, 255, 255),
       (226, 234, 255), (180, 197, 238), (131, 164, 222), (82, 115, 189), (205, 57, 49), (246, 123, 90),
       (255, 180, 57), (255, 230, 115), (139, 139, 156), (90, 90, 106)]
OUT, WOOD_D, WOOD, WOOD_L, SNOW, SNOW_L, SNOW_S, ICE, ICE_D, RED, RED_L, FLAME, GOLD, STONE, STONE_D = range(1, 16)


# ---------------------------------------------------------------- the sled run
def chute(c):
    """Packed, polished snow: a pale track."""
    c.a[:] = SNOW_L
    c.rect(0, 0, 15, 0, SNOW_S)


def sled_run(d):
    """A straight piece of the run, grooves along it and a chevron pointing
    the way you slide."""
    c = Canvas(16, 16)
    chute(c)
    if d in 'ns':
        c.a[:, 0] = c.a[:, 15] = ICE
        c.a[:, 1] = c.a[:, 14] = SNOW_S
        c.rect(5, 0, 5, 15, SNOW_S)
        c.rect(10, 0, 10, 15, SNOW_S)
        for i in range(3):                          # a chevron pointing downhill
            y = 7 - i if d == 's' else 6 + i
            c.px(7 - i, y, ICE_D)
            c.px(8 + i, y, ICE_D)
    else:
        c.a[0, :] = c.a[15, :] = ICE
        c.a[1, :] = c.a[14, :] = SNOW_S
        c.rect(0, 5, 15, 5, SNOW_S)
        c.rect(0, 10, 15, 10, SNOW_S)
        for i in range(3):
            x = 6 + i if d == 'e' else 9 - i
            c.px(x, 7 - (2 - i), ICE_D)
            c.px(x, 8 + (2 - i), ICE_D)
    return c


def sled_turn(frm, to):
    """A banked corner: you come in going frm and leave going to."""
    c = Canvas(16, 16)
    chute(c)
    walls = {'n', 's', 'e', 'w'} - {{'n': 's', 's': 'n', 'e': 'w', 'w': 'e'}[frm], to}
    for w in walls:
        if w == 'n':
            c.a[0, :] = ICE
            c.a[1, :] = SNOW_S
        if w == 's':
            c.a[15, :] = ICE
            c.a[14, :] = SNOW_S
        if w == 'w':
            c.a[:, 0] = ICE
            c.a[:, 1] = SNOW_S
        if w == 'e':
            c.a[:, 15] = ICE
            c.a[:, 14] = SNOW_S
    c.disc(8, 8, 2, SNOW_S)
    return c


def sled_stop():
    """The bottom of the run: the track ends in a soft bank with a flag."""
    c = Canvas(16, 16)
    chute(c)
    c.a[:, 0] = c.a[:, 15] = ICE
    c.rect(1, 11, 14, 15, SNOW)
    c.rect(1, 11, 14, 11, SNOW_S)
    c.rect(12, 1, 12, 11, WOOD_D)
    c.rect(13, 1, 15, 4, RED)
    c.rect(13, 1, 15, 1, RED_L)
    return c


# ---------------------------------------------------------------- buildings
def snow_roof(c, x0, x1, y0, y1, eave):
    """A gabled roof thick with snow, dark wood under the overhang."""
    for y in range(y0, y1 + 1):
        t = (y - y0) / max(1, y1 - y0)
        half = (x1 - x0) / 2 * (0.25 + 0.75 * t)
        mid = (x0 + x1) / 2
        c.rect(int(mid - half), y, int(mid + half), y, SNOW)
        c.px(int(mid - half), y, SNOW_S)
        c.px(int(mid + half), y, SNOW_S)
    c.rect(x0, y1 + 1, x1, y1 + 3, SNOW_S)                       # snow over the eave
    c.rect(x0 + 1, y1 + 4, x1 - 1, y1 + 5, WOOD_D)
    for x in range(x0 + 3, x1 - 2, 7):                            # icicles
        c.rect(x, y1 + 4, x, y1 + 6 + (x % 3), ICE)
    for y in range(y0 + 4, y1 - 1, 5):                            # a few roof shades
        c.rect(int((x0 + x1) / 2) - 2, y, int((x0 + x1) / 2) + 2, y, SNOW_L)


def logs(c, x0, x1, y0, y1):
    c.rect(x0, y0, x1, y1, WOOD)
    for y in range(y0 + 3, y1, 4):
        c.rect(x0, y, x1, y, WOOD_D)
        c.rect(x0, y - 1, x1, y - 1, WOOD_L)
    for x in (x0, x1 - 2):                                        # log ends at the corners
        for y in range(y0 + 1, y1 - 1, 4):
            c.rect(x, y, x + 2, y + 2, WOOD_L)
            c.px(x + 1, y + 1, WOOD_D)


def window(c, x, y):
    c.rect(x, y, x + 9, y + 9, WOOD_D)
    c.rect(x + 1, y + 1, x + 8, y + 8, GOLD)
    c.rect(x + 1, y + 1, x + 3, y + 3, (SNOW_L))
    c.rect(x + 4, y + 1, x + 5, y + 8, WOOD_D)
    c.rect(x + 1, y + 4, x + 8, y + 5, WOOD_D)
    c.rect(x - 1, y + 10, x + 10, y + 11, SNOW)                   # snow on the sill


def door(c, x, y, h=20):
    c.rect(x, y, x + 13, y + h - 1, WOOD_D)
    c.rect(x + 1, y + 1, x + 12, y + h - 1, WOOD)
    c.rect(x + 1, y + 1, x + 12, y + 2, WOOD_L)
    c.rect(x + 6, y + 1, x + 7, y + h - 1, WOOD_D)
    c.px(x + 10, y + h // 2, GOLD)


def cabin():
    """A log cabin; its door at block (3, 2)."""
    c = Canvas(80, 64)
    logs(c, 4, 75, 30, 61)
    snow_roof(c, 0, 79, 0, 24, None)
    c.rect(56, 2, 63, 14, STONE)                                  # chimney
    c.rect(56, 2, 63, 3, SNOW)
    c.rect(57, 4, 57, 14, STONE_D)
    window(c, 10, 36)
    window(c, 58, 36)
    door(c, 33, 44)
    c.rect(32, 60, 47, 63, SNOW_S)                                # the step
    c.outline(OUT)
    return c


def lodge():
    """The mountain lodge: wider, two windows a side and a sign; its door at block (4, 3)."""
    c = Canvas(112, 80)
    logs(c, 4, 107, 40, 77)
    snow_roof(c, 0, 111, 0, 34, None)
    for x in (16, 84):
        c.rect(x, 4, x + 9, 22, STONE)
        c.rect(x, 4, x + 9, 6, SNOW)
        c.rect(x + 1, 7, x + 1, 22, STONE_D)
    c.rect(36, 36, 75, 46, WOOD_D)                                # the sign board
    c.rect(37, 37, 74, 45, WOOD_L)
    for x in range(40, 72, 5):
        c.rect(x, 40, x + 2, 42, RED)
    window(c, 10, 52)
    window(c, 24, 52)
    window(c, 78, 52)
    window(c, 92, 52)
    door(c, 49, 56, 22)
    c.rect(48, 76, 63, 79, SNOW_S)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- props
def snowman():
    c = Canvas(16, 32)
    c.disc(8, 24, 6.5, SNOW)
    c.disc(8, 13, 5, SNOW)
    c.disc(10, 26, 4, SNOW_L)
    c.disc(10, 15, 3, SNOW_L)
    c.disc(5, 26, 3, SNOW_S, 2)
    c.px(6, 12, OUT)
    c.px(10, 12, OUT)
    c.rect(8, 14, 10, 14, FLAME)                                  # carrot nose
    c.rect(3, 17, 12, 18, RED)                                    # scarf
    c.rect(10, 18, 11, 21, RED_L)
    c.rect(4, 6, 12, 8, OUT)                                      # hat
    c.rect(6, 1, 10, 6, OUT)
    c.line(1, 18, 3, 15, WOOD_D)                                  # twig arms
    c.line(15, 18, 13, 15, WOOD_D)
    c.outline(OUT)
    return c


def campfire():
    c = Canvas(16, 16)
    for x0, y0, x1, y1 in ((2, 13, 13, 10), (2, 10, 13, 13)):
        c.line(x0, y0, x1, y1, WOOD_D, 2)
    c.disc(8, 8, 4.5, FLAME)
    c.disc(8, 9, 2.5, GOLD)
    c.px(8, 2, FLAME)
    c.px(6, 4, FLAME)
    c.px(10, 3, FLAME)
    for x in (1, 14):
        c.disc(x, 13, 1.5, STONE)
    c.outline(OUT)
    return c


def lantern():
    c = Canvas(16, 32)
    c.rect(7, 12, 8, 31, WOOD_D)
    c.rect(4, 3, 11, 11, OUT)
    c.rect(5, 4, 10, 10, GOLD)
    c.rect(6, 5, 9, 9, SNOW_L)
    c.rect(3, 1, 12, 3, SNOW)
    c.rect(5, 28, 10, 31, SNOW_S)
    c.outline(OUT)
    return c


def sled():
    c = Canvas(16, 16)
    c.rect(2, 5, 13, 10, RED)
    c.rect(2, 5, 13, 6, RED_L)
    c.rect(1, 12, 14, 12, STONE_D)
    c.px(1, 11, STONE_D)
    c.px(14, 11, STONE_D)
    for x in (4, 11):
        c.rect(x, 10, x, 12, STONE_D)
    c.outline(OUT)
    return c


def cairn():
    """The summit cairn, stones piled round a flag."""
    c = Canvas(16, 32)
    c.rect(7, 2, 7, 22, WOOD_D)
    c.rect(8, 2, 14, 8, RED)
    c.rect(8, 2, 14, 3, RED_L)
    for cx, cy, r in ((8, 27, 6), (5, 22, 3.5), (11, 22, 3.5), (8, 18, 3)):
        c.disc(cx, cy, r, STONE)
        c.disc(cx - 1, cy - 1, r - 1.5, SNOW_L)
    c.outline(OUT)
    return c


def snowdrift():
    """A bank of snow too deep to wade through."""
    c = Canvas(16, 16)
    c.disc(8, 12, 8, SNOW)
    c.disc(4, 9, 4.5, SNOW)
    c.disc(12, 8, 5, SNOW)
    c.disc(13, 7, 3, SNOW_L)
    c.disc(4, 13, 3, SNOW_S, 2)
    c.rect(0, 14, 15, 15, SNOW_S)
    c.outline(OUT)
    return c


def ice_rock():
    c = Canvas(16, 16)
    yy, xx = np.mgrid[0:16, 0:16]
    inside = (np.hypot((xx + .5 - 8) / 7, (yy + .5 - 9) / 6.5) < 1) & (yy >= 1)
    inside &= ~((xx + .5 - 2) + (yy + .5 - 1) < 5)                # a chipped corner
    c.a[inside] = ICE
    c.a[inside & (xx < 7) & (yy < 9)] = SNOW_L
    c.a[inside & (xx > 10)] = ICE_D
    c.outline(OUT)
    return c


WALK = {'walk': lambda r, c: True}
OBJECTS = {
    **{f'sled_{d}': (lambda d=d: sled_run(d), 12, {'behavior': lambda r, c, d=d: MB_SLIDE[d]}) for d in 'nsew'},
    **{f'sled_{a}{b}': (lambda a=a, b=b: sled_turn(a, b), 12, {'behavior': lambda r, c, b=b: MB_SLIDE[b]})
       for a, b in ('se', 'sw', 'es', 'ws', 'en', 'wn', 'ne', 'nw')},
    'sled_stop': (sled_stop, 12, WALK),
    'cabin': (cabin, 12, {'door': lambda r, c: (r, c) == (3, 2)}),
    'lodge': (lodge, 12, {'door': lambda r, c: (r, c) == (4, 3)}),
    'snowman': (snowman, 12, {'over': lambda r, c: r == 0}),
    'campfire': (campfire, 12, {}),
    'lantern': (lantern, 12, {'over': lambda r, c: r == 0}),
    'sled': (sled, 12, {}),
    'cairn': (cairn, 12, {'over': lambda r, c: r == 0}),
    'snowdrift': (snowdrift, 12, {}),
    'ice_rock': (ice_rock, 12, {}),
}


def main():
    pack('data/tilesets/secondary/fallarbor', 'data/tilesets/secondary/frost', OBJECTS, {9: PAL_ROCK, 11: PAL_SNOW, 12: PAL},
         GROUND, f'{ROOT}/tools/areas/frost_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
