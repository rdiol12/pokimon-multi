"""Amberwood Hollow: an original tileset (every pixel drawn here) and the map that uses it.

usage: python3 tools/amberwood/gen.py OUTDIR   -> OUTDIR/tiles.png, palettes/*.pal, metatiles.bin, metatile_attributes.bin,
                          map.bin, border.bin, preview.png, info.json
"""
import json, math, os, struct, sys
import numpy as np
from PIL import Image

OUT = sys.argv[1]
os.makedirs(OUT + '/palettes', exist_ok=True)

# ---------------------------------------------------------------- palettes
PALS = {  # colours follow Emerald's own field palettes: mint grass, navy outlines, sandy paths, sky-blue water
 6: [(255, 0, 255), (24, 41, 82), (65, 180, 131), (115, 197, 164), (164, 213, 197), (213, 238, 222), (205, 156, 82), (222, 205, 131),
     (238, 230, 164), (205, 65, 82), (238, 148, 115), (255, 213, 98), (255, 255, 255), (222, 106, 98), (131, 131, 139), (189, 189, 205)],
 7: [(255, 0, 255), (32, 48, 40), (106, 74, 57), (156, 106, 74), (197, 148, 98), (41, 115, 49), (74, 156, 65), (115, 197, 90),
     (164, 230, 123), (213, 246, 172), (172, 57, 41), (222, 98, 49), (246, 156, 57), (255, 213, 98), (222, 65, 57), (255, 255, 238)],
 8: [(255, 0, 255), (65, 106, 189), (98, 164, 238), (115, 189, 246), (172, 213, 246), (255, 255, 255), (106, 74, 57), (156, 106, 74),
     (197, 148, 98), (230, 197, 139), (24, 41, 82), (131, 131, 139), (189, 189, 205), (57, 139, 49), (131, 197, 98), (255, 189, 205)],
 9: [(255, 0, 255), (24, 41, 82), (65, 74, 106), (98, 98, 123), (156, 164, 189), (222, 230, 238), (57, 139, 49), (131, 197, 98),
     (82, 65, 156), (123, 106, 222), (172, 164, 246), (230, 238, 255), (106, 74, 57), (156, 106, 74), (197, 148, 98), (238, 197, 74)],
 10: [(255, 0, 255), (24, 41, 82), (106, 74, 57), (156, 106, 74), (205, 156, 106), (65, 123, 148), (98, 164, 189), (164, 213, 222),
      (65, 74, 106), (131, 131, 139), (222, 74, 41), (246, 156, 49), (255, 238, 131), (98, 106, 131), (238, 230, 205), (74, 57, 49)],
}
for slot in range(16):
    cols = PALS.get(slot, [(0, 0, 0)] * 16)
    with open(f'{OUT}/palettes/{slot:02d}.pal', 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n' + ''.join(f'{r} {g} {b}\n' for r, g, b in cols))

def rnd(*k):
    h = 2166136261
    for v in k:
        h = ((h ^ (int(v) & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
    h ^= h >> 13; h = (h * 0x5bd1e995) & 0xFFFFFFFF; h ^= h >> 15
    return h / 4294967296.0

# ---------------------------------------------------------------- textures (periodic per 16x16 cell)
def grass_tex(var):
    a = np.full((16, 16), 3, np.uint8)
    for y in range(16):
        for x in range(16):
            r = rnd(1, var, x, y)
            if r < 0.03: a[y, x] = 2
            elif r < 0.06: a[y, x] = 4
    # small blades: a light tip over a dark root
    for i in range(3 + var):
        bx, by = int(rnd(2, var, i) * 14) + 1, int(rnd(3, var, i) * 13) + 2
        a[by, bx] = 2; a[by - 1, bx] = 4; a[by - 2, bx - 1] = 5 if i % 2 else 4
    if var == 2:  # fallen leaves
        for i in range(5):
            lx, ly = int(rnd(4, i) * 15), int(rnd(5, i) * 15)
            c = (9, 10, 11)[i % 3]
            a[ly, lx] = c
            if lx + 1 < 16: a[ly, lx + 1] = c
    return a

def dirt_tex():
    a = np.full((16, 16), 7, np.uint8)
    for y in range(16):
        for x in range(16):
            r = rnd(7, x, y)
            if r < 0.12: a[y, x] = 6
            elif r < 0.22: a[y, x] = 8
    for i in range(3):  # pebbles
        px, py = int(rnd(8, i) * 14), int(rnd(9, i) * 14)
        a[py, px] = 15; a[py, px + 1] = 14; a[py + 1, px] = 6; a[py + 1, px + 1] = 6
    return a

def tall_tex():
    a = np.full((16, 16), 2, np.uint8)
    for col in range(0, 16, 2):
        h = 9 + int(rnd(10, col) * 6)
        off = int(rnd(11, col) * 2)
        for y in range(16 - h, 16):
            x = (col + off + (1 if y < 16 - h + 3 else 0)) % 16
            a[y, x] = 4 if y < 16 - h + 2 else 3
        tip = (16 - h) % 16
        a[tip, (col + off + 1) % 16] = 10 if col % 4 == 0 else 5
    for x in range(16):
        a[15, x] = 2 if x % 3 else 1
    a[7, :] = np.where(a[7, :] == 2, 1, a[7, :])
    return a

def flower_tex():
    a = grass_tex(0).copy()
    for i, (fx, fy) in enumerate(((3, 3), (11, 2), (7, 8), (2, 12), (12, 11))):
        c = 12 if i % 2 else 13
        for dx, dy in ((0, -1), (-1, 0), (1, 0), (0, 1)):
            a[fy + dy, fx + dx] = c
        a[fy, fx] = 11
    return a

def water_tex(var=0):
    a = np.full((16, 16), 2, np.uint8)
    for y in range(16):
        for x in range(16):
            if rnd(20, x // 2, y) < 0.08: a[y, x] = 1
    for i in range(2):
        wx, wy = int(rnd(21, i) * 10) + 1, 4 + i * 7
        for d in range(4): a[wy, wx + d] = 3
        a[wy - 1, wx + 1] = 3; a[wy - 1, wx + 2] = 4
    return a

GRASS = [grass_tex(v) for v in range(3)]
DIRT, TALL, FLOWERS, WATER = dirt_tex(), tall_tex(), flower_tex(), water_tex()

# ---------------------------------------------------------------- sprites (index arrays, 0 = clear)
def blank(w, h): return np.zeros((h, w), np.uint8)

def disc(a, cx, cy, r, c, squash=1.0):
    h, w = a.shape
    for y in range(h):
        for x in range(w):
            if ((x + .5 - cx) ** 2 + ((y + .5 - cy) / squash) ** 2) <= r * r:
                a[y, x] = c

def outline(a, c=1):
    h, w = a.shape
    m = a > 0
    out = a.copy()
    for y in range(h):
        for x in range(w):
            if not m[y, x]:
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    if 0 <= x + dx < w and 0 <= y + dy < h and m[y + dy, x + dx]:
                        out[y, x] = c
                        break
    return out

def shade_canopy(a, cols, light=(-0.6, -0.8)):
    """re-colour filled pixels of a blob by a fake light direction + clumps"""
    h, w = a.shape
    ys, xs = np.nonzero(a)
    cx, cy = xs.mean(), ys.mean()
    for y, x in zip(ys, xs):
        d = ((x - cx) * light[0] + (y - cy) * light[1]) / 12.0
        d += (rnd(30, x // 4, y // 4) - 0.5) * 0.25
        k = int(np.clip((d + 1) / 2 * len(cols), 0, len(cols) - 1))
        a[y, x] = cols[k]

def tree(kind):
    a = blank(32, 32)
    # trunk
    for y in range(20, 30):
        for x in range(13, 19):
            a[y, x] = 3 if x < 15 else 4 if x < 17 else 2
    a[29, 11:21] = 2; a[28, 12] = 2; a[28, 19] = 2
    can = blank(32, 32)
    if kind == 'pine':
        for y in range(1, 25):
            half = int((y - 1) * 0.55) + 2 - (3 if y in (9, 10, 17, 18) else 0)
            for x in range(16 - half, 16 + half):
                if 0 <= x < 32: can[y, x] = 1
        shade_canopy(can, [8, 7, 6, 5, 5])
    else:
        for cx, cy, r in ((16, 10, 9), (9, 15, 7), (23, 15, 7), (16, 17, 8), (11, 7, 5), (21, 7, 5)):
            disc(can, cx, cy, r, 1)
        cols = {'red': [13, 12, 11, 10, 10], 'gold': [13, 13, 12, 11, 10], 'green': [9, 8, 7, 6, 5]}[kind]
        shade_canopy(can, cols)
    m = can > 0
    a[m] = can[m]
    return outline(a)

def bush():
    a = blank(16, 16)
    for cx, cy, r in ((8, 9, 6), (5, 8, 4), (11, 8, 4)):
        disc(a, cx, cy, r, 1)
    shade_canopy(a, [9, 8, 7, 6])
    return outline(a)

def stump():
    a = blank(16, 16)
    for y in range(6, 14):
        for x in range(3, 13):
            a[y, x] = 3 if x < 7 else 4 if x < 10 else 2
    disc(a, 8, 6, 5, 4, 0.5)
    disc(a, 8, 6, 3, 13, 0.5); a[6, 8] = 12; a[6, 7] = 4
    a[13, 2] = 2; a[13, 13] = 2
    return outline(a)

def mushrooms():
    a = blank(16, 16)
    for mx, my in ((5, 9), (10, 12), (12, 6)):
        a[my, mx] = 15; a[my + 1, mx] = 15
        for dx in (-2, -1, 0, 1, 2): a[my - 1, mx + dx] = 14
        for dx in (-1, 0, 1): a[my - 2, mx + dx] = 14
        a[my - 2, mx] = 15
    return outline(a)

def rock(pal_hi=5):
    a = blank(16, 16)
    disc(a, 8, 9, 6.5, 1, 0.8)
    disc(a, 6, 7, 3, 1)
    shade_canopy(a, [5, 4, 3, 3, 2])
    a[11:14, 3:6] = np.where(a[11:14, 3:6] > 0, 6, 0)
    return outline(a)

def stone_wall():
    a = np.full((16, 16), 3, np.uint8)
    for y in range(16):
        row = y // 5
        a[y, :] = 3
        if y % 5 == 4: a[y, :] = 2
        for x in range(16):
            if (x + row * 5) % 10 == 0: a[y, x] = 2
            elif (x + row * 5) % 10 == 1 and y % 5 != 4: a[y, x] = 4
        if y % 5 == 0: a[y, :] = np.where(a[y, :] == 3, 4, a[y, :])
    for i in range(6):
        mx, my = int(rnd(40, i) * 15), int(rnd(41, i) * 15)
        a[my, mx] = 6 if i % 2 else 7
    return a

def stone_floor():
    a = np.full((16, 16), 4, np.uint8)
    a[0, :] = 3; a[:, 0] = 3; a[8, :] = 3; a[:, 8] = 3
    a[1, 1:8] = 5; a[9, 9:16] = 5; a[1, 9:16] = 5; a[9, 1:8] = 5
    for i in range(4):
        a[int(rnd(42, i) * 14) + 1, int(rnd(43, i) * 14) + 1] = 3
    a[3, 12] = 7; a[13, 4] = 6
    return a

def crystal(big=False):
    a = blank(16, 16)
    spikes = ((8, 1, 3), (4, 6, 2), (12, 5, 2)) if big else ((8, 4, 2), (5, 8, 2), (11, 8, 2))
    for sx, top, hw in spikes:
        for y in range(top, 15):
            w = min(hw, (y - top) // 2 + 1)
            for x in range(sx - w, sx + w):
                a[y, x] = 9 if x < sx else 8
        a[top + 1:top + 4, sx - 1] = 11
        a[top + 4:top + 6, sx - 1] = 10
    a[14, 3:13] = np.where(a[14, 3:13] == 0, 2, a[14, 3:13])
    return outline(a)

def barrier():
    a = stone_floor().copy()
    c = crystal(True)
    m = c > 0
    a[m] = c[m]
    return a

def pillar():  # 16x32
    a = blank(16, 32)
    a[2:30, 4:12] = 3
    a[2:30, 4:6] = 4
    a[2:30, 10:12] = 2
    a[0:3, 2:14] = 4; a[0, 2:14] = 5
    a[28:32, 2:14] = 3; a[28, 2:14] = 4
    for y in (8, 16, 22): a[y, 5:10] = 6 if y != 16 else 7
    a[10:14, 6:10] = 9; a[11:13, 7:9] = 11
    return outline(a)

def altar():  # 32x16, shrine centrepiece
    a = blank(32, 16)
    a[6:15, 2:30] = 3; a[6, 2:30] = 5; a[7, 2:30] = 4; a[14, 2:30] = 2
    a[8:14, 2:4] = 4; a[8:14, 28:30] = 2
    for x in range(6, 27, 5): a[10:12, x:x + 2] = 15
    return outline(a)

def lantern(lit):  # 16x32: head cell above, base cell below
    a = blank(16, 32)
    a[30:32, 4:12] = 8; a[29, 5:11] = 9
    a[16:29, 7:9] = 9; a[16:29, 9] = 8
    a[2:4, 3:13] = 8; a[1, 5:11] = 9; a[0, 7:9] = 8   # roof
    a[4:13, 4:12] = 8
    a[5:12, 5:11] = 12 if lit else 13
    if lit:
        a[7:11, 7:9] = 11; a[8:10, 7:9] = 10; a[5, 6:10] = 14
    else:
        a[6:11, 6] = 9
    a[13:16, 6:10] = 9
    return outline(a)

def glow(a):  # warm light spilled on the ground by a lit lantern (grass palette)
    b = a.copy()
    for y in range(16):
        for x in range(16):
            if (x - 8) ** 2 + (y - 7) ** 2 < 30 and b[y, x] in (2, 3):
                b[y, x] = 11 if (x + y) % 3 == 0 else 4
    return b

def tent():  # 48x32
    a = blank(48, 32)
    for y in range(2, 30):
        half = int((y - 2) * 0.85) + 1
        for x in range(24 - half, 24 + half):
            if 0 <= x < 48:
                a[y, x] = 7 if x < 24 - half / 3 else 6 if x < 24 + half / 3 else 5
    for y in range(12, 30):
        half = int((y - 12) * 0.45)
        a[y, 24 - half:24 + half + 1] = 1
    a[1:30, 24] = np.where(a[1:30, 24] == 1, 1, 15)
    a[0:3, 23:25] = 2
    a[29, 2:46] = np.where(a[29, 2:46] > 0, 15, 0)
    for px in (3, 44):
        a[24:31, px] = 2
    return outline(a)

def campfire():
    a = blank(16, 16)
    for i, (x0, y0) in enumerate(((2, 12), (3, 13))):
        a[y0, x0:x0 + 11] = 2 if i else 3
    for y in range(3, 12):
        w = (y - 2) // 2
        a[y, 8 - w:8 + w + 1] = 11
    a[6:11, 7:9] = 12; a[4:7, 8] = 10; a[9:12, 5] = 10; a[9:12, 11] = 10
    for x in (2, 13): a[11:14, x] = 9
    return outline(a)

def signpost():
    a = blank(16, 16)
    a[2:10, 1:15] = 4; a[2, 1:15] = 14; a[9, 1:15] = 2
    a[4, 3:13] = 15; a[6, 3:10] = 15
    a[10:15, 7:9] = 3; a[10:15, 9] = 2
    return outline(a)

def log_():  # 32x16
    a = blank(32, 16)
    a[6:13, 3:29] = 3; a[6:8, 3:29] = 4; a[12, 3:29] = 2
    disc(a, 28.5, 9.5, 3.6, 4); a[9, 28] = 2; a[10, 29] = 2
    a[8:10, 12:15] = 2; a[4:6, 10:12] = 6; a[5, 11] = 7
    return outline(a)

# ---------------------------------------------------------------- the map (semantic grid)
W, H = 40, 36
g = [['F'] * W for _ in range(H)]
def fill(ch, x0, y0, x1, y1):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g[y][x] = ch
def ellipse(ch, cx, cy, rx, ry, wob=0.0, seed=0):
    for y in range(H):
        for x in range(W):
            d = ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2
            d += (rnd(50, seed, x, y) - 0.5) * wob
            if d <= 1: g[y][x] = ch

ellipse('.', 20, 18, 17.5, 15.5, 0.25, 1)          # the hollow
fill('.', 15, 2, 24, 11)                            # shrine yard
ellipse('~', 20, 28.5, 11, 4.2, 0.2, 2)              # lake
for y in range(3, 27):                               # stream from a spring in the north-east
    cx = 28 + int(round(2.2 * math.sin(y / 4.0)))
    for x in (cx, cx + 1): g[y][x] = '~'
ellipse('"', 34, 15, 4.2, 6.5, 0.4, 3)              # red-leaf meadow
ellipse('"', 9, 24, 3.6, 2.4, 0.4, 4)
fill('F', 0, 0, 12, 8)                               # thick woods in the north-west...
fill('.', 3, 3, 6, 5)                                # ...hiding a secret clearing
for x in range(7, 11): g[4][x] = '.'                 # reached by a narrow crawlway
for y in range(5, 9): g[y][10] = '.'
# paths
for y in range(10, 27): g[y][19] = g[y][20] = ':'
for x in range(8, 19): g[16][x] = g[17][x] = ':'
for x in range(21, 37):
    if g[16][x] != '~': g[16][x] = g[17][x] = ':'
for y in range(16, 18):
    for x in range(21, 37):
        if g[y][x] == '~': g[y][x] = '='          # bridge planks
for y in range(24, 32): g[y][19] = g[y][20] = '='  # pier into the lake
fill('o', 17, 4, 22, 8)                                # shrine
for x in range(16, 24): g[3][x] = g[9][x] = 'W'
for y in range(3, 10): g[y][16] = g[y][23] = 'W'
g[9][19] = g[9][20] = 'X'
for p in ((9, 21), (11, 20), (25, 21), (26, 9), (7, 22)): g[p[1]][p[0]] = '*'
for y in range(H):
    for x in range(W):
        if g[y][x] == '.' and rnd(55, x, y) < 0.06: g[y][x] = '*'
for (x, y) in ((10, 21), (12, 21), (24, 22), (26, 10), (27, 11), (13, 9)):
    if g[y][x] == '.': g[y][x] = '*'

for _ in range(2):   # keep a strip of grass between forest and water
    for y in range(H):
        for x in range(W):
            if g[y][x] == 'F' and any(0 <= x + dx < W and 0 <= y + dy < H and g[y + dy][x + dx] == '~' for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                g[y][x] = '.'
# objects: name -> list of (x, y) top-left cell
OBJ = {
    'tent': [(5, 13)], 'campfire': [(9, 18)], 'log': [(11, 19)], 'sign_dock': [(22, 22)], 'sign_shrine': [(21, 11)],
    'stone_carving': [(6, 3)], 'pillar': [(17, 4), (22, 4)], 'altar': [(19, 4)],
    'crystal': [(17, 7), (22, 7), (3, 3)], 'rock': [(14, 21), (31, 23), (25, 5), (10, 10)], 'stump': [(24, 13), (6, 20)],
    'mushrooms': [(13, 13), (24, 19), (3, 5)], 'lantern': [(11, 12), (33, 9), (13, 22)],
    'tree_red': [(13, 6), (33, 24)], 'tree_gold': [(24, 7), (5, 10)], 'tree_pine': [(14, 13)],
}
SOLID = {'F', 'W', 'X'}

# ---------------------------------------------------------------- render the whole map, both layers, per pixel
PX = 16
bot = np.zeros((H * PX, W * PX), np.uint8); botpal = np.full((H, W), 6, np.uint8)
top = np.zeros((H * PX, W * PX), np.uint8); toppal = np.zeros((H, W), np.uint8)
layer = np.full((H, W), 1, np.uint8)       # 1 COVERED, 0 NORMAL
behav = np.zeros((H, W), np.uint8)
coll = np.zeros((H, W), np.uint8)
elev = np.full((H, W), 3, np.uint8)

def T(x, y):
    if 0 <= x < W and 0 <= y < H: return g[y][x]
    return 'F'
iswater = lambda c: c in '~='
ispath = lambda c: c == ':'

# signed-distance-ish masks: a pixel belongs to a region if its (rounded) distance to region cells is small
def region_mask(cx, cy, pred, inset):
    """16x16 bool: pixels of cell (cx,cy) that lie inside the smoothed region given by pred.
    Uses only the 3x3 neighbourhood, so equal neighbourhoods give equal art."""
    m = np.zeros((16, 16), bool)
    nb = {(dx, dy): pred(T(cx + dx, cy + dy)) for dx in (-1, 0, 1) for dy in (-1, 0, 1)}
    for py in range(16):
        for px in range(16):
            u, v = (px + 0.5) / 16, (py + 0.5) / 16
            # distance from this pixel to the nearest NON-region cell, in pixels
            best = 99.0
            for (dx, dy), inside in nb.items():
                if inside: continue
                # distance from point to the square [dx,dx+1]x[dy,dy+1]
                ddx = max(dx - u, 0, u - (dx + 1)); ddy = max(dy - v, 0, v - (dy + 1))
                best = min(best, math.hypot(ddx, ddy) * 16)
            if not nb[(0, 0)]:
                m[py, px] = False
                continue
            wob = (rnd(60, px, py) - 0.5) * 1.5
            m[py, px] = best + wob > inset
    return m

def outside_mask(cx, cy, pred, reach):
    """pixels of a non-region cell that the region spills into (for rounded convex shapes)"""
    m = np.zeros((16, 16), bool)
    if pred(T(cx, cy)): return m
    return m

for cy in range(H):
    for cx in range(W):
        c = T(cx, cy)
        ys, xs = slice(cy * 16, cy * 16 + 16), slice(cx * 16, cx * 16 + 16)
        var = 0
        nb_all_same = all(T(cx + dx, cy + dy) == c for dx in (-1, 0, 1) for dy in (-1, 0, 1))
        if nb_all_same and c in '.F': var = int(rnd(70, cx, cy) * 3)
        if c == '"': base = TALL.copy(); behav[cy, cx] = 2
        elif c == '*': base = FLOWERS.copy()
        else: base = GRASS[var].copy()
        # paths are grass cells painted with dirt where the smoothed path region covers them
        if ispath(c):
            pm = region_mask(cx, cy, ispath, 1.5)
            # pixels just outside the path get a 1px dark rim
            d = DIRT.copy()
            base = np.where(pm, d, base)
            rim = np.zeros_like(pm)
            rim[1:, :] |= pm[:-1, :] & ~pm[1:, :]
            base = np.where(~pm & rim, 2, base)
        elif any(ispath(T(cx + dx, cy + dy)) for dx in (-1, 0, 1) for dy in (-1, 0, 1)) and c in '.*"':
            pass
        if c in 'oX':
            botpal[cy, cx] = 9
            base = stone_floor() if c == 'o' else barrier()
        if c == 'W':
            botpal[cy, cx] = 9; base = stone_wall()
        if iswater(c):
            botpal[cy, cx] = 8
            wt = WATER.copy()
            # foam where land is near
            land_m = ~region_mask(cx, cy, iswater, 2.5) if True else None
            near = ~region_mask(cx, cy, iswater, 4.0)
            wt = np.where(near & ~land_m, 3, wt)
            wt = np.where(land_m, 5, wt)
            if c == '=':
                vertical = T(cx, cy - 1) == '=' and T(cx, cy + 1) in '=~' and (T(cx - 1, cy) != '=' or T(cx + 1, cy) != '=')
                horiz = not (T(cx, cy - 1) == '=' or T(cx, cy + 1) == '=') or (T(cx - 1, cy) == '=' and T(cx + 1, cy) == '=' and cx > 24)
                plank = np.zeros((16, 16), np.uint8)
                for py in range(16):
                    for px in range(16):
                        k = py if not horiz else px
                        plank[py, px] = (8 if k % 4 else 6) if not horiz else (8 if px % 4 else 6)
                        if (px if not horiz else py) % 8 == 0 and (py + px) % 2 == 0: plank[py, px] = 7
                        if rnd(80, px, py) < 0.08: plank[py, px] = 9
                if horiz:
                    plank = np.zeros((16, 16), np.uint8)
                    for py in range(16):
                        for px in range(16):
                            plank[py, px] = 6 if px % 4 == 0 else 9 if py in (1, 14) else 8 if rnd(81, px // 4, py) < 0.8 else 7
                else:
                    plank = np.zeros((16, 16), np.uint8)
                    for py in range(16):
                        for px in range(16):
                            plank[py, px] = 6 if py % 4 == 0 else 9 if px in (1, 14) else 8 if rnd(82, px, py // 4) < 0.8 else 7
                # rails / posts at the sides
                if not horiz:
                    if T(cx - 1, cy) != '=': plank[:, 0] = 10; plank[::5, 0] = 6
                    if T(cx + 1, cy) != '=': plank[:, 15] = 10; plank[::5, 15] = 6
                    if T(cx, cy + 1) != '=': plank[15, :] = 10
                else:
                    if T(cx, cy - 1) != '=': plank[0, :] = 10
                    if T(cx, cy + 1) != '=': plank[15, :] = 10
                wt = plank
            else:
                behav[cy, cx] = 0x10
                elev[cy, cx] = 1
                if rnd(83, cx, cy) < 0.12 and nb_all_same:   # lily pad
                    disc_ = np.zeros((16, 16), np.uint8); disc(disc_, 8, 8, 5, 13, 0.7); disc(disc_, 7, 7, 3, 14, 0.7)
                    disc_[8, 8:14] = 0; disc_[6, 9] = 15; disc_[5, 9] = 15
                    wt = np.where(disc_ > 0, disc_, wt)
            base = wt
        bot[ys, xs] = base
        # land cells bordering water: water underneath, grass overlay with a bank on top
        if not iswater(c) and c not in 'WoX' and any(T(cx + dx, cy + dy) == '~' for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
            landm = ~region_mask(cx, cy, lambda t: t != '~', -1.0) if False else None
            # land region = everything that is not open water
            lm = region_mask(cx, cy, lambda t: t != '~', 1.0)
            under = WATER.copy()
            near = region_mask(cx, cy, lambda t: t != '~', -1.5)
            under = np.where(near & ~lm, 5, under)
            ov = np.where(lm, base, 0).astype(np.uint8)
            # earthy bank where land meets water below it
            bank = np.zeros_like(lm)
            for k in (1, 2, 3):
                bank[:-k, :] |= lm[:-k, :] & ~lm[k:, :]
            ov = np.where(bank & lm, 7, ov)
            edge = np.zeros_like(lm)
            edge[:-1, :] |= lm[:-1, :] & ~lm[1:, :]
            edge[1:, :] |= lm[1:, :] & ~lm[:-1, :]
            edge[:, :-1] |= lm[:, :-1] & ~lm[:, 1:]
            edge[:, 1:] |= lm[:, 1:] & ~lm[:, :-1]
            ov = np.where(edge, 1, ov)
            bot[ys, xs] = under; botpal[cy, cx] = 8
            top[ys, xs] = ov; toppal[cy, cx] = 6
            if c == 'F': raise SystemExit(f'forest on shore at {cx},{cy}')
        if c in SOLID: coll[cy, cx] = 1

# forest: 2x2 trees where a whole aligned block is forest, bushes elsewhere
def put_sprite(spr, pal, x0, y0, solid=True, normal_rows=()):
    h, w = spr.shape
    for j in range(h // 16):
        for i in range(w // 16):
            cx, cy = x0 + i, y0 + j
            if not (0 <= cx < W and 0 <= cy < H): continue
            piece = spr[j * 16:j * 16 + 16, i * 16:i * 16 + 16]
            if not piece.any(): continue
            if toppal[cy, cx] and toppal[cy, cx] != pal:
                raise SystemExit(f'top palette clash at {cx},{cy}')
            cur = top[cy * 16:cy * 16 + 16, cx * 16:cx * 16 + 16]
            top[cy * 16:cy * 16 + 16, cx * 16:cx * 16 + 16] = np.where(piece > 0, piece, cur)
            toppal[cy, cx] = pal
            if solid and (j, i) not in normal_rows_passable(normal_rows, j): coll[cy, cx] = 1
            if j in normal_rows: layer[cy, cx] = 0
def normal_rows_passable(rows, j): return set()

TREES = {k: tree(k) for k in ('red', 'gold', 'green', 'pine')}
for y in range(0, H, 2):
    for x in range(0, W, 2):
        if all(T(x + i, y + j) == 'F' for i in (0, 1) for j in (0, 1)):
            r = rnd(90, x, y)
            kind = 'green' if r < 0.5 else 'red' if r < 0.68 else 'gold' if r < 0.8 else 'pine'
            put_sprite(TREES[kind], 7, x, y, normal_rows=(0,))
            for i in (0, 1):
                for j in (0, 1): g[y + j][x + i] = 'f'
for y in range(H):
    for x in range(W):
        if g[y][x] == 'F':
            put_sprite(bush(), 7, x, y)
            g[y][x] = 'f'

SPR = {'tent': (tent(), 10, (0,)), 'campfire': (campfire(), 10, ()), 'log': (log_(), 7, ()), 'sign_dock': (signpost(), 10, ()),
       'sign_shrine': (signpost(), 10, ()), 'stone_carving': (rock(), 9, ()), 'pillar': (pillar(), 9, (0,)),
       'altar': (altar(), 9, ()), 'crystal': (crystal(), 9, ()), 'rock': (rock(), 9, ()), 'stump': (stump(), 7, ())}
for name, spots in OBJ.items():
    for (x, y) in spots:
        if name == 'mushrooms':
            put_sprite(mushrooms(), 7, x, y, solid=False)
        elif name.startswith('tree_'):
            put_sprite(TREES[name[5:]], 7, x, y, normal_rows=(0,))
        elif name == 'lantern':
            spr = lantern(False)
            put_sprite(spr[0:16], 10, x, y - 1, solid=False, normal_rows=(0,))
            put_sprite(spr[16:32], 10, x, y, solid=True)
        else:
            spr, pal, nrows = SPR[name]
            put_sprite(spr, pal, x, y, normal_rows=nrows)
            if name == 'pillar':      # the pillar's head row is walk-behind
                coll[y, x] = 0
# the shrine's walls and the top row of the stone yard
# ---------------------------------------------------------------- extra metatiles that scripts swap in
EXTRA = {}
lit = lantern(True)
for i, (x, y) in enumerate(OBJ['lantern']):
    EXTRA[f'lantern{i}_head_lit'] = (x, y - 1, lit[0:16], 10, 0, True)
    EXTRA[f'lantern{i}_base_lit'] = (x, y, lit[16:32], 10, 1, False)
EXTRA['shrine_open_l'] = (19, 9, None, 9, 1, 'floor')
EXTRA['shrine_open_r'] = (20, 9, None, 9, 1, 'floor')

# ---------------------------------------------------------------- slice into 8x8 tiles + metatiles
tiles = [np.zeros((8, 8), np.uint8)]  # secondary tile 0 = blank
tindex = {tiles[0].tobytes(): (0, 0, 0)}
def add_tile(t):
    for hf in (0, 1):
        for vf in (0, 1):
            k = t[:, ::-1] if hf else t
            k = k[::-1, :] if vf else k
            key = k.tobytes()
            if key in tindex:
                i, a, b = tindex[key]
                return i, a ^ hf, b ^ vf
    tindex[t.tobytes()] = (len(tiles), 0, 0)
    tiles.append(t.copy())
    return len(tiles) - 1, 0, 0

def entries(img, pal, empty_is_zero):
    out = []
    for qy in (0, 8):
        for qx in (0, 8):
            t = img[qy:qy + 8, qx:qx + 8]
            if empty_is_zero and not t.any():
                out.append(0)
                continue
            i, hf, vf = add_tile(t)
            out.append((512 + int(i)) | (int(hf) << 10) | (int(vf) << 11) | (int(pal) << 12))
    return out

metas, mindex, attrs = [], {}, []
def metatile(b, bp, t, tp, lay, beh):
    e = entries(b, bp, False) + entries(t, tp, True)
    key = (tuple(e), lay, beh)
    if key not in mindex:
        mindex[key] = len(metas)
        metas.append(e); attrs.append(int(beh) | (int(lay) << 12))
    return mindex[key]

blocks = np.zeros((H, W), np.uint16)
for cy in range(H):
    for cx in range(W):
        b = bot[cy * 16:cy * 16 + 16, cx * 16:cx * 16 + 16]
        t = top[cy * 16:cy * 16 + 16, cx * 16:cx * 16 + 16]
        mid = metatile(b, botpal[cy, cx], t, toppal[cy, cx] or 6, layer[cy, cx], behav[cy, cx])
        blocks[cy, cx] = (0x200 + mid) | (int(coll[cy, cx]) << 10) | (int(elev[cy, cx]) << 12)

info = {'extra': {}}
for name, (x, y, spr, pal, lay, extra) in EXTRA.items():
    b = bot[y * 16:y * 16 + 16, x * 16:x * 16 + 16]
    if extra == 'floor':
        mid = metatile(stone_floor(), 9, np.zeros((16, 16), np.uint8), 9, 1, 0)
    else:
        if extra is True:   # lantern head: draw the glow on the head cell's grass too
            pass
        mid = metatile(b if botpal[y, x] != 6 else glow(b), botpal[y, x], spr, pal, lay, 0)
    info['extra'][name] = 0x200 + mid
# the lit lantern's base cell also gets glowing grass
for i, (x, y) in enumerate(OBJ['lantern']):
    b = bot[y * 16:y * 16 + 16, x * 16:x * 16 + 16]
    info['extra'][f'lantern{i}_base_lit'] = 0x200 + metatile(glow(b), 6, lit[16:32], 10, 1, 0)

border = [int(blocks[0, 0]), int(blocks[0, 1]), int(blocks[1, 0]), int(blocks[1, 1])]
print('tiles', len(tiles), 'metatiles', len(metas))
assert len(tiles) <= 512 and len(metas) <= 512

# ---------------------------------------------------------------- write files
n = len(tiles)
rows = (n + 15) // 16
sheet = np.zeros((rows * 8, 128), np.uint8)
for i, t in enumerate(tiles):
    sheet[(i // 16) * 8:(i // 16) * 8 + 8, (i % 16) * 8:(i % 16) * 8 + 8] = t
im = Image.fromarray(sheet, 'P')
im.putpalette([c for rgb in PALS[6] for c in rgb])
im.save(f'{OUT}/tiles.png', bits=4)
open(f'{OUT}/metatiles.bin', 'wb').write(struct.pack(f'<{len(metas) * 8}H', *[v for m in metas for v in m]))
open(f'{OUT}/metatile_attributes.bin', 'wb').write(struct.pack(f'<{len(attrs)}H', *attrs))
open(f'{OUT}/map.bin', 'wb').write(struct.pack(f'<{W * H}H', *[int(v) for v in blocks.flatten()]))
open(f'{OUT}/border.bin', 'wb').write(struct.pack('<4H', *border))
info.update({'width': W, 'height': H, 'lanterns': OBJ['lantern']})
json.dump(info, open(f'{OUT}/info.json', 'w'), indent=1)

# preview straight from the metatile data, so it shows exactly what the game will draw
def rgb(pal, idx): return PALS.get(pal, PALS[6])[idx]
lut = {p: np.array(PALS[p], np.uint8) for p in PALS}
prev = np.zeros((H * 16, W * 16, 3), np.uint8)
def draw_entry(dst, e, ox, oy, opaque):
    if e == 0 and not opaque: return
    ti, hf, vf, pal = (e & 0x3FF) - 512, (e >> 10) & 1, (e >> 11) & 1, e >> 12
    t = tiles[ti] if ti >= 0 else np.zeros((8, 8), np.uint8)
    if hf: t = t[:, ::-1]
    if vf: t = t[::-1, :]
    col = lut[pal][t]
    m = (t > 0) | opaque
    sub = dst[oy:oy + 8, ox:ox + 8]
    sub[m] = col[m]
for cy in range(H):
    for cx in range(W):
        m = metas[(int(blocks[cy, cx]) & 0x3FF) - 0x200]
        for q in range(4):
            draw_entry(prev, m[q], cx * 16 + (q % 2) * 8, cy * 16 + (q // 2) * 8, True)
        for q in range(4):
            draw_entry(prev, m[4 + q], cx * 16 + (q % 2) * 8, cy * 16 + (q // 2) * 8, False)
Image.fromarray(prev).resize((W * 32, H * 32), Image.NEAREST).save(f'{OUT}/preview.png')
