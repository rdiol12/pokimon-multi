#!/usr/bin/env python3
"""Writes the cast of roaming trainers ("rovers"): src/data/rovers_cast.h
for the game and server/data/rovers_cast.json for the server.

Each rover has a name, a look (an Emerald trainer's overworld sprite,
battle picture and class), a favourite type, a style and a first Pokémon.

Run from the repository root after changing the cast:
    python3 tools/rovers/make_cast.py
"""
import json
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools", "community"))
from make_legal_data import enum_values  # noqa: E402

# gfx, trainer pic, trainer class, female, encounter music
LOOKS = [
    ("YOUNGSTER", "YOUNGSTER", "YOUNGSTER", 0, "MALE"),
    ("LASS", "LASS", "LASS", 1, "FEMALE"),
    ("BUG_CATCHER", "BUG_CATCHER", "BUG_CATCHER", 0, "MALE"),
    ("HIKER", "HIKER", "HIKER", 0, "HIKER"),
    ("CAMPER", "CAMPER", "CAMPER", 0, "MALE"),
    ("PICNICKER", "PICNICKER", "PICNICKER", 1, "GIRL"),
    ("COOLTRAINER_M", "COOLTRAINER_M", "COOLTRAINER", 0, "COOL"),
    ("COOLTRAINER_F", "COOLTRAINER_F", "COOLTRAINER", 1, "COOL"),
    ("BLACK_BELT", "BLACK_BELT", "BLACK_BELT", 0, "INTENSE"),
    ("CRUSH_GIRL", "BATTLE_GIRL", "BATTLE_GIRL", 1, "INTENSE"),
    ("PSYCHIC_M", "PSYCHIC_M", "PSYCHIC", 0, "INTENSE"),
    ("HEX_MANIAC", "HEX_MANIAC", "HEX_MANIAC", 1, "SUSPICIOUS"),
    ("EXPERT_M", "EXPERT_M", "EXPERT", 0, "INTENSE"),
    ("EXPERT_F", "EXPERT_F", "EXPERT", 1, "INTENSE"),
    ("NINJA_BOY", "NINJA_BOY", "NINJA_BOY", 0, "SUSPICIOUS"),
    ("BEAUTY", "BEAUTY", "BEAUTY", 1, "FEMALE"),
    ("GENTLEMAN", "GENTLEMAN", "GENTLEMAN", 0, "RICH"),
    ("RICH_BOY", "RICH_BOY", "RICH_BOY", 0, "RICH"),
    ("POKEFAN_M", "POKEFAN_M", "POKEFAN", 0, "TWINS"),
    ("POKEFAN_F", "POKEFAN_F", "POKEFAN", 1, "TWINS"),
    ("FISHERMAN", "FISHERMAN", "FISHERMAN", 0, "HIKER"),
    ("SAILOR", "SAILOR", "SAILOR", 0, "MALE"),
    ("RUNNING_TRIATHLETE_M", "RUNNING_TRIATHLETE_M", "TRIATHLETE", 0, "MALE"),
    ("RUNNING_TRIATHLETE_F", "RUNNING_TRIATHLETE_F", "TRIATHLETE", 1, "FEMALE"),
    ("SCHOOL_KID_M", "SCHOOL_KID_M", "SCHOOL_KID", 0, "MALE"),
    ("ROCKER", "GUITARIST", "GUITARIST", 0, "INTENSE"),
    ("TUBER_F", "TUBER_F", "TUBER_F", 1, "GIRL"),
    ("TUBER_M_LAND", "TUBER_M", "TUBER_M", 0, "GIRL"),
]

# Which looks suit which favourite type, and a first Pokémon for each type.
TYPES = {
    "NORMAL": (["YOUNGSTER", "LASS", "POKEFAN_M", "POKEFAN_F", "SCHOOL_KID_M"], "ZIGZAGOON"),
    "FIGHTING": (["BLACK_BELT", "CRUSH_GIRL", "EXPERT_M"], "MAKUHITA"),
    "FLYING": (["COOLTRAINER_F", "SAILOR", "RUNNING_TRIATHLETE_F"], "TAILLOW"),
    "POISON": (["HEX_MANIAC", "NINJA_BOY"], "GULPIN"),
    "GROUND": (["HIKER", "CAMPER"], "PHANPY"),
    "ROCK": (["HIKER", "CAMPER", "EXPERT_M"], "NOSEPASS"),
    "BUG": (["BUG_CATCHER", "SCHOOL_KID_M"], "WURMPLE"),
    "GHOST": (["HEX_MANIAC", "PSYCHIC_M"], "SHUPPET"),
    "STEEL": (["COOLTRAINER_M", "RICH_BOY", "GENTLEMAN"], "ARON"),
    "FIRE": (["ROCKER", "COOLTRAINER_M", "CAMPER"], "NUMEL"),
    "WATER": (["FISHERMAN", "SAILOR", "TUBER_F", "TUBER_M_LAND"], "MARILL"),
    "GRASS": (["PICNICKER", "LASS", "BEAUTY"], "SEEDOT"),
    "ELECTRIC": (["ROCKER", "RUNNING_TRIATHLETE_M", "SCHOOL_KID_M"], "ELECTRIKE"),
    "PSYCHIC": (["PSYCHIC_M", "BEAUTY", "EXPERT_F"], "RALTS"),
    "ICE": (["EXPERT_F", "COOLTRAINER_F"], "SNORUNT"),
    "DRAGON": (["COOLTRAINER_M", "COOLTRAINER_F", "EXPERT_F"], "BAGON"),
    "DARK": (["NINJA_BOY", "ROCKER", "COOLTRAINER_M"], "POOCHYENA"),
    "FAIRY": (["LASS", "BEAUTY", "PICNICKER", "TUBER_F"], "AZURILL"),
}
STYLES = ["COLLECTOR", "TRAINER", "BALANCED", "BREEDER"]

NAMES_M = """Arlo Bram Cass Dax Eli Finn Gus Hal Ivo Jace Kai Leo Milo Nico Oren Pax Quin Rex Sol Tobi Uri Vic Wes Xan Yuri Zed
Ash Beck Cole Dane Ezra Ford Gray Hugh Iggy Jett Knox Lane Moss Nash Otto Pike Reed Sage Tate Vale Wade York Zane Abe Boyd
Cyrus Drew Emil Flint Gale Heath Isak Joel Kit Lyle Marv Ned Ozzy Perry Rhys Stan Teo Vern Wylie Zeke Basil
Axel Brock Clay Dean Enzo Felix Glen Hank Jules Kurt Linus Max Noel Orrin Pete Roy Seth Theo Vince Walt Yves Rafe""".split()
NAMES_F = """Ada Bea Cleo Dot Eve Fay Gwen Hana Iris Jo Kira Lia Mae Nell Opal Pia Rue Suki Tess Una Vera Wren Yara Zoe Amy Bree
Cora Dina Edie Fern Gia Hope Ivy June Kay Lola Mira Nina Olive Pearl Rosa Sky Tara Uma Viv Willa Zia Aria Bella Cami Dara
Elsa Flora Gemma Hazel Ines Jade Kiki Luna Maya Nora Poppy Ria Sasha Tilly Vivi Yumi Zara Amber Bonnie Coral Daisy Esme""".split()


def main():
    species = enum_values("SPECIES_", "species.h")
    looks = {g: i for i, (g, *_r) in enumerate(LOOKS)}
    type_names = list(TYPES)
    cast, used = [], set()
    males, females = iter(NAMES_M), iter(NAMES_F)
    for i in range(150):
        t = type_names[(i * 7) % len(type_names)]
        options = TYPES[t][0]
        gfx = options[(i // len(type_names)) % len(options)]
        look = looks[gfx]
        name = next(females if LOOKS[look][3] else males)
        assert name not in used and len(name) <= 7, name
        used.add(name)
        cast.append({"name": name, "look": look, "type": t, "style": STYLES[(i * 5 + i // 18) % len(STYLES)],
                     "starter": TYPES[t][1]})

    with open(os.path.join(ROOT, "src", "data", "rovers_cast.h"), "w") as f:
        f.write("// Generated by tools/rovers/make_cast.py. Do not edit.\n\n")
        f.write("static const struct RoverLook sRoverLooks[] =\n{\n")
        for gfx, pic, cls, female, music in LOOKS:
            f.write("    {OBJ_EVENT_GFX_%s, TRAINER_PIC_%s, TRAINER_CLASS_%s, %d, TRAINER_ENCOUNTER_MUSIC_%s},\n"
                    % (gfx, pic, cls, female, music))
        f.write("};\n\nconst struct RoverCast gRoverCast[ROVER_COUNT] =\n{\n")
        for r in cast:
            f.write('    {_("%s"), %d, TYPE_%s, ROVER_STYLE_%s, SPECIES_%s},\n'
                    % (r["name"], r["look"], r["type"], r["style"], r["starter"]))
        f.write("};\n")

    with open(os.path.join(ROOT, "server", "data", "rovers_cast.json"), "w") as f:
        out = [{"name": r["name"], "female": LOOKS[r["look"]][3], "type": r["type"], "style": r["style"],
                "starter": species["SPECIES_" + r["starter"]]} for r in cast]
        json.dump(out, f, indent=0)
    print("%d rovers" % len(cast))


if __name__ == "__main__":
    main()
