#!/usr/bin/env python3
"""Builds the player's house on Starfall Isle (data/layouts/
StarfallIsle_PlayerHouse/): a 13x10 room from the standard house metatiles
(gTileset_Building + gTileset_GenericBuilding), copied tile-for-tile from
the House1 layout so walls, floor and collision match the other houses.
The furniture is not part of the map: players place it (src/world_house.c)."""
import os, struct

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
src = os.path.join(root, "data", "layouts", "House1")
dst = os.path.join(root, "data", "layouts", "StarfallIsle_PlayerHouse")

house1 = open(os.path.join(src, "map.bin"), "rb").read()
def h1(x, y):  # a metatile word (id, collision, elevation) from House1
    return struct.unpack_from("<H", house1, (y * 10 + x) * 2)[0]

WALL_TOP, WALL_BOTTOM = h1(4, 0), h1(4, 1)
WINDOW_TOP, WINDOW_BOTTOM = h1(5, 0), h1(5, 1)
BASEBOARD, FLOOR, FLOOR_LEFT = h1(4, 2), h1(1, 7), h1(0, 7)
MAT_LEFT, MAT_RIGHT = h1(3, 8), h1(4, 8)
RUG = [[h1(1, 3), h1(2, 3), h1(7, 3)],
       [h1(1, 4), h1(5, 4), h1(7, 4)],
       [h1(1, 6), h1(2, 6), h1(7, 6)]]

W, H = 13, 10
DOOR_X = 6
grid = [[FLOOR] * W for _ in range(H)]
for x in range(W):
    window = x in (3, 9)
    grid[0][x] = WINDOW_TOP if window else WALL_TOP
    grid[1][x] = WINDOW_BOTTOM if window else WALL_BOTTOM
    grid[2][x] = BASEBOARD
for y in range(3, H):
    grid[y][0] = FLOOR_LEFT
# A rug in the middle, 5 wide and 3 deep.
for dy in range(3):
    for x in range(4, 9):
        col = 0 if x == 4 else 2 if x == 8 else 1
        grid[5 + dy][x] = RUG[dy][col]
grid[H - 1][DOOR_X], grid[H - 1][DOOR_X + 1] = MAT_LEFT, MAT_RIGHT

os.makedirs(dst, exist_ok=True)
with open(os.path.join(dst, "map.bin"), "wb") as f:
    for row in grid:
        f.write(struct.pack("<%dH" % W, *row))
with open(os.path.join(dst, "border.bin"), "wb") as f:
    f.write(open(os.path.join(src, "border.bin"), "rb").read())
