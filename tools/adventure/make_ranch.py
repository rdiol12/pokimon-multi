#!/usr/bin/env python3
"""Builds the Hoenn Ranch behind the Route 117 Day Care (data/layouts/
AdventureRanch/): a 24x18 meadow ringed by forest, with the ranch house,
a stone-fenced pasture where the ranch Pokémon wander, a flower bed and a
sand path from the way back to Route 117 at the bottom.

Every metatile is copied, collision and elevation included, from Route
117 (same tilesets: gTileset_General + gTileset_Mauville), so the ranch
looks like the Day Care's own surroundings."""
import os
import struct

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SRC = os.path.join(ROOT, "data", "layouts", "Route117")
DST = os.path.join(ROOT, "data", "layouts", "AdventureRanch")

r117 = open(os.path.join(SRC, "map.bin"), "rb").read()


def m(x, y):
    """A metatile word (id, collision, elevation) from Route 117."""
    return struct.unpack_from("<H", r117, (y * 60 + x) * 2)[0]


GRASS, FLOWERS = m(18, 2), m(9, 2)
FOREST = [m(0, 0), m(1, 0)]          # canopy, two variants
FOREST_BOTTOM = [m(1, 5), m(8, 0)]   # canopy over trunks, facing south
FOREST_TOP = m(37, 18)               # tree crowns over grass, facing north
SIGN = m(49, 5)
HOUSE = [[m(50 + dx, 2 + dy) for dx in range(4)] for dy in range(4)]  # the Day Care's house
# Stone fence (the Route 117 flower garden): corners, sides, bottom.
FENCE_TL, FENCE_T, FENCE_TR = m(7, 3), m(8, 3), m(14, 3)
FENCE_L, FENCE_R = m(7, 4), m(14, 4)
FENCE_BL, FENCE_BR = m(7, 7), m(14, 7)
FLOWER_BED = [[m(8 + dx, 4 + dy) for dx in range(6)] for dy in range(3)]


def find(metatile):
    """The first Route 117 word with this metatile id."""
    for i in range(len(r117) // 2):
        word = struct.unpack_from("<H", r117, i * 2)[0]
        if word & 0x3FF == metatile:
            return word
    raise KeyError(metatile)


# Sand path, a 3x3 set: corners, edges and middle.
PATH = [[find(0x118), find(0x119), find(0x11A)],
        [find(0x120), find(0x121), find(0x122)],
        [find(0x128), find(0x129), find(0x12A)]]

W, H = 24, 18
grid = [[GRASS] * W for _ in range(H)]

# Forest all around; an opening at the bottom middle for the way back.
for y in range(H):
    for x in range(W):
        edge = x < 2 or x >= W - 2 or y < 2 or y >= H - 1
        if edge:
            grid[y][x] = FOREST[(x + y) % 2]
for x in range(2, W - 2):
    grid[2][x] = FOREST_BOTTOM[x % 2]
    grid[H - 2][x] = FOREST_TOP
for x in (13, 14, 15):
    grid[H - 2][x] = GRASS
    grid[H - 1][x] = GRASS

# Some flowers in the meadow.
for x, y in [(3, 10), (8, 10), (10, 15), (21, 13), (21, 4), (3, 15), (8, 4), (18, 3)]:
    grid[y][x] = FLOWERS

# The ranch house, door at (5, 6), and its sign.
for dy in range(4):
    for dx in range(4):
        grid[3 + dy][3 + dx] = HOUSE[dy][dx]
grid[6][7] = SIGN

# The pasture: stone fence from (9, 4) to (20, 12), open at the bottom middle.
X0, Y0, X1, Y1 = 9, 4, 20, 12
for x in range(X0 + 1, X1):
    grid[Y0][x] = FENCE_T
    grid[Y1][x] = FENCE_T
for y in range(Y0 + 1, Y1):
    grid[y][X0] = FENCE_L
    grid[y][X1] = FENCE_R
grid[Y0][X0], grid[Y0][X1] = FENCE_TL, FENCE_TR
grid[Y1][X0], grid[Y1][X1] = FENCE_BL, FENCE_BR
for x in (14, 15):
    grid[Y1][x] = GRASS

# A flower bed south-east of the pasture.
for dy in range(2):
    for dx in range(3):
        grid[14 + dy][17 + dx] = FLOWER_BED[dy][dx + 1]

# Sand path from the way out to the pasture gate and to the house door,
# laid as a mask and edged by its neighbours.
mask = set()
for x0, y0, x1, y1 in [(13, 13, 15, 17), (4, 7, 6, 15), (4, 13, 15, 15)]:
    mask |= {(x, y) for x in range(x0, x1 + 1) for y in range(y0, y1 + 1)}
for (x, y) in mask:
    row = 0 if (x, y - 1) not in mask else 2 if (x, y + 1) not in mask and y + 1 < H else 1
    col = 0 if (x - 1, y) not in mask else 2 if (x + 1, y) not in mask else 1
    grid[y][x] = PATH[row][col]

os.makedirs(DST, exist_ok=True)
with open(os.path.join(DST, "map.bin"), "wb") as f:
    for row in grid:
        f.write(struct.pack("<%dH" % W, *row))
with open(os.path.join(DST, "border.bin"), "wb") as f:
    f.write(struct.pack("<4H", FOREST[0], FOREST[1], FOREST[1], FOREST[0]))
print("wrote", DST)
