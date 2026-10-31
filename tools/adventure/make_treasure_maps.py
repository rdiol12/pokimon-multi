#!/usr/bin/env python3
"""Draws the treasure maps for the treasure hunts (src/treasure.c,
docs/adventure.md): for each hiding spot, the 7x5 metatiles around it
from the real map, inked on old parchment, with a red X on the spot.

Writes graphics/adventure/treasure_maps/<n>.png (112x80, 16 colours). The
spots are in the same order as sTreasureSpots in src/treasure.c."""
import json
import os

from PIL import Image, ImageDraw

import render_map

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "graphics", "adventure", "treasure_maps")

SPOTS = [
    ("OldaleTown", 10, 10),
    ("PetalburgCity", 16, 11),
    ("Route104", 19, 53),
    ("DewfordTown", 12, 15),
    ("SlateportCity", 20, 32),
    ("Route113", 30, 8),
    ("MossdeepCity", 33, 22),
    ("Route119", 9, 35),
    ("FallarborTown", 13, 10),
    ("Route123", 24, 9),
    ("LilycoveCity", 22, 27),
    ("Route116", 40, 11),
]

W, H = 7, 5  # metatiles
# Parchment, dark ink to pale paper.
SEPIA = [(56, 32, 16), (88, 56, 32), (112, 80, 48), (136, 104, 64), (160, 128, 80),
         (184, 152, 104), (200, 176, 128), (216, 192, 144), (232, 208, 160), (240, 224, 184)]
RED, RED_DARK, EDGE = (216, 48, 32), (136, 24, 24), (72, 40, 24)
PALETTE = [(255, 0, 255)] + SEPIA + [RED, RED_DARK, EDGE] + [(0, 0, 0)] * 2


def draw(layout, x, y):
    full = render_map.render(layout)
    crop = full.crop(((x - 3) * 16, (y - 2) * 16, (x + 4) * 16, (y + 3) * 16)).convert("RGB")
    img = Image.new("P", crop.size)
    flat = [c for col in PALETTE for c in col]
    img.putpalette(flat + [0] * (768 - len(flat)))
    px = img.load()
    src = crop.load()
    for j in range(crop.height):
        for i in range(crop.width):
            r, g, b = src[i, j]
            lum = (r * 3 + g * 6 + b) / 10 / 255
            # A little contrast, so paths and walls stand out on the paper.
            lum = min(max((lum - 0.2) / 0.75, 0), 1)
            px[i, j] = 1 + min(int(lum * len(SEPIA)), len(SEPIA) - 1)
    d = ImageDraw.Draw(img)
    # The X on the middle metatile, with a darker shadow.
    cx, cy = 3 * 16, 2 * 16
    for off, col in ((1, 12), (0, 11)):
        d.line([(cx + 2 + off, cy + 2 + off), (cx + 13 + off, cy + 13 + off)], fill=col, width=3)
        d.line([(cx + 13 + off, cy + 2 + off), (cx + 2 + off, cy + 13 + off)], fill=col, width=3)
    d.rectangle([0, 0, crop.width - 1, crop.height - 1], outline=13)
    # Worn corners.
    for (a, b) in ((0, 0), (crop.width - 1, 0), (0, crop.height - 1), (crop.width - 1, crop.height - 1)):
        for dx, dy in ((0, 0), (1 if a == 0 else -1, 0), (0, 1 if b == 0 else -1)):
            px[a + dx, b + dy] = 0
    return img


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for n, (name, x, y) in enumerate(SPOTS):
        layout = json.load(open(os.path.join(ROOT, "data", "maps", name, "map.json")))["layout"]
        draw(layout, x, y).save(os.path.join(OUT, "%d.png" % n), transparency=0)
