#!/usr/bin/env python3
"""Makes every real Pokémon (all of Gen 1-9, except legendaries, mythicals,
Ultra Beasts and Paradox Pokémon) catchable somewhere in the wild.

For each family no wild table has yet, it picks the places that fit it best:
tables whose own Pokémon share its types (water Pokémon go in the water and
fishing tables, Ghost and Dark ones in night tables), at levels that suit it.
The family's member for that level goes in (e.g. Charmeleon on a level 25
route, Charmander on a level 8 one). It only takes slots that repeat a species
the table already has, so nothing that lived there before goes away, and it
never touches the starting area (Routes 101-104, Petalburg Woods).
Running it again changes nothing.   Usage: add_real.py [--dry]
"""
import collections
import glob
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PATH = os.path.join(ROOT, "src", "data", "wild_encounters.json")
STARTING = ("gRoute101", "gRoute102", "gRoute103", "gRoute104", "gPetalburgWoods")
# Not real places to catch things: battle facilities, the extraction zone,
# FireRed/LeafGreen tables the game doesn't use.
SKIP_LABEL = ("FireRed", "LeafGreen", "gStormwild", "gAdventureDungeon", "gAlteringCave", "Unused",
              "gSkyPillar", "gCaveOfOrigin", "gSunscarRuinsVault", "gIronmoorPrisonSolitary")

# What kind of place each land table is (first match wins).
PLACES = [
    ("volcano", ("gFieryPath", "gMagmaHideout", "gEmberSprings", "gCinderCaverns", "gRoute112", "gJaggedPass", "gRoute203")),
    ("snow", ("gShoalCave_LowTideIceRoom", "gFrostveinCave", "gMtFrostcrown")),
    ("desert", ("gRoute111", "gDesertUnderpass", "gMirageTower", "gSunscarDesert", "gMirageDunes", "gMirageOasis")),
    ("haunted", ("gMtPyre", "gHollowmereManor", "gIronmoorPrison")),
    ("city", ("gNewMauville", "gVoltspirePlant", "gMetroOldLine", "gIronmoorIsle", "gCobblerock_", "gCobblerock")),
    ("deepcave", ("gSeafloorCavern", "gVictoryRoad", "gSolmaraVictoryRoad", "gMeteorFalls", "gGraniteCave_B2F",
                  "gGraniteCave_Stevens", "gPrismCaverns", "gArtisanCave", "gCobblerockMine")),
    ("cave", ("gGraniteCave", "gRusturfTunnel", "gShoalCave")),
    ("mountain", ("gSkyropeCanyon", "gNimbusIsle", "gRoute113", "gRoute205", "gRoute206")),
    ("marsh", ("gMireboughMarsh", "gHollowmere", "gRoute120", "gRoute121", "gRoute122", "gRoute123", "gWhisperstemValley")),
    ("forest", ("gElderwoodForest", "gAmberwoodHollow", "gPetalburgWoods", "gRoute119", "gRoute114", "gRoute116",
                "gRoute204", "gVerdantRiver")),
    ("coast", ("gRoute110", "gRoute115", "gRoute118", "gRoute130", "gGullwingPoint", "gStarfallIsle", "gRoute208",
               "gRoute207", "gRoute201")),
    ("meadow", ("gSafariZone", "gRoute117", "gRoute202", "gRoute1")),
]
# Where each type likes to live, best first.
LIKES = {
    "NORMAL": ["meadow", "coast", "forest", "city"], "FIRE": ["volcano", "desert"],
    "WATER": [], "GRASS": ["forest", "meadow", "marsh"], "ELECTRIC": ["city", "meadow", "coast"],
    "ICE": ["snow"], "FIGHTING": ["mountain", "cave", "deepcave"], "POISON": ["marsh", "forest"],
    "GROUND": ["desert", "cave", "mountain"], "FLYING": ["coast", "mountain", "meadow"],
    "PSYCHIC": ["meadow", "deepcave", "desert"], "BUG": ["forest", "meadow", "marsh"],
    "ROCK": ["cave", "mountain", "deepcave", "desert"], "GHOST": ["haunted", "marsh"],
    "DRAGON": ["deepcave", "mountain", "desert"], "DARK": ["city", "forest", "cave"],
    "STEEL": ["deepcave", "city", "mountain"], "FAIRY": ["meadow", "forest"],
}


def place_of(label):
    for kind, prefixes in PLACES:
        if label.startswith(prefixes):
            return kind
    return None
PER_FAMILY = 2      # places per family
PER_TABLE = 6       # new families per table at most
NIGHT_TYPES = {"GHOST", "DARK"}


def load_species():
    info = {}
    macros = {}
    for f in sorted(glob.glob(os.path.join(ROOT, "src/data/pokemon/species_info/gen_*_families.h"))):
        for m in re.finditer(r"#define (\w+_TYPES)\s+(?:MON_TYPES\(([^)]*)\)|\{([^}]*)\})", open(f).read()):
            macros.setdefault(m.group(1), m.group(2) or m.group(3))
    for f in sorted(glob.glob(os.path.join(ROOT, "src/data/pokemon/species_info/gen_*_families.h"))):
        text = open(f).read()
        for m in re.finditer(r"\n    \[SPECIES_(\w+)\]\s*=\s*\n    \{(.*?)\n    \},", text, re.S):
            name, body = m.group(1), m.group(2)
            nat = re.search(r"\.natDexNum = NATIONAL_DEX_(\w+)", body)
            types = re.search(r"\.types = MON_TYPES\(([^)]*)\)", body)
            if not types:
                mac = re.search(r"\.types = (\w+)", body)
                if mac and mac.group(1) in macros:
                    types = re.match(r"(.*)", macros[mac.group(1)])
            evo = re.search(r"\.evolutions = EVOLUTION\((.*?)\),(?:\n|$)", body, re.S)
            evos = []
            if evo:
                for e in re.finditer(r"\{(EVO_\w+),\s*([^,]+),\s*SPECIES_(\w+)", evo.group(1)):
                    evos.append((e.group(1), e.group(2).strip(), e.group(3)))
            info[name] = {
                "nat": nat.group(1) if nat else None,
                "types": sorted({t.strip()[5:] for t in types.group(1).split(",")}) if types else [],
                "evos": evos,
                "special": any(re.search(r"\.%s = TRUE" % k, body) for k in
                               ("isLegendary", "isMythical", "isUltraBeast", "isParadox",
                                "isSubLegendary", "isRestrictedLegendary")),
            }
    return info


def main():
    dry = "--dry" in sys.argv
    info = load_species()
    data = json.load(open(PATH))
    tables = [e for e in data["wild_encounter_groups"][0]["encounters"]
              if not any(s in e["base_label"] for s in SKIP_LABEL)]

    # Base form of each species (the one nothing evolves into), only for the
    # main form whose name is the dex name.
    pre = {}
    for n, s in info.items():
        for _, _, t in s["evos"]:
            pre.setdefault(t, n)
    main = {n for n, s in info.items() if s["nat"] == n}

    def base(n):
        seen = set()
        while n in pre and n not in seen:
            seen.add(n)
            n = pre[n]
        return n

    def family(b):
        out, todo = [], [b]
        while todo:
            n = todo.pop()
            if n in out:
                continue
            out.append(n)
            todo += [t for _, _, t in info.get(n, {}).get("evos", [])]
        return out

    wild = set()
    for e in tables:
        for k, v in e.items():
            if isinstance(v, dict) and "mons" in v:
                wild |= {m["species"][8:] for m in v["mons"]}
    have = {base(n) for n in wild if n in info}
    missing = sorted({base(n) for n in main} - have)
    missing = [b for b in missing if b in info and not info[b]["special"]
               and not any(info.get(x, {}).get("special") for x in family(b))]

    # Who lives in each table, and its levels.
    def natives(e, key):
        return [m["species"][8:] for m in e[key]["mons"]] if key in e else []

    def level(e, key):
        mons = e[key]["mons"]
        return sum(m["max_level"] for m in mons) / len(mons)

    added = collections.Counter()
    taken = set()
    plan = []

    def stage_for(b, lvl):
        """The family member a wild one at this level would be."""
        n = b
        for _ in range(3):
            nxt = [(int(p), t) for kind, p, t in info.get(n, {}).get("evos", [])
                   if kind == "EVO_LEVEL" and p.isdigit() and t in main]
            if len(nxt) == 1 and lvl >= nxt[0][0] + 4:
                n = nxt[0][1]
            else:
                break
        return n

    def free_slots(e, key):
        mons = e[key]["mons"]
        seen, out = set(), []
        for i, m in enumerate(mons):
            if m["species"] in seen:
                out.append(i)
            seen.add(m["species"])
        if not out and e["base_label"].endswith("_Day"):
            # A daytime copy: its usual Pokémon still live there at other
            # times, so its rarer half can go to newcomers.
            out = [i for i in range(len(mons) // 2, len(mons)) if (e["base_label"], i) not in taken]
        return out[::-1]  # rarest first

    def fit(types, e, key):
        """How well a family with these types fits this table (0: not at all)."""
        if key in ("water_mons", "fishing_mons"):
            return 1.0 if "WATER" in types else 0.0
        if key == "rock_smash_mons":
            return 1.0 if types & {"ROCK", "GROUND"} else 0.0
        if "WATER" in types and len(types) == 1:
            return 0.0
        kind = place_of(e["base_label"])
        best = 0.0
        for t in types:
            likes = LIKES.get(t, [])
            if kind in likes:
                best = max(best, 1.0 - 0.2 * likes.index(kind))
        nat = natives(e, key)
        share = sum(1 for n in nat if n in info and types & set(info[n]["types"])) / max(1, len(nat))
        return best + 0.3 * share if best else 0.0

    def keys_for(types):
        keys = ["water_mons", "fishing_mons"] if "WATER" in types else []
        if types - {"WATER"} or "WATER" not in types:
            keys.append("land_mons")
        if types & {"ROCK", "GROUND"}:
            keys.append("rock_smash_mons")
        return keys

    def options(b):
        types = set(info[b]["types"])
        return sum(1 for e in tables for k in keys_for(types) if k in e and fit(types, e, k) > 0)

    missing.sort(key=options)
    for b in missing:
        types = set(info[b]["types"])
        night = bool(types & NIGHT_TYPES)
        scored = []
        for e in tables:
            if e["base_label"].startswith(STARTING) or added[e["base_label"]] >= PER_TABLE:
                continue
            is_night = e["base_label"].endswith("_Night")
            for key in keys_for(types):
                if key not in e or not free_slots(e, key):
                    continue
                score = fit(types, e, key)
                if score <= 0:
                    continue
                if is_night:
                    score += 0.5 if night else -0.3
                scored.append((score, e["base_label"], key, e))
        if len({lbl.replace("_Night", "").replace("_Day", "") for _, lbl, _, _ in scored}) < PER_FAMILY:
            # The fitting places are full: give the best one a daytime table
            # (a copy of its usual one, which then has room for newcomers).
            new = []
            for e in list(tables):
                lbl = e["base_label"]
                if "land_mons" not in e or lbl.endswith(("_Night", "_Day")) or lbl.startswith(STARTING):
                    continue
                if any(t["base_label"] == lbl + "_Day" for t in tables):
                    continue
                f = fit(types, e, "land_mons")
                if f > 0:
                    new.append((f, e))
            new.sort(key=lambda x: -x[0])
            for f, e in new[:PER_FAMILY]:
                day = {"map": e["map"], "base_label": e["base_label"] + "_Day",
                       "land_mons": json.loads(json.dumps(e["land_mons"]))}
                enc = data["wild_encounter_groups"][0]["encounters"]
                enc.insert(enc.index(e) + 1, day)
                tables.append(day)
                scored.append((f, day["base_label"], "land_mons", day))
        scored.sort(key=lambda s: -s[0])
        used = set()
        for score, label, key, e in scored:
            if len(used) >= PER_FAMILY:
                break
            place = label.replace("_Night", "").replace("_Day", "")
            if place in used or added[label] >= PER_TABLE:
                continue
            slots = free_slots(e, key)
            if not slots:
                continue
            i = slots[0]
            mon = e[key]["mons"][i]
            sp = stage_for(b, mon["max_level"])
            mon["species"] = "SPECIES_" + sp
            taken.add((label, i))
            added[label] += 1
            used.add(place)
            plan.append((b, sp, label, key, mon["min_level"], mon["max_level"]))
    for p in plan:
        print("%-12s %-12s %-28s %-15s Lv%d-%d" % p)
    placed = {p[0] for p in plan}
    print("families missing:", len(missing), "placed:", len(placed))
    print("not placed:", " ".join(b for b in missing if b not in placed))
    if not dry:
        raw = open(PATH).read()
        open(PATH, "w").write(json.dumps(data, indent=2) + ("\n" if raw.endswith("\n") else ""))


if __name__ == "__main__":
    main()
