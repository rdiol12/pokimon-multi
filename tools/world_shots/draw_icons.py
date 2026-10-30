#!/usr/bin/env python3
"""Draws the world features' small original icons as indexed 16-colour PNGs
(graphics/world/). Colours are on the GBA's 5-bit grid and follow Emerald's
item-icon style: a dark outline, a flat base, one shadow and one highlight
tone, light from the top left."""
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, "..", "..", "graphics", "world")

def png(name, palette, rows):
    h, w = len(rows), len(rows[0])
    img = Image.new("P", (w, h))
    flat = []
    for c in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += c
    img.putpalette(flat)
    keys = {ch: i for i, ch in enumerate(".odbshwgGx")}
    img.putdata([keys[ch] for row in rows for ch in row])
    img.save(os.path.join(out, name + ".png"))

# . transparent  o outline  d gold shadow  b gold  s light gold  h highlight
# w white  g stand  G stand shadow  x unused
TROPHY_PAL = [(0x78, 0xC8, 0xF8), (0x48, 0x30, 0x10), (0xC0, 0x80, 0x10), (0xF0, 0xB8, 0x28),
              (0xF8, 0xE0, 0x68), (0xF8, 0xF8, 0xC8), (0xF8, 0xF8, 0xF8), (0x70, 0x58, 0x48),
              (0x48, 0x38, 0x30)]
TROPHY = [
    "................",
    "...oooooooooo...",
    ".oo.ohhssbbdo.oo",
    "o..ohsssbbbbdo.o",
    "o..oshssbbbbdo.o",
    ".o.osssbbbbbdo.o",
    "..ooshsbbbbbdoo.",
    "....osbbbbbddo..",
    ".....osbbbddo...",
    "......osbbdo....",
    ".......obdo.....",
    ".......obdo.....",
    "......osbbdo....",
    ".....oggggGGo...",
    "....ogggggGGGo..",
    "....ooooooooooo.",
]
png("trophy", TROPHY_PAL, TROPHY)
