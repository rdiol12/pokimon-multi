"""The carnival's tileset: Petalburg's secondary tileset plus new pixel art
drawn here in Emerald's style (dark navy outlines, three-step shading lit
from the top left): a Ferris wheel, a big top tent, market stalls, an
entrance arch, lamp posts and a balloon cart.

    python3 tools/areas/carnival_art.py

Writes data/tilesets/secondary/carnival/ and tools/areas/carnival_tiles.json
(object name -> rows of metatile ids, -1 where the object is see-through,
and the blocks you can walk under).
"""
import json, os, shutil, struct
import numpy as np
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = f'{ROOT}/data/tilesets/secondary/petalburg'
OUT = f'{ROOT}/data/tilesets/secondary/carnival'
NUM_PRIMARY = 512
GROUND = 0x001            # General's grass shows under see-through pixels
LAYER_NORMAL = 0          # top layer over sprites (walk-under parts)
LAYER_COVERED = 1         # both layers under sprites

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


# ---------------------------------------------------------------- drawing helpers
class Canvas:
    def __init__(self, w, h):
        self.a = np.zeros((h, w), np.uint8)
        self.h, self.w = h, w

    def px(self, x, y, c):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < self.w and 0 <= y < self.h:
            self.a[y, x] = c

    def rect(self, x0, y0, x1, y1, c):
        self.a[max(0, y0):y1 + 1, max(0, x0):x1 + 1] = c

    def disc(self, cx, cy, r, c, r_in=-1):
        yy, xx = np.mgrid[0:self.h, 0:self.w]
        d = np.hypot(xx + 0.5 - cx, yy + 0.5 - cy)
        self.a[(d <= r) & (d > r_in)] = c

    def line(self, x0, y0, x1, y1, c, width=1):
        n = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for t in np.linspace(0, 1, n):
            x, y = x0 + (x1 - x0) * t, y0 + (y1 - y0) * t
            for dx in range(width):
                self.px(x - width // 2 + dx, y, c)

    def outline(self, c=1, where=None):
        """Dark outline around everything drawn so far (outside edge only)."""
        f = self.a > 0
        n = np.zeros_like(f)
        n[1:, :] |= f[:-1, :]
        n[:-1, :] |= f[1:, :]
        n[:, 1:] |= f[:, :-1]
        n[:, :-1] |= f[:, 1:]
        m = n & ~f
        if where is not None:
            m &= where
        self.a[m] = c

    def mirror(self):
        """Left half mirrored onto the right half (objects are symmetric, so flipped tiles are shared)."""
        h = self.w // 2
        self.a[:, self.w - h:] = self.a[:, :h][:, ::-1]


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


MB_COUNTER = 0x80
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
}


# ---------------------------------------------------------------- packing into a tileset
def write_pal(path, cols):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for r, g, b in cols:
            f.write(f'{r} {g} {b}\n')


def main():
    os.makedirs(f'{OUT}/palettes', exist_ok=True)
    for i in range(16):
        shutil.copy(f'{SRC}/palettes/{i:02d}.pal', f'{OUT}/palettes/{i:02d}.pal')
    write_pal(f'{OUT}/palettes/07.pal', PAL_WHEEL)
    write_pal(f'{OUT}/palettes/11.pal', PAL_TENT)

    base = Image.open(f'{SRC}/tiles.png')
    ba = np.array(base)
    tiles = [ba[y:y + 8, x:x + 8].copy() for y in range(0, ba.shape[0], 8) for x in range(0, ba.shape[1], 8)]
    n_base = len(tiles)
    lookup = {}

    def add_tile(t):
        """Index (in the secondary set) and flips for an 8x8 tile, sharing mirrored copies."""
        for hf in (0, 1):
            for vf in (0, 1):
                v = t[:, ::-1] if hf else t
                v = v[::-1, :] if vf else v
                k = v.tobytes()
                if k in lookup:
                    return lookup[k], hf, vf
        tiles.append(t.copy())
        lookup[t.tobytes()] = len(tiles) - 1
        return len(tiles) - 1, 0, 0

    meta = list(struct.unpack(f'<{os.path.getsize(SRC + "/metatiles.bin") // 2}H', open(f'{SRC}/metatiles.bin', 'rb').read()))
    attrs = list(struct.unpack(f'<{os.path.getsize(SRC + "/metatile_attributes.bin") // 2}H',
                               open(f'{SRC}/metatile_attributes.bin', 'rb').read()))
    gen = open(f'{ROOT}/data/tilesets/primary/general/metatiles.bin', 'rb').read()
    ground_bottom = list(struct.unpack('<4H', gen[GROUND * 16:GROUND * 16 + 8]))
    first_new = NUM_PRIMARY + len(attrs)
    table, walk = {}, {}
    no = lambda r, c: False
    for name, (fn, pal, opt) in OBJECTS.items():
        over, can_walk, counter = (opt.get(k, no) for k in ('over', 'walk', 'counter'))
        walk[name] = []
        art = fn().a
        h, w = art.shape
        rows = []
        for by in range(0, h, 16):
            row = []
            for bx in range(0, w, 16):
                block = art[by:by + 16, bx:bx + 16]
                if not block.any():
                    row.append(-1)
                    continue
                top = []
                for ty, tx in ((0, 0), (0, 8), (8, 0), (8, 8)):
                    t = block[ty:ty + 8, tx:tx + 8]
                    if not t.any():
                        top.append(0)
                        continue
                    i, hf, vf = add_tile(t)
                    top.append((NUM_PRIMARY + i) | hf << 10 | vf << 11 | pal << 12)
                meta += ground_bottom + top
                r, c = by // 16, bx // 16
                attrs.append((LAYER_NORMAL if over(r, c) else LAYER_COVERED) << 12 | (MB_COUNTER if counter(r, c) else 0))
                if can_walk(r, c):
                    walk[name].append([r, c])
                row.append(NUM_PRIMARY + len(attrs) - 1)
            rows.append(row)
        table[name] = rows
    assert len(tiles) <= 512, f'{len(tiles)} tiles'
    assert len(attrs) <= 512, f'{len(attrs)} metatiles'

    n = len(tiles)
    cols = 16
    img = np.zeros(((n + cols - 1) // cols * 8, cols * 8), np.uint8)
    for i, t in enumerate(tiles):
        img[i // cols * 8:i // cols * 8 + 8, i % cols * 8:i % cols * 8 + 8] = t
    out = Image.fromarray(img, 'P')
    out.putpalette(base.getpalette())
    out.save(f'{OUT}/tiles.png')
    open(f'{OUT}/metatiles.bin', 'wb').write(struct.pack(f'<{len(meta)}H', *meta))
    open(f'{OUT}/metatile_attributes.bin', 'wb').write(struct.pack(f'<{len(attrs)}H', *attrs))
    json.dump({'num_tiles': n, 'first_new_metatile': first_new, 'objects': table, 'walkable': walk},
              open(f'{ROOT}/tools/areas/carnival_tiles.json', 'w'), indent=1)
    print(f'carnival tileset: {n} tiles ({n - n_base} new), {len(attrs)} metatiles')


if __name__ == '__main__':
    main()
