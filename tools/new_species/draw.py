#!/usr/bin/env python3
"""Draws the new original Pokémon (docs/new_species.md) in the Emerald style.

Every sprite is built from shapes (raster.py) at the size the game shows it,
so there is no scaling blur: front (2 frames), back, icon (2 frames, on one of
the shared icon palettes) and the 6-frame overworld walking sheet.

    python3 tools/new_species/draw.py            # writes graphics/pokemon/<name>/
    python3 tools/new_species/draw.py --preview out.png
"""
import math
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(__file__))
from raster import Canvas, Material  # noqa: E402

ROOT = os.path.join(os.path.dirname(__file__), "..", "..")
GFX = os.path.join(ROOT, "graphics", "pokemon")


def g(c):
    return tuple((v >> 3) << 3 for v in c)


# The Jellume line: Water/Electric jellyfish that store lightning in their
# cores. Index 0 is the transparent colour.
NORMAL = [g(c) for c in [
    (152, 208, 160),
    (24, 32, 72),     # 1 outline
    (48, 88, 168),    # 2 bell dark
    (80, 144, 216),   # 3 bell mid
    (136, 200, 240),  # 4 bell light
    (216, 240, 248),  # 5 bell highlight
    (200, 112, 24),   # 6 core dark
    (248, 200, 40),   # 7 core
    (255, 248, 168),  # 8 core light
    (255, 255, 255),  # 9 white
    (104, 56, 144),   # 10 tentacle dark
    (168, 96, 200),   # 11 tentacle
    (216, 160, 232),  # 12 tentacle light
    (40, 64, 128),    # 13 underside
    (16, 16, 32),     # 14 eye
    (96, 224, 192),   # 15 aurora
]]
SHINY = [g(c) for c in [
    (152, 208, 160),
    (64, 24, 64),
    (168, 64, 128),
    (216, 104, 168),
    (240, 160, 208),
    (248, 224, 240),
    (40, 120, 176),
    (80, 200, 248),
    (200, 248, 255),
    (255, 255, 255),
    (176, 112, 24),
    (232, 176, 48),
    (248, 224, 128),
    (120, 40, 96),
    (16, 16, 32),
    (248, 136, 96),
]]

BELL = Material(outline=1, dark=2, mid=3, light=4, highlight=5, edge=2)
UNDER = Material(outline=1, dark=13, mid=13, light=2, highlight=3, edge=13)
CORE = Material(outline=6, dark=6, mid=7, light=8, highlight=9, edge=6)
TENT = Material(outline=1, dark=10, mid=11, light=12, highlight=12, edge=10)
AURORA = Material(outline=1, dark=3, mid=15, light=5, highlight=9, edge=3)
EYE = Material(outline=14, dark=14, mid=14, light=14, edge=14)


class T:
    """Maps design coordinates (a 64x64 box) onto a canvas."""

    def __init__(self, c, s=1.0, ox=0.0, oy=0.0, sx=1.0):
        self.c, self.s, self.ox, self.oy, self.sx = c, s, ox, oy, sx

    def p(self, x, y):
        return (32 + (x - 32) * self.s * self.sx + self.ox, 32 + (y - 32) * self.s + self.oy)

    def ellipse(self, x, y, rx, ry, mat, **kw):
        cx, cy = self.p(x, y)
        clip = kw.pop("clip_y", None)
        if clip is not None:
            limit = self.p(0, clip)[1]
            kw["clip"] = lambda px, py: py <= limit
        return self.c.ellipse(cx, cy, max(0.6, rx * self.s * self.sx), max(0.6, ry * self.s), mat, **kw)

    def poly(self, pts, mat, **kw):
        return self.c.polygon([self.p(*q) for q in pts], mat, **kw)

    def stroke(self, pts, w0, w1, mat, **kw):
        return self.c.stroke([self.p(*q) for q in pts], max(0.8, w0 * self.s), max(0.6, w1 * self.s), mat, **kw)

    def dot(self, x, y, color):
        self.c.dot(*self.p(x, y), color)


def eyes(t, y, gap, size, angry=False):
    for side in (-1, 1):
        x = 32 + side * gap
        t.ellipse(x, y, size * 0.7, size, EYE, edge=False, flat=0.0)
        t.dot(x - size * 0.25, y - size * 0.45, 9)
        if angry:
            t.stroke([(x - side * size * 1.4, y - size * 1.9), (x, y - size * 1.5),
                      (x + side * size * 1.2, y - size * 1.1)], 1.2, 1.0, EYE, edge=False)


def bolt(t, x, y, h):
    w = h * 0.45
    t.poly([(x + w * 0.2, y - h / 2), (x - w * 0.6, y + h * 0.08), (x - w * 0.02, y + h * 0.05),
            (x - w * 0.25, y + h / 2), (x + w * 0.6, y - h * 0.1), (x + w * 0.05, y - h * 0.06)],
           CORE, edge=False, flat=0.95)


def wave(x0, y0, length, amp, phase, lean=0.0):
    return [(x0, y0), (x0 + amp * math.sin(phase) + lean * 0.3, y0 + length * 0.35),
            (x0 - amp * math.sin(phase + 1.3) + lean * 0.7, y0 + length * 0.7),
            (x0 + amp * 0.5 * math.sin(phase + 2.1) + lean, y0 + length)]


def jellume(t, frame, view):
    sq = 1 if frame else 0
    for i, x in enumerate((25, 30, 34, 39)):
        t.stroke(wave(x, 38 - sq, 14 + sq * 2, 1.6, i * 1.7 + frame * 1.5), 3.2, 1.2, TENT)
    t.ellipse(32, 37 - sq, 12.5 + sq, 3.2, UNDER)
    t.ellipse(32, 28 + sq * 0.5, 14 + sq, 12 - sq, BELL, clip_y=37.5 - sq)
    t.ellipse(32, 26, 5, 4.6, CORE, bias=0.15)
    t.ellipse(26.5, 21.5, 2.2, 1.6, BELL, edge=False, flat=1.0)
    if view == "front":
        eyes(t, 32 + sq * 0.5, 5.2, 2.0)
        t.dot(32, 35 + sq * 0.5, 14)
    elif view == "side":
        t.ellipse(28, 32, 1.4, 2.0, EYE, edge=False, flat=0.0)


def lumedusa(t, frame, view):
    sq = 1 if frame else 0
    for i, x in enumerate((19, 24, 29, 35, 40, 45)):
        t.stroke(wave(x, 33 - sq, 22 + sq * 2, 2.4, i * 1.3 + frame * 1.4, (x - 32) * 0.15), 3.6, 1.0, TENT)
    for side in (-1, 1):  # two thick oral arms
        t.stroke([(32 + side * 4, 33), (32 + side * 9, 44 + sq), (32 + side * 3, 52), (32 + side * 7, 60)],
                 5.5, 2.0, AURORA)
    for k in range(7):  # frilled skirt
        x = 15 + k * 5.6
        t.ellipse(x, 33 - sq + abs(k - 3) * -0.6, 3.6, 3.0, UNDER)
    t.ellipse(32, 24 + sq * 0.5, 19 + sq, 15 - sq, BELL, clip_y=33.5 - sq)
    t.ellipse(32, 17.5, 6.5, 6, CORE, bias=0.1)
    bolt(t, 32, 18, 8)
    t.ellipse(23, 13.5, 3, 2, BELL, edge=False, flat=1.0)
    if view == "front":
        eyes(t, 29 + sq * 0.5, 7.5, 2.4, angry=True)
    elif view == "side":
        t.ellipse(23, 28, 1.8, 2.5, EYE, edge=False, flat=0.0)


def aurelluma(t, frame, view):
    sq = 1 if frame else 0
    # Aurora sails behind the bell.
    for side in (-1, 1):
        t.poly([(32 + side * 18, 14), (32 + side * 31, 6 - sq), (32 + side * 29, 18), (32 + side * 31, 30 + sq),
                (32 + side * 20, 28)], AURORA, grad=lambda x, y: 0.9 - (y - 6) / 34)
    for i, x in enumerate((13, 18, 23, 28, 36, 41, 46, 51)):
        t.stroke(wave(x, 33 - sq, 26 + sq * 2, 2.6, i * 1.1 + frame * 1.4, (x - 32) * 0.2), 3.8, 1.0, TENT)
    for side in (-1, 1):  # great arms ending in sparks
        t.stroke([(32 + side * 6, 32), (32 + side * 20, 42), (32 + side * 24, 54 - sq), (32 + side * 16, 60)],
                 6.5, 2.6, AURORA)
        t.ellipse(32 + side * 16, 59.5, 3.2, 3.2, CORE, bias=0.2)
    for k in range(9):
        x = 10 + k * 5.5
        t.ellipse(x, 32 - sq - (4 - abs(k - 4)) * 0.4, 3.6, 3.2, UNDER)
    t.ellipse(32, 23 + sq * 0.5, 24 + sq, 16 - sq, BELL, clip_y=32.5 - sq)
    for k, (x, h) in enumerate(((22, 9), (32, 13), (42, 9))):  # crown
        top = 23 - 16 + sq * 0.5 - h + (2 if k != 1 else 0)
        t.poly([(x - 3.5, top + h + 3), (x, top), (x + 3.5, top + h + 3)], CORE,
               grad=lambda px, py, top=top, h=h: 1.0 - (py - top) / (h + 4) * 0.8)
    t.ellipse(32, 17, 7.5, 6.5, CORE, bias=0.1)
    bolt(t, 32, 17.5, 9)
    t.ellipse(20, 14, 3.4, 2.2, BELL, edge=False, flat=1.0)
    if view == "front":
        eyes(t, 28.5 + sq * 0.5, 9, 2.5, angry=True)
    elif view == "side":
        t.ellipse(18, 28, 1.8, 2.6, EYE, edge=False, flat=0.0)


LINE = [
    ("jellume", jellume, 3, 1.0),
    ("lumedusa", lumedusa, 0, 1.0),
    ("aurelluma", aurelluma, 0, 1.0),
]


def render(fn, size, view, frame, s=1.0, ox=0.0, oy=0.0, sx=1.0):
    c = Canvas(*size)
    fn(T(c, s, ox, oy, sx), frame, view)
    c.outline()
    return c


def to_image(canvases, w, h, vertical=True):
    img = Image.new("P", (w * (1 if vertical else len(canvases)), h * (len(canvases) if vertical else 1)))
    for k, c in enumerate(canvases):
        for y in range(c.h):
            for x in range(c.w):
                if c.px[y][x]:
                    img.putpixel((x if vertical else x + k * w, y + k * h if vertical else y), c.px[y][x])
    return img


def bottom_of(c):
    for y in range(c.h - 1, -1, -1):
        if any(c.px[y]):
            return y
    return 0


def write_pal(path, colors):
    with open(path, "w", newline="\r\n") as f:
        f.write("JASC-PAL\n0100\n16\n")
        for col in colors:
            f.write("%d %d %d\n" % tuple(col))


def read_pal(path):
    lines = open(path).read().split("\n")
    return [tuple(int(v) for v in l.split()) for l in lines[3:19] if l.strip()]


def save(img, pal, path):
    flat = []
    for col in pal:
        flat += list(col)
    img.putpalette(flat + [0] * (768 - len(flat)))
    img.save(path, transparency=0)


# Icons use the shared icon palettes; palette 3 has the blues, golds and pink
# this line needs. Our colour index -> icon palette 3 index.
ICON_PAL = 3
ICON_MAP = {1: 15, 2: 4, 3: 5, 4: 6, 5: 6, 6: 13, 7: 10, 8: 11, 9: 3,
            10: 7, 11: 7, 12: 7, 13: 4, 14: 15, 15: 6}


def icon_remap(img):
    pal = read_pal(os.path.join(GFX, "icon_palettes", "pal%d.pal" % ICON_PAL))
    return img.point(lambda v: ICON_MAP.get(v, 0)), pal, ICON_PAL


def make(name, fn, scale_hint):
    d = os.path.join(GFX, name)
    os.makedirs(d, exist_ok=True)
    front = [render(fn, (64, 64), "front", f) for f in (0, 1)]
    save(to_image(front, 64, 64), NORMAL, os.path.join(d, "anim_front.png"))
    back = render(fn, (64, 64), "back", 0, s=1.15, oy=8)
    save(to_image([back], 64, 64), NORMAL, os.path.join(d, "back.png"))
    s_icon = {"jellume": 0.62, "lumedusa": 0.52, "aurelluma": 0.47}[name]
    icon = [render(fn, (32, 32), "front", f, s=s_icon, ox=-16, oy=-16 + (1 if f else 0)) for f in (0, 1)]
    iimg, ipal, ik = icon_remap(to_image(icon, 32, 32))
    save(iimg, ipal, os.path.join(d, "icon.png"))
    s_ow = {"jellume": 0.6, "lumedusa": 0.48, "aurelluma": 0.44}[name]
    ow = []
    for view, flip in (("front", False), ("back", False), ("side", True)):
        for f in (0, 1):
            c = render(fn, (32, 32), view, f, s=s_ow, ox=-16, oy=-16, sx=0.8 if view == "side" else 1.0)
            ow.append(c)
    owimg = to_image(ow, 32, 32, vertical=False)
    save(owimg, NORMAL, os.path.join(d, "overworld.png"))
    write_pal(os.path.join(d, "normal.pal"), NORMAL)
    write_pal(os.path.join(d, "shiny.pal"), SHINY)
    write_pal(os.path.join(d, "overworld_normal.pal"), NORMAL)
    write_pal(os.path.join(d, "overworld_shiny.pal"), SHINY)
    return ik, bottom_of(front[0])


def preview(path):
    tiles = []
    for name, fn, _, _ in LINE:
        d = os.path.join(GFX, name)
        for pal in (NORMAL, SHINY):
            for f in ("anim_front.png", "back.png"):
                im = Image.open(os.path.join(d, f)).crop((0, 0, 64, 64))
                im.putpalette(sum((list(c) for c in pal), []) + [0] * 720)
                tiles.append(im.convert("RGB"))
    W = 4
    sheet = Image.new("RGB", (64 * W * 3, 64 * 3), NORMAL[0])
    for i, im in enumerate(tiles):
        sheet.paste(im, ((i % W) * 64 * 3 // 3 * 1 + (i % W) * 0, 0) if False else ((i % 4) * 64, (i // 4) * 64))
    sheet = sheet.crop((0, 0, 256, 192)).resize((768, 576), Image.NEAREST)
    sheet.save(path)


if __name__ == "__main__":
    for name, fn, _, _ in LINE:
        ik, bot = make(name, fn, 0)
        print(name, "icon palette", ik, "front bottom row", bot)
    if "--preview" in sys.argv:
        preview(sys.argv[sys.argv.index("--preview") + 1])
