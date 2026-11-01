#!/usr/bin/env python3
"""Builds the colour-blind palette tables (OPTION > COLORS, src/palette.c).

Each table maps every GBA colour (BGR555, 32768 entries) to a corrected one,
so the game can filter a whole palette with one lookup per colour. The
correction is daltonization: simulate how the colour looks with the colour
vision deficiency (Machado, Oliveira & Fernandes 2009, full severity, in
linear light), then move the lost difference into the channels that are still
seen (Fidaner et al.), so colours that would look the same stay apart.

    python3 tools/qol/make_cvd_tables.py   # writes graphics/qol/cvd_*.bin
"""
import os
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "graphics", "qol")

SIMULATE = {
    # red-weak (protanopia)
    "protan": [[0.152286, 1.052583, -0.204868],
               [0.114503, 0.786281, 0.099216],
               [-0.003882, -0.048116, 1.051998]],
    # green-weak (deuteranopia)
    "deutan": [[0.367322, 0.860646, -0.227968],
               [0.280085, 0.672501, 0.047413],
               [-0.011820, 0.042940, 0.968881]],
    # blue-weak (tritanopia)
    "tritan": [[1.255528, -0.076749, -0.178779],
               [-0.078411, 0.930809, 0.147602],
               [0.004733, 0.691367, 0.303900]],
}

# Where the difference the eye can't see goes.
SHIFT = {
    "protan": [[0, 0, 0], [0.7, 1, 0], [0.7, 0, 1]],
    "deutan": [[0, 0, 0], [0.7, 1, 0], [0.7, 0, 1]],
    "tritan": [[1, 0, 0.7], [0, 1, 0.7], [0, 0, 0]],
}


def to_linear(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def to_srgb(c):
    c = min(1.0, max(0.0, c))
    return c * 12.92 if c <= 0.0031308 else 1.055 * c ** (1 / 2.4) - 0.055


def mul(m, v):
    return [sum(m[r][i] * v[i] for i in range(3)) for r in range(3)]


def correct(kind, rgb5):
    lin = [to_linear(c / 31) for c in rgb5]
    sim = mul(SIMULATE[kind], lin)
    err = [lin[i] - sim[i] for i in range(3)]
    fix = mul(SHIFT[kind], err)
    return [round(to_srgb(lin[i] + fix[i]) * 31) for i in range(3)]


def simulate(kind, rgb5):
    """How a colour looks with the deficiency (for the comparison screenshots)."""
    lin = [to_linear(c / 31) for c in rgb5]
    return [round(to_srgb(c) * 31) for c in mul(SIMULATE[kind], lin)]


def table(kind):
    out = bytearray()
    for color in range(0x8000):
        r, g, b = color & 31, (color >> 5) & 31, (color >> 10) & 31
        r, g, b = correct(kind, (r, g, b))
        out += struct.pack("<H", r | (g << 5) | (b << 10))
    return bytes(out)


def main():
    os.makedirs(OUT, exist_ok=True)
    for kind in ("protan", "deutan", "tritan"):
        with open(os.path.join(OUT, f"cvd_{kind}.bin"), "wb") as f:
            f.write(table(kind))


if __name__ == "__main__":
    main()
