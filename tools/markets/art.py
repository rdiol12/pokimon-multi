"""Pixel art for the two trade buildings, drawn in Emerald's style (dark
navy outlines, two or three tones lit from the top left):

  * the MARKET HALL in Slateport: red tiled roof, gold sign, striped awning,
    shop windows full of fruit baskets;
  * the AUCTION HOUSE in Lilycove: purple roof, gold pediment with a gavel,
    a columned front and big double doors.

    python3 tools/markets/art.py

Each building goes into the town's own secondary tileset (palette 12 was
unused in both), so the town keeps its door animations and lit windows.
The art is added to the tileset as it was in the base import, so running
this again gives the same result. Writes tools/markets/<town>_tiles.json
(rows of metatile ids per object, and the blocks you can walk on).
"""
import os, subprocess, sys, shutil
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'areas'))
from art import Canvas, pack, ROOT

BASE = 'e55ba59f'          # the pokeemerald-expansion import: the untouched tilesets
GRASS = 0x001
PAL_SLOT = 12

PAL = [(115, 197, 164), (41, 49, 90), (255, 246, 222), (238, 222, 180), (197, 172, 131),
       (90, 57, 139), (139, 98, 197), (189, 156, 238), (197, 139, 24), (255, 213, 74),
       (172, 41, 57), (230, 82, 82), (98, 156, 238), (189, 230, 255), (115, 74, 49), (255, 255, 255)]
(OUT, CREAM_L, CREAM, CREAM_D, PUR_D, PUR, PUR_L, GOLD_D, GOLD, RED_D, RED, GLASS, GLASS_L,
 WOOD, WHITE) = range(1, 16)

FONT = {  # 3x5 capitals, like the letters on the MART and P.C. signs
    'A': ['.#.', '#.#', '###', '#.#', '#.#'], 'C': ['.##', '#..', '#..', '#..', '.##'],
    'E': ['###', '#..', '##.', '#..', '###'], 'H': ['#.#', '#.#', '###', '#.#', '#.#'],
    'I': ['###', '.#.', '.#.', '.#.', '###'], 'K': ['#.#', '#.#', '##.', '#.#', '#.#'],
    'L': ['#..', '#..', '#..', '#..', '###'], 'M': ['#.#', '###', '###', '#.#', '#.#'],
    'N': ['##.', '#.#', '#.#', '#.#', '#.#'], 'O': ['.#.', '#.#', '#.#', '#.#', '.#.'],
    'R': ['##.', '#.#', '##.', '#.#', '#.#'], 'T': ['###', '.#.', '.#.', '.#.', '.#.'],
    'S': ['.##', '#..', '.#.', '..#', '##.'], 'U': ['#.#', '#.#', '#.#', '#.#', '###'],
}


def text(c, word, cx, y, col):
    x = cx - (len(word) * 4 - 1) // 2
    for ch in word:
        for dy, row in enumerate(FONT[ch]):
            for dx, p in enumerate(row):
                if p == '#':
                    c.px(x + dx, y + dy, col)
        x += 4


def box(c, x0, y0, x1, y1, col):
    """A filled rectangle with a dark outline."""
    c.rect(x0, y0, x1, y1, OUT)
    c.rect(x0 + 1, y0 + 1, x1 - 1, y1 - 1, col)


def window(c, x0, y0, x1, y1):
    box(c, x0, y0, x1, y1, GLASS)
    for i in range(3):                           # the usual diagonal shine
        for k in range(3):
            c.px(x0 + 2 + i + k, y0 + 2 + (y1 - y0) // 2 - i, GLASS_L)
    c.rect(x0 + 1, y0 + 1, x1 - 1, y0 + 1, GLASS_L)


def basket(c, x, y, fruit):
    """A wicker basket with three fruit on top (x, y = top left, 10 wide)."""
    for i, col in enumerate(fruit):
        c.disc(x + 2.5 + i * 2.5, y + 2.5, 1.8, col)
        c.px(x + 2 + i * 2.5, y + 1.6, WHITE)
    c.rect(x, y + 3, x + 9, y + 6, WOOD)
    c.rect(x + 1, y + 4, x + 8, y + 4, CREAM_D)
    c.rect(x, y + 7, x + 9, y + 7, OUT)


# ---------------------------------------------------------------- the market hall (5 x 4 blocks)
def market_hall():
    c = Canvas(80, 64)
    # walls
    c.rect(3, 34, 76, 62, CREAM)
    c.rect(3, 34, 4, 62, CREAM_L)
    c.rect(73, 34, 76, 62, CREAM_D)
    c.rect(3, 59, 76, 62, CREAM_D)
    for x in range(10, 72, 16):                  # pale brick courses
        for y in range(40, 58, 6):
            c.rect(x + (8 if (y // 6) % 2 else 0), y, x + (8 if (y // 6) % 2 else 0), y + 2, CREAM_D)
    # shop windows with fruit on the sill
    for x0 in (7, 51):
        c.rect(x0 - 1, 41, x0 + 22, 58, CREAM)
        window(c, x0, 42, x0 + 21, 56)
        c.rect(x0 - 1, 56, x0 + 22, 58, WOOD)
        c.rect(x0 - 1, 56, x0 + 22, 56, OUT)
        basket(c, x0 + 1, 49, (RED, GOLD, RED))
        basket(c, x0 + 11, 49, (PUR, GOLD, PUR))
    # door: wooden frame, glass panes, a little OPEN plate
    c.rect(31, 40, 48, 63, OUT)
    c.rect(32, 41, 47, 63, WOOD)
    c.rect(34, 43, 45, 63, CREAM_D)
    c.rect(35, 44, 44, 55, GLASS)
    c.rect(35, 44, 44, 44, GLASS_L)
    c.rect(39, 44, 40, 63, WOOD)
    c.rect(35, 56, 44, 63, CREAM_D)
    c.px(37, 59, GOLD), c.px(42, 59, GOLD)
    # roof: red tiles in rows, lit from the left, darker toward the eaves
    for y in range(4, 30):
        inset = max(0, 6 - (y - 4))
        for x in range(1 + inset, 79 - inset):
            col = RED
            if (y - 4) % 5 == 4:
                col = RED_D
            elif x < 4 + inset or x > 75 - inset:
                col = RED_D
            elif (x + (y // 5) * 4) % 8 == 0 and (y - 4) % 5 in (1, 2, 3):
                col = RED_D
            c.px(x, y, col)
    c.rect(7, 3, 72, 4, GOLD)                    # ridge cap
    c.rect(7, 3, 72, 3, WHITE)
    # sign on the roof
    box(c, 22, 9, 57, 21, GOLD)
    c.rect(23, 10, 56, 10, WHITE)
    c.rect(23, 20, 56, 20, GOLD_D)
    text(c, 'MARKET', 40, 13, OUT)
    # striped awning with a scalloped hem
    for x in range(0, 80):
        col = RED if (x // 8) % 2 == 0 else WHITE
        c.rect(x, 29, x, 35, col)
        c.px(x, 29, RED_D if col == RED else CREAM_D)
    for x0 in range(0, 80, 8):
        col = RED if (x0 // 8) % 2 == 0 else WHITE
        for dx in range(8):
            d = 2 if 2 <= dx <= 5 else 1
            c.rect(x0 + dx, 36, x0 + dx, 36 + d - 1, RED_D if col == RED else CREAM_D)
    c.outline()
    return c


# ---------------------------------------------------------------- the auction house (6 x 5 blocks)
def gavel(c, x, y):
    """A gold gavel and its block, 16 x 12, x/y top left."""
    c.rect(x + 2, y + 1, x + 8, y + 5, WOOD)      # head
    c.rect(x + 2, y + 1, x + 8, y + 1, CREAM_D)
    c.rect(x + 1, y + 2, x + 1, y + 4, GOLD)
    c.rect(x + 9, y + 2, x + 9, y + 4, GOLD)
    for i in range(7):                            # handle
        c.px(x + 7 + i, y + 5 + i // 2, WOOD)
        c.px(x + 7 + i, y + 6 + i // 2, GOLD_D)
    c.rect(x + 1, y + 10, x + 10, y + 11, WOOD)   # sound block
    c.rect(x + 1, y + 10, x + 10, y + 10, GOLD)


def auction_house():
    c = Canvas(96, 80)
    # facade
    c.rect(2, 40, 93, 77, CREAM)
    c.rect(2, 40, 3, 77, CREAM_L)
    c.rect(90, 40, 93, 77, CREAM_D)
    # arched windows between the columns
    for x0 in (11, 69):
        c.rect(x0, 50, x0 + 15, 68, OUT)
        c.disc(x0 + 8, 52, 8, OUT)
        c.rect(x0 + 1, 51, x0 + 14, 67, GLASS)
        c.disc(x0 + 8, 52, 7, GLASS)
        c.rect(x0 + 1, 40, x0 + 14, 44, CREAM)   # trim back what the disc overshot
        c.rect(x0 + 7, 46, x0 + 8, 67, CREAM_L)  # mullion
        c.rect(x0 + 1, 58, x0 + 14, 58, CREAM_L)
        for i in range(4):
            c.px(x0 + 3 + i, 54 - i, GLASS_L)
            c.px(x0 + 10 + i, 63 - i, GLASS_L)
        c.rect(x0 - 1, 68, x0 + 16, 70, CREAM_D)
        c.rect(x0 - 1, 68, x0 + 16, 68, CREAM_L)
    # columns
    for x0 in (4, 28, 62, 86):
        c.rect(x0, 44, x0 + 5, 74, CREAM_L)
        c.rect(x0 + 4, 44, x0 + 5, 74, CREAM_D)
        c.rect(x0 + 2, 46, x0 + 2, 72, CREAM)
        c.rect(x0 - 1, 43, x0 + 6, 45, CREAM_D)   # capital
        c.rect(x0 - 1, 43, x0 + 6, 43, WHITE)
        c.rect(x0 - 1, 73, x0 + 6, 75, CREAM_D)   # base
    # double doors (rows 3-4, columns 2-3)
    c.rect(35, 49, 60, 79, OUT)
    c.disc(48, 52, 12, OUT)
    c.rect(36, 50, 59, 79, RED_D)
    c.disc(48, 52, 11, RED_D)
    c.rect(36, 40, 59, 41, CREAM)
    c.disc(48, 52, 9, GOLD)                       # fanlight
    c.disc(48, 52, 8, GLASS)
    c.rect(36, 52, 59, 53, GOLD)
    c.rect(47, 53, 48, 79, OUT)
    for x0 in (38, 50):                           # panels
        c.rect(x0, 56, x0 + 7, 65, RED)
        c.rect(x0, 56, x0 + 7, 56, WHITE)
        c.rect(x0, 68, x0 + 7, 76, RED)
    c.px(45, 67, GOLD), c.px(50, 67, GOLD)
    # steps
    c.rect(0, 76, 95, 79, CREAM_D)
    c.rect(0, 76, 95, 76, CREAM_L)
    c.rect(33, 76, 62, 79, CREAM_D)
    # frieze with the name
    c.rect(1, 34, 94, 42, CREAM_L)
    c.rect(1, 41, 94, 42, CREAM_D)
    box(c, 30, 34, 65, 42, GOLD)
    c.rect(31, 35, 64, 35, WHITE)
    text(c, 'AUCTION', 48, 36, OUT)
    # purple roof: slate rows, darker at the edges and eaves
    for y in range(8, 35):
        inset = max(0, 8 - (y - 8))
        for x in range(inset, 96 - inset):
            col = PUR
            if (y - 8) % 4 == 3:
                col = PUR_D
            elif x < 3 + inset or x > 92 - inset:
                col = PUR_D
            elif y < 14:
                col = PUR_L if (x // 3) % 2 == 0 else PUR
            c.px(x, y, col)
    c.rect(8, 7, 87, 8, GOLD)
    c.rect(8, 7, 87, 7, WHITE)
    # gold pediment with the gavel
    for y in range(0, 26):
        half = 4 + y * 1.25
        c.rect(int(48 - half), y, int(48 + half) - 1, y, OUT)
        if y > 1:
            c.rect(int(48 - half) + 2, y, int(48 + half) - 3, y, GOLD)
            c.px(int(48 - half) + 2, y, WHITE)
    c.rect(18, 26, 77, 28, OUT)
    c.rect(19, 26, 76, 27, GOLD_D)
    c.rect(28, 24, 67, 25, GOLD_D)
    box(c, 37, 9, 58, 24, PUR_D)
    gavel(c, 42, 11)
    c.outline()
    return c


# ---------------------------------------------------------------- inside: the market hall (Shop tileset)
def notice_board():
    """The MARKET BOARD on the hall's back wall: a cork board with notes, 32 x 32."""
    c = Canvas(32, 32)
    c.rect(1, 3, 30, 28, WOOD)
    c.rect(1, 3, 30, 3, CREAM_D)
    c.rect(3, 5, 28, 26, CREAM_D)
    for x, y in ((5, 8), (11, 19), (20, 7), (24, 21), (15, 13), (8, 24)):
        c.px(x, y, WOOD)
    notes = ((4, 6, 11, 13, CREAM_L, RED), (13, 7, 19, 14, WHITE, GLASS), (21, 6, 27, 12, CREAM_L, PUR),
             (5, 16, 12, 24, WHITE, GOLD), (14, 17, 20, 25, CREAM_L, RED), (21, 15, 27, 23, WHITE, GLASS))
    for x0, y0, x1, y1, paper, pin in notes:
        c.rect(x0, y0, x1, y1, paper)
        c.rect(x0 + 1, y0 + 3, x1 - 1, y0 + 3, CREAM_D)
        c.rect(x0 + 1, y0 + 5, x1 - 2, y0 + 5, CREAM_D)
        c.px((x0 + x1) // 2, y0, pin)
    c.disc(16.5, 2.5, 3, GOLD)                    # a coin badge on top
    c.px(16, 1, WHITE)
    c.px(16, 2, GOLD_D), c.px(16, 3, GOLD_D)
    c.outline()
    return c


def produce_crate():
    """A crate of fruit for the hall floor, 16 x 16."""
    c = Canvas(16, 16)
    c.rect(1, 7, 14, 14, WOOD)
    c.rect(1, 7, 14, 7, CREAM_D)
    c.rect(1, 10, 14, 10, OUT)
    c.rect(4, 8, 4, 13, CREAM_D), c.rect(11, 8, 11, 13, CREAM_D)
    for i, (x, y, col) in enumerate(((4, 5, RED), (8, 4, GOLD), (12, 5, RED), (6, 7, GOLD), (10, 7, PUR))):
        c.disc(x, y, 2.2, col)
        c.px(x - 1, y - 1, WHITE)
    c.outline()
    return c


def scale_stand():
    """A weighing scale on a little wooden stand, 16 x 16."""
    c = Canvas(16, 16)
    c.rect(3, 10, 12, 14, WOOD)
    c.rect(3, 10, 12, 10, CREAM_D)
    c.rect(7, 3, 8, 9, GOLD_D)
    c.rect(2, 3, 13, 3, GOLD)
    for x0 in (1, 10):
        c.rect(x0, 5, x0 + 4, 6, GOLD)
        c.px(x0 + 2, 4, GOLD_D)
    c.disc(3, 4.5, 1.5, RED)
    c.outline()
    return c


# ---------------------------------------------------------------- inside: the auction house (Contest tileset)
def podium():
    """The auctioneer's lectern, 16 x 32: wood, a purple drape and the gavel."""
    c = Canvas(16, 32)
    c.rect(1, 14, 14, 30, WOOD)
    c.rect(1, 14, 14, 15, GOLD)
    c.rect(1, 14, 14, 14, WHITE)
    c.rect(3, 17, 12, 27, PUR)
    c.rect(3, 17, 12, 17, PUR_L)
    c.rect(11, 18, 12, 27, PUR_D)
    c.rect(6, 20, 9, 23, GOLD)                    # a little gold badge
    c.px(6, 20, WHITE)
    c.rect(1, 29, 14, 30, GOLD_D)
    # gavel resting on top
    c.rect(4, 9, 9, 12, WOOD)
    c.rect(4, 9, 9, 9, CREAM_D)
    c.rect(9, 11, 13, 12, GOLD_D)
    c.outline()
    return c


def chair():
    """A red velvet bidder's chair seen from behind (it faces the stage), 16 x 16."""
    c = Canvas(16, 16)
    c.rect(3, 2, 12, 6, RED)                      # seat
    c.rect(3, 2, 12, 2, WHITE)
    c.rect(2, 7, 13, 12, RED_D)                   # back of the backrest
    c.rect(2, 7, 13, 7, RED)
    c.rect(4, 9, 11, 10, RED)
    c.rect(3, 13, 3, 14, GOLD_D), c.rect(12, 13, 12, 14, GOLD_D)
    c.outline()
    return c


def lot_board():
    """The lot board beside the stage: a gold frame round three numbered lots, 32 x 32."""
    c = Canvas(32, 32)
    c.rect(1, 2, 30, 29, GOLD)
    c.rect(1, 2, 30, 2, WHITE)
    c.rect(3, 4, 28, 27, PUR_D)
    text(c, 'LOTS', 16, 6, GOLD)
    for i, y in enumerate((13, 18, 23)):
        c.rect(5, y, 26, y + 3, PUR)
        c.rect(5, y, 7, y + 3, GOLD)
        c.rect(9, y + 1, 9 + 4 + 5 * (i % 2), y + 1, CREAM_L)
        c.rect(9, y + 2, 21 - 3 * i, y + 2, PUR_L)
    c.rect(14, 29, 17, 31, GOLD_D)               # stand
    c.outline()
    return c


OBJECTS_SHOP = {
    'board_top': (lambda: crop(notice_board(), 0, 0, 32, 16), PAL_SLOT, {'ground': 0x288}),
    'board_bottom': (lambda: crop(notice_board(), 0, 16, 32, 16), PAL_SLOT, {'ground': 0x290}),
    'produce_crate': (produce_crate, PAL_SLOT, {'ground': 0x201}),
    'scale_stand': (scale_stand, PAL_SLOT, {'ground': 0x201}),
}
OBJECTS_CONTEST = {
    'podium': (podium, PAL_SLOT, {'ground': 0x271, 'over': lambda r, c: r == 0, 'counter': lambda r, c: r == 1}),
    'chair': (chair, PAL_SLOT, {'ground': 0x260}),
    'lot_board': (lot_board, PAL_SLOT, {'ground': 0x260}),
}


def crop(c, x, y, w, h):
    out = Canvas(w, h)
    out.a[:, :] = c.a[y:y + h, x:x + w]
    return out


OBJECTS_SLATEPORT = {
    'market_hall': (market_hall, PAL_SLOT, {'door': lambda r, c: (r, c) == (3, 2)}),
}
OBJECTS_LILYCOVE = {
    'auction_house': (auction_house, PAL_SLOT, {'door': lambda r, c: r == 4 and c in (2, 3)}),
}


def pristine(name):
    """The tileset as imported, under build/, as the source to add the art to."""
    rel = f'data/tilesets/secondary/{name}'
    out = f'build/markets_base/{name}'
    shutil.rmtree(f'{ROOT}/{out}', ignore_errors=True)
    os.makedirs(f'{ROOT}/{out}/palettes', exist_ok=True)
    files = ['tiles.png', 'metatiles.bin', 'metatile_attributes.bin'] + [f'palettes/{i:02d}.pal' for i in range(16)]
    for f in files:
        data = subprocess.run(['git', 'show', f'{BASE}:{rel}/{f}'], cwd=ROOT, capture_output=True, check=True).stdout
        if f.endswith('.pal'):                    # the checkout keeps palettes with CRLF
            data = data.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
        open(f'{ROOT}/{out}/{f}', 'wb').write(data)
    return out


# ---------------------------------------------------------------- the two NPCs: existing frames, new colours
NPC_COLOURS = {
    # the MARKET MASTER: the fat man's frames, a grocer's green apron, brown trousers, grey hair
    'market_master': ('npc_1', {5: (197, 148, 98), 6: (148, 98, 65), 7: (90, 57, 41),
                                8: (148, 222, 123), 9: (74, 164, 82), 10: (24, 82, 41),
                                11: (156, 156, 164), 12: (106, 106, 115), 13: (57, 57, 65)}),
    # the AUCTIONEER: the gentleman's frames in a purple suit with a gold hatband
    'auctioneer': ('npc_3', {8: (255, 213, 74), 9: (205, 148, 24), 10: (123, 82, 8),
                             11: (164, 115, 213), 12: (106, 65, 164), 13: (49, 24, 90)}),
}


def npc_palettes():
    from art import write_pal
    os.makedirs(f'{ROOT}/graphics/markets', exist_ok=True)
    for name, (base, swaps) in NPC_COLOURS.items():
        vals = open(f'{ROOT}/graphics/object_events/palettes/{base}.pal').read().split()[3:]
        cols = [tuple(map(int, vals[i:i + 3])) for i in range(0, 48, 3)]
        for i, col in swaps.items():
            cols[i] = col
        write_pal(f'{ROOT}/graphics/markets/{name}.pal', cols)


def main():
    npc_palettes()
    here = os.path.dirname(os.path.abspath(__file__))
    pack(pristine('slateport'), 'data/tilesets/secondary/slateport', OBJECTS_SLATEPORT, {PAL_SLOT: PAL}, GRASS,
         f'{here}/slateport_tiles.json', fill_free=True)
    pack(pristine('lilycove'), 'data/tilesets/secondary/lilycove', OBJECTS_LILYCOVE, {PAL_SLOT: PAL}, GRASS,
         f'{here}/lilycove_tiles.json', fill_free=True)
    pack(pristine('shop'), 'data/tilesets/secondary/shop', OBJECTS_SHOP, {PAL_SLOT: PAL}, 0x201,
         f'{here}/shop_tiles.json', fill_free=True)
    pack(pristine('contest'), 'data/tilesets/secondary/contest', OBJECTS_CONTEST, {PAL_SLOT: PAL}, 0x260,
         f'{here}/contest_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
