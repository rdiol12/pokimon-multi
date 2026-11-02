#!/usr/bin/env python3
"""Widens the shared Pokémon Center 1F layout (LAYOUT_POKEMON_CENTER_1F) with
an east wing for crowds online, and gives every center using it a second
nurse there (docs/hoenn_multi.md). Run once; it refuses to run twice.

New columns copy the plain wall at x=11 (top three rows) over plain floor,
and go in at x=11, so everything from x=11 on moves right.
"""
import json
import os
import struct

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LAYOUT_ID = "LAYOUT_POKEMON_CENTER_1F"
AT, ADD = 11, 4
FLOOR = 0x202
NURSE = (AT + 2, 2)


def main():
    layouts_path = os.path.join(ROOT, "data", "layouts", "layouts.json")
    layouts = json.load(open(layouts_path))
    layout = next(l for l in layouts["layouts"] if l["id"] == LAYOUT_ID)
    w, h = layout["width"], layout["height"]
    if w != 14:
        raise SystemExit("already widened")
    path = os.path.join(ROOT, layout["blockdata_filepath"])
    blocks = list(struct.unpack(f"<{w * h}H", open(path, "rb").read()))
    rows = [blocks[y * w:(y + 1) * w] for y in range(h)]
    out = []
    for y, row in enumerate(rows):
        new = row[AT] if y < 3 else FLOOR  # keeps the collision bits of the wall
        out.extend(row[:AT] + [new] * ADD + row[AT:])
    open(path, "wb").write(struct.pack(f"<{len(out)}H", *out))
    layout["width"] = w + ADD
    with open(layouts_path, "w") as f:
        json.dump(layouts, f, indent=2)
        f.write("\n")

    maps_dir = os.path.join(ROOT, "data", "maps")
    for name in sorted(os.listdir(maps_dir)):
        map_path = os.path.join(maps_dir, name, "map.json")
        if not os.path.isfile(map_path):
            continue
        data = json.load(open(map_path))
        if data.get("layout") != LAYOUT_ID:
            continue
        for key in ("object_events", "warp_events", "coord_events", "bg_events"):
            for ev in data.get(key, []):
                if ev["x"] >= AT:
                    ev["x"] += ADD
        data["object_events"].append({
            "graphics_id": "OBJ_EVENT_GFX_NURSE",
            "x": NURSE[0],
            "y": NURSE[1],
            "elevation": 3,
            "movement_type": "MOVEMENT_TYPE_FACE_DOWN",
            "movement_range_x": 0,
            "movement_range_y": 0,
            "trainer_type": "TRAINER_TYPE_NONE",
            "trainer_sight_or_berry_tree_id": "0",
            "script": "HoennMulti_EventScript_SecondNurse",
            "flag": "0",
        })
        with open(map_path, "w") as f:
            json.dump(data, f, indent=2)
            f.write("\n")
        print("widened", name)


if __name__ == "__main__":
    main()
