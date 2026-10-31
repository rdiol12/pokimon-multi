"""Shared helpers for new tileset art: a small pixel canvas for drawing in
Emerald's style, and pack(), which turns drawn objects into a secondary
tileset (a copy of an existing one plus the new tiles and metatiles).

Used by tools/areas/carnival_art.py and tools/areas/abyss_art.py.
"""
import json, os, re, shutil, struct
import numpy as np
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
NUM_PRIMARY = 512
LAYER_NORMAL = 0          # top layer over sprites (walk-under parts)
LAYER_COVERED = 1         # both layers under sprites
MB_NON_ANIMATED_DOOR = 0x60
MB_COUNTER = 0x80


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



def write_pal(path, cols):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for r, g, b in cols:
            f.write(f'{r} {g} {b}\n')


def _u16s(path):
    return list(struct.unpack(f'<{os.path.getsize(path) // 2}H', open(path, 'rb').read()))


def pack(src, out, objects, palettes, ground, table_path, fill_free=False, keep=None):
    """Copies the secondary tileset in src to out and adds the drawn objects.

    objects: {name: (draw, palette, options)}, where draw() returns a Canvas
    and options may hold over (blocks drawn over sprites), walk (blocks you
    can walk on), counter (blocks you can talk across), door (blocks that
    warp you when you step in) and behavior (any other metatile behaviour,
    or 0), each a function of (row, col). palettes: {slot: 16 colours} replacing the copied ones.
    ground: the metatile whose bottom layer shows under see-through pixels
    (an object's options can name its own, e.g. water under a ship).
    fill_free: put new tiles in slots no metatile uses (for full tilesets
    whose animations need every tile to stay where it is) before appending.
    keep: for a tileset that is already full, the metatile ids to keep (the
    ones the new maps use); every other metatile and the tiles only they
    used are cleared, and the new art goes into their slots.
    Writes table_path: object name -> rows of metatile ids (-1 where the
    object is see-through) and the blocks you can walk on.
    """
    src, out = f'{ROOT}/{src}', f'{ROOT}/{out}'
    os.makedirs(f'{out}/palettes', exist_ok=True)
    for i in range(16):
        shutil.copy(f'{src}/palettes/{i:02d}.pal', f'{out}/palettes/{i:02d}.pal')
    for i, cols in palettes.items():
        write_pal(f'{out}/palettes/{i:02d}.pal', cols)

    base = Image.open(f'{src}/tiles.png')
    ba = np.array(base)
    tiles = [ba[y:y + 8, x:x + 8].copy() for y in range(0, ba.shape[0], 8) for x in range(0, ba.shape[1], 8)]
    n_base = len(tiles)
    meta = _u16s(f'{src}/metatiles.bin')
    attrs = _u16s(f'{src}/metatile_attributes.bin')
    free_mt = []
    if keep is not None:
        free_mt = [i for i in range(len(attrs)) if NUM_PRIMARY + i not in keep]
        for i in free_mt:
            meta[i * 8:i * 8 + 8] = [0] * 8
            attrs[i] = 0
    used = {(v & 0x3FF) - NUM_PRIMARY for v in meta if (v & 0x3FF) >= NUM_PRIMARY}
    if keep is not None:
        for i in range(1, n_base):
            if i not in used:
                tiles[i][:] = 0
    free = [i for i in range(1, n_base) if i not in used and not tiles[i].any()] if fill_free else []
    lookup = {}
    added = 0

    def add_tile(t):
        """Index (in the secondary set) and flips for an 8x8 tile, sharing mirrored copies."""
        nonlocal added
        for hf in (0, 1):
            for vf in (0, 1):
                v = t[:, ::-1] if hf else t
                v = v[::-1, :] if vf else v
                k = v.tobytes()
                if k in lookup:
                    return lookup[k], hf, vf
        added += 1
        if free:
            i = free.pop(0)
            tiles[i] = t.copy()
        else:
            tiles.append(t.copy())
            i = len(tiles) - 1
        lookup[t.tobytes()] = i
        return i, 0, 0

    gm = _u16s(f'{ROOT}/data/tilesets/primary/general/metatiles.bin')

    def bottom(m):
        return gm[m * 8:m * 8 + 4] if m < NUM_PRIMARY else meta[(m - NUM_PRIMARY) * 8:(m - NUM_PRIMARY) * 8 + 4]
    first_new = NUM_PRIMARY + len(attrs)
    table, walk = {}, {}
    no = lambda r, c: False
    for name, (fn, pal, opt) in objects.items():
        over, can_walk, counter, door, other = (opt.get(k, no) for k in ('over', 'walk', 'counter', 'door', 'behavior'))
        walk[name] = []
        ground_bottom = bottom(opt.get('ground', ground))
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
                r, c = by // 16, bx // 16
                beh = MB_COUNTER if counter(r, c) else MB_NON_ANIMATED_DOOR if door(r, c) else int(other(r, c))
                entry = (LAYER_NORMAL if over(r, c) else LAYER_COVERED) << 12 | beh
                if free_mt:
                    slot = free_mt.pop(0)
                    meta[slot * 8:slot * 8 + 8] = ground_bottom + top
                    attrs[slot] = entry
                else:
                    slot = len(attrs)
                    meta += ground_bottom + top
                    attrs.append(entry)
                if can_walk(r, c) or door(r, c) or other(r, c):
                    walk[name].append([r, c])
                row.append(NUM_PRIMARY + slot)
            rows.append(row)
        table[name] = rows
    assert len(tiles) <= 512, f'{len(tiles)} tiles'
    assert len(attrs) <= 512, f'{len(attrs)} metatiles'

    n = len(tiles)
    cols = 16 if base.width == 128 else base.width // 8
    img = np.zeros(((n + cols - 1) // cols * 8, cols * 8), np.uint8)
    for i, t in enumerate(tiles):
        img[i // cols * 8:i // cols * 8 + 8, i % cols * 8:i % cols * 8 + 8] = t
    im = Image.fromarray(img, 'P')
    im.putpalette(base.getpalette())
    im.save(f'{out}/tiles.png')
    open(f'{out}/metatiles.bin', 'wb').write(struct.pack(f'<{len(meta)}H', *meta))
    open(f'{out}/metatile_attributes.bin', 'wb').write(struct.pack(f'<{len(attrs)}H', *attrs))
    json.dump({'num_tiles': n, 'first_new_metatile': first_new, 'objects': table, 'walkable': walk},
              open(table_path, 'w'), indent=1)
    # keep the tile count the build reads in step
    gp = f'{ROOT}/src/data/tilesets/graphics.h'
    g = open(gp).read()
    rel = os.path.relpath(out, ROOT)
    g2 = re.sub(rf'("{re.escape(rel)}/tiles\.png", "[^"]*", "-num_tiles )\d+', rf'\g<1>{n}', g)
    if g2 != g:
        open(gp, 'w').write(g2)
    print(f'{os.path.basename(out)} tileset: {n} tiles ({added} new), {len(attrs)} metatiles')
