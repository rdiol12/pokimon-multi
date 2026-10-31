#!/usr/bin/env python3
"""Renders a map layout from the source data (tiles, palettes, metatiles,
blockdata) to a PNG, for picking the colours that light up at night.

Usage: render_map.py LAYOUT_NAME out.png [--mark tileset:pal:idx ...] [--night]

--mark paints that colour magenta (tileset is primary or secondary);
--night applies the lit-window table (src/data/daynight_lights.h) the way
the game does at midnight: a dark blue tint, lights blended to warm yellow.
"""
import json
import os
import re
import sys
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def read(path):
    with open(os.path.join(ROOT, path)) as f:
        return f.read()


def tileset_paths(symbol):
    headers = read("src/data/tilesets/headers.h")
    m = re.search(r"const struct Tileset " + symbol + r" =\s*\{(.*?)\};", headers, re.S)
    body = m.group(1)
    tiles = re.search(r"\.tiles = (\w+)", body).group(1)
    pals = re.search(r"\.palettes = (\w+)", body).group(1)
    metas = re.search(r"\.metatiles = (\w+)", body).group(1)
    gfx = read("src/data/tilesets/graphics.h") + read("src/graphics.c")
    tiles_png = re.search(tiles + r"\[\] = INCGFX_U32\(\"([^\"]+)\"", gfx).group(1)
    pm = re.search(r"const u16 (?:ALIGNED\(4\) )?" + pals + r"\[\]\[16\] =\s*\{(.*?)\};", gfx, re.S)
    pal_files = re.findall(r"\"([^\"]+\.pal)\"", pm.group(1))
    mt = read("src/data/tilesets/metatiles.h")
    meta_bin = re.search(metas + r"\[\] = INCBIN_U16\(\"([^\"]+)\"", mt).group(1)
    return tiles_png, pal_files, meta_bin


def load_pal(path):
    lines = read(path).split("\n")[3:]
    cols = []
    for line in lines[:16]:
        r, g, b = map(int, line.split())
        cols.append((r, g, b))
    return cols


def load_tiles(path):
    im = Image.open(os.path.join(ROOT, path))
    if im.mode != "P":
        raise SystemExit(path + " is not indexed")
    w, h = im.size
    px = im.load()
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tiles.append([[px[tx * 8 + x, ty * 8 + y] & 15 for x in range(8)] for y in range(8)])
    return tiles


def load_u16(path):
    data = open(os.path.join(ROOT, path), "rb").read()
    return [data[i] | data[i + 1] << 8 for i in range(0, len(data), 2)]


def render(layout_name, marks=(), lights=None, night=False, index=None):
    layouts = json.loads(read("data/layouts/layouts.json"))["layouts"]
    lay = next(l for l in layouts if l.get("name") == layout_name or l.get("id") == layout_name)
    prim = tileset_paths(lay["primary_tileset"])
    sec = tileset_paths(lay["secondary_tileset"])
    tiles = load_tiles(prim[0])[:512]
    tiles += [[[0] * 8 for _ in range(8)]] * (512 - len(tiles))
    tiles += load_tiles(sec[0])
    ppal = [load_pal(p) for p in prim[1]]
    spal = [load_pal(p) for p in sec[1]]
    pals = [list(ppal[i]) for i in range(6)] + [list(spal[i]) for i in range(6, 13)]
    lit = set()
    for (which, p, i) in (lights or []):
        lit.add((p, i))
    if night:
        for p in range(len(pals)):
            for i in range(16):
                r, g, b = pals[p][i]
                if (p, i) in lit:
                    tr, tg, tb = 248, 224, 120
                    pals[p][i] = (r + (tr - r) * 20 // 32, g + (tg - g) * 20 // 32, b + (tb - b) * 20 // 32)
                else:
                    pals[p][i] = (int(r * 0.456), int(g * 0.456), int(b * 0.615))
    for (which, p, i) in marks:
        pals[p][i] = (255, 0, 255)
    metas = load_u16(prim[2])[:512 * 8]
    metas += [0] * (512 * 8 - len(metas))
    metas += load_u16(sec[2])
    w, h = lay["width"], lay["height"]
    block = load_u16(lay["blockdata_filepath"])
    out = Image.new("RGB", (w * 16, h * 16))
    px = out.load()
    for my in range(h):
        for mx in range(w):
            mid = block[my * w + mx] & 0x3FF
            for layer in range(2):
                for q in range(4):
                    e = metas[mid * 8 + layer * 4 + q] if mid * 8 + layer * 4 + q < len(metas) else 0
                    t, hf, vf, pal = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
                    if t >= len(tiles) or pal >= len(pals):
                        continue
                    tile = tiles[t]
                    ox, oy = mx * 16 + (q & 1) * 8, my * 16 + (q >> 1) * 8
                    for y in range(8):
                        for x in range(8):
                            c = tile[7 - y if vf else y][7 - x if hf else x]
                            if c == 0 and layer == 1:
                                continue
                            if c == 0 and layer == 0:
                                px[ox + x, oy + y] = pals[0][0]
                                if index is not None:
                                    index[ox + x, oy + y] = (0, 0)
                                continue
                            px[ox + x, oy + y] = pals[pal][c]
                            if index is not None:
                                index[ox + x, oy + y] = (pal, c)
    return out


def parse_lights():
    """(tileset symbol, palette, index) from src/data/daynight_lights.h."""
    text = read("src/data/daynight_lights.h")
    out = {}
    for sym, body in re.findall(r"\{\s*&(gTileset_\w+),\s*\{(.*?)\}\s*\}", text, re.S):
        out[sym] = [(sym, int(p), int(i)) for p, i in re.findall(r"LIGHT\((\d+),\s*(\d+)\)", body)]
    return out


if __name__ == "__main__":
    args = sys.argv[1:]
    name, path = args[0], args[1]
    marks = []
    night = "--night" in args
    if "--mark" in args:
        for m in args[args.index("--mark") + 1:]:
            if m.startswith("--"):
                break
            t, p, i = m.split(":")
            marks.append((t, int(p), int(i)))
    lights = []
    if night:
        layouts = json.loads(read("data/layouts/layouts.json"))["layouts"]
        lay = next(l for l in layouts if l.get("name") == name or l.get("id") == name)
        table = parse_lights()
        lights = table.get(lay["primary_tileset"], []) + table.get(lay["secondary_tileset"], [])
    render(name, marks, lights, night).save(path)
