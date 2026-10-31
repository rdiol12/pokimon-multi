#!/usr/bin/env python3
"""Builds the base layout of the Rift Below (data/layouts/AdventureDungeon/):
32x32 of solid cave rock (gTileset_General + gTileset_Cave, as in Altering
Cave). src/dungeon.c draws a new floor over it every time the player goes
down, so this file only has to be solid where nothing is dug out."""
import os
import struct

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
DST = os.path.join(ROOT, "data", "layouts", "AdventureDungeon")

SOLID = 0x0400 | 0x211  # cave rock, impassable
W, H = 32, 32

os.makedirs(DST, exist_ok=True)
with open(os.path.join(DST, "map.bin"), "wb") as f:
    f.write(struct.pack("<%dH" % (W * H), *([SOLID] * (W * H))))
with open(os.path.join(DST, "border.bin"), "wb") as f:
    f.write(struct.pack("<4H", *([SOLID] * 4)))
