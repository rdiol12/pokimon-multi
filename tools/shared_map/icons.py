#!/usr/bin/env python3
"""Draws the shared wild spawn icons for src/shared_map.c (16x16, 4bpp):
the VS bubble over a Pokémon someone is battling and the star over a rare
one. Prints them as C arrays. Palette: 1 outline, 2 white, 3 red, 4 yellow,
5 orange, 6 light yellow."""

VS = [
    "................",
    "..111111111111..",
    ".12222222222221.",
    ".12222222222221.",
    ".12332332333221.",
    ".12332332332221.",
    ".12332332333221.",
    ".12233322223221.",
    ".12223222333221.",
    ".12222222222221.",
    "..111112211111..",
    "......12221.....",
    ".......1221.....",
    "........11......",
    "................",
    "................",
]

STAR = [
    "................",
    ".......11.......",
    "......1461......",
    "......1461......",
    ".11111444411111.",
    ".14664444444441.",
    "..144444444441..",
    "...1444444441...",
    "....14444441....",
    "....14455441....",
    "...1445115441...",
    "...1451..1541...",
    "..1551....1551..",
    "..111......111..",
    "................",
    "................",
]


def conv(rows):
    assert len(rows) == 16 and all(len(r) == 16 for r in rows), rows
    out = []
    for ty in range(2):
        for tx in range(2):
            for y in range(8):
                v = 0
                for x in range(8):
                    c = rows[ty * 8 + y][tx * 8 + x]
                    v |= (0 if c == "." else int(c, 16)) << (4 * x)
                out.append(v)
    return out


def emit(name, rows):
    v = conv(rows)
    body = ",\n".join("    " + ", ".join("0x%08X" % x for x in v[i:i + 8]) for i in range(0, len(v), 8))
    print(f"static const u32 {name}[] =\n{{\n{body},\n}};\n")


if __name__ == "__main__":
    emit("sVsGfx", VS)
    emit("sStarGfx", STAR)
