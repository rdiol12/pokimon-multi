#!/usr/bin/env python3
"""Draws the house furniture (graphics/world/furniture/) as indexed PNGs
that share one 16-colour palette. Emerald's indoor style: a dark brown
outline, flat colours with one shadow and one highlight tone, light from
the top left, and a soft shadow line along the bottom edge."""
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(here, "..", "..", "graphics", "world", "furniture")

PAL = [
    (0x78, 0xC8, 0xF8),  # 0 transparent
    (0x48, 0x30, 0x28),  # 1 o outline
    (0x90, 0x58, 0x30),  # 2 D wood shadow
    (0xC0, 0x88, 0x48),  # 3 W wood
    (0xE8, 0xB8, 0x70),  # 4 L wood light
    (0x30, 0x80, 0x40),  # 5 g green shadow
    (0x58, 0xB8, 0x50),  # 6 G green
    (0xA0, 0xE0, 0x78),  # 7 l green light
    (0xF8, 0xF8, 0xF8),  # 8 w white
    (0xC8, 0xC8, 0xD8),  # 9 s silver
    (0x80, 0x80, 0x98),  # 10 S grey
    (0xE8, 0x50, 0x40),  # 11 r red
    (0xA0, 0x28, 0x28),  # 12 R red shadow
    (0x60, 0xA8, 0xF0),  # 13 b blue
    (0x30, 0x60, 0xB0),  # 14 B blue shadow
    (0xF8, 0xD0, 0x50),  # 15 y gold
]
KEYS = ".oDWLgGlwsSrRbBy"


class Canvas:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [[0] * w for _ in range(h)]

    def put(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = KEYS.index(c)

    def rect(self, x0, y0, x1, y1, c):  # inclusive
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.put(x, y, c)

    def box(self, x0, y0, x1, y1, fill, light=None, shadow=None):
        """A filled box with an outline, a highlight on the top/left inside
        edge and a shadow on the bottom/right inside edge."""
        self.rect(x0, y0, x1, y1, "o")
        self.rect(x0 + 1, y0 + 1, x1 - 1, y1 - 1, fill)
        if light:
            self.rect(x0 + 1, y0 + 1, x1 - 1, y0 + 1, light)
            self.rect(x0 + 1, y0 + 1, x0 + 1, y1 - 1, light)
        if shadow:
            self.rect(x0 + 1, y1 - 1, x1 - 1, y1 - 1, shadow)
            self.rect(x1 - 1, y0 + 2, x1 - 1, y1 - 1, shadow)

    def art(self, x0, y0, rows):
        for dy, row in enumerate(rows):
            for dx, ch in enumerate(row):
                if ch != ".":
                    self.put(x0 + dx, y0 + dy, ch)

    def save(self, name):
        img = Image.new("P", (self.w, self.h))
        flat = []
        for c in PAL:
            flat += c
        img.putpalette(flat)
        img.putdata([v for row in self.px for v in row])
        img.save(os.path.join(out, name + ".png"))


def plant():
    c = Canvas(16, 32)
    c.art(0, 2, [
        "......oooo......",
        "....ooGGlGoo....",
        "...oGGlllGGGo...",
        "..oGlllGGGGgGo..",
        ".oGGllGGGgGGGgo.",
        ".oGlGGGgGGlGGgo.",
        "oGGGGgGGGllGggGo",
        "oGlGGGGgGGGGGgGo",
        "oGllGgGGlGGgGggo",
        "oGGGGGGGllGGggGo",
        ".oGgGGlGGGGgggo.",
        ".ogGGGGGgGgGggo.",
        "..oggGgGGggggo..",
        "...ooggggggoo...",
        ".....oo5go......".replace("5", "g"),
        "......ogo.......",
    ])
    c.box(3, 18, 12, 29, "r", "w", "R")
    c.rect(2, 18, 13, 20, "o")
    c.rect(3, 19, 12, 19, "r")
    c.rect(3, 19, 4, 19, "w")
    c.rect(4, 30, 11, 30, "o")
    c.rect(5, 31, 10, 31, "D")
    return c


def bookcase():
    c = Canvas(16, 32)
    c.box(0, 2, 15, 31, "W", "L", "D")
    books = ["rRbBGgyDrRsSbB", "GgrRyDbBsSrRGg", "bByDrRGgsSbByD"]
    for i, shelf_y in enumerate((5, 13, 21)):
        c.rect(2, shelf_y, 13, shelf_y + 6, "o")
        for x in range(12):
            col = books[i][x]
            top = shelf_y + (1 if x % 3 else 2)
            c.rect(2 + x, top, 2 + x, shelf_y + 6, col)
        c.rect(2, shelf_y + 7, 13, shelf_y + 7, "L")
    c.rect(1, 30, 14, 30, "D")
    return c


def table():
    c = Canvas(32, 32)
    c.box(1, 6, 30, 20, "W", "L", "D")
    for x in range(4, 28, 6):
        c.rect(x, 9, x + 3, 9, "L")
    c.rect(1, 21, 30, 23, "o")
    c.rect(2, 21, 29, 22, "D")
    for x in (3, 26):
        c.box(x, 21, x + 3, 29, "D", None, None)
    c.rect(4, 30, 28, 30, "o")
    return c


def stool():
    c = Canvas(16, 16)
    c.box(2, 3, 13, 9, "r", "w", "R")
    c.rect(2, 10, 13, 10, "o")
    c.box(3, 10, 5, 15, "D")
    c.box(10, 10, 12, 15, "D")
    c.rect(4, 10, 11, 11, "o")
    return c


def bed():
    c = Canvas(16, 32)
    c.box(0, 0, 15, 8, "W", "L", "D")       # headboard
    c.box(1, 6, 14, 30, "w", None, "s")     # mattress
    c.box(3, 8, 12, 13, "w", None, "s")     # pillow
    c.box(1, 15, 14, 30, "b", "w", "B")     # blanket
    c.rect(2, 17, 13, 17, "w")
    for y in (21, 25):
        c.rect(3, y, 12, y, "B")
    c.rect(1, 31, 14, 31, "o")
    return c


def tv():
    c = Canvas(16, 32)
    c.box(0, 4, 15, 17, "S", "s", "o")      # casing
    c.box(2, 6, 12, 15, "B", None, None)    # screen
    c.rect(3, 7, 5, 7, "b")
    c.rect(3, 8, 3, 9, "b")
    c.put(14, 7, "r")
    c.put(14, 9, "y")
    c.rect(4, 1, 4, 3, "o")                 # antenna
    c.rect(11, 1, 11, 3, "o")
    c.rect(6, 3, 9, 3, "o")
    c.box(1, 18, 14, 30, "W", "L", "D")     # stand
    c.rect(3, 22, 12, 22, "D")
    c.rect(3, 26, 12, 26, "D")
    c.rect(7, 23, 8, 23, "y")
    c.rect(7, 27, 8, 27, "y")
    c.rect(2, 31, 13, 31, "o")
    return c


def lamp():
    c = Canvas(16, 32)
    c.art(1, 1, [
        "....oooooo....",
        "...oywwwyyo...",
        "..oywwyyyyyo..",
        "..owyyyyyyDo..",
        ".oyyyyyyyyyDo.",
        ".owyyyyyyyyDo.",
        "oyyyyyyyyyyyDo",
        "oooooooooooooo",
    ])
    c.rect(7, 9, 8, 26, "o")
    c.rect(7, 9, 7, 26, "s")
    c.box(3, 26, 12, 30, "S", "s", "o")
    c.rect(4, 31, 11, 31, "o")
    return c


def aquarium():
    c = Canvas(32, 32)
    c.box(0, 2, 31, 22, "b", "w", "B")      # glass
    c.rect(1, 3, 30, 4, "s")                # lid
    c.rect(1, 5, 30, 5, "o")
    for x, y, col in ((7, 10, "y"), (19, 14, "r"), (24, 8, "y")):
        c.art(x, y, [".oo.o", "o" + col + col + "oo", ".oo.o"])
    for x in (4, 5, 26):
        c.rect(x, 14, x, 20, "g")
        c.put(x + 1, 16, "G")
    c.rect(1, 20, 30, 21, "L")              # sand
    c.box(1, 22, 30, 30, "W", "L", "D")     # cabinet
    c.rect(15, 23, 16, 29, "D")
    c.rect(2, 31, 29, 31, "o")
    return c


def cushion():
    c = Canvas(16, 16)
    c.art(0, 1, [
        "....oooooooo....",
        "..oorrrrrrrroo..",
        ".orrwrrrrrrrrRo.",
        ".orwrrrrrrrrrRo.",
        "orrrrrrrrrrrrrRo",
        "orrrrrrrrrrrrRRo",
        "oooooooowwoooooo",
        "owwwwwwowwowwwso",
        "owwwwwwwoowwwsso",
        ".owwwwwwwwwwsso.",
        ".osswwwwwwwssso.",
        "..oossssssssoo..",
        "....oooooooo....",
    ])
    c.rect(4, 15, 11, 15, "D")
    return c


def trophies():
    c = Canvas(16, 32)
    c.box(0, 4, 15, 31, "W", "L", "D")
    for shelf_y in (7, 18):
        c.rect(2, shelf_y, 13, shelf_y + 8, "B")
        c.rect(2, shelf_y + 9, 13, shelf_y + 9, "L")
    for x, y in ((3, 9), (9, 9), (6, 20)):
        c.art(x, y, ["oyyyo", "oyLyo", ".oyo.", "..y..", ".oDo.", "oDDDo"])
    c.rect(1, 30, 14, 30, "D")
    return c


def telescope():
    c = Canvas(16, 32)
    c.art(0, 2, [
        "............oo..",
        "..........oowwo.",
        "........oosbbwo.",
        "......oosbbbbo..",
        "....oosbbbbBo...",
        "..oosbbbbBBo....",
        ".osbbbbBBoo.....",
        "osbbbBBoo.......",
        "oSbBBoo.........",
        ".oSoo...........",
    ])
    c.rect(6, 10, 9, 12, "o")
    c.rect(7, 11, 8, 11, "s")
    for y in range(13, 29):
        d = (y - 13) // 3
        c.put(7 - d, y, "o")
        c.put(8 + d, y, "o")
        c.put(7, y, "D")
        c.put(8, y, "W")
    c.rect(1, 29, 14, 29, "o")
    return c


FURNITURE = {
    "plant": plant, "bookcase": bookcase, "table": table, "stool": stool,
    "bed": bed, "tv": tv, "lamp": lamp, "aquarium": aquarium,
    "cushion": cushion, "trophies": trophies, "telescope": telescope,
}

if __name__ == "__main__":
    os.makedirs(out, exist_ok=True)
    for name, draw in FURNITURE.items():
        draw().save(name)
