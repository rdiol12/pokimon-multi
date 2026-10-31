#!/usr/bin/env python3
"""Writes server/data/legal.json.gz, what the server's legality checks
(server/community_legal.py) know about the game: how many species, moves
and items there are, each species' growth rate, and every move each species
(or anything it evolves from) can learn in some game.

Run from the repository root after changing species, moves or items:
    python3 tools/community/make_legal_data.py
Needs a host C compiler, to read the game's enums the way the game does.
"""
import glob
import gzip
import json
import os
import re
import subprocess
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "server", "data", "legal.json.gz")
GROWTH = ["GROWTH_MEDIUM_FAST", "GROWTH_ERRATIC", "GROWTH_FLUCTUATING", "GROWTH_MEDIUM_SLOW", "GROWTH_FAST", "GROWTH_SLOW"]


def enum_values(prefix, header, extra=()):
    """{name: value} for every PREFIX_* name in an enum of `header`, as the C
    compiler sees it."""
    text = open(os.path.join(ROOT, "include", "constants", header)).read()
    names = sorted(set(re.findall(r"^\s+(" + prefix + r"\w+)\s*(?:=|,)", text, re.M)) | set(extra))
    with tempfile.TemporaryDirectory() as tmp:
        src = os.path.join(tmp, "dump.c")
        with open(src, "w") as f:
            f.write('#include <stdio.h>\n#include "constants/global.h"\n#include "constants/%s"\nint main(void) {\n' % header)
            for n in names:
                f.write('    printf("%%s %%d\\n", "%s", (int)(%s));\n' % (n, n))
            f.write("    return 0;\n}\n")
        exe = os.path.join(tmp, "dump")
        subprocess.check_call(["gcc", "-w", "-I", os.path.join(ROOT, "include"), "-include",
                               os.path.join(ROOT, "include", "config", "general.h"), src, "-o", exe])
        out = subprocess.check_output([exe]).decode()
    return {n: int(v) for n, v in (line.split() for line in out.splitlines())}


def main():
    species = enum_values("SPECIES_", "species.h")
    moves = enum_values("MOVE_", "moves.h", ["MOVES_COUNT"])
    items = enum_values("ITEM_", "items.h", ["ITEMS_COUNT"])

    info = "".join(open(f).read() for f in sorted(glob.glob(os.path.join(ROOT, "src/data/pokemon/species_info/gen_*.h"))))
    growth, pre_evo = {}, {}
    for name, body in re.findall(r"\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\n    \},", info, re.S):
        m = re.search(r"\.growthRate\s*=\s*(GROWTH_\w+)", body)
        if m and name in species:
            growth[species[name]] = GROWTH.index(m.group(1))
        evo = re.search(r"\.evolutions\s*=\s*EVOLUTION\((.*?)\)\s*,\s*\n", body, re.S)
        if evo:
            for target in re.findall(r"(SPECIES_\w+)\s*[,}]", evo.group(1)):
                if target in species and name in species:
                    pre_evo.setdefault(species[target], species[name])

    learnables = json.load(open(os.path.join(ROOT, "src/data/pokemon/all_learnables.json")))
    special = json.load(open(os.path.join(ROOT, "src/data/pokemon/special_movesets.json")))
    universal = {moves[m] for m in special.get("universalMoves", []) if m in moves}

    def own_moves(name):
        key = name[len("SPECIES_"):]
        while key and key not in learnables:
            key = key.rsplit("_", 1)[0] if "_" in key else ""
        return {moves[m] for m in learnables.get(key, []) if m in moves}

    by_id = {v: n for n, v in species.items()}
    learn = {}
    for sid, name in by_id.items():
        if sid == 0 or name in ("SPECIES_EGG",):
            continue
        known, seen, cur = set(universal), set(), sid
        while cur is not None and cur not in seen:
            seen.add(cur)
            known |= own_moves(by_id[cur])
            cur = pre_evo.get(cur)
        learn[sid] = sorted(known)

    move_names = {}
    for name, value in moves.items():
        if name != "MOVES_COUNT":
            move_names.setdefault(str(value), name[len("MOVE_"):].replace("_", " ").title())

    data = {
        "species_count": max(species.values()) + 1,
        "species_egg": species.get("SPECIES_EGG", 0),
        "moves_count": moves["MOVES_COUNT"],
        "items_count": items["ITEMS_COUNT"],
        "growth": {str(k): v for k, v in growth.items()},
        "learn": {str(k): v for k, v in learn.items()},
        "move_names": move_names,
    }
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with gzip.open(OUT, "wt") as f:
        json.dump(data, f, separators=(",", ":"))
    print(f"{OUT}: {len(growth)} growth rates, {len(learn)} learnsets")


if __name__ == "__main__":
    main()
