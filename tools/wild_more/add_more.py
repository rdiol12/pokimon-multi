#!/usr/bin/env python3
"""Gives Hoenn's routes and caves more kinds of wild Pokémon.

Each table keeps all the Pokémon it already had (the common slots are
untouched); the rarer slots that only repeated a species already on the table
get a new one that fits the place and its levels. Run it once on
src/data/wild_encounters.json; running it again changes nothing.
Usage: add_more.py [--dry]
"""
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PATH = os.path.join(ROOT, "src", "data", "wild_encounters.json")

# Table label -> (new land Pokémon, new water Pokémon), rarest last.
# Picked to fit the place (grass, forest, cave, desert, sea...) and its levels;
# the starting routes only get Pokémon from Hoenn itself.
SEA_EARLY = ["CHINCHOU", "WAILMER", "MANTYKE"]
SEA_LATE = ["CHINCHOU", "WAILMER", "LAPRAS"]
SEA_DEEP = ["LANTURN", "WAILMER", "MANTINE"]
FRESH = ["SURSKIT", "WOOPER", "AZUMARILL"]
MORE = {
    "gRoute101": (["TAILLOW", "NINCADA", "WHISMUR"], []),
    "gRoute101_Night": (["NINCADA", "SHROOMISH"], []),
    "gRoute102": (["SURSKIT", "TAILLOW", "SHROOMISH"], ["SURSKIT", "LOTAD"]),
    "gRoute102_Night": (["SHROOMISH", "SURSKIT"], []),
    "gRoute103": (["TAILLOW", "ELECTRIKE", "PLUSLE", "MINUN"], SEA_EARLY),
    "gRoute103_Night": (["ELECTRIKE", "ZUBAT"], []),
    "gRoute104": (["SEEDOT", "SURSKIT", "LOTAD"], ["SURSKIT", "WAILMER", "MARILL"]),
    "gRoute104_Night": (["SEEDOT", "SHROOMISH"], []),
    "gRoute105": ([], SEA_EARLY), "gRoute106": ([], SEA_EARLY), "gRoute107": ([], SEA_EARLY),
    "gRoute108": ([], SEA_EARLY), "gRoute109": ([], SEA_EARLY),
    "gPetalburgWoods": (["NINCADA", "SEWADDLE", "PIKIPEK", "SKWOVET"], []),
    "gPetalburgWoods_Night": (["NINCADA", "NOIBAT", "PHANTUMP"], []),
    "gRusturfTunnel": (["ZUBAT", "GEODUDE", "WOOBAT", "ROLYCOLY", "NOSEPASS", "DRILBUR"], []),
    "gGraniteCave_1F": (["WOOBAT", "TIMBURR", "NOSEPASS"], []),
    "gGraniteCave_B1F": (["GEODUDE", "WOOBAT", "NOSEPASS", "MAWILE"], []),
    "gGraniteCave_B2F": (["GEODUDE", "NOSEPASS", "MAWILE", "CARBINK"], []),
    "gGraniteCave_StevensRoom": (["NOSEPASS", "SABLEYE", "MAWILE"], []),
    "gRoute110": (["MAGNEMITE", "VOLTORB", "PAWMI", "TOGEDEMARU"], SEA_EARLY),
    "gRoute110_Night": (["VOLTORB", "NOIBAT"], []),
    "gRoute111": (["DIGLETT", "SANDILE", "SILICOBRA", "GIBLE", "LARVESTA"], FRESH),
    "gRoute112": (["MACHOP", "SLUGMA", "TORKOAL", "SIZZLIPEDE"], []),
    "gRoute113": (["SANDSHREW", "MUDBRAY", "TORKOAL"], []),
    "gRoute113_Night": (["NOIBAT", "HOUNDOUR"], []),
    "gRoute114": (["ZANGOOSE", "SEVIPER", "NUZLEAF", "SKIDDO"], FRESH),
    "gRoute115": (["JIGGLYPUFF", "TROPIUS", "ROOKIDEE", "BAGON"], SEA_EARLY),
    "gRoute116": (["SKITTY", "BUNNELBY", "MINCCINO", "SMEARGLE"], []),
    "gRoute116_Night": (["SHUPPET", "MURKROW"], []),
    "gRoute117": (["ROSELIA", "PETILIL", "CUTIEFLY", "FLABEBE"], FRESH),
    "gRoute117_Night": (["ROSELIA", "MORELULL", "VENONAT"], []),
    "gRoute118": (["WINGULL", "PIDOVE", "MANECTRIC", "TOGEDEMARU"], SEA_LATE),
    "gRoute119": (["TROPIUS", "YANMA", "HERACROSS", "CASTFORM"], SEA_LATE),
    "gRoute119_Night": (["YANMA", "NOCTOWL", "VENOMOTH"], []),
    "gRoute120": (["NUZLEAF", "ABSOL", "SEEDOT", "PHANTUMP"], FRESH),
    "gRoute120_Night": (["ZORUA", "SABLEYE", "PHANTUMP"], []),
    "gRoute121": (["SHUPPET", "DUSKULL", "CHIMECHO", "ZORUA"], SEA_LATE),
    "gRoute122": ([], SEA_LATE),
    "gRoute123": (["KECLEON", "SKARMORY", "ZANGOOSE", "SEVIPER"], SEA_LATE),
    "gRoute123_Night": (["DUSKULL", "SABLEYE", "SHUPPET"], []),
    "gRoute124": ([], SEA_DEEP), "gRoute125": ([], SEA_DEEP), "gRoute126": ([], SEA_DEEP),
    "gRoute127": ([], SEA_DEEP), "gRoute128": ([], ["LUVDISC", "CORSOLA", "RELICANTH"]),
    "gRoute129": ([], SEA_DEEP), "gRoute130": ([], SEA_DEEP), "gRoute131": ([], SEA_DEEP),
    "gRoute132": ([], ["SHARPEDO", "MANTINE", "WAILMER"]), "gRoute133": ([], ["SHARPEDO", "MANTINE", "WAILMER"]),
    "gRoute134": ([], ["SHARPEDO", "MANTINE", "LAPRAS"]),
    "gFieryPath": (["TORKOAL", "SIZZLIPEDE", "HEATMOR", "LITWICK"], []),
    "gJaggedPass": (["CAMERUPT", "SPOINK", "TORKOAL", "BALTOY"], []),
    "gMtPyre_1F": (["DUSKULL", "LITWICK", "MISDREAVUS", "SINISTEA"], []),
    "gMtPyre_2F": (["DUSKULL", "LITWICK", "MISDREAVUS", "GREAVARD"], []),
    "gMtPyre_3F": (["DUSKULL", "GASTLY", "MISDREAVUS", "GREAVARD"], []),
    "gMtPyre_4F": (["GASTLY", "LITWICK", "SINISTEA"], []),
    "gMtPyre_5F": (["GASTLY", "MISDREAVUS", "HONEDGE"], []),
    "gMtPyre_6F": (["HAUNTER", "LAMPENT", "HONEDGE"], []),
    "gMtPyre_Exterior": (["DUSKULL", "MISDREAVUS", "CHIMECHO"], []),
    "gMtPyre_Summit": (["BANETTE", "MISDREAVUS", "LAMPENT"], []),
    "gMeteorFalls_1F_1R": (["LUNATONE", "ROGGENROLA", "DRILBUR", "BAGON"], ["MARILL", "BARBOACH", "WOOPER"]),
    "gMeteorFalls_1F_2R": (["LUNATONE", "BOLDORE", "SHELGON"], ["MARILL", "BARBOACH", "WHISCASH"]),
    "gMeteorFalls_B1F_1R": (["LUNATONE", "BOLDORE", "SHELGON"], ["MARILL", "BARBOACH", "WHISCASH"]),
    "gMeteorFalls_B1F_2R": (["LUNATONE", "BOLDORE", "SHELGON"], ["MARILL", "BARBOACH", "WHISCASH"]),
    "gMeteorFalls_StevensCave": (["LUNATONE", "BOLDORE", "SHELGON"], []),
    "gNewMauville_Entrance": (["ROTOM", "KLINK", "TOGEDEMARU"], []),
    "gNewMauville_Inside": (["ROTOM", "KLINK", "TOGEDEMARU"], []),
    "gSeafloorCavern_Room1": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Room2": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Room3": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Room4": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Room5": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Room6": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], ["SEALEO", "CLAUNCHER", "LANTURN"]),
    "gSeafloorCavern_Room7": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], ["SEALEO", "CLAUNCHER", "LANTURN"]),
    "gSeafloorCavern_Room8": (["SABLEYE", "BOLDORE", "WOOBAT", "CROBAT"], []),
    "gSeafloorCavern_Entrance": ([], ["SEALEO", "CLAUNCHER", "LANTURN"]),
    "gCaveOfOrigin_Entrance": (["SABLEYE", "BOLDORE", "SWOOBAT"], []),
    "gCaveOfOrigin_1F": (["BOLDORE", "SWOOBAT", "CROBAT"], []),
    "gShoalCave_LowTideStairsRoom": (["SNORUNT", "SNOM", "CUBCHOO"], []),
    "gShoalCave_LowTideLowerRoom": (["SNORUNT", "SNOM", "BERGMITE"], []),
    "gShoalCave_LowTideInnerRoom": (["SNORUNT", "SNOM", "CUBCHOO"], ["SEALEO", "SPHEAL", "LAPRAS"]),
    "gShoalCave_LowTideEntranceRoom": (["SNORUNT", "SNOM", "CUBCHOO"], ["SEALEO", "SPHEAL", "LAPRAS"]),
    "gShoalCave_LowTideIceRoom": (["SNOM", "BERGMITE", "SWINUB", "DELIBIRD"], []),
    "gVictoryRoad_1F": (["LAIRON", "SABLEYE", "MEDITITE", "AXEW"], []),
    "gVictoryRoad_B1F": (["MEDICHAM", "SABLEYE", "BOLDORE", "GABITE"], []),
    "gVictoryRoad_B2F": (["MEDICHAM", "CROBAT", "BOLDORE", "GABITE"], ["WHISCASH", "BARBOACH", "AZUMARILL"]),
    "gSkyPillar_1F": (["ALTARIA", "DUSCLOPS", "BRONZOR"], []),
    "gSkyPillar_3F": (["ALTARIA", "DUSCLOPS", "BRONZOR"], []),
    "gSkyPillar_5F": (["DUSCLOPS", "BRONZONG", "DRAGONAIR"], []),
    "gMagmaHideout_1F": (["SLUGMA", "NUMEL", "MAGBY", "ROLYCOLY"], []),
    "gMagmaHideout_2F_1R": (["SLUGMA", "NUMEL", "MAGBY", "ROLYCOLY"], []),
    "gMagmaHideout_2F_2R": (["SLUGMA", "NUMEL", "MAGBY", "ROLYCOLY"], []),
    "gMagmaHideout_2F_3R": (["SLUGMA", "NUMEL", "MAGBY", "ROLYCOLY"], []),
    "gMagmaHideout_3F_1R": (["SLUGMA", "CAMERUPT", "MAGBY", "CARKOL"], []),
    "gMagmaHideout_3F_2R": (["SLUGMA", "CAMERUPT", "MAGBY", "CARKOL"], []),
    "gMagmaHideout_3F_3R": (["SLUGMA", "CAMERUPT", "MAGBY", "CARKOL"], []),
    "gMagmaHideout_4F": (["SLUGMA", "CAMERUPT", "MAGMAR", "CARKOL"], []),
    "gMirageTower_1F": (["BALTOY", "CACNEA", "SANDILE"], []),
    "gMirageTower_2F": (["BALTOY", "CACNEA", "SANDILE"], []),
    "gMirageTower_3F": (["BALTOY", "CACNEA", "HIPPOPOTAS"], []),
    "gMirageTower_4F": (["BALTOY", "VIBRAVA", "HIPPOPOTAS"], []),
    "gDesertUnderpass": (["DRILBUR", "ROGGENROLA", "SANDILE"], []),
    "gAbandonedShip_Rooms_B1F": ([], ["FRILLISH", "WAILMER", "CHINCHOU"]),
    "gAbandonedShip_HiddenFloorCorridors": ([], ["FRILLISH", "WAILMER", "CHINCHOU"]),
}

LAND, WATER = "land_mons", "water_mons"


def species_ok():
    text = open(os.path.join(ROOT, "include", "constants", "species.h"), encoding="utf-8").read()
    return set(re.findall(r"^\s+SPECIES_(\w+) = ", text, re.M))


def widen(mons, new):
    """Puts each new species on a rare slot that only repeated another one."""
    changed = 0
    have = {m["species"] for m in mons}
    for sp in new:
        full = "SPECIES_" + sp
        if full in have:
            continue
        # The rarest slot (last) whose species also sits on another slot.
        for i in range(len(mons) - 1, 0, -1):
            cur = mons[i]["species"]
            if sum(1 for m in mons if m["species"] == cur) > 1:
                mons[i] = dict(mons[i], species=full)
                have.add(full)
                changed += 1
                break
    return changed


def main():
    dry = "--dry" in sys.argv
    ok = species_ok()
    bad = [s for land, water in MORE.values() for s in land + water if s not in ok]
    if bad:
        sys.exit("unknown species: " + ", ".join(sorted(set(bad))))
    data = json.load(open(PATH, encoding="utf-8"))
    group = next(g for g in data["wild_encounter_groups"] if g["label"] == "gWildMonHeaders")
    seen, before, after = set(), 0, 0
    for e in group["encounters"]:
        label = e.get("base_label")
        if label not in MORE:
            continue
        seen.add(label)
        for key, new in zip((LAND, WATER), MORE[label]):
            if key in e and new:
                n0 = len({m["species"] for m in e[key]["mons"]})
                widen(e[key]["mons"], new)
                before += n0
                after += len({m["species"] for m in e[key]["mons"]})
    missing = set(MORE) - seen
    if missing:
        sys.exit("no such tables: " + ", ".join(sorted(missing)))
    print(f"kinds per table, summed: {before} -> {after}")
    if not dry:
        with open(PATH, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            f.write("\n")


if __name__ == "__main__":
    main()
