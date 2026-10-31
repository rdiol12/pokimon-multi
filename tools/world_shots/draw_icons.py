#!/usr/bin/env python3
"""Draws the world features' small original icons as indexed 16-colour PNGs
(graphics/world/). Colours are on the GBA's 5-bit grid and follow Emerald's
item-icon style: a dark outline, a flat base, one shadow and one highlight
tone, light from the top left."""
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, "..", "..", "graphics", "world")

def png(name, palette, rows, keys=".odbshwgGx"):
    h, w = len(rows), len(rows[0])
    img = Image.new("P", (w, h))
    flat = []
    for c in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += c
    img.putpalette(flat)
    index = {ch: i for i, ch in enumerate(keys)}
    img.putdata([index[ch] for row in rows for ch in row])
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

# Timing meter (fishing reel, berry picking): a groove with a dark outline,
# the green "hit" zone drawn as 8-pixel tiles over it, and the needle.
# . transparent  o outline  r red end  R red shadow  y groove  Y groove shadow
# g green  G green shadow  l green light  w white  n needle  N needle shadow
METER_PAL = [(0x78, 0xC8, 0xF8), (0x38, 0x38, 0x48), (0xE8, 0x50, 0x38), (0xA8, 0x30, 0x20),
             (0xF0, 0xE0, 0x98), (0xC8, 0xB0, 0x68), (0x58, 0xC0, 0x48), (0x30, 0x88, 0x30),
             (0xA0, 0xE8, 0x78), (0xF8, 0xF8, 0xF8), (0xF8, 0x60, 0x40), (0x90, 0x28, 0x18)]
METER_KEYS = ".orRyYgGlwnN"
bar = []
for y in range(8):
    if y in (0, 7):
        bar.append("." + "o" * 62 + ".")
        continue
    row = "o"
    for x in range(1, 63):
        end = x < 7 or x > 56
        if y == 1:
            row += "w" if not end else "r"
        elif y == 6:
            row += "R" if end else "Y"
        else:
            row += ("r" if y < 4 else "R") if end else ("y" if y < 5 else "Y")
    bar.append(row + "o")
png("meter_bar", METER_PAL, bar, METER_KEYS)
png("meter_zone", METER_PAL, [
    "oooooooo",
    "llllllll",
    "glglglgl",
    "gggggggg",
    "gggggggg",
    "GGGGGGGG",
    "GGGGGGGG",
    "oooooooo",
], METER_KEYS)
png("meter_needle", METER_PAL, [
    "..oooo..",
    "..onwo..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onno..",
    "..onNo..",
    ".onnNNo.",
    ".onNNNo.",
    "..oNNo..",
    "...oo...",
    "........",
], METER_KEYS)
