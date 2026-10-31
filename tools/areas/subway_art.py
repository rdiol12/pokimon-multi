"""The Hoenn Metro's tileset: the New Mauville tiles (gTileset_BikeShop) plus
new pixel art in Emerald's style: a track bed with rails, the platform edge
with its yellow safety line, silver train cars with a green stripe (and an
old rusted one for the abandoned line), turnstiles, ticket machines,
benches, station boards, stamp stands for the stamp rally, stairs up to the
street, the old line's shutter and the tunnel mouth behind it.

    python3 tools/areas/subway_art.py

Writes data/tilesets/secondary/subway/ and tools/areas/subway_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

FLOOR = 0x210             # New Mauville's floor shows under see-through pixels
MB_NON_ANIMATED_DOOR = 0x60
MB_EAST_ARROW_WARP, MB_WEST_ARROW_WARP = 0x62, 0x63

PAL = [(115, 197, 164), (41, 41, 57), (82, 90, 106), (139, 148, 164), (197, 205, 213), (246, 246, 255),
       (205, 164, 41), (255, 222, 82), (41, 106, 205), (106, 172, 255), (156, 82, 41), (205, 123, 74),
       (49, 156, 74), (123, 222, 106), (238, 131, 41), (172, 230, 255)]
OUT, STEEL_D, STEEL, STEEL_L, WHITE, YEL_D, YEL, BLUE_D, BLUE, RUST_D, RUST, GRN_D, GRN, ORANGE, GLASS = range(1, 16)


# ---------------------------------------------------------------- the track
def track(rails=True):
    """The track bed: dark gravel, sleepers and two rails running east-west."""
    c = Canvas(16, 16)
    c.a[:] = STEEL_D
    r = np.random.RandomState(3)
    for _ in range(14):
        c.px(r.randint(0, 16), r.randint(0, 16), STEEL)
    for x in (2, 10):
        c.rect(x, 1, x + 3, 14, RUST_D)
        c.rect(x, 1, x + 3, 1, RUST)
    if rails:
        for y in (4, 11):
            c.rect(0, y, 15, y + 1, STEEL_L)
            c.rect(0, y + 1, 15, y + 1, STEEL)
    return c


def track_wall():
    """The back wall of the track bed, below the platform on the far side."""
    c = Canvas(16, 16)
    c.a[:] = STEEL
    c.rect(0, 0, 15, 2, STEEL_L)
    c.rect(0, 13, 15, 15, STEEL_D)
    for x in (3, 11):
        c.rect(x, 5, x + 1, 9, STEEL_D)
    return c


def edge():
    """The platform's edge: floor tiles, the yellow safety line, the lip."""
    c = Canvas(16, 16)
    c.rect(0, 0, 15, 9, WHITE)
    c.rect(0, 0, 15, 0, STEEL_L)
    c.rect(7, 0, 7, 9, STEEL_L)
    c.rect(0, 6, 15, 8, YEL)
    for x in range(1, 16, 4):
        c.px(x, 7, YEL_D)
    c.rect(0, 10, 15, 12, STEEL_L)
    c.rect(0, 13, 15, 15, STEEL_D)
    return c


def train(old=False):
    """Two coupled cars, 10 blocks long and 3 tall, standing on the track:
    roof, windows and doors, the stripe, the undercarriage."""
    body, body_l, body_d = (RUST, ORANGE, RUST_D) if old else (STEEL_L, WHITE, STEEL)
    stripe, stripe_d = (STEEL_D, OUT) if old else (GRN, GRN_D)
    glass = STEEL_D if old else GLASS
    c = Canvas(160, 48)
    for x0, x1 in ((0, 78), (81, 159)):
        c.rect(x0, 2, x1, 43, body)
        c.rect(x0, 2, x1, 5, body_l)                  # the roof
        c.rect(x0, 6, x1, 6, body_d)
        c.rect(x0, 34, x1, 37, stripe)
        c.rect(x0, 37, x1, 37, stripe_d)
        c.rect(x0, 40, x1, 43, body_d)
        for wx in range(x0 + 6, x1 - 8, 12):           # windows, doors between
            if (wx - x0) // 12 in (2, 4):
                c.rect(wx - 1, 9, wx + 10, 39, OUT)   # a door, two leaves
                c.rect(wx, 10, wx + 4, 39, body_d)
                c.rect(wx + 5, 10, wx + 9, 39, body_d)
                c.rect(wx + 1, 13, wx + 3, 22, glass)
                c.rect(wx + 6, 13, wx + 8, 22, glass)
            else:
                c.rect(wx, 11, wx + 8, 23, OUT)
                c.rect(wx + 1, 12, wx + 7, 22, glass)
                if not old:
                    c.rect(wx + 1, 12, wx + 3, 14, WHITE)
        c.rect(x0 + 3, 44, x0 + 11, 47, OUT)          # bogies
        c.rect(x1 - 11, 44, x1 - 3, 47, OUT)
    c.rect(79, 24, 80, 30, OUT)                       # the coupling
    if old:
        r = np.random.RandomState(7)
        for _ in range(60):
            c.px(r.randint(0, 160), r.randint(6, 40), RUST_D)
    c.outline(OUT)
    return c


# ---------------------------------------------------------------- station furniture
def turnstile():
    c = Canvas(16, 16)
    c.rect(2, 3, 13, 13, STEEL)
    c.rect(2, 3, 13, 5, STEEL_L)
    c.rect(4, 7, 11, 8, BLUE_D)
    c.px(12, 4, GRN)
    c.line(13, 9, 15, 9, STEEL_D)
    c.outline(OUT)
    return c


def ticket_machine():
    c = Canvas(16, 32)
    c.rect(1, 3, 14, 31, BLUE)
    c.rect(1, 3, 14, 5, BLUE_D)
    c.rect(3, 8, 12, 15, GLASS)
    c.rect(3, 8, 12, 9, WHITE)
    for y in (18, 21):
        for x in (3, 7, 11):
            c.rect(x, y, x + 1, y + 1, WHITE)
    c.rect(4, 25, 11, 26, STEEL_D)                   # the ticket slot
    c.outline(OUT)
    return c


def bench():
    c = Canvas(32, 16)
    c.rect(1, 6, 30, 9, ORANGE)
    c.rect(1, 6, 30, 6, YEL)
    c.rect(1, 2, 30, 4, ORANGE)
    for x in (3, 27):
        c.rect(x, 10, x + 1, 15, STEEL_D)
    c.outline(OUT)
    return c


def stamp_stand():
    """A stamp rally stand: a little roofed desk with the stamp and its pad."""
    c = Canvas(16, 32)
    c.rect(1, 2, 14, 6, RUST)
    c.rect(1, 2, 14, 3, ORANGE)
    c.rect(3, 7, 4, 16, RUST_D)
    c.rect(11, 7, 12, 16, RUST_D)
    c.rect(1, 16, 14, 30, WHITE)
    c.rect(1, 16, 14, 17, STEEL_L)
    c.disc(5, 21, 2.5, GRN)                          # the stamp
    c.rect(4, 17, 6, 19, RUST_D)
    c.rect(8, 21, 13, 24, BLUE_D)                    # the ink pad
    c.outline(OUT)
    return c


def station_board(colour):
    """The station's name board on two posts, in the line's colour."""
    c = Canvas(32, 32)
    c.rect(4, 18, 5, 31, STEEL_D)
    c.rect(26, 18, 27, 31, STEEL_D)
    c.rect(0, 2, 31, 19, WHITE)
    c.rect(0, 2, 31, 6, colour)
    for x in range(4, 28, 3):                        # the name, too small to read
        c.rect(x, 10, x + 1, 14, OUT)
    c.rect(2, 16, 29, 16, STEEL_L)
    c.outline(OUT)
    return c


def stairs():
    """Stairs up to the street, set into the wall."""
    c = Canvas(32, 32)
    c.a[:] = STEEL_D
    for i, y in enumerate(range(2, 32, 5)):
        c.rect(2, y, 29, y + 2, STEEL_L if i % 2 else WHITE)
        c.rect(2, y + 3, 29, y + 4, STEEL)
    c.rect(0, 0, 1, 31, YEL)
    c.rect(30, 0, 31, 31, YEL)
    c.outline(OUT)
    return c


def shutter():
    """The old line's rolling shutter, padlocked."""
    c = Canvas(32, 32)
    c.rect(0, 0, 31, 31, STEEL)
    for y in range(2, 30, 3):
        c.rect(1, y, 30, y, STEEL_D)
    c.rect(0, 0, 31, 2, STEEL_D)
    c.rect(13, 22, 18, 27, YEL)
    c.rect(14, 19, 17, 22, YEL_D, )
    c.rect(15, 20, 16, 21, STEEL)
    c.rect(4, 8, 27, 12, YEL)                         # KEEP OUT stripes
    for x in range(5, 27, 4):
        c.rect(x, 8, x + 1, 12, OUT)
    c.outline(OUT)
    return c


def tunnel():
    """The dark mouth of the old line's tunnel."""
    c = Canvas(16, 16)
    c.a[:] = OUT
    c.rect(0, 0, 15, 1, STEEL_D)
    return c


def pillar():
    c = Canvas(16, 32)
    c.rect(3, 0, 12, 31, WHITE)
    c.rect(3, 0, 4, 31, STEEL_L)
    c.rect(11, 0, 12, 31, STEEL)
    c.rect(1, 0, 14, 2, STEEL_L)
    c.rect(1, 28, 14, 31, STEEL)
    c.rect(3, 12, 12, 15, GRN)                         # the line's colour band
    c.outline(OUT)
    return c


OBJECTS = {
    'track': (track, 12, {}),
    'track_bare': (lambda: track(False), 12, {}),
    'track_wall': (track_wall, 12, {}),
    'edge': (edge, 12, {'walk': lambda r, c: True}),
    'train': (train, 12, {}),
    'train_old': (lambda: train(True), 12, {}),
    'turnstile': (turnstile, 12, {}),
    'ticket_machine': (ticket_machine, 12, {'over': lambda r, c: r == 0}),
    'bench': (bench, 12, {}),
    'stamp_stand': (stamp_stand, 12, {'over': lambda r, c: r == 0}),
    'board_green': (lambda: station_board(GRN), 12, {'over': lambda r, c: r == 0}),
    'board_blue': (lambda: station_board(BLUE), 12, {'over': lambda r, c: r == 0}),
    'board_orange': (lambda: station_board(ORANGE), 12, {'over': lambda r, c: r == 0}),
    'board_rust': (lambda: station_board(RUST_D), 12, {'over': lambda r, c: r == 0}),
    'stairs': (stairs, 12, {'door': lambda r, c: r == 1}),
    'shutter': (shutter, 12, {}),
    'tunnel_e': (tunnel, 12, {'behavior': lambda r, c: MB_EAST_ARROW_WARP}),
    'tunnel_w': (tunnel, 12, {'behavior': lambda r, c: MB_WEST_ARROW_WARP}),
    'pillar': (pillar, 12, {'over': lambda r, c: r == 0}),
}


def main():
    pack('data/tilesets/secondary/bike_shop', 'data/tilesets/secondary/subway', OBJECTS, {12: PAL}, FLOOR,
         f'{ROOT}/tools/areas/subway_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
