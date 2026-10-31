"""The Nimbus Isles' tileset: Petalburg's secondary tileset (General's grass
and trees stay for the islands) plus new pixel art in Emerald's style: open
sky, clouds you can walk on (an edge set that joins up in any shape), wind
streams that carry you along, rainbow bridges, marble houses with gold
trim, a chime tower, columns, a cloud fountain and a hot-air balloon.

    python3 tools/areas/sky_art.py

Writes data/tilesets/secondary/sky/ and tools/areas/sky_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001
MB_WALK = {'e': 0x40, 'w': 0x41, 'n': 0x42, 's': 0x43}
MB_EAST_ARROW_WARP, MB_WEST_ARROW_WARP = 0x62, 0x63

# Palette 7: sky, clouds, wind and the rainbow.
PAL_SKY = [(115, 197, 164), (82, 115, 180), (99, 148, 222), (123, 180, 246), (164, 205, 255), (180, 197, 230),
           (222, 230, 246), (255, 255, 255), (148, 222, 238), (82, 172, 205), (238, 90, 90), (246, 164, 74),
           (255, 230, 98), (106, 213, 106), (90, 148, 238), (172, 106, 222)]
S_OUT, SKY_D, SKY, SKY_L, CLOUD_S, CLOUD_M, CLOUD, WIND, WIND_D, R_RED, R_ORANGE, R_YEL, R_GRN, R_BLUE, R_VIOLET = \
    range(1, 16)
# Palette 11: marble, gold and roofs.
PAL_MARBLE = [(115, 197, 164), (57, 57, 82), (164, 172, 189), (213, 218, 230), (255, 255, 255), (172, 123, 24),
              (238, 189, 57), (255, 238, 139), (57, 98, 180), (90, 148, 230), (148, 197, 255), (123, 205, 246),
              (205, 238, 255), (98, 74, 57), (156, 222, 238), (222, 74, 74)]
M_OUT, MARBLE_D, MARBLE, MARBLE_L, GOLD_D, GOLD, GOLD_L, ROOF_D, ROOF, ROOF_L, WATER, WATER_L, WOOD, GLASS, BALLOON = \
    range(1, 16)


# ---------------------------------------------------------------- sky and clouds
def sky(wisp=False):
    c = Canvas(16, 16)
    c.a[:] = SKY
    if wisp:
        c.rect(3, 5, 9, 5, SKY_L)
        c.rect(5, 4, 7, 4, SKY_L)
        c.rect(10, 11, 13, 11, SKY_L)
    else:
        c.px(12, 3, SKY_L)
        c.px(4, 12, SKY_L)
    return c


def cloud_floor(c, seed=0):
    """The walkable top of a cloud: white with soft puffs."""
    c.a[:] = CLOUD
    r = np.random.RandomState(seed)
    x, y = r.randint(4, 12), r.randint(4, 12)
    c.disc(x, y, 2.5, CLOUD_M)
    c.disc(x - 1, y - 1, 2.5, CLOUD)                 # leaves a soft crescent of shade


def cloud(edges, inner=None, seed=0):
    """A cloud tile. edges: the sides that face open sky ('n', 's', 'e',
    'w'); inner: a corner ('ne', 'nw', 'se', 'sw') where only the diagonal
    is sky."""
    c = Canvas(16, 16)
    cloud_floor(c, seed)
    yy, xx = np.mgrid[0:16, 0:16] + 0.5
    bump = lambda t: 4 + 1.5 * np.sin(2 * np.pi * t / 16) + 0.8 * np.sin(2 * np.pi * t / 8 + 1)   # repeats every tile
    skymask = np.zeros((16, 16), bool)
    if 'n' in edges:
        skymask |= yy < bump(xx)
    if 's' in edges:
        skymask |= (16 - yy) < bump(xx + 4)
    if 'w' in edges:
        skymask |= xx < bump(yy + 1)
    if 'e' in edges:
        skymask |= (16 - xx) < bump(yy + 8)
    for corner in ([inner] if inner else []):
        cx = 16 if 'e' in corner else 0
        cy = 16 if 's' in corner else 0
        skymask |= np.hypot(xx - cx, yy - cy) < 4.5
    # round the outer corners
    for a, b in (('n', 'w'), ('n', 'e'), ('s', 'w'), ('s', 'e')):
        if a in edges and b in edges:
            cx = 16 if b == 'e' else 0
            cy = 16 if a == 's' else 0
            skymask |= np.hypot(xx - cx, yy - cy) < 8
    c.a[skymask] = SKY
    # shading: a grey-blue rim under the bottom edge, a soft rim elsewhere
    def shift(m, dy, dx):                            # like np.roll, without wrapping round
        out = np.zeros_like(m)
        out[max(dy, 0):16 + min(dy, 0), max(dx, 0):16 + min(dx, 0)] = \
            m[max(-dy, 0):16 + min(-dy, 0), max(-dx, 0):16 + min(-dx, 0)]
        return out
    edge = np.zeros_like(skymask)
    for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        edge |= shift(skymask, dy, dx)
    c.a[edge & ~skymask] = CLOUD_M
    under = np.zeros_like(skymask)                   # the shaded underside of the cloud
    for dy in (1, 2, 3):
        under |= shift(skymask, -dy, 0)
    c.a[under & ~skymask & (yy > 8)] = CLOUD_S
    return c


def wind(d):
    """A wind stream over the clouds: streaks and an arrow along it."""
    c = Canvas(16, 16)
    cloud_floor(c, 7)
    for y, x0, x1 in ((3, 1, 9), (8, 5, 14), (12, 0, 6)):
        c.rect(x0, y, x1, y, WIND)
        c.rect(x0, y + 1, x1 - 2, y + 1, WIND_D)
    c.line(9, 6, 12, 8, WIND_D, 1)                   # a chevron pointing east
    c.line(12, 8, 9, 10, WIND_D, 1)
    if d == 'w':
        c.a = c.a[:, ::-1].copy()
    elif d == 'n':
        c.a = np.rot90(c.a, 1).copy()
    elif d == 's':
        c.a = np.rot90(c.a, -1).copy()
    return c


def rainbow(vertical=False):
    c = Canvas(16, 16)
    c.a[:] = SKY
    bands = (R_RED, R_ORANGE, R_YEL, R_GRN, R_BLUE, R_VIOLET)
    for i, col in enumerate(bands):
        c.rect(0, 2 + i * 2, 15, 3 + i * 2, col)
    c.rect(0, 1, 15, 1, CLOUD)
    c.rect(0, 14, 15, 14, CLOUD_S)
    if vertical:
        c.a = c.a.T.copy()
    return c


# ---------------------------------------------------------------- buildings
def marble_house():
    """A small marble house with a blue dome; its door at block (3, 2)."""
    c = Canvas(80, 64)
    c.rect(6, 28, 73, 61, MARBLE)
    c.rect(6, 28, 73, 29, MARBLE_L)
    c.rect(4, 58, 75, 63, MARBLE_D)
    c.rect(4, 58, 75, 58, MARBLE_L)
    for x in (8, 22, 54, 68):                       # pilasters
        c.rect(x, 30, x + 3, 57, MARBLE_L)
        c.rect(x + 3, 30, x + 3, 57, MARBLE_D)
    for x0 in (13, 59):                             # arched windows
        c.rect(x0, 38, x0 + 7, 50, GLASS)
        c.disc(x0 + 3.5, 38, 4, GLASS)
        c.rect(x0 + 3, 34, x0 + 4, 50, MARBLE_L)
    c.rect(32, 42, 47, 61, M_OUT)                   # door
    c.disc(40, 42, 8, M_OUT)
    c.rect(33, 43, 46, 61, WOOD)
    c.disc(40, 43, 7, WOOD)
    c.rect(39, 37, 40, 61, M_OUT)
    c.rect(2, 22, 77, 28, GOLD)                     # cornice
    c.rect(2, 22, 77, 23, GOLD_L)
    c.rect(2, 28, 77, 28, GOLD_D)
    yy, xx = np.mgrid[0:64, 0:80]
    dome = np.hypot((xx + 0.5 - 40) / 26, (yy + 0.5 - 22) / 20) < 1
    dome &= yy < 22
    c.a[dome] = ROOF
    c.a[dome & (xx < 34) & (yy > 8)] = ROOF_L
    c.a[dome & (xx > 52)] = ROOF_D
    c.rect(38, 0, 41, 3, GOLD)
    c.outline(M_OUT)
    return c


def chime_tower(ringing=False):
    """A slender tower; wind chimes hang in its open belfry."""
    c = Canvas(48, 96)
    c.rect(8, 40, 39, 95, MARBLE)
    c.rect(8, 40, 10, 95, MARBLE_L)
    c.rect(37, 40, 39, 95, MARBLE_D)
    c.rect(4, 88, 43, 95, MARBLE_D)
    c.rect(4, 88, 43, 88, MARBLE_L)
    c.rect(16, 74, 31, 95, M_OUT)                   # door
    c.disc(24, 74, 8, M_OUT)
    c.rect(17, 75, 30, 95, WOOD)
    c.disc(24, 75, 7, WOOD)
    c.rect(23, 70, 24, 95, M_OUT)
    c.rect(6, 36, 41, 40, GOLD)
    c.rect(10, 16, 37, 36, M_OUT)                   # the belfry
    c.rect(12, 16, 13, 36, MARBLE)
    c.rect(34, 16, 35, 36, MARBLE)
    c.rect(22, 16, 25, 36, MARBLE)
    for i, x in enumerate((16, 19, 28, 31)):
        h = 26 + (i % 2) * 4
        c.rect(x, 17, x, h, GOLD_L if ringing else GOLD_D)
        c.rect(x - 1, h, x + 1, h + 2, GOLD if ringing else GOLD_D)
    for y in range(4, 17):                          # the spire
        half = (y - 2) * 1.3
        c.rect(int(24 - half), y, int(23 + half), y, ROOF if y > 9 else ROOF_D)
    c.rect(23, 0, 24, 4, GOLD)
    if ringing:
        for x, y in ((3, 22), (44, 26), (5, 30), (42, 18)):
            c.px(x, y, GOLD_L)
            c.px(x + 1, y - 1, GOLD_L)
    c.outline(M_OUT)
    return c


def on_cloud(draw):
    """Objects that stand on a cloud: white where they are see-through."""
    def f():
        c = draw()
        c.a[c.a == 0] = MARBLE_L
        return c
    return f


def column():
    c = Canvas(16, 32)
    c.rect(4, 6, 11, 28, MARBLE)
    c.rect(4, 6, 5, 28, MARBLE_L)
    c.rect(10, 6, 11, 28, MARBLE_D)
    for x in (7, 8):
        c.rect(x, 7, x, 27, MARBLE_D)
    c.rect(2, 2, 13, 6, MARBLE_L)
    c.rect(2, 5, 13, 6, GOLD)
    c.rect(2, 27, 13, 31, MARBLE_D)
    c.rect(2, 27, 13, 27, MARBLE_L)
    c.outline(M_OUT)
    return c


def fountain():
    c = Canvas(48, 48)
    yy, xx = np.mgrid[0:48, 0:48]
    rim = np.hypot((xx + 0.5 - 24) / 22, (yy + 0.5 - 30) / 15) < 1
    c.a[rim] = MARBLE
    pool = np.hypot((xx + 0.5 - 24) / 19, (yy + 0.5 - 30) / 12) < 1
    c.a[pool] = WATER
    c.a[pool & ((xx + yy) % 7 == 0)] = WATER_L
    c.rect(21, 10, 26, 30, MARBLE)
    c.rect(21, 10, 22, 30, MARBLE_L)
    c.disc(24, 10, 7, MARBLE)
    c.disc(24, 9, 5, WATER)
    for x0, d in ((16, -1), (32, 1)):               # arcs of spray
        for t in range(10):
            c.px(24 + d * (2 + t * 0.9), 6 + (t - 4) ** 2 * 0.4, WATER_L)
    c.outline(M_OUT)
    return c


def balloon():
    c = Canvas(32, 48)
    yy, xx = np.mgrid[0:48, 0:32]
    env = np.hypot((xx + 0.5 - 16) / 15, (yy + 0.5 - 15) / 15) < 1
    env |= (yy >= 15) & (yy < 26) & (np.abs(xx + 0.5 - 16) < 15 - (yy - 15) * 0.9)
    c.a[env] = BALLOON
    c.a[env & ((xx // 5) % 2 == 0)] = GOLD
    c.a[env & (xx < 9) & (yy < 14)] = GOLD_L
    for x0, x1 in ((7, 11), (25, 21)):              # ropes
        c.line(x0, 25, x1, 36, WOOD)
    c.rect(10, 36, 21, 45, WOOD)
    c.rect(10, 36, 21, 37, GOLD_D)
    for x in (13, 17):
        c.rect(x, 38, x, 45, M_OUT)
    c.outline(M_OUT)
    return c


def shrine():
    """The open-air shrine on the summit: a ring of columns under a dome."""
    c = Canvas(80, 64)
    c.rect(2, 54, 77, 63, MARBLE_D)
    c.rect(2, 54, 77, 55, MARBLE_L)
    c.rect(8, 50, 71, 54, MARBLE)
    for x in (8, 24, 50, 66):
        c.rect(x, 22, x + 5, 50, MARBLE)
        c.rect(x, 22, x + 1, 50, MARBLE_L)
        c.rect(x + 4, 22, x + 5, 50, MARBLE_D)
    c.rect(4, 16, 75, 22, GOLD)
    c.rect(4, 16, 75, 17, GOLD_L)
    yy, xx = np.mgrid[0:64, 0:80]
    dome = (np.hypot((xx + 0.5 - 40) / 30, (yy + 0.5 - 16) / 15) < 1) & (yy < 16)
    c.a[dome] = MARBLE
    c.a[dome & (xx < 32)] = MARBLE_L
    c.a[dome & (xx > 56)] = MARBLE_D
    c.rect(38, 0, 41, 2, GOLD)
    c.outline(M_OUT)
    return c


CLOUD_EDGES = {
    'cloud': ('', None), 'cloud_n': ('n', None), 'cloud_s': ('s', None), 'cloud_e': ('e', None),
    'cloud_w': ('w', None), 'cloud_nw': ('nw', None), 'cloud_ne': ('ne', None), 'cloud_sw': ('sw', None),
    'cloud_se': ('se', None), 'cloud_in_nw': ('', 'nw'), 'cloud_in_ne': ('', 'ne'), 'cloud_in_sw': ('', 'sw'),
    'cloud_in_se': ('', 'se'), 'cloud_ns': ('ns', None), 'cloud_ew': ('ew', None),
}
WALK = {'walk': lambda r, c: True}
OBJECTS = {
    'sky': (sky, 7, {}),
    'sky_wisp': (lambda: sky(True), 7, {}),
    **{name: (lambda e=e, i=i, k=k: cloud(e, i, seed=k), 7, WALK) for k, (name, (e, i)) in enumerate(CLOUD_EDGES.items())},
    'cloud_b': (lambda: cloud('', None, seed=40), 7, WALK),
    **{f'wind_{d}': (lambda d=d: wind(d), 7, {'walk': lambda r, c: True, 'behavior': lambda r, c, d=d: MB_WALK[d]})
       for d in 'nsew'},
    'rainbow_h': (rainbow, 7, WALK),
    'rainbow_v': (lambda: rainbow(True), 7, WALK),
    # the far ends of the bridge between the city and the isle (arrow warps)
    'rainbow_exit_e': (rainbow, 7, {'behavior': lambda r, c: MB_EAST_ARROW_WARP}),
    'rainbow_exit_w': (rainbow, 7, {'behavior': lambda r, c: MB_WEST_ARROW_WARP}),
    'marble_house': (on_cloud(marble_house), 11, {'door': lambda r, c: (r, c) == (3, 2)}),
    'chime_tower': (on_cloud(chime_tower), 11, {'over': lambda r, c: r < 2}),
    'chime_tower_ringing': (on_cloud(lambda: chime_tower(True)), 11, {'over': lambda r, c: r < 2}),
    'column': (on_cloud(column), 11, {'over': lambda r, c: r == 0}),
    'fountain': (on_cloud(fountain), 11, {}),
    'balloon': (on_cloud(balloon), 11, {'over': lambda r, c: r == 0}),
    'shrine': (shrine, 11, {'walk': lambda r, c: r >= 3 and 1 <= c <= 3, 'over': lambda r, c: r < 2}),
}


def main():
    pack('data/tilesets/secondary/petalburg', 'data/tilesets/secondary/sky', OBJECTS, {7: PAL_SKY, 11: PAL_MARBLE},
         GROUND, f'{ROOT}/tools/areas/sky_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
