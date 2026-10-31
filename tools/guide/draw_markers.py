"""Draws the guide map's markers (graphics/guide/markers.png): a yellow "!"
bubble for something new (two frames, the second with a glint) and a red
pin for a quest to finish. 16x16 each, stacked, in Emerald's style: 1px dark
outline, light from the top left, colors from the region map icons."""
from PIL import Image

PAL = [
    (0, 255, 0),       # 0 transparent
    (41, 41, 41),      # 1 outline
    (255, 255, 255),   # 2 white
    (255, 222, 82),    # 3 yellow
    (222, 164, 41),    # 4 yellow shade
    (238, 82, 57),     # 5 red
    (172, 41, 41),     # 6 red shade
    (255, 255, 197),   # 7 yellow light
    (255, 156, 131),   # 8 red light
    (98, 98, 98),      # 9 shadow
] + [(0, 0, 0)] * 6

BUBBLE = [
    "................",
    "................",
    "................",
    "......1111......",
    ".....173341.....",
    "....17336341....",
    "....13336341....",
    "....13336341....",
    "....13333441....",
    "....13336441....",
    ".....134441.....",
    "......1441......",
    ".......11.......",
    "................",
    "................",
    "................",
]

PIN = [
    "................",
    "................",
    "................",
    "......1111......",
    ".....182551.....",
    ".....125551.....",
    ".....155561.....",
    "......1561......",
    ".......161......",
    ".......11.......",
    ".......9........",
    "................",
    "................",
    "................",
    "................",
    "................",
]


def frame(rows, glint=False):
    img = []
    for y, row in enumerate(rows):
        line = []
        for x, c in enumerate(row):
            v = 0 if c == "." else int(c)
            if glint and v == 3 and (x + y) % 5 == 0 and y < 9:
                v = 7
            line.append(v)
        img.append(line)
    return img


frames = [frame(BUBBLE), frame(BUBBLE, glint=True), frame(PIN)]
out = Image.new("P", (16, 16 * len(frames)))
out.putpalette([c for rgb in PAL for c in rgb])
for i, f in enumerate(frames):
    for y in range(16):
        for x in range(16):
            out.putpixel((x, 16 * i + y), f[y][x])
out.save("graphics/guide/markers.png")
out.resize((64, 64 * len(frames)), Image.NEAREST).convert("RGB").save("/tmp/markers_big.png")
