"""Ironmoor Prison's tileset: the Mt. Pyre tiles (the rocky slopes, graves
and the stone halls inside; the rest of the Facility tiles are cleared to
make room) plus new pixel art in Emerald's style: the prison's outer wall
and the breach in it, the iron portcullis (shut or raised), watchtowers
with their searchlights (lit or dark) and the beams they throw across the
yard, the generator, the cell block's front, the landing pier, and inside:
barred cell walls, cell gates (locked or open), bunks, chains, the spirit
wisps, the Odd Keystone, the armoury's sword rack and the warden's desk.

    python3 tools/areas/prison_art.py

Writes data/tilesets/secondary/prison/ and tools/areas/prison_tiles.json.
"""
import json, os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

GROUND = 0x001            # the slopes' ground shows under see-through pixels
FLOOR = 0x2E6             # the halls' stone floor, for the inside art

# Palette 12: the prison's stone and iron, moss, rust and lamplight.
PAL = [(115, 197, 164), (33, 33, 41), (74, 74, 90), (115, 115, 131), (156, 156, 172), (197, 197, 205),
       (49, 57, 66), (90, 98, 106), (148, 82, 49), (189, 123, 74), (57, 98, 57), (90, 139, 74),
       (255, 238, 156), (255, 255, 222), (24, 24, 32), (205, 57, 49)]
OUT, ST_D, ST, ST_L, ST_LL, IR_D, IR, RUST, RUST_L, MOSS_D, MOSS, LIGHT, LIGHT_L, DARK, RED = range(1, 16)

# Palette 8: the cells: wood, cloth, iron, the spirits' ghost-fire and the keystone.
PAL_IN = [(115, 197, 164), (33, 33, 41), (90, 57, 33), (139, 90, 49), (189, 139, 82), (131, 131, 148),
          (74, 82, 98), (123, 131, 148), (180, 189, 205), (82, 65, 164), (123, 139, 238), (197, 213, 255),
          (255, 255, 255), (106, 115, 98), (148, 82, 180), (238, 230, 205)]
I_OUT, WOOD_D, WOOD, WOOD_L, CLOTH, I_IR_D, I_IR, I_IR_L, SP_D, SP, SP_L, WHITE, KS, PURPLE, PAPER = range(1, 16)


# ---------------------------------------------------------------- outside
def bricks(c, x0, y0, x1, y1, base=ST, mortar=ST_D, light=ST_L, h=4, w=8):
    c.rect(x0, y0, x1, y1, base)
    for y in range(y0, y1 + 1, h):
        c.rect(x0, y, x1, y, mortar)
        off = (y // h) % 2 * (w // 2)
        for x in range(x0 + off, x1 + 1, w):
            c.rect(x, y, x, min(y1, y + h - 1), mortar)
            c.px(x + 1, y + 1, light)


def wall():
    """The outer wall, one block wide and two tall: iron spikes along the top
    over grey stone, moss creeping up from the foot."""
    c = Canvas(16, 32)
    bricks(c, 0, 6, 15, 31)
    c.rect(0, 6, 15, 7, ST_LL)
    c.rect(0, 8, 15, 8, ST_D)
    for x in range(1, 16, 4):                     # the spikes
        c.rect(x, 1, x + 1, 5, IR)
        c.px(x, 0, IR_D)
        c.px(x + 1, 0, IR_D)
    c.rect(0, 4, 15, 5, IR_D)
    r = np.random.RandomState(3)
    for x in range(16):
        for y in range(31 - r.randint(1, 6), 32):
            c.px(x, y, MOSS if (x + y) % 3 else MOSS_D)
    c.rect(0, 31, 15, 31, ST_D)
    return c


def breach():
    """Where the wall has fallen in: two blocks of rubble you can climb over."""
    c = Canvas(32, 32)
    r = np.random.RandomState(5)
    bricks(c, 0, 6, 4, 22)                        # jagged stubs of wall either side
    bricks(c, 27, 8, 31, 24)
    c.outline(OUT)
    for _ in range(14):                           # fallen blocks
        x, y = r.randint(3, 28), r.randint(14, 30)
        w, h = r.randint(3, 6), r.randint(2, 4)
        c.rect(x, y, x + w, y + h, ST)
        c.rect(x, y, x + w, y, ST_L)
        c.rect(x, y + h, x + w, y + h, ST_D)
    for _ in range(10):
        c.px(r.randint(2, 30), r.randint(12, 31), MOSS)
    return c


def gate(open_):
    """The main gate, three blocks by two: a stone arch with an iron
    portcullis, rusted shut or raised into the arch."""
    c = Canvas(48, 32)
    bricks(c, 0, 0, 47, 31)
    c.rect(0, 0, 47, 1, ST_LL)
    c.disc(24, 14, 13, DARK)                      # the archway
    c.rect(11, 14, 36, 31, DARK)
    c.disc(24, 14, 14, ST_D, 13)
    c.rect(10, 14, 10, 31, ST_D)
    c.rect(37, 14, 37, 31, ST_D)
    top = 6 if open_ else 31
    for x in range(13, 36, 4):                    # the bars
        c.rect(x, 2, x + 1, min(top, 31), IR)
        c.px(x, min(top, 31), IR_D)
    for y in range(8, top, 6):
        c.rect(11, y, 36, y, IR_D)
    if not open_:
        c.rect(21, 20, 26, 25, RUST)              # a rusted padlock
        c.rect(22, 21, 25, 22, RUST_L)
        c.disc(24, 19, 2.5, IR_D, 1.4)
    c.outline(OUT)
    return c


def tower(on):
    """A watchtower, two blocks by four: a stone shaft with a lamp room on
    top; the searchlight glares when the power is on."""
    c = Canvas(32, 64)
    bricks(c, 6, 24, 25, 63)
    c.rect(6, 24, 7, 63, ST_L)
    c.rect(2, 18, 29, 23, ST_D)                   # the gallery
    c.rect(2, 18, 29, 18, ST_L)
    c.rect(5, 4, 26, 17, IR_D)                    # the lamp room
    c.rect(3, 1, 28, 4, ST)
    c.rect(3, 1, 28, 1, ST_L)
    if on:
        c.disc(16, 11, 6, LIGHT)
        c.disc(16, 11, 3.5, LIGHT_L)
    else:
        c.disc(16, 11, 6, IR)
        c.disc(16, 11, 3.5, ST_D)
    for x in (12, 20):
        c.rect(x, 40, x + 2, 46, DARK)
    c.outline(OUT)
    return c


def beam():
    """A searchlight beam on the ground: light dappled over the grass."""
    c = Canvas(16, 16)
    for y in range(16):
        for x in range(16):
            if (x + y) % 2 == 0:
                c.a[y, x] = LIGHT
            elif (x * 3 + y * 5) % 7 == 0:
                c.a[y, x] = LIGHT_L
    return c


def generator(on):
    """The yard's generator, two blocks by two: an iron box with gauges and
    the big switch, up (running) or down (off)."""
    c = Canvas(32, 32)
    c.rect(2, 6, 29, 29, IR)
    c.rect(2, 6, 29, 8, ST_L)
    c.rect(2, 27, 29, 29, IR_D)
    for x in (6, 14):
        c.disc(x + 3, 15, 3.5, ST_LL)
        c.line(x + 3, 15, x + 5, 13 if on else 17, RED)
    c.rect(4, 21, 18, 24, DARK)
    for x in range(5, 18, 3):
        c.rect(x, 22, x + 1, 23, LIGHT if on else ST_D)
    c.rect(23, 10, 26, 26, IR_D)                  # the switch
    if on:
        c.line(24, 18, 24, 3, ST_LL, 2)
        c.disc(24, 3, 2.5, RED)
    else:
        c.line(24, 18, 24, 30, ST_LL, 2)
        c.disc(24, 30, 2.5, RED)
    c.outline(OUT)
    return c


def block():
    """The cell block, nine blocks by five: a grim stone front with barred
    windows in two rows, a slate roof and the iron-bound door."""
    c = Canvas(144, 80)
    c.rect(2, 0, 141, 13, IR_D)                   # the roof
    for x in range(2, 142, 6):
        c.rect(x, 0, x, 13, DARK)
    c.rect(0, 12, 143, 15, ST_D)
    bricks(c, 2, 16, 141, 79)
    c.rect(2, 16, 141, 17, ST_LL)
    for row, y in enumerate((22, 46)):
        for x in range(10, 136, 16):
            if row == 1 and 56 <= x <= 80:
                continue
            c.rect(x, y, x + 7, y + 11, DARK)
            c.rect(x, y + 12, x + 7, y + 12, ST_LL)
            for bx in range(x + 1, x + 7, 2):
                c.rect(bx, y, bx, y + 11, IR)
    c.rect(56, 44, 87, 79, ST_D)                  # the doorway
    c.rect(60, 50, 83, 79, DARK)
    c.rect(64, 50, 79, 79, RUST)
    for y in (56, 68):
        c.rect(64, y, 79, y + 1, IR)
    c.rect(56, 44, 87, 45, ST_LL)
    c.outline(OUT)
    return c



def pier():
    c = Canvas(16, 16)
    c.a[:] = RUST
    for y in (0, 5, 10, 15):
        c.rect(0, y, 15, y, IR_D)
    for y in (1, 6, 11):
        c.rect(0, y, 15, y, RUST_L)
    return c


def sea():
    """Cold grey sea round the landing."""
    c = Canvas(16, 16)
    c.a[:] = I_IR_D
    r = np.random.RandomState(8)
    for _ in range(6):
        x, y = r.randint(0, 13), r.randint(0, 16)
        c.rect(x, y, x + 3, y, I_IR)
    for _ in range(3):
        c.px(r.randint(0, 16), r.randint(0, 16), I_IR_L)
    return c


# ---------------------------------------------------------------- inside
def bars():
    """A barred cell wall, one block wide and two tall."""
    c = Canvas(16, 32)
    c.rect(0, 0, 15, 2, I_IR_D)
    c.rect(0, 0, 15, 0, I_IR_L)
    c.rect(0, 29, 15, 31, I_IR_D)
    for x in (1, 5, 9, 13):
        c.rect(x, 3, x + 1, 28, I_IR)
        c.rect(x, 3, x, 28, I_IR_L)
    c.rect(0, 14, 15, 15, I_IR_D)
    return c


def cell_gate(open_):
    """A cell gate: shut with a padlock, or swung open."""
    c = Canvas(16, 32)
    c.rect(0, 0, 15, 2, I_IR_D)
    c.rect(0, 0, 15, 0, I_IR_L)
    if open_:
        c.rect(0, 3, 2, 31, I_IR)                 # the gate folded back against the frame
        c.rect(0, 3, 0, 31, I_IR_L)
        c.rect(14, 3, 15, 31, I_IR_D)
    else:
        c.rect(0, 29, 15, 31, I_IR_D)
        for x in (1, 5, 9, 13):
            c.rect(x, 3, x + 1, 28, I_IR)
            c.rect(x, 3, x, 28, I_IR_L)
        c.rect(0, 14, 15, 15, I_IR_D)
        c.rect(5, 16, 10, 21, WOOD_L)             # the padlock
        c.rect(6, 17, 9, 18, PAPER)
        c.disc(7.5, 15, 2.5, I_IR_D, 1.3)
    return c


def bunk():
    c = Canvas(32, 16)
    c.rect(1, 3, 30, 13, WOOD)
    c.rect(1, 3, 30, 4, WOOD_L)
    c.rect(3, 5, 28, 11, CLOTH)
    c.rect(3, 5, 9, 9, PAPER)                     # the pillow
    c.rect(1, 13, 30, 14, WOOD_D)
    c.outline(I_OUT)
    return c


def chains():
    """Chains and shackles fixed to the wall."""
    c = Canvas(16, 16)
    c.rect(6, 0, 9, 2, I_IR_D)
    for i, y in enumerate(range(3, 12, 3)):
        c.rect(6 + i % 2, y, 8 + i % 2, y + 2, I_IR)
        c.px(7 + i % 2, y + 1, I_IR_D)
    c.disc(5, 13, 2.5, I_IR_L, 1.2)
    c.disc(11, 13, 2.5, I_IR_L, 1.2)
    return c


def wisp():
    """A trapped spirit: a cold blue ghost-fire flickering over the floor."""
    c = Canvas(16, 16)
    yy, xx = np.mgrid[0:16, 0:16]
    flame = ((xx + 0.5 - 8) / 5) ** 2 + ((yy + 0.5 - 10) / 5) ** 2 <= 1
    flame |= (np.abs(xx + 0.5 - 8) < (yy - 1) * 0.55) & (yy < 10)
    c.a[flame] = SP
    inner = ((xx + 0.5 - 8) / 3) ** 2 + ((yy + 0.5 - 11) / 3) ** 2 <= 1
    c.a[inner] = SP_L
    c.px(7, 11, WHITE)
    c.px(9, 11, WHITE)
    c.outline(SP_D)
    return c


def keystone(cracked):
    """The Odd Keystone on its plinth; cracked once the spirits are free."""
    c = Canvas(16, 16)
    c.rect(2, 12, 13, 15, I_IR)
    c.rect(2, 12, 13, 12, I_IR_L)
    c.disc(8, 7, 5.5, KS)
    c.disc(7, 6, 2.5, CLOTH)
    c.disc(8, 7, 2, PURPLE)                       # the spirits' face in the stone
    c.px(7, 6, WHITE)
    if cracked:
        c.line(3, 3, 7, 7, I_OUT)
        c.line(7, 7, 12, 5, I_OUT)
        c.line(7, 7, 9, 12, I_OUT)
    c.outline(I_OUT)
    return c


def sword_rack():
    """The armoury's rack, two blocks by two, with an old sword hung on it."""
    c = Canvas(32, 32)
    c.rect(3, 4, 28, 7, WOOD)
    c.rect(3, 4, 28, 4, WOOD_L)
    c.rect(3, 26, 28, 29, WOOD)
    for x in (4, 26):
        c.rect(x, 4, x + 2, 30, WOOD_D)
    c.rect(15, 6, 16, 25, I_IR_L)                 # the blade
    c.rect(15, 6, 15, 25, WHITE)
    c.rect(11, 9, 20, 10, WOOD_L)                 # the hilt and guard
    c.rect(15, 2, 16, 8, PURPLE)
    c.disc(15.5, 2, 2, SP)
    c.outline(I_OUT)
    return c


def desk():
    """The warden's desk, three blocks by two, papers and a lamp on it."""
    c = Canvas(48, 32)
    c.rect(1, 8, 46, 29, WOOD)
    c.rect(1, 8, 46, 11, WOOD_L)
    c.rect(1, 27, 46, 29, WOOD_D)
    c.rect(8, 14, 20, 24, WOOD_D)
    c.rect(28, 14, 40, 24, WOOD_D)
    c.rect(6, 2, 17, 9, PAPER)
    c.rect(8, 4, 15, 4, CLOTH)
    c.rect(8, 6, 13, 6, CLOTH)
    c.rect(34, 0, 39, 6, SP)                      # a ghost-fire lamp
    c.rect(35, 1, 38, 4, SP_L)
    c.rect(33, 7, 40, 9, I_IR_D)
    c.outline(I_OUT)
    return c


def cobweb():
    c = Canvas(16, 16)
    for i in range(0, 16, 2):
        c.line(0, 0, 15, i, CLOTH)
        c.line(0, 0, i, 15, CLOTH)
    for r in (5, 9, 13):
        for t in np.linspace(0, np.pi / 2, 14):
            c.px(r * np.cos(t), r * np.sin(t), WHITE)
    return c


def over_top(n):
    return lambda r, c: r < n


IN = {'ground': FLOOR}
OBJECTS = {
    'wall': (wall, 12, {}),
    'breach': (breach, 12, {'walk': lambda r, c: True}),
    'gate': (lambda: gate(False), 12, {}),
    'gate_open': (lambda: gate(True), 12, {'walk': lambda r, c: c == 1, 'over': lambda r, c: r == 0}),
    'tower_on': (lambda: tower(True), 12, {'over': over_top(2)}),
    'tower_off': (lambda: tower(False), 12, {'over': over_top(2)}),
    'beam': (beam, 12, {'walk': lambda r, c: True}),
    'gen_on': (lambda: generator(True), 12, {'over': over_top(1)}),
    'gen_off': (lambda: generator(False), 12, {'over': over_top(1)}),
    'block': (block, 12, {'over': over_top(1), 'door': lambda r, c: (r, c) == (4, 4)}),
    'pier': (pier, 12, {'walk': lambda r, c: True}),
    'sea': (sea, 8, {}),
    'bars': (bars, 8, IN),
    'cell_gate': (lambda: cell_gate(False), 8, IN),
    'cell_gate_open': (lambda: cell_gate(True), 8, dict(IN, walk=lambda r, c: True, over=lambda r, c: r == 0)),
    'bunk': (bunk, 8, IN),
    'chains': (chains, 8, IN),
    'wisp': (wisp, 8, IN),
    'keystone': (lambda: keystone(False), 8, IN),
    'keystone_cracked': (lambda: keystone(True), 8, IN),
    'sword_rack': (sword_rack, 8, dict(IN, over=over_top(1))),
    'desk': (desk, 8, IN),
    'cobweb': (cobweb, 8, dict(IN, walk=lambda r, c: True)),
}


def corpus_metatiles():
    L = {l['id']: l for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts'] if 'id' in l}
    keep = set()
    for lid in ('LAYOUT_MT_PYRE_EXTERIOR', 'LAYOUT_MT_PYRE_SUMMIT', 'LAYOUT_MT_PYRE_1F', 'LAYOUT_MT_PYRE_2F'):
        keep |= set((np.fromfile(f"{ROOT}/{L[lid]['blockdata_filepath']}", '<u2') & 0x3FF).tolist())
    return keep


def main():
    pack('data/tilesets/secondary/facility', 'data/tilesets/secondary/prison', OBJECTS, {8: PAL_IN, 12: PAL},
         GROUND, f'{ROOT}/tools/areas/prison_tiles.json', fill_free=True, keep=corpus_metatiles())


if __name__ == '__main__':
    main()
