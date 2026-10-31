#!/usr/bin/env python3
"""Draws the fifth batch's original sprites as indexed 16-colour PNGs
(graphics/life/). Colours sit on the GBA's 5-bit grid and follow Emerald's
style: a dark 1px outline, a flat base, one shadow and one highlight tone,
light from the top left. Index 0 is the transparent colour."""
import os
import sys
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, "..", "..", "graphics", "life")


def png(name, palette, rows, keys):
    h, w = len(rows), len(rows[0])
    assert all(len(r) == w for r in rows), name
    img = Image.new("P", (w, h))
    flat = []
    for c in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += c
    img.putpalette(flat)
    index = {ch: i for i, ch in enumerate(keys)}
    img.putdata([index[ch] for row in rows for ch in row])
    os.makedirs(os.path.dirname(os.path.join(out, name)), exist_ok=True)
    img.save(os.path.join(out, name + ".png"))


# --- Bonding (src/life_bond.c) ---------------------------------------------
# One palette for the hand and the hearts.
# . transparent  o outline  w glove white  g glove shadow  G glove dark
# c cuff blue  C cuff shadow  r heart red  R heart shadow  p heart pink
# h highlight  e empty heart  E empty shadow
BOND_KEYS = ".owgGcCrRpheE"
BOND_PAL = [(0x78, 0xC8, 0xF8), (0x30, 0x30, 0x38), (0xF8, 0xF8, 0xF8), (0xC8, 0xC8, 0xD0),
            (0x98, 0x98, 0xA8), (0x50, 0x88, 0xE0), (0x30, 0x58, 0xA8), (0xE8, 0x40, 0x58),
            (0xA8, 0x20, 0x38), (0xF8, 0x90, 0xA0), (0xF8, 0xE0, 0xE8), (0xD8, 0xD0, 0xD0),
            (0xA0, 0x98, 0x98)]
HAND = [
    "....oo..........",
    "...owgo.........",
    "...owgo.........",
    "...owgo.........",
    "...owgoooo......",
    "...owgowgooo....",
    ".oooowgowgowgo..",
    "owwgowwwwgowgoo.",
    "owwwowwwwwwwwgGo",
    ".owwwwwwwwwwwgGo",
    "..owwwwwwwwwggGo",
    "..owwwwwwwwwgGo.",
    "...owwwwwwwggGo.",
    "....occcccccCo..",
    "....occcccccCo..",
    ".....oooooooo...",
]
png("bond_hand", BOND_PAL, HAND, BOND_KEYS)

HEART_BIG = [
    "................",
    "................",
    "..ooo.....ooo...",
    ".ohhpo...oprro..",
    "ohhpprro.orrrRo.",
    "ohprrrrrorrrrRo.",
    "oprrrrrrrrrrrRo.",
    "oprrrrrrrrrrrRo.",
    ".orrrrrrrrrrRo..",
    "..orrrrrrrrRo...",
    "...orrrrrrRo....",
    "....orrrrRo.....",
    ".....orrRo......",
    "......oRo.......",
    ".......o........",
    "................",
]
png("bond_heart", BOND_PAL, HEART_BIG, BOND_KEYS)

# 16x8: a full heart and an empty one, for the meter.
HEARTS_SMALL = [
    ".oo.oo...oo.oo..",
    "ohproRo.oeeoeEo.",
    "oprrrRo.oeeeeEo.",
    "orrrrRo.oeeeeEo.",
    ".orrRo...oeeEo..",
    "..oRo.....oEo...",
    "...o.......o....",
    "................",
]
png("bond_hearts_small", BOND_PAL, HEARTS_SMALL, BOND_KEYS)

if __name__ == "__main__" and len(sys.argv) > 1:
    print("wrote", out)
