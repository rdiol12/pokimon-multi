#!/usr/bin/env python3
"""Writes src/data/trainer_look.h for the trainer look (docs/trainer_look.md):
the colour choices, the accessory pictures for Brendan and May, and where
each accessory goes on every frame of their sprites.

Accessories are drawn once per facing (tools/trainer_look/art.py). For each
frame of each sprite sheet this finds the head (by matching the walking
frames' heads) and stores where the picture goes, which way it faces, and
which pixels the trainer covers there (for the "behind" layer).

Run from the repository root:  python3 tools/trainer_look/gen_trainer_look.py [--preview out.png]
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import art  # noqa: E402

PEOPLE = os.path.join(ROOT, "graphics", "object_events", "pics", "people")
PAL_DIR = os.path.join(ROOT, "graphics", "object_events", "palettes")
OUT_H = os.path.join(ROOT, "src", "data", "trainer_look.h")

BODIES = ["brendan", "may"]

# Colour choices, light to dark, picked from Emerald's own palettes. Choice 0
# of every part is the body's own colours.
SKIN = [
    ("LIGHT", [(255, 230, 213), (255, 213, 189), (230, 172, 148), (131, 82, 74)]),
    ("TAN", [(238, 189, 139), (222, 164, 115), (189, 123, 82), (106, 57, 41)]),
    ("BROWN", [(205, 148, 98), (180, 123, 74), (139, 90, 57), (82, 41, 32)]),
    ("DARK", [(164, 106, 74), (139, 82, 57), (106, 57, 41), (57, 32, 24)]),
    ("DEEP", [(123, 82, 57), (106, 65, 41), (82, 49, 32), (41, 24, 16)]),
]
HAIR = [
    ("BROWN", [(164, 106, 82), (74, 49, 57)]),
    ("BLACK", [(74, 74, 90), (32, 32, 41)]),
    ("BLONDE", [(255, 222, 123), (164, 123, 57)]),
    ("RED", [(222, 98, 65), (123, 41, 32)]),
    ("SILVER", [(222, 230, 238), (57, 74, 123)]),
    ("BLUE", [(98, 148, 230), (41, 57, 123)]),
    ("PINK", [(255, 156, 197), (148, 65, 98)]),
    ("GREEN", [(115, 197, 115), (41, 90, 57)]),
]
BRIGHT = [
    ("RED", [(255, 98, 90), (197, 65, 65)]),
    ("ORANGE", [(255, 164, 57), (205, 106, 32)]),
    ("YELLOW", [(255, 222, 82), (205, 164, 32)]),
    ("GREEN", [(115, 205, 115), (74, 148, 82)]),
    ("BLUE", [(98, 156, 238), (57, 98, 189)]),
    ("PURPLE", [(180, 139, 238), (123, 90, 189)]),
    ("PINK", [(255, 148, 197), (205, 90, 148)]),
    ("WHITE", [(238, 238, 246), (180, 180, 197)]),
    ("BLACK", [(82, 82, 98), (41, 41, 57)]),
]
DARK = [
    ("NAVY", [(57, 74, 123), (16, 32, 57)]),
    ("BLACK", [(65, 65, 74), (24, 24, 32)]),
    ("BROWN", [(115, 74, 57), (57, 32, 24)]),
    ("FOREST", [(57, 106, 65), (24, 57, 32)]),
    ("WINE", [(131, 49, 65), (74, 24, 32)]),
    ("PLUM", [(98, 65, 131), (49, 32, 82)]),
    ("SLATE", [(115, 123, 139), (57, 65, 82)]),
    ("TEAL", [(41, 123, 131), (16, 65, 74)]),
]

# Parts in menu order: (field, choices, {body: (label, palette indices light to dark)}).
# A ramp shorter than the index list repeats its darkest colour; Brendan's
# hair index 8 also draws his eyes and shirt, so it takes the shade.
PARTS = [
    ("skin", SKIN, {"brendan": ("SKIN", [1, 2, 3, 4]), "may": ("SKIN", [1, 2, 3, 4])}),
    ("hat", HAIR, {"brendan": ("HAT", [9, 5]), "may": ("HAIR", [7, 8])}),
    ("band", BRIGHT, {"brendan": ("BANDANA", [10, 11]), "may": ("BANDANA", [10, 11])}),
    ("clothes", BRIGHT, {"brendan": ("CLOTHES", [12, 13]), "may": ("TOP", [12, 13])}),
    ("dark", DARK, {"brendan": ("HAIR", [6, 7, 8]), "may": ("SHORTS", [5, 6])}),
]

# Which palette index each accessory letter uses, per body.
LETTERS = {
    "brendan": {"o": 15, "w": 14, "a": 10, "b": 11, "h": 7, "H": 8},
    "may": {"o": 15, "w": 14, "a": 10, "b": 11, "h": 7, "H": 8},
}
BACK = {"O": "o", "W": "w", "x": "a", "y": "b", "k": "h", "K": "H"}

FACINGS = ["S", "N", "W"]

# Sheets, in the order of the avatar states that use them: (name, file,
# frame width, the picture table's frames as indices into the file).
SHEETS = [
    ("NORMAL", ["walking.png", "running.png"], 16, None),
    ("MACH_BIKE", ["mach_bike.png"], 32, None),
    ("ACRO_BIKE", ["acro_bike.png"], 32, None),
    ("SURFING", ["surfing.png"], 32, [0, 2, 4, 0, 0, 2, 2, 4, 4, 1, 3, 5]),
    ("FIELD_MOVE", ["field_move.png"], 32, None),
    ("FISHING", ["fishing.png"], 32, None),
    ("WATERING", ["watering.png"], 32, [0, 2, 4, 1, 1, 3, 3, 5, 5]),
]

MATCH_THRESHOLD = 0.55


def rgb5(c):
    r, g, b = c
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def read_pal(path):
    lines = open(path).read().split("\n")[3:19]
    return [tuple(map(int, x.split())) for x in lines]


def frames_of(body, sheet):
    name, files, width, table = sheet
    frames = []
    for f in files:
        im = Image.open(os.path.join(PEOPLE, body, f))
        px = im.load()
        for i in range(im.size[0] // width):
            frames.append([[px[i * width + x, y] for x in range(width)] for y in range(32)])
    if table is not None:
        frames = [frames[i] for i in table]
    return frames


def head_refs(body):
    walk = frames_of(body, SHEETS[0])
    refs = {}
    for n, facing in enumerate(FACINGS):
        f = walk[n]
        refs[facing] = [(x, y, f[y][x]) for y in range(10, 18) for x in range(16) if f[y][x] != 0]
    return refs


def find_head(frame, refs):
    """Best (score, facing, dx, dy): where the head of a walking frame sits in frame."""
    w = len(frame[0])
    best = (0, None, 0, 0)
    for facing, ref in refs.items():
        for dy in range(-10, 12):
            for dx in range(-4, w - 12):
                hits = 0
                for x, y, v in ref:
                    tx, ty = x + dx, y + dy
                    if 0 <= tx < w and 0 <= ty < 32 and frame[ty][tx] == v:
                        hits += 1
                score = hits / len(ref)
                if score > best[0]:
                    best = (score, facing, dx, dy)
    return best


def analyse(body):
    """{sheet: [(facing or None, x, y, mask rows)]}, x and y being where the
    16x16 picture's top left goes in the frame."""
    refs = head_refs(body)
    out = {}
    for sheet in SHEETS:
        rows = []
        for frame in frames_of(body, sheet):
            score, facing, dx, dy = find_head(frame, refs)
            if score < MATCH_THRESHOLD:
                rows.append((None, 0, 0, [0] * 16))
                continue
            ox, oy = dx, 8 + dy
            w = len(frame[0])
            mask = []
            for ty in range(16):
                bits = 0
                for tx in range(16):
                    x, y = ox + tx, oy + ty
                    if 0 <= x < w and 0 <= y < 32 and frame[y][x] != 0:
                        bits |= 1 << tx
                mask.append(bits)
            rows.append((facing, ox, oy, mask))
        out[sheet[0]] = rows
    return out


def picture(body, rows):
    """16x16 bytes: palette index, | 0x80 for the back layer."""
    assert len(rows) == 16, rows
    out = []
    for r in rows:
        assert len(r) == 16, r
        for ch in r:
            if ch == ".":
                out.append(0)
            elif ch in BACK:
                out.append(LETTERS[body][BACK[ch]] | 0x80)
            else:
                out.append(LETTERS[body][ch])
    return out


def compose(frame, pal, pics, anchor, offset_x=0):
    """For previews: the frame with the pictures drawn as the game does."""
    facing, ox, oy, mask = anchor
    w = len(frame[0])
    img = [row[:] for row in frame]
    if facing is not None:
        for pic in pics:
            data = pic[facing]
            for ty in range(16):
                for tx in range(16):
                    v = data[ty * 16 + tx]
                    if not v:
                        continue
                    if v & 0x80 and mask[ty] >> tx & 1:
                        continue
                    x, y = ox + tx, oy + ty
                    if 0 <= x < w and 0 <= y < 32:
                        img[y][x] = v & 0xF
    im = Image.new("RGBA", (w, 32), (0, 0, 0, 0))
    px = im.load()
    for y in range(32):
        for x in range(w):
            if img[y][x]:
                px[x, y] = pal[img[y][x]] + (255,)
    return im


def all_pictures(body):
    groups = [art.HAIR, art.HEAD, art.FACE]
    return [{name: {f: picture(body, acc[f]) for f in FACINGS} for name, acc in g.items()} for g in groups]


def c_array(values, per_line=16, fmt="0x%02X"):
    lines = []
    for i in range(0, len(values), per_line):
        lines.append("    " + ", ".join(fmt % v for v in values[i:i + per_line]) + ",")
    return "\n".join(lines)


def write_header(analysis, pics):
    o = []
    o.append("// Generated by tools/trainer_look/gen_trainer_look.py; do not edit.\n")
    o.append(f"#define LOOK_SHEET_COUNT {len(SHEETS)}")
    for i, s in enumerate(SHEETS):
        o.append(f"#define LOOK_SHEET_{s[0]} {i}")
    o.append("")
    # Colours.
    for field, choices, _ in PARTS:
        o.append(f"static const u16 sLookColours_{field}[][4] =\n{{")
        for name, ramp in choices:
            ramp = ramp + [ramp[-1]] * (4 - len(ramp))
            o.append("    {" + ", ".join("0x%04X" % rgb5(c) for c in ramp) + "}, // " + name)
        o.append("};")
        o.append(f"static const u8 *const sLookNames_{field}[] =\n{{")
        o.append('    COMPOUND_STRING("ORIGINAL"),')
        for name, _ in choices:
            o.append(f'    COMPOUND_STRING("{name}"),')
        o.append("};")
    o.append("")
    o.append("static const struct LookPart sLookParts[LOOK_COLOUR_PARTS] =\n{")
    for field, choices, bodies in PARTS:
        o.append("    {")
        o.append(f"        .colours = sLookColours_{field},")
        o.append(f"        .names = sLookNames_{field},")
        o.append(f"        .count = {len(choices) + 1},")
        o.append("        .label = {" + ", ".join(f'COMPOUND_STRING("{bodies[b][0]}")' for b in BODIES) + "},")
        o.append("        .indices = {" + ", ".join("{" + ", ".join(str(i) for i in bodies[b][1] + [0] * (4 - len(bodies[b][1]))) + "}" for b in BODIES) + "},")
        o.append("        .numIndices = {" + ", ".join(str(len(bodies[b][1])) for b in BODIES) + "},")
        o.append("    },")
    o.append("};")
    o.append("")
    # Accessories.
    group_names = ["HAIR", "HEAD", "FACE"]
    for gi, g in enumerate([art.HAIR, art.HEAD, art.FACE]):
        o.append(f"static const u8 *const sLookAccessoryNames_{group_names[gi]}[] =\n{{")
        o.append('    COMPOUND_STRING("NONE"),')
        for name in g:
            o.append(f'    COMPOUND_STRING("{name}"),')
        o.append("};")
        for bi, body in enumerate(BODIES):
            o.append(f"static const u8 sLookPics_{group_names[gi]}_{body}[][3][256] =\n{{")
            for name in g:
                o.append(f"    {{ // {name}")
                for f in FACINGS:
                    o.append("        {")
                    o.append(c_array(pics[body][gi][name][f]).replace("    ", "            ", 1).replace("\n    ", "\n            "))
                    o.append("        },")
                o.append("    },")
            o.append("};")
    o.append("")
    o.append("static const struct LookAccessoryGroup sLookAccessories[LOOK_ACCESSORY_PARTS] =\n{")
    for gi, g in enumerate([art.HAIR, art.HEAD, art.FACE]):
        n = group_names[gi]
        label = {"HAIR": "HAIRSTYLE", "HEAD": "HEADWEAR", "FACE": "FACE"}[n]
        o.append(f'    {{.label = COMPOUND_STRING("{label}"), .names = sLookAccessoryNames_{n}, .count = {len(g) + 1}, '
                 f".pics = {{sLookPics_{n}_brendan, sLookPics_{n}_may}}}},")
    o.append("};")
    o.append("")
    # Anchors.
    for body in BODIES:
        for sheet in SHEETS:
            rows = analysis[body][sheet[0]]
            o.append(f"static const struct LookAnchor sLookAnchors_{body}_{sheet[0]}[] =\n{{")
            for facing, ox, oy, mask in rows:
                fval = 0 if facing is None else FACINGS.index(facing) + 1
                o.append(f"    {{.facing = {fval}, .x = {ox}, .y = {oy}, .mask = {{" + ", ".join("0x%04X" % m for m in mask) + "}},")
            o.append("};")
    o.append("")
    o.append("static const struct LookSheet sLookSheets[2][LOOK_SHEET_COUNT] =\n{")
    for body in BODIES:
        o.append("    {")
        for sheet in SHEETS:
            o.append(f"        [LOOK_SHEET_{sheet[0]}] = {{sLookAnchors_{body}_{sheet[0]}, ARRAY_COUNT(sLookAnchors_{body}_{sheet[0]}), {sheet[2]}}},")
        o.append("    },")
    o.append("};")
    open(OUT_H, "w").write("\n".join(o) + "\n")


def preview(path, analysis, pics):
    """Every sheet of both bodies wearing a few combinations."""
    combos = [
        ("CAP", None, None), ("BEANIE", "GLASSES", None), ("SUN HAT", None, "LONG"),
        ("BOW", "SHADES", "PONYTAIL"), ("FLOWER", None, "SPIKY"), (None, "GLASSES", "LONG"),
    ]
    scale = 3
    tiles = []
    for body in BODIES:
        pal = read_pal(os.path.join(PAL_DIR, body + ".pal"))
        for combo in combos:
            chosen = []
            for gi, name in zip((1, 2, 0), combo):
                if name:
                    chosen.append((gi, pics[body][gi][name]))
            chosen.sort(key=lambda t: [0, 2, 1].index(t[0]))  # hair, face, head
            row = []
            for sheet in SHEETS:
                for frame, anchor in zip(frames_of(body, sheet), analysis[body][sheet[0]]):
                    row.append(compose(frame, pal, [p for _, p in chosen], anchor))
            tiles.append(row)
    width = max(sum(im.size[0] + 2 for im in row) for row in tiles)
    out = Image.new("RGB", (width * scale, len(tiles) * 34 * scale), (115, 197, 164))
    for r, row in enumerate(tiles):
        x = 0
        for im in row:
            big = im.resize((im.size[0] * scale, 32 * scale), Image.NEAREST)
            out.paste(big, (x * scale, r * 34 * scale), big)
            x += im.size[0] + 2
    out.save(path)


def main():
    analysis = {b: analyse(b) for b in BODIES}
    pics = {b: all_pictures(b) for b in BODIES}
    for b in BODIES:
        for s in SHEETS:
            missing = sum(1 for a in analysis[b][s[0]] if a[0] is None)
            print(f"{b} {s[0]}: {len(analysis[b][s[0]])} frames, {missing} without a head match", file=sys.stderr)
    if "--preview" in sys.argv:
        preview(sys.argv[sys.argv.index("--preview") + 1], analysis, pics)
    else:
        write_header(analysis, pics)


if __name__ == "__main__":
    main()
