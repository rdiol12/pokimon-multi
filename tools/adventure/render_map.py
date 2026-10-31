#!/usr/bin/env python3
"""Renders a map layout to a PNG from its tilesets (both metatile layers,
no objects), for designing maps and for full-map pictures.

Usage: render_map.py LAYOUT_ID out.png [--grid]
"""
import json
import os
import re
import struct
import sys

from PIL import Image, ImageDraw

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
NUM_PRIMARY_TILES, NUM_PRIMARY_METATILES, NUM_PRIMARY_PALS = 512, 512, 6


def tileset_dir(symbol):
    """gTileset_Foo -> data/tilesets/{primary,secondary}/foo"""
    name = symbol.replace("gTileset_", "")
    for path in ("src/data/tilesets/graphics.h", "src/graphics.c"):
        text = open(os.path.join(ROOT, path)).read()
        m = re.search(r'gTilesetTiles_%s\[\] = INC\w+\("(data/tilesets/\w+/\w+)/tiles' % name, text)
        if m:
            return os.path.join(ROOT, m.group(1))
    raise KeyError(symbol)


def load_pals(d):
    pals = []
    for i in range(16):
        p = os.path.join(d, "palettes", "%02d.pal" % i)
        if os.path.exists(p):
            lines = open(p).read().split("\n")[3:19]
            pals.append([tuple(int(v) for v in l.split()) for l in lines if l.strip()])
        else:
            pals.append([(0, 0, 0)] * 16)
    return pals


def load(d):
    tiles = Image.open(os.path.join(d, "tiles.png"))
    meta = open(os.path.join(d, "metatiles.bin"), "rb").read()
    return tiles, meta, load_pals(d)


def render(layout_id, grid=False):
    layouts = json.load(open(os.path.join(ROOT, "data", "layouts", "layouts.json")))["layouts"]
    lay = [l for l in layouts if l["id"] == layout_id][0]
    w, h = lay["width"], lay["height"]
    prim, sec = load(tileset_dir(lay["primary_tileset"])), load(tileset_dir(lay["secondary_tileset"]))
    pals = prim[2][:NUM_PRIMARY_PALS] + sec[2][NUM_PRIMARY_PALS:]
    raw = open(os.path.join(ROOT, lay["blockdata_filepath"]), "rb").read()
    out = Image.new("RGB", (w * 16, h * 16))
    cache = {}

    def tile(idx, pal, hf, vf):
        key = (idx, pal, hf, vf)
        if key in cache:
            return cache[key]
        src = prim[0] if idx < NUM_PRIMARY_TILES else sec[0]
        i = idx if idx < NUM_PRIMARY_TILES else idx - NUM_PRIMARY_TILES
        cols = src.width // 8
        t = src.crop(((i % cols) * 8, (i // cols) * 8, (i % cols) * 8 + 8, (i // cols) * 8 + 8))
        px = list(t.getdata())
        img = Image.new("RGBA", (8, 8))
        img.putdata([(*pals[pal][v & 15], 0 if v & 15 == 0 else 255) for v in px])
        if hf:
            img = img.transpose(Image.FLIP_LEFT_RIGHT)
        if vf:
            img = img.transpose(Image.FLIP_TOP_BOTTOM)
        cache[key] = img
        return img

    for y in range(h):
        for x in range(w):
            mt = struct.unpack_from("<H", raw, (y * w + x) * 2)[0] & 0x3FF
            meta = prim[1] if mt < NUM_PRIMARY_METATILES else sec[1]
            m = mt if mt < NUM_PRIMARY_METATILES else mt - NUM_PRIMARY_METATILES
            out.paste(pals[0][0], (x * 16, y * 16, x * 16 + 16, y * 16 + 16))
            for layer in range(2):
                for q in range(4):
                    e = struct.unpack_from("<H", meta, m * 16 + (layer * 4 + q) * 2)[0]
                    t = tile(e & 0x3FF, (e >> 12) & 15, e & 0x400, e & 0x800)
                    out.paste(t, (x * 16 + (q % 2) * 8, y * 16 + (q // 2) * 8), t)
    if grid:
        d = ImageDraw.Draw(out)
        for x in range(0, w, 5):
            d.line([(x * 16, 0), (x * 16, h * 16)], fill=(255, 0, 0))
        for y in range(0, h, 5):
            d.line([(0, y * 16), (w * 16, y * 16)], fill=(255, 0, 0))
    return out


if __name__ == "__main__":
    render(sys.argv[1], "--grid" in sys.argv).save(sys.argv[2])
