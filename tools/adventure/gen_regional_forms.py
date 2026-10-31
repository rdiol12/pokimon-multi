#!/usr/bin/env python3
"""Draws the regional forms of Starfall Isle and Amberwood Hollow
(docs/adventure.md) from the official Emerald-style sprites of their
species: new palettes (normal and shiny), the forms' own markings painted
on, and matching party icons and following-Pokémon sprites.

    Starfall Wingull: night-blue and full of stars, with gold wing tips
        and a silver beak.
    Starfall Pelipper: pale lavender, its bill a pouch of starry night.
    Starfall Luvdisc: a midnight-blue heart speckled with stars, its blush
        a warm star-glow.
    Amberwood Teddiursa, Amberwood Ursaring: autumn-red fur, a gold
        crescent and ring, and a maple leaf on the head.

Writes graphics/pokemon/<species>/<form>/: anim_front.png (2 frames),
back.png, icon.png, overworld.png, normal.pal, shiny.pal,
overworld_normal.pal and overworld_shiny.pal.
"""
import colorsys
import os

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
GFX = os.path.join(ROOT, "graphics", "pokemon")

# Shared ramps, light to dark.
LAVENDER = [(248, 240, 255), (216, 208, 240), (176, 168, 216), (120, 112, 168)]
NIGHT = [(128, 136, 232), (88, 96, 208), (56, 64, 168), (40, 40, 120), (24, 24, 72)]
GOLD = [(255, 240, 160), (248, 208, 80), (216, 152, 40), (144, 88, 24)]
AUTUMN = [(240, 112, 72), (208, 72, 56), (152, 40, 40), (96, 24, 32), (56, 16, 24)]
LEAF = [(152, 200, 64), (88, 136, 40), (48, 80, 24)]

# A four-point star, (dx, dy, ramp step): bright middle, dimmer arms.
STAR = [(0, 0, 0), (-1, 0, 1), (1, 0, 1), (0, -1, 1), (0, 1, 1)]
# A small maple leaf, 7x6, rows top to bottom: L light, D dark, O outline.
MAPLE = [
    "...O...",
    ".O.L.O.",
    "OLLLLLO",
    ".OLDLO.",
    "..ODO..",
    "...O...",
]

FORMS = {
    ("wingull", "starfall"): {
        "colors": {3: NIGHT[1], 4: NIGHT[2], 13: NIGHT[3], 8: NIGHT[4],
                   6: GOLD[0], 7: GOLD[1], 5: GOLD[2], 15: LAVENDER[2],
                   10: LAVENDER[0], 12: LAVENDER[2], 9: LAVENDER[3],
                   1: (24, 24, 56), 14: (24, 24, 56), 11: (24, 16, 48)},
        # Stars on its night-blue body; gold wing tips, a silver beak.
        "stars": {"on": [3, 4], "ramp": [6, 7], "count": 6},
        "icon_pal": 3,
        "shiny_hue": 0.45,
    },
    ("pelipper", "starfall"): {
        "colors": {8: LAVENDER[0], 6: LAVENDER[1], 15: LAVENDER[2], 7: LAVENDER[3],
                   4: NIGHT[0], 5: NIGHT[1], 3: NIGHT[2], 1: NIGHT[3],
                   13: NIGHT[1], 10: NIGHT[2], 12: NIGHT[3], 11: NIGHT[4], 9: (16, 16, 48),
                   14: GOLD[2]},
        "stars": {"on": [13, 10, 12], "ramp": [8, 6], "count": 9},
        "icon_pal": 2,
        "shiny_hue": 0.45,
    },
    ("luvdisc", "starfall"): {
        "colors": {3: NIGHT[0], 5: NIGHT[1], 12: NIGHT[1], 6: NIGHT[2], 7: NIGHT[3],
                   2: (24, 24, 64), 1: (64, 48, 120), 11: GOLD[1],
                   8: (24, 24, 64), 10: GOLD[0]},
        "stars": {"on": [3, 5, 12, 6], "ramp": [9, 11], "count": 7},
        "shiny_hue": 0.55,
    },
    ("teddiursa", "amberwood"): {
        "colors": {3: AUTUMN[0], 4: AUTUMN[1], 2: AUTUMN[2], 1: AUTUMN[3],
                   7: GOLD[2], 9: GOLD[0], 13: LEAF[0], 14: LEAF[1]},
        "leaf": {"light": 13, "dark": 14, "outline": 1},
        # Its following sprite's fur is closer to the crescent's colours.
        "ow_colors": {3: AUTUMN[0], 5: AUTUMN[1], 4: AUTUMN[2], 1: AUTUMN[3], 10: GOLD[1], 8: GOLD[0]},
        "shiny_hue": -0.12,
    },
    ("ursaring", "amberwood"): {
        "colors": {6: AUTUMN[0], 2: AUTUMN[1], 1: AUTUMN[2], 3: AUTUMN[4],
                   8: GOLD[0], 11: GOLD[1], 12: GOLD[2], 15: LEAF[0]},
        "leaf": {"light": 15, "dark": 15, "outline": 3},
        "icon_pal": 2,
        # The icon's fur is closer to the ring's colours.
        "icon_colors": {6: AUTUMN[1], 7: AUTUMN[0]},
        "shiny_hue": -0.12,
    },
}


def read_pal(path):
    lines = open(path).read().split("\n")
    return [tuple(int(v) for v in l.split()) for l in lines[3:19] if l.strip()]


def write_pal(path, colors):
    with open(path, "w", newline="\r\n") as f:
        f.write("JASC-PAL\n0100\n16\n")
        for c in colors:
            f.write("%d %d %d\n" % tuple(c))


def gba(c):
    """Rounds a colour to what the GBA shows (5 bits a channel)."""
    return tuple((v >> 3) << 3 for v in c)


def nearest(c, pal, skip0=True, only=None):
    best, bi = None, 0
    for i, p in enumerate(pal):
        if (skip0 and i == 0) or (only is not None and i not in only):
            continue
        d = sum((a - b) ** 2 for a, b in zip(c, p))
        if best is None or d < best:
            best, bi = d, i
    return bi


def shiny_of(pal, hue):
    """The shiny palette: every coloured entry turned round the colour
    wheel; greys, black and white stay."""
    out = [pal[0]]
    for c in pal[1:]:
        h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
        if s > 0.25 and 0.08 < l < 0.95:
            h = (h + hue) % 1.0
        out.append(gba(tuple(int(round(v * 255)) for v in colorsys.hls_to_rgb(h, l, s))))
    return out


def interior(img, x, y, allowed):
    w, h = img.size
    for dx, dy in [(0, 0), (-1, 0), (1, 0), (0, -1), (0, 1)]:
        if not (0 <= x + dx < w and 0 <= y + dy < h) or img.getpixel((x + dx, y + dy)) not in allowed:
            return False
    return True


def paint_stars(img, frame_h, spec, seed):
    """Stars where the sprite has room for one: spots inside the night-sky
    colours, spread out, the same spots on every animation frame."""
    w, h = img.size
    allowed = set(spec["on"])
    frames = h // frame_h
    spots = []
    rng = seed
    candidates = [(x, y) for y in range(frame_h) for x in range(w)
                  if interior(img, x, y, allowed)]
    while candidates and len(spots) < spec["count"]:
        rng = (rng * 1103515245 + 12345) & 0x7FFFFFFF
        x, y = candidates[rng % len(candidates)]
        candidates = [(cx, cy) for cx, cy in candidates if abs(cx - x) + abs(cy - y) > 6]
        spots.append((x, y))
    for f in range(frames):
        for x, y in spots:
            for dx, dy, step in STAR:
                px, py = x + dx, y + dy + f * frame_h
                if img.getpixel((px, py)) in allowed:
                    img.putpixel((px, py), spec["ramp"][step])
    return spots


def head_top(img, frame_h, frame):
    """The top middle of the sprite: the highest opaque pixel near its
    horizontal middle."""
    w = img.size[0]
    cols = [x for x in range(w) if any(img.getpixel((x, y + frame * frame_h)) for y in range(frame_h))]
    mid = (min(cols) + max(cols)) // 2
    for y in range(frame_h):
        for x in range(mid - 4, mid + 5):
            if img.getpixel((x, y + frame * frame_h)):
                return x, y
    return mid, 0


def paint_leaf(img, frame_h, spec, at=None):
    """A maple leaf sitting on top of the head on every frame."""
    frames = img.size[1] // frame_h
    for f in range(frames):
        x0, y0 = at if at else head_top(img, frame_h, f)
        for ry, row in enumerate(MAPLE):
            for rx, ch in enumerate(row):
                px, py = x0 - 3 + rx, y0 - 4 + ry + f * frame_h
                if ch == "." or not (0 <= px < img.size[0] and 0 <= py - f * frame_h < frame_h):
                    continue
                if ry >= 4 and img.getpixel((px, py)):
                    continue  # the stem tucks behind the head's outline
                img.putpixel((px, py), {"L": spec["light"], "D": spec["dark"], "O": spec["outline"]}[ch])


def recolor_palette(base, colors):
    pal = list(base)
    for i, c in colors.items():
        pal[i] = c
    return [pal[0]] + [gba(c) for c in pal[1:]]


def save_indexed(img, pal, path):
    out = img.copy()
    flat = []
    for c in pal:
        flat += list(c)
    out.putpalette(flat + [0] * (768 - len(flat)))
    out.save(path, transparency=0)


def map_by_color(img, src_pal, base_pal, new_pal, used, dst_pal=None, overrides=None):
    """Recolours a sprite drawn with another palette (icon, overworld):
    each colour becomes what its nearest colour in the battle palette
    became, then (for icons) the nearest colour of the icon palette."""
    out = img.copy()
    px = out.load()
    w, h = out.size
    remap = {}
    for i, c in enumerate(src_pal):
        if i == 0:
            remap[i] = (0, c)
            continue
        j = nearest(c, base_pal, only=used)
        moved = new_pal[j] if new_pal[j] != gba(base_pal[j]) and new_pal[j] != base_pal[j] else c
        remap[i] = (i, (overrides or {}).get(i, moved))
    if dst_pal is None:
        pal = [src_pal[0]] + [gba(remap[i][1]) for i in range(1, len(src_pal))]
        return out, pal
    for y in range(h):
        for x in range(w):
            if px[x, y]:
                px[x, y] = nearest(remap[px[x, y]][1], dst_pal)
    return out, dst_pal


def icon_palettes():
    return [read_pal(os.path.join(GFX, "icon_palettes", "pal%d.pal" % i)) for i in range(6)]


def best_icon_palette(icon, src_pal, base_pal, new_pal, used_base):
    """The icon palette that shows the recoloured icon best."""
    best, bi = None, 0
    used = set(icon.getdata()) - {0}
    for k, dst in enumerate(icon_palettes()):
        err = 0
        for i in used:
            j = nearest(src_pal[i], base_pal, only=used_base)
            c = new_pal[j]
            err += min(sum((a - b) ** 2 for a, b in zip(c, p)) for p in dst[1:])
        if best is None or err < best:
            best, bi = err, k
    return bi


def make(species, form, spec):
    src = os.path.join(GFX, species)
    dst = os.path.join(src, form)
    os.makedirs(dst, exist_ok=True)
    base = read_pal(os.path.join(src, "normal.pal"))
    pal = recolor_palette(base, spec["colors"])
    shiny = shiny_of(pal, spec["shiny_hue"])

    front = Image.open(os.path.join(src, "anim_front.png"))
    back = Image.open(os.path.join(src, "back.png"))
    # The battle colours the species really uses, to match other sprites to.
    used = (set(front.getdata()) | set(back.getdata())) - {0}
    if "stars" in spec:
        paint_stars(front, 64, spec["stars"], seed=len(species) * 7 + 3)
        paint_stars(back, 64, spec["stars"], seed=len(species) * 11 + 5)
    if "leaf" in spec:
        paint_leaf(front, 64, spec["leaf"])
        paint_leaf(back, 64, spec["leaf"])
    save_indexed(front, pal, os.path.join(dst, "anim_front.png"))
    save_indexed(back, pal, os.path.join(dst, "back.png"))
    write_pal(os.path.join(dst, "normal.pal"), pal)
    write_pal(os.path.join(dst, "shiny.pal"), shiny)

    icon = Image.open(os.path.join(src, "icon.png"))
    icon_src_pal = [tuple(icon.getpalette()[i * 3:i * 3 + 3]) for i in range(16)]
    k = spec.get("icon_pal", best_icon_palette(icon, icon_src_pal, base, pal, used))
    icon_out, ipal = map_by_color(icon, icon_src_pal, base, pal, used, icon_palettes()[k],
                                 spec.get("icon_colors"))
    save_indexed(icon_out, ipal, os.path.join(dst, "icon.png"))

    ow = Image.open(os.path.join(src, "overworld.png"))
    ow_pal = read_pal(os.path.join(src, "overworld_normal.pal"))
    ow_out, ow_new = map_by_color(ow, ow_pal, base, pal, used)
    for i, c in spec.get("ow_colors", {}).items():
        ow_new[i] = gba(c)
    save_indexed(ow_out, ow_new, os.path.join(dst, "overworld.png"))
    write_pal(os.path.join(dst, "overworld_normal.pal"), ow_new)
    write_pal(os.path.join(dst, "overworld_shiny.pal"), shiny_of(ow_new, spec["shiny_hue"]))
    return k


if __name__ == "__main__":
    for (species, form), spec in FORMS.items():
        k = make(species, form, spec)
        print("%s/%s: icon palette %d" % (species, form, k))
