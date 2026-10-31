"""Reads pokeemerald tilesets and layouts and renders them to images.

    python3 tools/areas/tiles.py render LAYOUT_ID out.png [--grid]
    python3 tools/areas/tiles.py sheet gTileset_General gTileset_Lilycove out.png
"""
import json, os, re, struct, sys
from functools import lru_cache
import numpy as np
from PIL import Image, ImageDraw

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
NUM_PRIMARY = 512


@lru_cache(None)
def tileset_dirs():
    """gTileset_X -> data/tilesets/... directory, read from the C headers."""
    hdr = open(f'{ROOT}/src/data/tilesets/headers.h').read()
    gfx = open(f'{ROOT}/src/data/tilesets/graphics.h').read() + open(f'{ROOT}/src/graphics.c').read()
    mt = open(f'{ROOT}/src/data/tilesets/metatiles.h').read()
    out = {}
    for name, body in re.findall(r'const struct Tileset (gTileset_\w+) =\s*\{(.*?)\};', hdr, re.S):
        tiles = re.search(r'\.tiles = (\w+)', body).group(1)
        m = re.search(tiles + r'\[\] = INC\w+_U32\("([^"]+)"', gfx)
        meta = re.search(r'\.metatiles = (\w+)', body).group(1)
        mm = re.search(meta + r'\[\] = INCBIN_U16\("([^"]+)"', mt)
        if m and mm:
            out[name] = os.path.dirname(mm.group(1))
            out[name + ':tiles'] = m.group(1)
            out[name + ':tiles'] = re.sub(r'\.4bpp.*', '.png', out[name + ':tiles'])
    return out


def read_pal(path):
    lines = open(path).read().split('\n')[3:19]
    return [tuple(int(v) for v in l.split()[:3]) for l in lines if l.strip()]


class Tileset:
    def __init__(self, name):
        d = tileset_dirs()
        self.name, self.dir = name, f'{ROOT}/' + d[name]
        img = Image.open(f'{ROOT}/' + d[name + ':tiles'])
        a = np.array(img)
        if img.mode != 'P':
            a = a // 16 if a.max() > 15 else a
        h, w = a.shape
        self.tiles = a.reshape(h // 8, 8, w // 8, 8).swapaxes(1, 2).reshape(-1, 8, 8)
        raw = open(f'{self.dir}/metatiles.bin', 'rb').read()
        self.metatiles = np.frombuffer(raw, '<u2').reshape(-1, 8)
        self.attrs = np.frombuffer(open(f'{self.dir}/metatile_attributes.bin', 'rb').read(), '<u2')
        self.pals = [read_pal(f'{self.dir}/palettes/{i:02d}.pal') for i in range(16)]


class Pair:
    """A primary + secondary tileset, the way the game loads them."""
    def __init__(self, primary, secondary):
        self.p, self.s = Tileset(primary), Tileset(secondary)
        self.pals = self.p.pals[:6] + self.s.pals[6:13] + [[(0, 0, 0)] * 16] * 3
        self._cache = {}

    def tile(self, n):
        return self.p.tiles[n] if n < NUM_PRIMARY else self.s.tiles[n - NUM_PRIMARY]

    def entries(self, mt):
        return self.p.metatiles[mt] if mt < NUM_PRIMARY else self.s.metatiles[mt - NUM_PRIMARY]

    def attr(self, mt):
        return int(self.p.attrs[mt] if mt < NUM_PRIMARY else self.s.attrs[mt - NUM_PRIMARY])

    def count(self):
        return NUM_PRIMARY + len(self.s.metatiles)

    def metatile(self, mt):
        """16x16 RGB array of metatile mt (bottom layer, then top)."""
        if mt in self._cache:
            return self._cache[mt]
        out = np.zeros((16, 16, 3), np.uint8)
        try:
            ent = self.entries(mt)
        except IndexError:
            out[:] = (255, 0, 255)
            return out
        for layer in range(2):
            for q in range(4):
                e = int(ent[layer * 4 + q])
                t, hf, vf, pal = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
                try:
                    px = self.tile(t)
                except IndexError:
                    continue
                if hf: px = px[:, ::-1]
                if vf: px = px[::-1]
                cols = np.array(self.pals[pal] + [(0, 0, 0)] * (16 - len(self.pals[pal])), np.uint8)
                oy, ox = (q // 2) * 8, (q % 2) * 8
                region = out[oy:oy + 8, ox:ox + 8]
                if layer == 0:
                    region[:] = cols[px]
                else:
                    m = px != 0
                    region[m] = cols[px][m]
        self._cache[mt] = out
        return out


def layouts():
    return {l['id']: l for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts'] if 'id' in l}


def load_blocks(layout):
    raw = open(f"{ROOT}/{layout['blockdata_filepath']}", 'rb').read()
    return np.frombuffer(raw, '<u2').reshape(layout['height'], layout['width']).copy()


def render(pair, blocks, grid=False):
    h, w = blocks.shape
    img = np.zeros((h * 16, w * 16, 3), np.uint8)
    for y in range(h):
        for x in range(w):
            img[y * 16:y * 16 + 16, x * 16:x * 16 + 16] = pair.metatile(int(blocks[y, x]) & 0x3FF)
    im = Image.fromarray(img)
    if grid:
        d = ImageDraw.Draw(im)
        for y in range(h):
            for x in range(w):
                if blocks[y, x] & 0x0C00:
                    d.line((x * 16 + 2, y * 16 + 2, x * 16 + 13, y * 16 + 13), fill=(255, 0, 0))
    return im


def sheet(pair, cols=16, scale=1):
    n = pair.count()
    rows = (n + cols - 1) // cols
    im = Image.new('RGB', (cols * 16 + 40, rows * 16), (40, 40, 40))
    d = ImageDraw.Draw(im)
    for i in range(n):
        x, y = i % cols, i // cols
        im.paste(Image.fromarray(pair.metatile(i)), (40 + x * 16, y * 16))
        if x == 0:
            d.text((1, y * 16 + 3), f'{i:03X}', fill=(255, 255, 255))
    return im.resize((im.width * scale, im.height * scale), Image.NEAREST)


if __name__ == '__main__':
    cmd = sys.argv[1]
    if cmd == 'render':
        l = layouts()[sys.argv[2]]
        render(Pair(l['primary_tileset'], l['secondary_tileset']), load_blocks(l), '--grid' in sys.argv).save(sys.argv[3])
    elif cmd == 'sheet':
        sheet(Pair(sys.argv[2], sys.argv[3]), scale=2).save(sys.argv[4])
