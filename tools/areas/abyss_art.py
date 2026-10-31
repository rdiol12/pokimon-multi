"""The sea floor's tileset: Hoenn's underwater tiles plus new pixel art in
Emerald's style, toned to sit in the deep blue: coral, a giant clam, the
domes, columns, lamp pillars and temple of a sunken city, and a shipwreck.

    python3 tools/areas/abyss_art.py

Writes data/tilesets/secondary/abyss/ and tools/areas/abyss_tiles.json.
The underwater tiles keep their places (their seaweed animates), so the new
tiles go into the slots nothing uses (pack(fill_free=True)).
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

FLOOR = 0x216             # the dark sea floor shows under see-through pixels
PAL = [(0, 0, 0), (24, 24, 90), (156, 57, 139), (222, 98, 180), (255, 164, 222), (238, 139, 98), (255, 230, 131),
       (57, 82, 139), (98, 131, 189), (156, 189, 230), (230, 238, 255), (82, 197, 222), (172, 238, 246),
       (82, 49, 82), (131, 90, 106), (172, 131, 139)]
OUT, PINK_D, PINK, PINK_L, ORANGE, GLOW, STONE_D, STONE, STONE_L, PEARL, GLASS, GLASS_L, WOOD_D, WOOD, WOOD_L = \
    range(1, 16)


def branch(c, x, y, ang, length, width, col, depth):
    """A coral branch that forks as it grows."""
    x1, y1 = x + length * np.sin(ang), y - length * np.cos(ang)
    c.line(x, y, x1, y1, col, width)
    if depth:
        for d in (-0.5, 0.45):
            branch(c, x1, y1, ang + d, length * 0.7, max(1, width - 1), col, depth - 1)
    else:
        c.disc(x1 + 0.5, y1 + 0.5, 1.6, PINK_L)


def coral_branch():
    c = Canvas(32, 32)
    branch(c, 16, 31, -0.15, 10, 3, PINK, 2)
    branch(c, 13, 31, -0.8, 7, 2, PINK, 2)
    branch(c, 19, 31, 0.7, 8, 2, PINK_D, 2)
    c.outline()
    return c


def coral_fan():
    c = Canvas(32, 32)
    yy, xx = np.mgrid[0:32, 0:32]
    d = np.hypot(xx + 0.5 - 16, yy + 0.5 - 30)
    ang = np.arctan2(xx + 0.5 - 16, 30 - yy - 0.5)
    fan = (d < 15) & (np.abs(ang) < 1.15)
    c.a[fan] = ORANGE
    rib = fan & ((np.round(ang * 6) % 2 == 0) & (d > 4))
    c.a[rib & (d % 4 < 1.2)] = PINK
    c.a[fan & (d > 13)] = GLOW
    c.rect(15, 26, 16, 31, PINK_D)
    c.outline()
    return c


def coral_tubes():
    c = Canvas(32, 16)
    for x, h, col in ((5, 9, GLOW), (10, 13, ORANGE), (15, 8, GLOW), (20, 12, ORANGE), (25, 7, PINK)):
        c.rect(x, 16 - h, x + 3, 15, col)
        c.rect(x, 16 - h, x + 3, 16 - h, PINK_L)
        c.rect(x + 1, 16 - h, x + 2, 16 - h, PINK_D)
    c.outline()
    return c


def clam():
    c = Canvas(32, 32)
    yy, xx = np.mgrid[0:32, 0:32]
    lower = (np.hypot((xx + 0.5 - 16) / 14, (yy + 0.5 - 22) / 8) < 1) & (yy >= 22)
    upper = (np.hypot((xx + 0.5 - 16) / 14, (yy + 0.5 - 20) / 13) < 1) & (yy < 20)
    c.a[lower] = STONE
    c.a[upper] = STONE
    c.a[upper & ((xx // 4) % 2 == 0)] = STONE_L
    c.a[lower & ((xx // 4) % 2 == 0)] = STONE_D
    c.rect(4, 19, 27, 22, PINK_D)                  # the soft inside
    c.disc(16.5, 20.5, 3, PEARL)
    c.disc(15.5, 19.5, 1, GLASS_L)
    c.outline()
    return c


def dome():
    c = Canvas(80, 64)
    # stone drum
    c.rect(6, 34, 73, 63, STONE)
    c.rect(6, 34, 73, 36, STONE_L)
    for x in range(10, 72, 12):
        c.rect(x, 38, x + 1, 62, STONE_D)
    c.rect(6, 60, 73, 63, STONE_D)
    # glass dome with ribs
    yy, xx = np.mgrid[0:64, 0:80]
    d = np.hypot((xx + 0.5 - 40) / 34, (yy + 0.5 - 35) / 31)
    glass = (d < 1) & (yy < 35)
    c.a[glass] = GLASS
    c.a[glass & (d > 0.86)] = STONE_L
    ang = np.arctan2(xx + 0.5 - 40, 35 - yy)
    c.a[glass & (np.abs(np.sin(ang * 4)) < 0.12)] = STONE_L
    c.a[glass & (d < 0.55) & (xx < 34) & (yy < 22) & (yy > 12)] = GLASS_L
    c.disc(40, 4, 3, GLOW)
    # door and round windows
    c.rect(33, 46, 46, 63, OUT)
    c.disc(40, 47, 7, OUT)
    c.rect(34, 47, 45, 63, WOOD_D)
    c.disc(40, 47.5, 6, WOOD_D)
    for wx in (18, 62):
        c.disc(wx, 47, 5, OUT)
        c.disc(wx, 47, 4, GLOW)
        c.disc(wx - 1, 46, 1.5, PEARL)
    c.mirror()
    c.outline()
    return c


def column(broken=False):
    h = 32 if broken else 48
    c = Canvas(16, h)
    c.rect(3, 4, 12, h - 4, STONE)
    c.rect(3, 4, 4, h - 4, STONE_L)
    c.rect(11, 4, 12, h - 4, STONE_D)
    c.rect(7, 6, 8, h - 6, STONE_D)
    c.rect(1, h - 4, 14, h - 1, STONE_D)
    c.rect(1, h - 4, 14, h - 4, STONE_L)
    if broken:
        for x in range(3, 13):
            c.rect(x, 0, x, 3 + (x * 7) % 5, 0)
    else:
        c.rect(1, 0, 14, 4, STONE_L)
        c.rect(1, 4, 14, 4, STONE_D)
    c.outline()
    return c


def lamp(lit):
    c = Canvas(16, 32)
    c.rect(5, 10, 10, 31, STONE)
    c.rect(5, 10, 6, 31, STONE_L)
    c.rect(9, 10, 10, 31, STONE_D)
    c.rect(3, 27, 12, 31, STONE_D)
    c.disc(8, 6, 6, OUT)
    c.disc(8, 6, 5, GLOW if lit else STONE_D)
    if lit:
        c.disc(7, 5, 2.5, PEARL)
    else:
        c.disc(7, 5, 2, STONE)
    c.outline()
    return c


def temple(open_door=False):
    c = Canvas(128, 96)
    # steps
    for i in range(4):
        c.rect(4 + i * 4, 84 - i * 4, 123 - i * 4, 95 - i * 4, STONE_D if i % 2 else STONE)
        c.rect(4 + i * 4, 84 - i * 4, 123 - i * 4, 84 - i * 4, STONE_L)
    # body and roof
    c.rect(16, 30, 111, 72, STONE)
    for y in range(6, 30):
        half = 8 + (y - 6) * 54 / 24
        c.rect(int(64 - half), y, int(63 + half), y, STONE_L if y < 10 else STONE)
    c.rect(10, 26, 117, 31, STONE_D)
    c.rect(10, 26, 117, 26, STONE_L)
    c.disc(64, 18, 6, OUT)
    c.disc(64, 18, 5, GLOW)
    c.disc(63, 17, 2, PEARL)
    # columns
    for x in range(20, 108, 16):
        if 48 <= x <= 72:
            continue
        c.rect(x, 32, x + 7, 72, STONE_L)
        c.rect(x + 5, 32, x + 7, 72, STONE_D)
    # door
    c.rect(48, 40, 79, 72, OUT)
    if open_door:
        c.rect(50, 42, 77, 72, OUT)
    else:
        c.rect(50, 42, 77, 72, STONE_D)
        c.rect(63, 42, 64, 72, OUT)
        for k, col in enumerate((PINK, GLASS, GLOW, ORANGE)):
            c.disc(57 + (k % 2) * 14, 50 + (k // 2) * 12, 3, col)
    c.mirror()
    c.outline()
    return c


def wreck():
    c = Canvas(160, 80)
    yy, xx = np.mgrid[0:80, 0:160]
    hull = (np.hypot((xx + 0.5 - 80) / 74, (yy + 0.5 - 30) / 46) < 1) & (yy > 30) & (yy < 76)
    c.a[hull] = WOOD
    c.a[hull & (yy % 8 == 0)] = WOOD_D
    c.a[hull & (yy % 8 == 1)] = WOOD_L
    # deck line and a broken mast
    c.rect(8, 30, 151, 33, WOOD_D)
    c.rect(8, 30, 151, 30, WOOD_L)
    c.line(96, 30, 118, 2, WOOD_D, 4)
    c.line(95, 30, 117, 2, WOOD_L, 1)
    c.line(104, 18, 132, 12, WOOD, 3)
    # portholes
    for x in (30, 50, 110, 130):
        c.disc(x, 46, 4, OUT)
        c.disc(x, 46, 3, GLASS)
    # the hole in the hull you swim through (block row 4, column 4)
    c.disc(72, 66, 6, OUT)
    c.rect(66, 66, 78, 79, OUT)
    # seaweed and coral grown over it
    for x in (14, 22, 140, 148):
        c.line(x, 79, x + 3, 62, PINK_D, 2)
    c.outline()
    c.a[hull & (yy > 72)] = 0
    c.outline()
    return c


def anchor():
    c = Canvas(32, 32)
    c.disc(16, 5, 4, STONE_D, 2)
    c.rect(15, 8, 17, 26, STONE)
    c.rect(9, 12, 23, 13, STONE)
    yy, xx = np.mgrid[0:32, 0:32]
    arc = (np.abs(np.hypot(xx + 0.5 - 16, yy + 0.5 - 18) - 10) < 1.5) & (yy > 20)
    c.a[arc] = STONE
    c.rect(4, 22, 7, 24, STONE_L)
    c.rect(25, 22, 28, 24, STONE_L)
    c.outline()
    return c


def glyph():
    """A glowing ring on the floor: the way out of a room."""
    c = Canvas(16, 16)
    c.disc(8, 8, 7, GLASS, 4.5)
    c.disc(8, 8, 5.5, GLASS_L, 4.5)
    c.disc(8, 8, 2, GLOW)
    return c


MB_SOUTH_ARROW_WARP = 0x65
OBJECTS = {
    'exit_glyph': (glyph, 12, {'behavior': lambda r, c: MB_SOUTH_ARROW_WARP}),
    'coral_branch': (coral_branch, 12, {}),
    'coral_fan': (coral_fan, 12, {}),
    'coral_tubes': (coral_tubes, 12, {}),
    'clam': (clam, 12, {}),
    'dome': (dome, 12, {}),
    'column': (lambda: column(False), 12, {'over': lambda r, c: r == 0}),
    'column_broken': (lambda: column(True), 12, {}),
    'lamp_dark': (lambda: lamp(False), 12, {'over': lambda r, c: r == 0}),
    'lamp_lit': (lambda: lamp(True), 12, {'over': lambda r, c: r == 0}),
    'temple': (temple, 12, {'walk': lambda r, c: r == 5}),
    'temple_open': (lambda: temple(True), 12, {'walk': lambda r, c: r == 5, 'door': lambda r, c: r == 4 and c in (3, 4)}),
    'wreck': (wreck, 12, {'door': lambda r, c: (r, c) == (4, 4)}),
    'anchor': (anchor, 12, {}),
}


def main():
    pack('data/tilesets/secondary/underwater', 'data/tilesets/secondary/abyss', OBJECTS, {12: PAL}, FLOOR,
         f'{ROOT}/tools/areas/abyss_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
