"""Wonderwild Park's tileset: Petalburg's secondary tileset (General's grass,
trees and plaza stay) plus new pixel art in Emerald's style: a roller
coaster on stilts (straights and turns you ride along, its station deck and
the photo deck), the teacup ride's spinning floor, clipped hedges for the
maze, a castle with blue turrets, a carousel, ticket booths, a star statue,
flower beds and a popcorn cart.

    python3 tools/areas/park_art.py

Writes data/tilesets/secondary/park/ and tools/areas/park_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001            # General's grass shows under see-through pixels
MB_SLIDE = {'e': 0x44, 'w': 0x45, 'n': 0x46, 's': 0x47}
MB_SPIN = {'e': 0x54, 'w': 0x55, 'n': 0x56, 's': 0x57}
MB_STOP_SPINNING = 0x58
MB_SOUTH_ARROW_WARP = 0x65

# Palette 7: the rides.
PAL = [(115, 197, 164), (49, 41, 57), (131, 32, 41), (205, 57, 57), (255, 123, 106), (98, 98, 115),
       (164, 164, 180), (238, 238, 246), (205, 156, 32), (255, 213, 74), (230, 115, 172), (255, 189, 222),
       (106, 189, 246), (49, 115, 205), (139, 90, 49), (197, 148, 90)]
OUT, RED_D, RED, RED_L, STEEL_D, STEEL, WHITE, GOLD_D, GOLD, PINK, PINK_L, SKY, BLUE, WOOD_D, WOOD = range(1, 16)
# Palette 11: the castle and the gardens.
PAL_C = [(115, 197, 164), (41, 41, 65), (123, 123, 148), (180, 180, 197), (230, 230, 238), (41, 82, 172),
         (74, 131, 222), (139, 189, 255), (205, 156, 32), (255, 213, 74), (32, 98, 57), (57, 148, 82),
         (115, 197, 106), (230, 74, 98), (255, 238, 115), (148, 222, 255)]
C_OUT, STONE_D, STONE, STONE_L, ROOF_D, ROOF, ROOF_L, C_GOLD_D, C_GOLD, HEDGE_D, HEDGE, HEDGE_L, FLOWER, FLOWER_Y, GLASS = \
    range(1, 16)

D_VEC = {'e': (1, 0), 'w': (-1, 0), 'n': (0, -1), 's': (0, 1)}


# ---------------------------------------------------------------- the coaster
def stilts(c):
    """The coaster's steel stilts showing under the track."""
    for x in (3, 12):
        c.rect(x, 0, x, 15, STEEL)
        c.px(x, 15, STEEL_D)


def coaster(sides):
    """A piece of track: red rails along the open sides, white ties across."""
    c = Canvas(16, 16)
    stilts(c)
    horiz = sides & {'e', 'w'}
    vert = sides & {'n', 's'}
    if horiz and not vert:
        c.rect(0, 4, 15, 11, STEEL_D)
        for x in range(1, 16, 4):
            c.rect(x, 4, x + 1, 11, WHITE)
        c.rect(0, 4, 15, 5, RED)
        c.rect(0, 10, 15, 11, RED)
        c.rect(0, 4, 15, 4, RED_L)
    elif vert and not horiz:
        c.rect(4, 0, 11, 15, STEEL_D)
        for y in range(1, 16, 4):
            c.rect(4, y, 11, y + 1, WHITE)
        c.rect(4, 0, 5, 15, RED)
        c.rect(10, 0, 11, 15, RED)
        c.rect(4, 0, 4, 15, RED_L)
    else:                                            # a curve: a quarter ring
        (sx, sy), (tx, ty) = [D_VEC[s] for s in sorted(sides)]
        cx, cy = 7.5 + 8 * (sx + tx), 7.5 + 8 * (sy + ty)
        yy, xx = np.mgrid[0:16, 0:16]
        d = np.hypot(xx + .5 - cx - .5, yy + .5 - cy - .5)
        c.a[(d >= 4) & (d <= 12)] = STEEL_D
        for r in (4, 5, 11, 12):
            c.a[(d >= r - .5) & (d < r + .5)] = RED
        ang = np.arctan2(yy + .5 - cy - .5, xx + .5 - cx - .5)
        ties = (np.abs(np.sin(ang * 6)) < .25) & (d > 5.5) & (d < 10.5)
        c.a[ties] = WHITE
    c.outline(OUT)
    return c


def deck():
    """The raised wooden deck of the coaster's station and photo stop."""
    c = Canvas(16, 16)
    c.a[:] = WOOD
    for y in (0, 5, 10, 15):
        c.rect(0, y, 15, y, WOOD_D)
    c.px(4, 2, WOOD_D)
    c.px(11, 7, WOOD_D)
    c.px(6, 12, WOOD_D)
    return c


def deck_steps():
    c = deck()
    c.rect(1, 3, 14, 15, WOOD_D)
    for y in (4, 8, 12):
        c.rect(2, y, 13, y + 2, WOOD)
    return c


def coaster_car():
    """A coaster car waiting at the station."""
    c = Canvas(32, 16)
    c.rect(1, 3, 30, 12, RED)
    c.rect(1, 3, 30, 4, RED_L)
    c.rect(1, 11, 30, 12, RED_D)
    for x in (6, 18):
        c.rect(x, 1, x + 7, 6, OUT)                  # the seats' backs
        c.rect(x + 1, 2, x + 6, 6, GOLD)
    c.rect(0, 13, 31, 15, STEEL_D)
    c.outline(OUT)
    return c


def station_roof():
    """The coaster station's striped awning on posts, 4 by 2."""
    c = Canvas(64, 32)
    for x in range(0, 64, 8):
        c.rect(x, 0, x + 3, 11, RED)
        c.rect(x + 4, 0, x + 7, 11, WHITE)
    c.rect(0, 11, 63, 13, RED_D)
    for x in range(2, 64, 8):                        # scalloped edge
        c.disc(x + 2, 13, 2.5, RED if (x // 8) % 2 == 0 else WHITE)
    for x in (3, 58):
        c.rect(x, 14, x + 2, 31, STEEL)
    c.rect(20, 18, 43, 25, GOLD)                     # the ride's sign
    c.rect(20, 18, 43, 19, WHITE)
    for x in range(23, 41, 3):
        c.rect(x, 21, x + 1, 23, RED_D)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- the teacups
def teacup_floor(c):
    c.a[:] = PINK_L
    c.rect(0, 0, 15, 0, PINK)
    c.rect(0, 0, 0, 15, PINK)


def teacup(d):
    """A spinning teacup plate: you whirl off the way its arrow points."""
    c = Canvas(16, 16)
    teacup_floor(c)
    c.disc(8, 8, 6.5, SKY)
    c.disc(8, 8, 5, WHITE)
    dx, dy = D_VEC[d]
    for i in range(4):                               # the arrow
        x, y = 8 + dx * (i - 1), 8 + dy * (i - 1)
        c.disc(x, y, 1.2, BLUE)
    tip_x, tip_y = 8 + dx * 3, 8 + dy * 3
    for k in (-1, 1):
        c.px(tip_x - dx * 2 + dy * k * 2, tip_y - dy * 2 + dx * k * 2, BLUE)
        c.px(tip_x - dx + dy * k, tip_y - dy + dx * k, BLUE)
    return c


def teacup_stop():
    c = Canvas(16, 16)
    teacup_floor(c)
    c.disc(8, 8, 6, GOLD)
    c.disc(8, 8, 4.5, PINK)
    c.disc(8, 8, 2, WHITE)
    return c


def teacup_plain():
    c = Canvas(16, 16)
    teacup_floor(c)
    c.px(8, 8, PINK)
    return c


def teacup_rail():
    """The low railing round the ride."""
    c = Canvas(16, 16)
    c.rect(0, 6, 15, 8, GOLD)
    c.rect(0, 6, 15, 6, WHITE)
    for x in (2, 13):
        c.rect(x, 8, x + 1, 15, GOLD_D)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- gardens and landmarks
def hedge():
    c = Canvas(16, 16)
    c.a[:] = HEDGE
    c.rect(0, 0, 15, 3, HEDGE_L)
    c.rect(0, 13, 15, 15, HEDGE_D)
    r = np.random.RandomState(5)
    for _ in range(10):
        c.px(r.randint(0, 16), r.randint(4, 13), HEDGE_D)
    for _ in range(5):
        c.px(r.randint(0, 16), r.randint(0, 4), FLOWER_Y)
    return c


def flower_bed():
    c = Canvas(16, 16)
    c.rect(1, 6, 14, 15, HEDGE_D)
    c.rect(1, 6, 14, 7, HEDGE)
    for i, (x, y) in enumerate(((3, 4), (8, 3), (12, 5), (5, 9), (10, 9))):
        c.disc(x, y, 2, FLOWER if i % 2 else FLOWER_Y)
    c.outline(C_OUT)
    return c


def castle():
    """The park's castle: a gate between two blue-capped turrets, 6 by 5."""
    c = Canvas(96, 80)
    c.rect(14, 30, 81, 79, STONE)
    c.rect(14, 30, 81, 32, STONE_L)
    for x in range(14, 82, 8):                       # battlements
        c.rect(x, 24, x + 4, 30, STONE)
        c.rect(x, 24, x + 4, 25, STONE_L)
    for x0 in (0, 68):                               # turrets
        c.rect(x0 + 2, 26, x0 + 25, 79, STONE_L)
        c.rect(x0 + 22, 26, x0 + 25, 79, STONE)
        for y in range(0, 25):
            half = int(12 * y / 24) + 1
            c.rect(x0 + 14 - half, y, x0 + 13 + half, y, ROOF)
            c.rect(x0 + 14 - half, y, x0 + 14 - half // 2, y, ROOF_L)
            c.rect(x0 + 14 + half // 2, y, x0 + 13 + half, y, ROOF_D)
        c.rect(x0 + 13, 0, x0 + 14, 2, C_GOLD)
        c.rect(x0 + 9, 40, x0 + 18, 52, C_OUT)       # windows
        c.disc(x0 + 13.5, 40, 4.5, C_OUT)
        c.rect(x0 + 10, 41, x0 + 17, 52, GLASS)
    c.rect(34, 6, 61, 30, STONE_L)                   # the keep
    for y in range(0, 10):
        half = int(16 * y / 9) + 1
        c.rect(48 - half, y - 4 + 4, 47 + half, y - 4 + 4, ROOF)
    c.rect(44, 14, 51, 24, C_OUT)
    c.disc(47.5, 14, 4, C_OUT)
    c.rect(45, 15, 50, 24, C_GOLD)                   # the clock
    c.disc(47.5, 18, 2.5, STONE_L)
    c.rect(36, 50, 59, 79, C_OUT)                    # the gate
    c.disc(47.5, 50, 12, C_OUT)
    c.rect(38, 52, 57, 79, ROOF_D)
    c.disc(47.5, 52, 10, ROOF_D)
    for x in range(40, 58, 5):
        c.rect(x, 44, x + 1, 79, C_GOLD_D)
    c.outline(C_OUT)
    return c


def carousel():
    """A carousel: striped canopy, a gold pole and horses, 3 by 3."""
    c = Canvas(48, 48)
    for i in range(6):
        x0 = 4 + i * 7
        c.rect(x0, 6, x0 + 6, 16, RED if i % 2 == 0 else WHITE)
    for y in range(0, 7):
        half = int(20 * y / 6) + 3
        c.rect(24 - half, y, 23 + half, y, GOLD)
    c.rect(22, 0, 25, 2, RED)
    c.rect(3, 16, 44, 18, GOLD_D)
    c.rect(22, 18, 25, 40, GOLD)
    for x, h in ((8, 26), (36, 30), (15, 32)):
        c.rect(x + 2, 18, x + 2, h, GOLD)
        c.rect(x - 1, h, x + 6, h + 5, WHITE)         # a horse
        c.rect(x + 5, h - 3, x + 7, h + 1, WHITE)
        c.rect(x, h + 6, x, h + 8, WHITE)
        c.rect(x + 5, h + 6, x + 5, h + 8, WHITE)
    c.rect(2, 40, 45, 47, PINK)
    c.rect(2, 40, 45, 41, PINK_L)
    c.outline(OUT)
    return c


def booth():
    """A ticket booth, 2 by 2."""
    c = Canvas(32, 32)
    c.rect(2, 10, 29, 31, WHITE)
    c.rect(6, 14, 25, 22, SKY)
    c.rect(6, 14, 25, 15, WHITE)
    c.rect(2, 23, 29, 25, RED)
    for x in range(0, 32, 8):
        c.rect(x, 2, x + 3, 9, RED)
        c.rect(x + 4, 2, x + 7, 9, GOLD)
    c.rect(0, 9, 31, 10, RED_D)
    c.outline(OUT)
    return c


def star_statue():
    c = Canvas(16, 32)
    c.rect(2, 22, 13, 31, STEEL)
    c.rect(2, 22, 13, 23, WHITE)
    c.rect(6, 15, 9, 22, GOLD_D)
    pts = []
    for k in range(10):
        r = 7 if k % 2 == 0 else 3
        a = -np.pi / 2 + k * np.pi / 5
        pts.append((8 + r * np.cos(a), 9 + r * np.sin(a)))
    yy, xx = np.mgrid[0:32, 0:16]
    inside = np.zeros((32, 16), bool)
    for i in range(10):                              # fan of triangles from the centre
        (x1, y1), (x2, y2) = pts[i], pts[(i + 1) % 10]
        d1 = (xx + .5 - 8) * (y1 - 9) - (yy + .5 - 9) * (x1 - 8)
        d2 = (xx + .5 - x1) * (y2 - y1) - (yy + .5 - y1) * (x2 - x1)
        d3 = (xx + .5 - x2) * (9 - y2) - (yy + .5 - y2) * (8 - x2)
        inside |= ((d1 <= 0) & (d2 <= 0) & (d3 <= 0)) | ((d1 >= 0) & (d2 >= 0) & (d3 >= 0))
    c.a[inside] = GOLD
    c.a[inside & (xx < 8) & (yy < 9)] = WHITE
    c.outline(OUT)
    return c


def popcorn():
    c = Canvas(16, 32)
    c.rect(2, 12, 13, 27, RED)
    c.rect(2, 12, 13, 13, RED_L)
    c.rect(4, 4, 11, 12, SKY)
    for x, y in ((5, 8), (7, 6), (9, 9), (10, 7), (6, 10)):
        c.px(x, y, WHITE)
    c.rect(1, 2, 14, 4, RED)
    for x in (4, 11):
        c.disc(x, 29, 2, STEEL_D)
    c.outline(OUT)
    return c


WALK = {'walk': lambda r, c: True}
TURNS = {(a, b) for a in 'nsew' for b in 'nsew' if a != b and {a, b} not in ({'n', 's'}, {'e', 'w'})}
OPP = {'n': 's', 's': 'n', 'e': 'w', 'w': 'e'}
OBJECTS = {
    # coaster_<out>: a straight you slide along; coaster_<in><out>: a curve
    **{f'coaster_{d}': (lambda d=d: coaster({d, OPP[d]}), 7, {'behavior': lambda r, c, d=d: MB_SLIDE[d]}) for d in 'nsew'},
    **{f'coaster_{a}{b}': (lambda a=a, b=b: coaster({OPP[a], b}), 7, {'behavior': lambda r, c, b=b: MB_SLIDE[b]})
       for a, b in sorted(TURNS)},
    'deck': (deck, 7, WALK),
    'deck_steps': (deck_steps, 7, {'behavior': lambda r, c: MB_SOUTH_ARROW_WARP}),
    'coaster_car': (coaster_car, 7, {}),
    'station_roof': (station_roof, 7, {'over': lambda r, c: True, 'walk': lambda r, c: r == 1 and c in (1, 2)}),
    **{f'teacup_{d}': (lambda d=d: teacup(d), 7, {'behavior': lambda r, c, d=d: MB_SPIN[d]}) for d in 'nsew'},
    'teacup_stop': (teacup_stop, 7, {'behavior': lambda r, c: MB_STOP_SPINNING}),
    'teacup_plain': (teacup_plain, 7, WALK),
    'teacup_rail': (teacup_rail, 7, {}),
    'hedge': (hedge, 11, {}),
    'flower_bed': (flower_bed, 11, {}),
    'castle': (castle, 11, {'over': lambda r, c: r < 2}),
    'carousel': (carousel, 7, {'over': lambda r, c: r == 0}),
    'booth': (booth, 7, {'over': lambda r, c: r == 0}),
    'star_statue': (star_statue, 7, {'over': lambda r, c: r == 0}),
    'popcorn': (popcorn, 7, {'over': lambda r, c: r == 0}),
}


def main():
    pack('data/tilesets/secondary/petalburg', 'data/tilesets/secondary/park', OBJECTS, {7: PAL, 11: PAL_C},
         GROUND, f'{ROOT}/tools/areas/park_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
