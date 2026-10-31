"""Fusion breeding on the server (docs/breed_fusion.md).

The game asks the server to build a fusion (COMMUNITY_REQ_FUSION_BUILD,
a0 = the mother's species, which is the baby's shape, a1 = the father's
species) whenever its party holds one it hasn't got yet: a fresh Day Care
egg, a fusion that just evolved, or one it traded for. The server answers
with the fusion's colors, which the game saves and paints onto the baby's
own sprite (src/breed_fusion.c), and with its name, types and base stats.
Those three follow the same rules in the game, so two games always agree
in a link battle even if one of them hasn't synced yet.

What the server knows about each species comes from
server/data/fusion.json.gz (tools/breed_fusion/make_fusion_data.py).
"""

import gzip
import json
import os
import struct

from community import Answer, OFFLINE, request

REQ_BUILD = 70
CANT_FUSE = 2
COLORS = 4  # FUSION_RECIPE_COLORS
NAME_LENGTH = 12  # POKEMON_NAME_LENGTH
DATA_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "fusion.json.gz")
STAT_NAMES = ["HP", "Atk", "Def", "Spe", "SpA", "SpD"]

_data = None


def data():
    global _data
    if _data is None:
        with gzip.open(DATA_PATH, "rt") as f:
            raw = json.load(f)
        raw["species"] = {int(k): v for k, v in raw["species"].items()}
        _data = raw
    return _data


def can_fuse_with(partner):
    """Whether `partner` is a species a fusion can have as its father."""
    info = data()["species"].get(partner)
    return bool(info) and info["breeds"] and not info["ditto"]


def fusion_name(a, b):
    """The first half of the mother's name and the last half of the
    father's (Wingull + Pikachu = Wingchu), as BreedFusion_BufferName."""
    a, b = a[:NAME_LENGTH], b[:NAME_LENGTH]
    head = a[:(len(a) + 1) // 2]
    start = len(b) - len(b) // 2
    while start < len(b) and not b[start].isalpha():
        start += 1
    return (head + b[start:].lower())[:NAME_LENGTH]


def fusion_types(mother, father):
    """The mother's first type and the father's first type (or his second,
    if the first matches), as BreedFusion_GetTypes. Type ids."""
    t = list(mother["type_ids"])
    f = father["type_ids"]
    if f[0] != t[0]:
        t[1] = f[0]
    elif f[1] != t[0]:
        t[1] = f[1]
    return t


def fusion_stats(mother, father):
    """The average of both base stats, rounded up, as BreedFusion_GetBaseStat."""
    return [(a + b + 1) // 2 for a, b in zip(mother["stats"], father["stats"])]


def build(species, partner):
    """The fusion of `species` (shape) and `partner` (colors), or None."""
    d = data()["species"]
    mother, father = d.get(species), d.get(partner)
    if not mother or not father or not can_fuse_with(partner) or not mother["breeds"] or mother["ditto"]:
        return None
    colors = father["colors"][:COLORS]
    shiny = (father["shiny_colors"] or father["colors"])[:COLORS]
    if not colors:
        return None
    type_names = {i: n for m in d.values() for i, n in zip(m["type_ids"], m["types"])}
    types = fusion_types(mother, father)
    return {
        "name": fusion_name(mother["name"], father["name"]),
        "types": types,
        "type_names": [type_names.get(t, "?") for t in types],
        "stats": fusion_stats(mother, father),
        "colors": colors,
        "shiny_colors": shiny,
    }


def pack(species, partner, fusion):
    """The answer's bytes, as SaveRecipe reads them: species, partner, the
    number of colors, a spare byte, the colors and the shiny colors."""
    def pad(c):
        return (list(c) + [0] * COLORS)[:COLORS]
    return struct.pack("<HHBB%dH%dH" % (COLORS, COLORS), species, partner, len(fusion["colors"][:COLORS]), 0,
                       *pad(fusion["colors"]), *pad(fusion["shiny_colors"][:len(fusion["colors"][:COLORS])]))


@request(REQ_BUILD)
def build_request(server, player, args, data_):
    species, partner = args[0] & 0xFFFF, args[1] & 0xFFFF
    try:
        fusion = build(species, partner)
    except (OSError, ValueError):
        return Answer("FUSION", ["The server can't build fusions."], status=OFFLINE)
    if fusion is None:
        return Answer("FUSION", ["These two can't fuse."], status=CANT_FUSE, args=(species, partner))
    t = fusion["type_names"]
    rows = [fusion["name"], "/".join(t if t[0] != t[1] else t[:1]),
            " ".join(f"{n} {v}" for n, v in zip(STAT_NAMES[:3], fusion["stats"][:3])),
            " ".join(f"{n} {v}" for n, v in zip(STAT_NAMES[3:], fusion["stats"][3:]))]
    return Answer("FUSION", rows, args=(species, partner, *fusion["types"]), data=pack(species, partner, fusion))
