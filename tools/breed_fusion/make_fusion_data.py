#!/usr/bin/env python3
"""Writes server/data/fusion.json.gz, what the server needs to build fusions
(server/community_fusion.py, docs/breed_fusion.md): for every species its
name, types, base stats, whether it can breed, and its color families
(the colors that stand for each group of hues on its front sprite, the
biggest group first) for its normal and shiny palettes.

Run from the repository root after a build (it reads pokeemerald.elf for
the species data and palettes, and the front sprite PNGs for how much of
the sprite each color covers):
    python3 tools/breed_fusion/make_fusion_data.py
"""
import gzip
import json
import os
import re
import struct
import subprocess
import sys
import tempfile

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "server", "data", "fusion.json.gz")
sys.path.insert(0, os.path.join(ROOT, "server"))
import community  # noqa: E402  (the game's charset)
from fusion_colors import families  # noqa: E402

FIELDS = ["sizeof(struct SpeciesInfo)", "offsetof(struct SpeciesInfo, baseHP)", "offsetof(struct SpeciesInfo, types)",
          "offsetof(struct SpeciesInfo, eggGroups)", "offsetof(struct SpeciesInfo, speciesName)",
          "offsetof(struct SpeciesInfo, frontPic)", "offsetof(struct SpeciesInfo, palette)",
          "offsetof(struct SpeciesInfo, shinyPalette)", "NUM_SPECIES", "EGG_GROUP_NO_EGGS_DISCOVERED",
          "EGG_GROUP_DITTO", "NUMBER_OF_MON_TYPES", "sizeof(enum Type)", "sizeof(enum EggGroup)"]


def layout():
    """The numbers above, as the game's compiler sees them."""
    with tempfile.TemporaryDirectory() as tmp:
        src = os.path.join(tmp, "layout.c")
        with open(src, "w") as f:
            f.write('#include "global.h"\n#include "pokemon.h"\nconst int kLayout[] = {\n')
            f.write("".join(f"    {x},\n" for x in FIELDS) + "};\n")
        cpp = subprocess.run(["arm-none-eabi-cpp", "-iquote", os.path.join(ROOT, "include"), "-DMODERN=1",
                              "-DTESTING=0", "-DEMERALD", "-std=gnu17", src], capture_output=True, check=True, cwd=ROOT)
        pre = subprocess.run([os.path.join(ROOT, "tools/preproc/preproc"), "-i", "-g", "build/assets", src,
                              "charmap.txt"], input=cpp.stdout, capture_output=True, check=True, cwd=ROOT)
        asm = os.path.join(tmp, "layout.s")
        cc1 = subprocess.check_output(["arm-none-eabi-gcc", "-print-prog-name=cc1"]).decode().strip()
        subprocess.run([cc1, "-quiet", "-mthumb", "-O2", "-mabi=apcs-gnu", "-march=armv4t", "-std=gnu17", "-o", asm],
                       input=pre.stdout, check=True, cwd=ROOT)
        words = re.findall(r"\.word\s+(-?\d+)", open(asm).read())
    return dict(zip(FIELDS, (int(w) for w in words)))


def symbols():
    out = subprocess.check_output(["arm-none-eabi-nm", os.path.join(ROOT, "pokeemerald.elf")]).decode()
    by_name, by_addr = {}, {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            addr = int(parts[0], 16)
            by_name[parts[2]] = addr
            by_addr.setdefault(addr, parts[2])
    return by_name, by_addr


def pic_files():
    """gMonFrontPic_X -> its PNG (the modern sprites, not *_gba)."""
    text = open(os.path.join(ROOT, "src/data/graphics/pokemon.h")).read()
    files = {}
    for name, path in re.findall(r'const u32 (gMonFrontPic_\w+)\[\] = INCGFX_U32\("([^"]+\.png)"', text):
        if "_gba" not in path:
            files.setdefault(name, path)
    return files


def main():
    lay = layout()
    by_name, by_addr = symbols()
    rom = open(os.path.join(ROOT, "pokeemerald.gba"), "rb").read()
    pics = pic_files()
    type_names = {int(v): n.title() for n, v in re.findall(r"^\s+TYPE_(\w+) = (\d+),",
                  open(os.path.join(ROOT, "include/constants/pokemon.h")).read(), re.M)}

    def rom_at(addr, n):
        off = addr - 0x08000000
        return rom[off:off + n]

    base = by_name["gSpeciesInfo"]
    size = lay["sizeof(struct SpeciesInfo)"]
    tsize, esize = lay["sizeof(enum Type)"], lay["sizeof(enum EggGroup)"]
    fmt = {1: "B", 2: "H", 4: "I"}
    species, found = {}, 0
    for sp in range(1, lay["NUM_SPECIES"]):
        raw = rom_at(base + sp * size, size)
        hp, atk, df, spe, spa, spd = raw[lay["offsetof(struct SpeciesInfo, baseHP)"]:][:6]
        t_off = lay["offsetof(struct SpeciesInfo, types)"]
        types = struct.unpack_from("<2" + fmt[tsize], raw, t_off)
        eggs = struct.unpack_from("<2" + fmt[esize], raw, lay["offsetof(struct SpeciesInfo, eggGroups)"])
        name = community.decode_text(raw[lay["offsetof(struct SpeciesInfo, speciesName)"]:][:13])
        pic_ptr, pal_ptr, shiny_ptr = (struct.unpack_from("<I", raw, lay[f"offsetof(struct SpeciesInfo, {f})"])[0]
                                       for f in ("frontPic", "palette", "shinyPalette"))
        if not name or not pal_ptr:
            continue
        entry = {
            "name": name,
            "types": [type_names.get(t, str(t)) for t in types],
            "type_ids": list(types),
            "stats": [hp, atk, df, spe, spa, spd],
            "breeds": eggs[0] != lay["EGG_GROUP_NO_EGGS_DISCOVERED"],
            "ditto": lay["EGG_GROUP_DITTO"] in eggs,
        }
        png = pics.get(by_addr.get(pic_ptr, ""))
        counts = [1] * 16
        if png and os.path.exists(os.path.join(ROOT, png)):
            im = Image.open(os.path.join(ROOT, png))
            w, h = im.size
            counts = [0] * 16
            for p in im.crop((0, 0, w, min(h, w))).tobytes():  # the first frame's color indexes
                counts[p & 15] += 1
            found += 1
        for key, ptr in (("colors", pal_ptr), ("shiny_colors", shiny_ptr)):
            pal = list(struct.unpack("<16H", rom_at(ptr, 32))) if ptr else None
            entry[key] = families(pal, counts) if pal else []
        species[sp] = entry
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with gzip.open(OUT, "wt") as f:
        json.dump({"species_count": lay["NUM_SPECIES"], "species": species}, f, separators=(",", ":"))
    print(f"{len(species)} species ({found} with sprites) -> {os.path.relpath(OUT, ROOT)}")


if __name__ == "__main__":
    main()
