#!/usr/bin/env python3
"""Draws the original art for the second online batch (docs/netplay_ext.md)
in the style of Emerald's own menu art: a dark 1px outline around every
part, a light edge on the top-left and a shaded edge on the bottom-right,
and small 16-colour palettes.

Writes indexed PNGs into graphics/netplay_ext/ (the build turns them into
.4bpp/.gbapal) plus the Online Plaza tilemap (.bin). Run from the repo root:

    python3 tools/netplay_ext_art/make_art.py
"""

import math
import os
import struct

from PIL import Image

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "graphics", "netplay_ext")


def rgb(h):
    return ((h >> 16) & 0xFF, (h >> 8) & 0xFF, h & 0xFF)


class Canvas:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [[0] * w for _ in range(h)]

    def inside(self, x, y):
        return 0 <= x < self.w and 0 <= y < self.h

    def set(self, x, y, c):
        if self.inside(x, y):
            self.px[y][x] = c

    def get(self, x, y):
        return self.px[y][x] if self.inside(x, y) else 0

    def part(self, mask, mat, outline=1, shade=True):
        """Paints one part: mask is a set of (x, y). mat = (light, base, dark).
        Edge pixels become the outline, the ring inside them is lit on the
        top-left and shaded on the bottom-right."""
        light, base, dark = mat
        edge = {(x, y) for (x, y) in mask
                if any((x + dx, y + dy) not in mask for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))}
        for (x, y) in mask:
            if (x, y) in edge:
                if outline is not None:
                    self.set(x, y, outline)
                else:
                    self.set(x, y, base)
                continue
            c = base
            if shade:
                if (x, y - 1) in edge or (x - 1, y) in edge:
                    c = light
                elif (x, y + 1) in edge or (x + 1, y) in edge:
                    c = dark
            self.set(x, y, c)

    def fill(self, mask, c):
        for (x, y) in mask:
            self.set(x, y, c)

    def blit(self, other, ox, oy):
        for y in range(other.h):
            for x in range(other.w):
                if other.px[y][x]:
                    self.set(ox + x, oy + y, other.px[y][x])

    def image(self, palette):
        im = Image.new("P", (self.w, self.h))
        flat = []
        for c in palette:
            flat.extend(rgb(c))
        flat.extend([0] * (768 - len(flat)))
        im.putpalette(flat)
        im.putdata([c for row in self.px for c in row])
        return im


# Shapes (sets of pixels) -------------------------------------------------

def rect(x0, y0, x1, y1):
    return {(x, y) for x in range(x0, x1 + 1) for y in range(y0, y1 + 1)}


def rrect(x0, y0, x1, y1, r):
    out = set()
    for (x, y) in rect(x0, y0, x1, y1):
        cx = min(max(x, x0 + r), x1 - r)
        cy = min(max(y, y0 + r), y1 - r)
        if (x - cx) ** 2 + (y - cy) ** 2 <= r * r + r * 0.6:
            out.add((x, y))
    return out


def ellipse(cx, cy, rx, ry):
    return {(x, y) for x in range(int(cx - rx) - 1, int(cx + rx) + 2)
            for y in range(int(cy - ry) - 1, int(cy + ry) + 2)
            if ((x - cx) / (rx + 0.35)) ** 2 + ((y - cy) / (ry + 0.35)) ** 2 <= 1.0}


def circle(cx, cy, r):
    return ellipse(cx, cy, r, r)


def poly(points):
    xs = [p[0] for p in points]
    ys = [p[1] for p in points]
    out = set()
    for y in range(int(min(ys)), int(max(ys)) + 1):
        for x in range(int(min(xs)), int(max(xs)) + 1):
            px, py = x + 0.5, y + 0.5
            inside = False
            j = len(points) - 1
            for i in range(len(points)):
                xi, yi = points[i]
                xj, yj = points[j]
                if (yi > py) != (yj > py) and px < (xj - xi) * (py - yi) / (yj - yi) + xi:
                    inside = not inside
                j = i
            if inside:
                out.add((x, y))
    return out


def star(cx, cy, r_out, r_in, points=5, rot=-90):
    pts = []
    for i in range(points * 2):
        r = r_out if i % 2 == 0 else r_in
        a = math.radians(rot + i * 180 / points)
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return poly(pts)


def line(x0, y0, x1, y1):
    out = set()
    n = max(abs(x1 - x0), abs(y1 - y0), 1)
    for i in range(n + 1):
        out.add((round(x0 + (x1 - x0) * i / n), round(y0 + (y1 - y0) * i / n)))
    return out


# Feature icons (32x32), one shared palette ------------------------------

ICON_PAL = [
    0x98D0B8,  # 0 transparent
    0x383840,  # 1 outline
    0xF8F8F8,  # 2 white
    0xC8C8D0,  # 3 light gray
    0x8890A0,  # 4 gray
    0xF89878,  # 5 red light
    0xE83828,  # 6 red
    0xA01820,  # 7 red dark
    0x98D8F8,  # 8 blue light
    0x3878E8,  # 9 blue
    0x1848A8,  # 10 blue dark
    0xF8E060,  # 11 yellow
    0xE0A018,  # 12 gold
    0xA06020,  # 13 brown
    0x88E070,  # 14 green light
    0x309838,  # 15 green
]
WHITE = (2, 2, 3)
GRAY = (3, 3, 4)
STEEL = (2, 3, 4)
RED = (5, 6, 7)
BLUE = (8, 9, 10)
GOLD = (11, 12, 13)
GREEN = (14, 15, 15)
BROWN = (12, 13, 13)
SKY = (2, 8, 9)


def pokeball(c, cx, cy, r):
    ball = circle(cx, cy, r)
    c.part({p for p in ball if p[1] < cy}, RED)
    c.part({p for p in ball if p[1] >= cy}, WHITE)
    c.fill({p for p in ball if p[1] == cy}, 1)
    c.part(circle(cx, cy, max(1, r // 3)), WHITE)
    # one outline around the whole ball
    edge = {(x, y) for (x, y) in ball if any((x + dx, y + dy) not in ball for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))}
    c.fill(edge, 1)


def icon_guilds():
    c = Canvas(32, 32)
    c.part(rect(4, 27, 17, 30), BROWN)                        # base
    c.part(rect(9, 4, 11, 28), STEEL)                         # pole
    c.part(poly([(12, 5), (28, 5), (24, 12), (28, 19), (12, 19)]), RED)  # swallowtail flag
    c.part(poly([(18, 7.5), (21.5, 9), (21.5, 13), (18, 16.5), (14.5, 13), (14.5, 9)]), GOLD)  # crest
    c.part(star(18, 12, 2.6, 1.2), WHITE, outline=None, shade=False)
    c.part(circle(10, 3, 2), GOLD)                           # knob
    return c


def icon_wonder():
    c = Canvas(32, 32)
    c.part(circle(15, 15, 12), BLUE)
    land = Canvas(32, 32)
    for blob in (ellipse(10, 11, 4, 3), ellipse(18, 20, 5, 3), ellipse(20, 9, 3, 2), ellipse(8, 20, 2, 2)):
        land.fill(blob & circle(15, 15, 10), 15)
    for (x, y) in list(circle(15, 15, 10)):
        if land.get(x, y) == 15 and land.get(x - 1, y - 1) != 15:
            land.set(x, y, 14)
    c.blit(land, 0, 0)
    c.fill({(9, 7), (10, 7), (8, 8)}, 2)                     # glint
    pokeball(c, 24, 24, 6)
    c.part(star(26, 6, 4, 1.6, 4, -90), (2, 11, 12))         # sparkle
    return c


def icon_bases():
    c = Canvas(32, 32)
    c.part(rect(3, 27, 28, 30), (14, 15, 15))                # grass
    bush = ellipse(9, 17, 7, 7) | ellipse(22, 17, 7, 7) | ellipse(15, 10, 9, 8) | rect(4, 18, 27, 28)
    c.part(bush, GREEN)
    for leaf in ((10, 9), (18, 7), (7, 16), (22, 14), (13, 14)):
        c.fill({leaf, (leaf[0] + 1, leaf[1]), (leaf[0], leaf[1] + 1)}, 14)
    door = rrect(12, 18, 19, 28, 3) - rect(12, 29, 19, 31)
    c.fill(door, 1)
    c.fill(rect(14, 21, 17, 27), 13)
    c.fill({(15, 20), (16, 20)}, 13)
    return c


def icon_tower():
    c = Canvas(32, 32)
    c.part(rect(6, 26, 25, 30), STEEL)                       # base
    c.part(poly([(10, 27), (12, 8), (20, 8), (22, 27)]), SKY) # body
    c.part(rect(11, 6, 21, 9), STEEL)                        # top ring
    c.part(poly([(12, 6), (16, 1), (20, 6)]), BLUE)          # spire
    for y in (12, 17, 22):
        c.fill(rect(14, y, 15, y + 2), 10)
        c.fill(rect(17, y, 18, y + 2), 10)
    c.part(rect(14, 26, 18, 30), GOLD)                       # door
    c.part(poly([(16, 0), (22, 1), (16, 3)]), RED, outline=None)  # pennant
    return c


def icon_bounties():
    c = Canvas(32, 32)
    c.part(rect(5, 3, 25, 28), (2, 11, 12))                  # poster
    c.fill(rect(8, 6, 22, 6) | rect(10, 8, 20, 8), 13)        # "WANTED"
    pokeball(c, 15, 16, 6)
    c.fill(rect(8, 24, 22, 24), 13)
    c.part(circle(24, 25, 5), GOLD)                          # coin
    c.fill(line(24, 22, 24, 28), 13)
    c.fill({(9, 2), (21, 2)}, 4)                             # pins
    return c


def icon_titles():
    c = Canvas(32, 32)
    c.part(poly([(9, 1), (15, 1), (19, 15), (13, 15)]), BLUE)    # ribbon
    c.part(poly([(23, 1), (17, 1), (13, 15), (19, 15)]), RED)
    c.part(circle(16, 21, 9), GOLD)
    c.part(circle(16, 21, 6), (11, 12, 12), outline=13)
    c.part(star(16, 21, 4.5, 2), WHITE, outline=None)
    return c


def icon_replays():
    c = Canvas(32, 32)
    c.fill(line(10, 2, 14, 7) | line(22, 2, 18, 7), 1)        # antenna
    c.part(rect(9, 25, 22, 29), STEEL)                       # stand
    c.part(rrect(2, 6, 29, 26, 3), STEEL)                    # set
    c.part(rrect(5, 9, 24, 23, 2), (8, 9, 10))               # screen
    c.part(poly([(12, 11), (20, 16), (12, 21)]), WHITE)      # play
    c.part(circle(27, 13, 1), RED, outline=None)
    c.part(circle(27, 18, 1), (11, 12, 12), outline=None)
    return c


def icon_draft():
    c = Canvas(32, 32)
    c.part(rrect(2, 8, 14, 26, 2), BLUE)
    c.part(rrect(9, 4, 21, 22, 2), RED)
    c.part(rrect(16, 8, 28, 28, 2), WHITE)
    pokeball(c, 22, 18, 4)
    c.part(star(6, 13, 2.5, 1.1), (11, 11, 12), outline=None)
    c.part(star(13, 9, 2.5, 1.1), (11, 11, 12), outline=None)
    return c


def icon_mail():
    c = Canvas(32, 32)
    c.part(rect(2, 8, 29, 26), WHITE)
    c.fill(line(2, 8, 15, 18) | line(29, 8, 16, 18), 1)       # flap
    c.fill(line(2, 26, 12, 17) | line(29, 26, 19, 17), 3)
    heart = ellipse(13.5, 17, 3, 3) | ellipse(18.5, 17, 3, 3) | poly([(10.5, 18), (21.5, 18), (16, 24)])
    c.part(heart, RED)
    c.fill({(13, 16)}, 5)
    return c


def icon_dex():
    c = Canvas(32, 32)
    c.part(rrect(5, 2, 26, 29, 3), RED)
    c.fill(rect(6, 11, 25, 11), 7)                           # hinge
    c.part(circle(11, 7, 3), (8, 9, 10))                     # lens
    c.fill({(10, 6), (11, 6), (10, 5)}, 2)
    c.part(circle(17, 5, 1), RED, outline=1)
    c.part(circle(20, 5, 1), (11, 11, 12), outline=1)
    c.part(circle(23, 5, 1), (14, 14, 15), outline=1)
    c.part(rect(8, 14, 23, 23), (8, 8, 9))                   # screen
    c.fill(rect(10, 16, 16, 16) | rect(10, 18, 19, 18) | rect(10, 20, 14, 20), 10)
    c.part(rect(9, 25, 13, 27), STEEL)
    c.part(rect(18, 25, 22, 27), STEEL)
    return c


ICONS = [icon_guilds, icon_wonder, icon_bases, icon_tower, icon_bounties,
         icon_titles, icon_replays, icon_draft, icon_mail, icon_dex]


# Title badges (16x16) ----------------------------------------------------

BADGE_PAL = [
    0x98D0B8,  # 0 transparent
    0x383840,  # 1 outline
    0xF8F8F8,  # 2 white
    0xC8C8D0,  # 3 light gray
    0x8890A0,  # 4 gray
    0xF8E868,  # 5 gold light
    0xE0A818,  # 6 gold
    0xB06820,  # 7 bronze
    0xF06048,  # 8 red
    0xA82028,  # 9 red dark
    0x80C8F8,  # 10 blue light
    0x3070E0,  # 11 blue
    0x88E070,  # 12 green light
    0x309838,  # 13 green
    0xD098F8,  # 14 purple light
    0x8048C8,  # 15 purple
]
B_GOLD = (5, 6, 7)
B_SILVER = (2, 3, 4)
B_BRONZE = (6, 7, 7)
B_RED = (8, 8, 9)
B_BLUE = (10, 11, 11)
B_GREEN = (12, 13, 13)
B_PURPLE = (14, 15, 15)


def badge_rookie():
    c = Canvas(16, 16)
    c.part(poly([(8, 1), (14, 7), (8, 15), (2, 7)]), B_GREEN)   # leaf
    c.fill(line(8, 4, 8, 12), 13)
    return c


def badge_champion():
    c = Canvas(16, 16)
    c.part(poly([(1, 5), (5, 9), (8, 2), (11, 9), (15, 5), (13, 14), (3, 14)]), B_GOLD)
    c.part(circle(8, 11, 1), B_RED, outline=None)
    return c


def badge_elite():
    c = Canvas(16, 16)
    c.part(poly([(4, 2), (12, 2), (15, 6), (8, 15), (1, 6)]), B_PURPLE)
    c.fill(line(1, 6, 15, 6), 15)
    c.fill({(5, 4), (6, 4)}, 2)
    return c


def badge_ace():
    c = Canvas(16, 16)
    c.part(star(8, 8.5, 7.5, 3.2), B_BLUE)
    c.fill({(7, 5), (8, 5)}, 2)
    return c


def badge_veteran():
    c = Canvas(16, 16)
    c.part(poly([(2, 2), (14, 2), (14, 8), (8, 15), (2, 8)]), B_BRONZE)
    c.part(star(8, 7.5, 3.5, 1.5), B_GOLD, outline=None)
    return c


def badge_climber():
    c = Canvas(16, 16)
    c.part(poly([(1, 14), (1, 11), (5, 11), (5, 8), (9, 8), (9, 5), (14, 5), (14, 14)]), B_SILVER)
    c.part(poly([(11, 0), (15, 2), (11, 4)]), B_RED, outline=None)
    c.fill(line(11, 0, 11, 5), 1)
    return c


def badge_tycoon():
    c = Canvas(16, 16)
    c.part(poly([(5, 14), (6, 4), (10, 4), (11, 14)]), B_GOLD)
    c.part(poly([(6, 4), (8, 0), (10, 4)]), B_GOLD)
    c.part(rect(3, 12, 13, 15), B_BRONZE)
    c.fill({(8, 7), (8, 9)}, 7)
    return c


def badge_collector():
    c = Canvas(16, 16)
    pokeball(c, 8, 8, 6)
    # recolour to the badge palette: red 6->8, white stays
    for y in range(16):
        for x in range(16):
            if c.px[y][x] in (5, 6, 7):
                c.px[y][x] = {5: 8, 6: 8, 7: 9}[c.px[y][x]]
            elif c.px[y][x] == 3:
                c.px[y][x] = 3
    return c


def badge_dexmaster():
    c = Canvas(16, 16)
    c.part(rrect(3, 1, 12, 14, 2), B_RED)
    c.part(circle(6, 4, 1.5), B_BLUE, outline=1)
    c.part(rect(5, 8, 10, 12), (10, 10, 11))
    c.part(star(13, 12, 3, 1.3), B_GOLD)
    return c


def badge_hunter():
    c = Canvas(16, 16)
    c.part(circle(8, 8, 7), B_RED)
    c.part(circle(8, 8, 4), (2, 2, 3))
    c.part(circle(8, 8, 1), B_RED, outline=None, shade=False)
    c.fill(rect(8, 0, 8, 3) | rect(8, 12, 8, 15) | rect(0, 8, 3, 8) | rect(12, 8, 15, 8), 1)
    return c


def badge_founder():
    c = Canvas(16, 16)
    c.part(rect(3, 1, 4, 15), B_SILVER)
    c.part(poly([(5, 2), (15, 2), (12, 6), (15, 10), (5, 10)]), B_BLUE)
    c.part(star(9, 6, 2.5, 1.1), (5, 5, 6), outline=None)
    return c


def badge_generous():
    c = Canvas(16, 16)
    c.part(rect(2, 7, 13, 15), B_RED)
    c.part(rect(1, 4, 14, 7), B_RED)
    c.fill(rect(7, 4, 8, 15), 5)
    c.fill(rect(1, 5, 14, 5), 5)
    c.part(ellipse(5, 2, 2.5, 1.5) | ellipse(10, 2, 2.5, 1.5), B_GOLD)
    return c


def badge_wonderer():
    c = Canvas(16, 16)
    c.part(circle(8, 8, 7), B_PURPLE)
    swirl = set()
    for i in range(60):
        a = i * 0.21
        r = 0.9 + i * 0.09
        swirl.add((round(8 + r * math.cos(a)), round(8 + r * math.sin(a))))
    c.fill(swirl & circle(8, 8, 6), 14)
    c.part(star(13, 3, 3, 1.1, 4), (2, 5, 6), outline=None)
    return c


BADGES = [badge_rookie, badge_champion, badge_elite, badge_ace, badge_veteran,
          badge_climber, badge_tycoon, badge_collector, badge_dexmaster,
          badge_hunter, badge_founder, badge_generous, badge_wonderer]


# Plaza cursor (64x64, two frames) ----------------------------------------

CURSOR_PAL = [0x98D0B8, 0x383840, 0xF8F8F8, 0xF8E060, 0xE0A018, 0xB06818] + [0] * 10


def cursor_frame(bright):
    c = Canvas(64, 64)
    outer = rrect(10, 10, 53, 53, 6)
    inner = rrect(13, 13, 50, 50, 4)
    c.part(outer - inner, (2, 3, 4) if bright else (3, 4, 5))
    # corner sparkles
    if bright:
        for (x, y) in ((11, 11), (52, 11), (11, 52), (52, 52)):
            c.fill({(x, y)}, 2)
    return c


# Plaza background (256x160 -> tiles + tilemap) ---------------------------

PLAZA_PAL = [
    0x000000,  # 0 backdrop (never shown)
    0x283868,  # 1 banner outline
    0x3868C8,  # 2 banner blue
    0x5890E8,  # 3 banner light
    0x98C0F8,  # 4 banner highlight
    0xF8F0D8,  # 5 floor light
    0xE8D8B0,  # 6 floor dark
    0xD0B888,  # 7 floor grout
    0xC8E8F8,  # 8 pedestal light
    0x88B8E0,  # 9 pedestal
    0x4878B0,  # 10 pedestal outline
    0xF8F8F8,  # 11 panel white
    0xD8D8E0,  # 12 panel shade
    0x606878,  # 13 panel frame
    0x90A0B8,  # 14 panel frame light
    0xB89868,  # 15 floor shadow
]


def plaza_bg():
    c = Canvas(256, 160)
    # floor: Pokémon Center style tiles
    for y in range(24, 160):
        for x in range(256):
            tx, ty = x // 16, (y - 24) // 16
            col = 5 if (tx + ty) % 2 == 0 else 6
            if x % 16 == 0 or (y - 24) % 16 == 0:
                col = 7
            c.set(x, y, col)
    # banner
    for y in range(0, 24):
        for x in range(256):
            col = 2
            if y < 2:
                col = 3
            if y == 2:
                col = 4
            if y >= 20:
                col = 3
            if y == 23:
                col = 1
            c.set(x, y, col)
    # diagonal stripes on the banner, like Emerald's menu headers
    for y in range(3, 20):
        for x in range(256):
            if (x + y) % 16 < 2 and x > 150:
                c.set(x, y, 3)
    c.fill(rect(0, 24, 255, 25), 15)   # banner shadow
    # pedestals
    for i in range(10):
        cx = 32 + (i % 5) * 44
        cy = 48 + (i // 5) * 42
        shadow = rrect(cx - 19, cy - 16, cx + 20, cy + 22, 6)
        c.fill(shadow, 15)
        c.part(rrect(cx - 20, cy - 19, cx + 19, cy + 19, 6), (8, 9, 9), outline=10)
        c.fill(rrect(cx - 16, cy - 15, cx + 15, cy + 15, 4) - rrect(cx - 15, cy - 14, cx + 14, cy + 14, 4), 8)
    # info panel
    c.fill(rrect(9, 113, 233, 159, 5), 15)
    c.part(rrect(8, 110, 231, 157, 5), (14, 13, 13), outline=13)
    c.part(rrect(11, 113, 228, 154, 3), (11, 11, 12), outline=14)
    return c


def tiles_and_map(c):
    """Unique 8x8 tiles (tile 0 blank) and a 32x32 tilemap, palette 0."""
    tiles = [tuple([0] * 64)]
    index = {tiles[0]: 0}
    tilemap = [0] * (32 * 32)
    for ty in range(c.h // 8):
        for tx in range(c.w // 8):
            t = tuple(c.px[ty * 8 + y][tx * 8 + x] for y in range(8) for x in range(8))
            if t not in index:
                index[t] = len(tiles)
                tiles.append(t)
            tilemap[ty * 32 + tx] = index[t]
    cols = 16
    rows = (len(tiles) + cols - 1) // cols
    sheet = Canvas(cols * 8, rows * 8)
    for i, t in enumerate(tiles):
        for y in range(8):
            for x in range(8):
                sheet.px[(i // cols) * 8 + y][(i % cols) * 8 + x] = t[y * 8 + x]
    return sheet, tilemap


def save(canvas, palette, name):
    path = os.path.join(OUT, name)
    canvas.image(palette).save(path)
    return path


def strip(frames, w, h):
    sheet = Canvas(w, h * len(frames))
    for i, f in enumerate(frames):
        sheet.blit(f, 0, i * h)
    return sheet


def preview(path, scale=4):
    im = Image.open(path).convert("RGBA")
    im.resize((im.width * scale, im.height * scale), Image.NEAREST).save(path.replace(".png", "_preview.png"))


def main():
    os.makedirs(OUT, exist_ok=True)
    save(strip([f() for f in ICONS], 32, 32), ICON_PAL, "icons.png")
    save(strip([f() for f in BADGES], 16, 16), BADGE_PAL, "badges.png")
    save(strip([cursor_frame(True), cursor_frame(False)], 64, 64), CURSOR_PAL, "cursor.png")
    bg = plaza_bg()
    sheet, tilemap = tiles_and_map(bg)
    save(sheet, PLAZA_PAL, "plaza_tiles.png")
    with open(os.path.join(OUT, "plaza_tilemap.bin"), "wb") as f:
        f.write(struct.pack("<%dH" % len(tilemap), *tilemap))
    print("plaza tiles:", sheet.w * sheet.h // 64)


if __name__ == "__main__":
    main()
