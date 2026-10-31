"""The carnival's tileset: Petalburg's secondary tileset plus new pixel art
drawn here in Emerald's style (dark navy outlines, three-step shading lit
from the top left): a Ferris wheel, a big top tent, market stalls, an
entrance arch, lamp posts and a balloon cart for Sunfair Carnival, and a
lighthouse, beam mirrors, a pirate ship and its deck for Gullwing Point.

    python3 tools/areas/carnival_art.py

Writes data/tilesets/secondary/carnival/ and tools/areas/carnival_tiles.json
(object name -> rows of metatile ids, -1 where the object is see-through,
and the blocks you can walk under).
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001            # General's grass shows under see-through pixels
WATER = 0x170             # General's sea, under the pirate ship
MB_WEST_ARROW_WARP = 0x63

OUTLINE = (41, 49, 90)
# Palette 7: the wheel and anything metal.
PAL_WHEEL = [(115, 197, 164), OUTLINE, (98, 106, 139), (156, 164, 197), (213, 222, 238), (255, 255, 255),
             (172, 41, 57), (230, 82, 82), (255, 156, 139), (205, 148, 24), (255, 213, 74), (49, 98, 197),
             (98, 156, 238), (41, 148, 82), (106, 205, 115), (189, 238, 255)]
# Palette 11: canvas, wood and paint.
PAL_TENT = [(115, 197, 164), OUTLINE, (172, 41, 57), (230, 82, 82), (255, 148, 131), (205, 205, 222),
            (255, 255, 255), (41, 90, 180), (90, 148, 230), (156, 205, 255), (205, 148, 24), (255, 213, 74),
            (115, 74, 49), (172, 115, 65), (222, 172, 106), (74, 57, 82)]
W_OUT, W_STEEL_D, W_STEEL, W_LIGHT, W_WHITE = 1, 2, 3, 4, 5
W_RED_D, W_RED, W_RED_L, W_YEL_D, W_YEL, W_BLUE_D, W_BLUE, W_GRN_D, W_GRN, W_GLASS = range(6, 16)
T_OUT, T_RED_D, T_RED, T_RED_L, T_WHITE_D, T_WHITE, T_BLUE_D, T_BLUE, T_BLUE_L = range(1, 10)
T_YEL_D, T_YEL, T_WOOD_D, T_WOOD, T_WOOD_L, T_DARK = range(10, 16)


# ---------------------------------------------------------------- the objects
def ferris_wheel():
    c = Canvas(128, 144)
    cx, cy, R = 64, 60, 50
    # legs and base
    for sx in (20, 108):
        c.line(cx, cy, sx, 134, W_STEEL, 4)
        c.line(cx - 1, cy, sx - 1, 134, W_STEEL_D, 1)
    c.line(36, 108, 92, 108, W_STEEL_D, 2)
    c.outline()
    # rings and spokes
    c.disc(cx, cy, R + 1, W_LIGHT, R - 3)
    c.disc(cx, cy, R - 1, W_WHITE, R - 2)
    c.disc(cx, cy, 30, W_STEEL, 28)
    for k in range(16):
        t = k * np.pi / 8
        c.line(cx + 7 * np.sin(t), cy - 7 * np.cos(t), cx + (R - 3) * np.sin(t), cy - (R - 3) * np.cos(t), W_STEEL_D)
    yy, xx = np.mgrid[0:c.h, 0:c.w]
    d = np.hypot(xx + 0.5 - cx, yy + 0.5 - cy)
    ring = (d > R + 1) & (d <= R + 2) | (d > R - 4) & (d <= R - 3)
    c.a[ring & (c.a == 0)] = W_OUT
    c.a[(d > R + 1) & (d <= R + 2)] = W_OUT
    # light bulbs around the rim
    for k in range(24):
        t = k * np.pi / 12 + np.pi / 24
        c.px(cx - 0.5 + (R - 1.5) * np.sin(t), cy - 0.5 - (R - 1.5) * np.cos(t), W_YEL)
    # hub
    c.disc(cx, cy, 8, W_OUT)
    c.disc(cx, cy, 7, W_RED_D)
    c.disc(cx, cy, 6, W_RED)
    c.disc(cx - 2, cy - 2, 2.5, W_RED_L)
    # gondolas, hanging below their pins
    colours = {0: (W_RED_D, W_RED, W_RED_L), 1: (W_YEL_D, W_YEL, W_WHITE), 2: (W_BLUE_D, W_BLUE, W_GLASS),
               3: (W_GRN_D, W_GRN, W_WHITE), 4: (W_RED_D, W_RED, W_RED_L)}
    for k in range(5):
        t = k * np.pi / 4
        px, py = int(round(cx - 0.5 - R * np.sin(t))), int(round(cy - 0.5 - R * np.cos(t)))
        dark, mid, light = colours[k]
        x0, y0 = px - 6, py + 2
        g = Canvas(15, 14)
        g.rect(1, 0, 12, 2, dark)          # roof
        g.rect(2, 3, 11, 11, mid)          # cabin
        g.rect(2, 3, 11, 3, light)
        g.rect(4, 5, 9, 8, W_GLASS)        # window
        g.rect(4, 5, 5, 6, W_WHITE)
        g.rect(2, 11, 11, 11, dark)
        g.outline()
        m = g.a > 0
        c.a[y0:y0 + 14, x0:x0 + 15][m] = g.a[m]
        c.px(px, py, W_OUT)
        c.px(px, py + 1, W_STEEL_D)
    # platform
    c.rect(8, 132, 119, 143, W_STEEL_D)
    c.rect(9, 133, 118, 134, W_LIGHT)
    c.rect(9, 135, 118, 140, W_STEEL)
    c.rect(44, 136, 83, 143, W_RED)
    c.rect(44, 136, 83, 137, W_RED_L)
    for x in range(48, 82, 6):
        c.rect(x, 139, x + 3, 140, W_RED_D)
    c.rect(8, 143, 119, 143, W_OUT)
    c.rect(7, 132, 7, 143, W_OUT)
    c.rect(8, 131, 119, 131, W_OUT)
    c.mirror()
    return c


def striped(c, x0, y0, x1, y1, a, b, width, shade=True):
    for x in range(x0, x1 + 1):
        col = a if ((x - x0) // width) % 2 == 0 else b
        c.rect(x, y0, x, y1, col)


def big_top():
    c = Canvas(96, 80)
    cx = 48
    # walls
    for x in range(8, 48):
        k = (x - 8) // 6
        col = T_RED if k % 2 == 0 else T_WHITE
        if x < 12:
            col = T_RED_D if col == T_RED else T_WHITE_D
        c.rect(x, 40, x, 77, col)
    c.rect(8, 74, 47, 77, T_RED_D)
    # entrance with curtains tied back
    c.rect(38, 52, 47, 77, T_DARK)
    for y in range(52, 78):
        w = max(0, 6 - (y - 52) // 3) if y < 70 else (y - 70) // 2
        c.rect(38, y, 38 + w, y, T_RED)
    c.rect(38, 52, 47, 53, T_YEL)
    # roof: stripes fanning out from the peak
    for y in range(6, 42):
        half = 4 + (y - 6) * 40 / 36
        for x in range(int(cx - half), cx):
            ang = np.arctan2(x + 0.5 - cx, y + 2)
            k = int(np.floor(ang / 0.16))
            col = T_RED if k % 2 == 0 else T_WHITE
            if y > 36:
                col = T_RED_D if col == T_RED else T_WHITE_D
            elif x < cx - half + 3:
                col = T_RED_D if col == T_RED else T_WHITE_D
            elif y < 16 and x > cx - 6:
                col = T_RED_L if col == T_RED else T_WHITE
            c.px(x, y, col)
    # scalloped eave
    for i, x in enumerate(range(4, 48, 8)):
        col = T_YEL if i % 2 == 0 else T_RED
        for dx in range(8):
            depth = 4 - abs(dx - 3.5) // 1.5
            c.rect(x + dx, 40, x + dx, int(40 + depth), col)
        c.rect(x, 40, x + 7, 40, T_YEL_D if col == T_YEL else T_RED_D)
    # peak and flag pole
    c.rect(46, 0, 47, 6, T_WOOD_D)
    c.disc(48, 2, 2.5, T_YEL)
    c.px(46, 1, T_WHITE)
    c.mirror()
    c.outline()
    return c


def stall(main, main_d, sign):
    c = Canvas(48, 48)
    # back wall and goods shelf
    c.rect(5, 14, 42, 34, T_WOOD_D)
    c.rect(7, 18, 40, 19, T_WOOD)
    for i, x in enumerate(range(9, 24, 5)):
        c.disc(x + 2, 16.5, 2, (T_RED, T_BLUE_L, T_YEL)[i % 3])
    for i, x in enumerate(range(8, 24, 4)):
        c.rect(x, 22, x + 2, 25, (T_YEL, T_RED_L, T_BLUE, T_WHITE)[i % 4])
    # posts
    c.rect(4, 12, 6, 44, T_WOOD)
    c.rect(4, 12, 4, 44, T_WOOD_L)
    # counter
    c.rect(4, 32, 23, 44, T_WOOD)
    c.rect(4, 32, 23, 33, T_WOOD_L)
    c.rect(4, 38, 23, 38, T_WOOD_D)
    c.rect(4, 44, 23, 44, T_WOOD_D)
    # awning with scallops
    striped(c, 1, 6, 23, 13, main, T_WHITE, 4)
    c.rect(1, 6, 23, 6, T_WHITE)
    for x in range(1, 24, 4):
        col = main_d if ((x - 1) // 4) % 2 == 0 else T_WHITE_D
        c.rect(x, 14, x + 3, 15, col)
        c.rect(x + 1, 16, x + 2, 16, col)
    # sign board
    c.rect(14, 0, 23, 5, sign)
    c.rect(14, 0, 23, 0, T_WHITE)
    c.mirror()
    c.outline()
    return c


def arch():
    c = Canvas(96, 48)
    for x0 in (2,):
        c.rect(x0, 10, x0 + 11, 47, T_WHITE)
        for y in range(10, 48, 6):
            c.rect(x0, y, x0 + 11, y + 2, T_RED)
        c.rect(x0, 10, x0 + 1, 47, T_WHITE_D)
        c.rect(x0 - 1, 6, x0 + 12, 9, T_YEL)
        c.disc(x0 + 6, 3, 3, T_YEL)
        c.px(x0 + 5, 2, T_WHITE)
    # banner
    for x in range(14, 48):
        sag = int(round(3 * np.sin((x - 14) / 68 * np.pi)))
        c.rect(x, 8 + sag, x, 21 + sag, T_RED)
        c.rect(x, 8 + sag, x, 9 + sag, T_RED_L)
        c.rect(x, 20 + sag, x, 21 + sag, T_RED_D)
        if (x - 14) % 7 in (2, 3) and x < 46:
            c.rect(x, 13 + sag, x, 16 + sag, T_YEL)
        if (x - 14) % 7 in (1, 4):
            c.rect(x, 14 + sag, x, 15 + sag, T_YEL)
    # little flags hanging under the banner
    for i, x in enumerate(range(16, 48, 6)):
        sag = int(round(3 * np.sin((x - 14) / 68 * np.pi)))
        col = (T_BLUE, T_YEL, T_WHITE)[i % 3]
        for k in range(4):
            c.rect(x + k // 2, 22 + sag + k, x + 3 - k // 2, 22 + sag + k, col)
    c.mirror()
    c.outline()
    return c


def lamp():
    c = Canvas(16, 32)
    c.rect(7, 8, 8, 29, T_OUT)
    c.rect(7, 8, 7, 29, T_BLUE_D)
    c.rect(5, 29, 10, 31, T_BLUE_D)
    c.rect(4, 2, 11, 7, T_YEL)
    c.rect(5, 3, 7, 4, T_WHITE)
    c.rect(4, 7, 11, 7, T_YEL_D)
    c.rect(5, 0, 10, 1, T_BLUE_D)
    c.outline()
    return c


def balloons():
    c = Canvas(32, 32)
    # cart
    c.rect(6, 22, 25, 27, T_WOOD)
    c.rect(6, 22, 25, 22, T_WOOD_L)
    c.rect(6, 27, 25, 27, T_WOOD_D)
    c.disc(10, 29.5, 2.5, T_DARK)
    c.disc(22, 29.5, 2.5, T_DARK)
    # strings and balloons
    for bx, by, col, hl in ((9, 6, T_RED, T_RED_L), (16, 4, T_BLUE, T_BLUE_L), (23, 7, T_YEL, T_WHITE),
                            (12, 12, T_BLUE, T_BLUE_L), (20, 12, T_RED, T_RED_L)):
        c.line(bx, by + 4, 16, 22, T_WHITE_D)
    c.outline(where=np.mgrid[0:32, 0:32][0] >= 20)
    for bx, by, col, hl in ((9, 6, T_RED, T_RED_L), (16, 4, T_BLUE, T_BLUE_L), (23, 7, T_YEL, T_WHITE),
                            (12, 12, T_BLUE, T_BLUE_L), (20, 12, T_RED, T_RED_L)):
        b = Canvas(32, 32)
        b.disc(bx + 0.5, by + 0.5, 4, col)
        b.disc(bx - 0.5, by - 0.5, 1.5, hl)
        b.outline()
        m = b.a > 0
        c.a[m] = b.a[m]
    return c


# ---------------------------------------------------------------- Gullwing Point: lighthouse, mirrors, pirate ship
def lighthouse():
    c = Canvas(48, 112)
    # tower, narrowing towards the top, in red and white bands
    for y in range(36, 108):
        half = 10 + (y - 36) * 6 / 72
        band = ((y - 36) // 12) % 2
        for x in range(int(24 - half), 24):
            col = T_RED if band == 0 else T_WHITE
            if x < 24 - half + 3:
                col = T_RED_D if band == 0 else T_WHITE_D
            c.px(x, y, col)
    c.rect(4, 104, 23, 111, T_WOOD_D)
    c.rect(4, 104, 23, 104, T_WOOD_L)
    # door
    c.rect(19, 90, 23, 103, T_DARK)
    c.rect(19, 89, 23, 89, T_YEL_D)
    # gallery and lantern room
    c.rect(8, 30, 23, 35, T_BLUE_D)
    c.rect(8, 30, 23, 30, T_BLUE_L)
    for x in range(9, 24, 3):
        c.rect(x, 31, x, 34, T_BLUE)
    c.rect(13, 14, 23, 29, T_YEL)
    c.rect(13, 14, 15, 29, T_YEL_D)
    c.rect(18, 16, 23, 24, T_WHITE)
    c.rect(13, 21, 23, 21, T_BLUE_D)
    # roof
    for y in range(4, 14):
        half = 2 + (y - 4) * 10 / 9
        c.rect(int(24 - half), y, 23, y, T_RED if y > 6 else T_RED_L)
    c.disc(24, 3, 2.5, T_YEL)
    c.mirror()
    c.outline()
    return c


def mirror(flip):
    """A beam mirror on a stone base: '\\' (flip False) or '/' (flip True)."""
    c = Canvas(16, 16)
    c.rect(4, 12, 11, 15, W_STEEL_D)
    c.rect(4, 12, 11, 12, W_LIGHT)
    x0, x1 = (13, 2) if flip else (2, 13)
    c.line(x0, 1, x1, 11, W_GLASS, 3)
    c.line(x0, 1, x1, 11, W_WHITE, 1)
    c.outline()
    return c


def beam(vertical):
    c = Canvas(16, 16)
    if vertical:
        c.rect(5, 0, 10, 15, T_YEL)
        c.rect(7, 0, 8, 15, T_WHITE)
    else:
        c.rect(0, 5, 15, 10, T_YEL)
        c.rect(0, 7, 15, 8, T_WHITE)
    return c


def planks(c, x0=0, y0=0, x1=None, y1=None):
    x1 = c.w - 1 if x1 is None else x1
    y1 = c.h - 1 if y1 is None else y1
    c.rect(x0, y0, x1, y1, T_WOOD)
    for y in range(y0, y1 + 1):
        if (y - y0) % 8 == 7:
            c.rect(x0, y, x1, y, T_WOOD_D)
        elif (y - y0) % 8 == 0:
            c.rect(x0, y, x1, y, T_WOOD_L)
    for y in range(y0, y1 + 1, 8):
        off = 5 if (y // 8) % 2 else 13
        for x in range(x0 + off, x1 + 1, 16):
            c.rect(x, y + 1, x, y + 6, T_WOOD_D)


def pirate_ship():
    c = Canvas(176, 112)
    yy, xx = np.mgrid[0:112, 0:176]
    # hull: a long curved shape with a raised stern on the left
    hull = (yy >= 62) & (yy <= 104) & (np.hypot((xx + 0.5 - 92) / 86, (yy + 0.5 - 60) / 46) < 1)
    hull |= (xx >= 10) & (xx < 44) & (yy >= 52) & (yy < 62)
    c.a[hull] = T_WOOD
    c.a[hull & (yy % 8 == 0)] = T_WOOD_D
    c.a[hull & (yy % 8 == 1)] = T_WOOD_L
    c.a[hull & (yy >= 73) & (yy <= 74)] = T_YEL
    for x in range(40, 160, 20):
        c.rect(x, 78, x + 6, 83, T_DARK)
        c.rect(x, 78, x + 6, 78, T_OUT)
    # rails along the deck
    c.rect(10, 50, 43, 51, T_WOOD_D)
    c.rect(44, 60, 170, 61, T_WOOD_D)
    # the entry hatch in the hull side, by the gangplank (block row 4, column 0)
    c.rect(9, 67, 15, 79, T_DARK)
    c.rect(9, 66, 15, 66, T_YEL)
    # masts and dark sails with a pale ball emblem
    for mx, top in ((76, 2), (128, 10)):
        c.rect(mx - 1, top, mx + 1, 62, T_WOOD_D)
        sail = (yy > top + 6) & (yy < 54) & (np.abs(xx + 0.5 - mx) < 22 - (54 - yy) * 0.12)
        c.a[sail] = T_DARK
        c.a[sail & (np.abs(xx + 0.5 - mx) > 19 - (54 - yy) * 0.12)] = T_OUT
        c.disc(mx + 0.5, (top + 54) / 2, 8, T_WHITE)
        c.disc(mx + 0.5, (top + 54) / 2, 8, T_WHITE_D, 6.5)
        c.rect(mx - 8, int((top + 54) / 2), mx + 8, int((top + 54) / 2) + 1, T_DARK)
        c.disc(mx + 0.5, (top + 54) / 2 + 0.5, 2.5, T_DARK)
        c.disc(mx + 0.5, (top + 54) / 2 + 0.5, 1.5, T_WHITE)
    # flag on the main mast
    c.rect(77, 2, 92, 9, T_DARK)
    c.rect(82, 4, 85, 7, T_WHITE)
    c.outline()
    return c


def gangplank():
    c = Canvas(16, 16)
    planks(c, 0, 4, 15, 11)
    c.rect(0, 3, 15, 3, T_OUT)
    c.rect(0, 12, 15, 12, T_OUT)
    return c


def deck():
    c = Canvas(16, 16)
    planks(c)
    return c


def rail(side):
    c = Canvas(16, 16)
    planks(c)
    if side == 'top':
        c.rect(0, 0, 15, 5, T_WOOD_D)
        c.rect(0, 0, 15, 1, T_WOOD_L)
        c.rect(0, 6, 15, 6, T_OUT)
    elif side == 'bottom':
        c.rect(0, 10, 15, 15, T_WOOD_D)
        c.rect(0, 10, 15, 10, T_WOOD_L)
        c.rect(0, 9, 15, 9, T_OUT)
    elif side == 'left':
        c.rect(0, 0, 5, 15, T_WOOD_D)
        c.rect(0, 0, 1, 15, T_WOOD_L)
        c.rect(6, 0, 6, 15, T_OUT)
    else:
        c.rect(10, 0, 15, 15, T_WOOD_D)
        c.rect(14, 0, 15, 15, T_WOOD_L)
        c.rect(9, 0, 9, 15, T_OUT)
    return c


def deck_thing(kind):
    c = Canvas(32 if kind in ('cannon', 'hatch', 'mast') else 16, 48 if kind == 'mast' else 32 if kind in ('hatch', 'helm') else 16)
    planks(c)
    if kind == 'barrel':
        c.rect(3, 2, 12, 14, T_OUT)
        c.rect(4, 3, 11, 13, T_WOOD)
        c.rect(4, 3, 11, 4, T_WOOD_L)
        c.rect(4, 6, 11, 6, T_BLUE_D)
        c.rect(4, 11, 11, 11, T_BLUE_D)
    elif kind == 'chest':
        c.rect(2, 4, 13, 14, T_OUT)
        c.rect(3, 5, 12, 13, T_WOOD_D)
        c.rect(3, 5, 12, 8, T_WOOD)
        c.rect(3, 9, 12, 9, T_YEL)
        c.rect(7, 8, 8, 11, T_YEL)
    elif kind == 'cannon':
        c.rect(4, 4, 27, 11, T_OUT)
        c.rect(5, 5, 26, 10, T_BLUE_D)
        c.rect(5, 5, 26, 6, T_BLUE)
        c.rect(24, 4, 27, 11, T_BLUE_D)
        c.disc(10, 12, 3.5, T_OUT)
        c.disc(10, 12, 2.5, T_WOOD_D)
        c.disc(20, 12, 3.5, T_OUT)
        c.disc(20, 12, 2.5, T_WOOD_D)
    elif kind == 'helm':
        c.rect(7, 18, 8, 31, T_WOOD_D)
        c.disc(8, 11, 8, T_OUT)
        c.disc(8, 11, 7, T_WOOD, 4.5)
        c.disc(8, 11, 2.5, T_YEL)
        for k in range(8):
            t = k * np.pi / 4
            c.line(8 + 2 * np.cos(t), 11 + 2 * np.sin(t), 8 + 8.5 * np.cos(t), 11 + 8.5 * np.sin(t), T_WOOD_D)
    elif kind == 'hatch':
        c.rect(2, 2, 29, 29, T_OUT)
        c.rect(3, 3, 28, 28, T_WOOD_D)
        for y in range(6, 28, 5):
            c.rect(5, y, 26, y + 1, T_DARK)
    elif kind == 'mast':
        c.disc(16, 40, 9, T_OUT)
        c.disc(16, 40, 8, T_WOOD_D)
        c.rect(13, 0, 18, 40, T_OUT)
        c.rect(14, 0, 17, 40, T_WOOD)
        c.rect(14, 0, 14, 40, T_WOOD_L)
        c.line(4, 2, 16, 26, T_WHITE_D)
        c.line(28, 2, 16, 26, T_WHITE_D)
    return c


# name: (draw, palette, options). over: blocks drawn over sprites; walk: blocks
# you can walk on; counter: blocks you can talk across.
STALL = {'over': lambda r, c: r == 0, 'counter': lambda r, c: r == 2}
OBJECTS = {
    'ferris_wheel': (ferris_wheel, 7, {}),
    'big_top': (big_top, 11, {}),
    'stall_red': (lambda: stall(T_RED, T_RED_D, T_YEL), 11, STALL),
    'stall_blue': (lambda: stall(T_BLUE, T_BLUE_D, T_YEL), 11, STALL),
    'arch': (arch, 11, {'over': lambda r, c: r < 2 and 1 <= c <= 4, 'walk': lambda r, c: r < 2 and 1 <= c <= 4}),
    'lamp': (lamp, 11, {'over': lambda r, c: r == 0}),
    'balloons': (balloons, 11, {'over': lambda r, c: r == 0}),
    # Gullwing Point
    'lighthouse': (lighthouse, 11, {'over': lambda r, c: r < 2, 'door': lambda r, c: (r, c) == (6, 1)}),
    'mirror_a': (lambda: mirror(False), 7, {}),
    'mirror_b': (lambda: mirror(True), 7, {}),
    'beam_h': (lambda: beam(False), 11, {'walk': lambda r, c: True}),
    'beam_v': (lambda: beam(True), 11, {'walk': lambda r, c: True}),
    'pirate_ship': (pirate_ship, 11, {'ground': WATER, 'door': lambda r, c: (r, c) == (4, 0)}),
    'gangplank': (gangplank, 11, {'ground': WATER, 'walk': lambda r, c: True}),
    'gangplank_exit': (gangplank, 11, {'ground': WATER, 'behavior': lambda r, c: MB_WEST_ARROW_WARP}),
    'deck': (deck, 11, {'walk': lambda r, c: True}),
    'rail_top': (lambda: rail('top'), 11, {}),
    'rail_bottom': (lambda: rail('bottom'), 11, {}),
    'rail_left': (lambda: rail('left'), 11, {}),
    'rail_right': (lambda: rail('right'), 11, {}),
    'barrel': (lambda: deck_thing('barrel'), 11, {}),
    'chest': (lambda: deck_thing('chest'), 11, {}),
    'cannon': (lambda: deck_thing('cannon'), 11, {}),
    'helm': (lambda: deck_thing('helm'), 11, {'over': lambda r, c: r == 0}),
    'hatch': (lambda: deck_thing('hatch'), 11, {'door': lambda r, c: r == 1}),
    'mast': (lambda: deck_thing('mast'), 11, {'over': lambda r, c: r < 2}),
}


def main():
    pack('data/tilesets/secondary/petalburg', 'data/tilesets/secondary/carnival', OBJECTS,
         {7: PAL_WHEEL, 11: PAL_TENT}, GROUND, f'{ROOT}/tools/areas/carnival_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
