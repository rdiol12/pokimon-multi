#!/usr/bin/env python3
"""Writes server/data/rovers_world.json.gz: the map graph the roaming
trainers (server/rovers.py) travel on, and what lives on each map.

Every outdoor map and cave is a node. Two nodes are joined when the game
joins them: a map connection, a warp, or a warp through one indoor map
(a gatehouse or a tunnel entrance). Each node lists the land Pokémon its
wild encounter table has, with their levels.

Run from the repository root after adding or changing maps:
    python3 tools/rovers/make_rover_world.py
"""
import glob
import gzip
import json
import re
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools", "community"))
from make_legal_data import enum_values  # noqa: E402

OUT = os.path.join(ROOT, "server", "data", "rovers_world.json.gz")
# Story and legendary places a trainer wouldn't wander into.
SKIP = ("MagmaHideout", "AquaHideout", "SkyPillar", "MirageTower", "TerraCave", "MarineCave", "AbandonedShip",
        "SafariZone", "AncientTomb", "DesertRuins", "IslandCave", "SealedChamber", "GraniteCave_StevensRoom",
        "MeteorFalls_StevensCave", "Underwater", "SouthernIsland", "FarawayIsland", "BirthIsland", "NavelRock",
        "HavenReserve")
# Laurel City (the Rover League, tools/areas/laurel.py): the league ferry from Slateport.
EXTRA_LINKS = [("MAP_LAUREL_CITY", "MAP_SLATEPORT_CITY")]
# Healing without a Pokémon Center building: the stadium's nurse.
EXTRA_CENTERS = {"MAP_LAUREL_CITY"}
KINDS = {"MAP_TYPE_ROUTE": "route", "MAP_TYPE_TOWN": "town", "MAP_TYPE_CITY": "town",
         "MAP_TYPE_UNDERGROUND": "cave", "MAP_TYPE_OCEAN_ROUTE": "sea"}


def pretty(name):
    """Route101 -> Route 101, NewMauville_Entrance -> New Mauville Entrance."""
    name = re.sub(r"([a-z])([A-Z0-9])", r"\1 \2", name.replace("_", " "))
    return re.sub(r"\s+", " ", name).strip()


def main():
    groups = json.load(open(os.path.join(ROOT, "data", "maps", "map_groups.json")))
    maps = {}
    for g, group in enumerate(groups["group_order"]):
        for n, folder in enumerate(groups[group]):
            path = os.path.join(ROOT, "data", "maps", folder, "map.json")
            if not os.path.exists(path):
                continue
            m = json.load(open(path))
            maps[m["id"]] = {"group": g, "num": n, "json": m, "folder": folder}

    species = enum_values("SPECIES_", "species.h")
    wild = {}
    data = json.load(open(os.path.join(ROOT, "src", "data", "wild_encounters.json")))
    for group in data["wild_encounter_groups"]:
        if not group.get("for_maps"):
            continue
        for enc in group["encounters"]:
            mons = (enc.get("land_mons") or {}).get("mons") or []
            if enc["map"] in wild or not mons:
                continue
            out = {}
            for mon in mons:
                sid = species.get(mon["species"])
                if sid:
                    lo, hi = out.get(sid, (100, 1))
                    out[sid] = (min(lo, mon["min_level"]), max(hi, mon["max_level"]))
            wild[enc["map"]] = [[s, lo, hi] for s, (lo, hi) in sorted(out.items())]

    def kind(mid):
        if maps[mid]["folder"].startswith(SKIP):
            return None
        return KINDS.get(maps[mid]["json"].get("map_type"))

    def warps(mid):
        return {w["dest_map"] for w in maps[mid]["json"].get("warp_events", []) if w["dest_map"] in maps}

    nodes = [mid for mid in maps if kind(mid)]
    links = {mid: set() for mid in nodes}
    for mid in nodes:
        for c in maps[mid]["json"].get("connections") or []:
            if c["map"] in links and c["direction"] in ("up", "down", "left", "right"):
                links[mid].add(c["map"])
                links[c["map"]].add(mid)
        for dest in warps(mid):
            if dest in links:
                links[mid].add(dest)
            elif not kind(dest) and not maps[dest]["folder"].startswith(SKIP):  # through a gatehouse
                for far in warps(dest):
                    if far in links and far != mid:
                        links[mid].add(far)
    # Places reached by boat or a guide's script, not a warp.
    for x, y in EXTRA_LINKS:
        if x in links and y in links:
            links[x].add(y)
            links[y].add(x)
    for mid in nodes:
        links[mid].discard(mid)

    def key(mid):
        return "%d.%d" % (maps[mid]["group"], maps[mid]["num"])

    world = {}
    for mid in nodes:
        folder = maps[mid]["folder"]
        has_center = any(maps[d]["folder"].endswith("PokemonCenter_1F") for d in warps(mid)) or mid in EXTRA_CENTERS
        world[key(mid)] = {
            "id": mid,
            "name": maps[mid]["json"]["name"],
            "title": pretty(maps[mid]["json"]["name"]),
            "kind": kind(mid),
            "links": sorted(key(d) for d in links[mid]),
            "wild": wild.get(mid, []),
            "center": has_center,
            "daycare": folder == "Route117",
        }
    # Keep only the part of Hoenn reachable from Littleroot.
    start = key("MAP_LITTLEROOT_TOWN")
    seen, todo = {start}, [start]
    while todo:
        for nxt in world[todo.pop()]["links"]:
            if nxt not in seen:
                seen.add(nxt)
                todo.append(nxt)
    world = {k: v for k, v in world.items() if k in seen}
    for v in world.values():
        v["links"] = [k for k in v["links"] if k in seen]
    # Each species' first stage (for Day Care babies) and name (for news).
    info = "".join(open(f).read() for f in sorted(glob.glob(os.path.join(ROOT, "src/data/pokemon/species_info/gen_*.h"))))
    pre, names = {}, {}
    current = None
    for line in info.splitlines():
        m = re.match(r"\s*\[(SPECIES_\w+)\]\s*=", line)
        if m:
            current = species.get(m.group(1))
            continue
        if current is None:
            continue
        m = re.search(r'\.speciesName\s*=\s*_\("([^"]*)"\)', line)
        if m:
            names.setdefault(current, m.group(1))
        if ".evolutions" in line and "EVOLUTION(" in line:
            for target in re.findall(r"(SPECIES_\w+)\s*[,}]", line):
                if target in species and species[target] != current:
                    pre.setdefault(species[target], current)
    base = {}
    for sid in names:
        cur, seen = sid, set()
        while cur in pre and cur not in seen:
            seen.add(cur)
            cur = pre[cur]
        base[sid] = cur
    # Item ids for what rovers carry and give (server/rovers_life.py).
    sys.path.insert(0, os.path.join(ROOT, "server"))
    import rovers_life
    wanted = set(rovers_life.APRICORNS + rovers_life.BERRIES + rovers_life.CAVE_FINDS + rovers_life.BALLS
                 + rovers_life.BAG_WIRE + [r[0] for r in rovers_life.RECIPES]
                 + [i for finds in rovers_life.TYPE_FINDS.values() for i in finds])
    item_ids = enum_values("ITEM_", "items.h")
    items = {name: item_ids["ITEM_" + name] for name in sorted(wanted) if "ITEM_" + name in item_ids}
    missing = sorted(n for n in wanted if n not in items)
    if missing:
        sys.exit("unknown items: " + ", ".join(missing))
    world = {"maps": world, "base": base, "names": names, "items": items}
    with gzip.open(OUT, "wt") as f:
        json.dump(world, f, separators=(",", ":"), sort_keys=True)
    maps_ = world["maps"]
    print("%d maps, %d with wild Pokémon, %d with a Pokémon Center, %d species -> %s" % (
        len(maps_), sum(1 for v in maps_.values() if v["wild"]),
        sum(1 for v in maps_.values() if v["center"]), len(names), os.path.relpath(OUT, ROOT)))


if __name__ == "__main__":
    main()
