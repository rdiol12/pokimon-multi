"""Mount racing: a rival's time is on the PETALBURG RUN board already. Also
plans the harness's ride (race_path.lua in the output folder): a path from
each lane of Route 102's start line to its west end, avoiding walls, ledges,
water and the people standing on the route."""
import json
import os
import struct
from collections import deque

ENV = {"NETPLAY_FISHT_LENGTH": 0, "NETPLAY_RACE_WAIT": 25}
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
LANES = [(49, 10), (49, 11)]  # sCourses[0].lanes in src/online_racing.c


JUMPS = {0x38: (1, 0), 0x39: (-1, 0), 0x3A: (0, -1), 0x3B: (0, 1)}  # MB_JUMP_EAST/WEST/NORTH/SOUTH


def route_grid(name, tilesets):
    """{(x, y): behavior} of the tiles one can ride on (people stand on
    none of them)."""
    layouts = {l["id"]: l for l in json.load(open(os.path.join(ROOT, "data/layouts/layouts.json")))["layouts"]
               if "id" in l}
    m = json.load(open(os.path.join(ROOT, f"data/maps/{name}/map.json")))
    layout = layouts[m["layout"]]
    blocks = open(os.path.join(ROOT, layout["blockdata_filepath"]), "rb").read()
    attrs = b"".join(open(os.path.join(ROOT, f"data/tilesets/{t}/metatile_attributes.bin"), "rb").read()
                     for t in tilesets)
    w, h = layout["width"], layout["height"]
    people = {(o["x"], o["y"]) for o in m["object_events"]}
    free = {}
    for y in range(h):
        for x in range(w):
            v = struct.unpack_from("<H", blocks, 2 * (y * w + x))[0]
            if (v >> 10) & 3 == 0 and v >> 12 in (0, 3) and (x, y) not in people:
                free[(x, y)] = struct.unpack_from("<H", attrs, 2 * (v & 0x3FF))[0] & 0xFF
    return free


def plan(free, start, avoid=frozenset()):
    """The path to the west edge (x = 0) with the fewest steps and turns, as
    the corners to ride to. Ledges are jumped only the way they go."""
    import heapq
    dirs = [(-1, 0), (0, -1), (0, 1), (1, 0)]
    best = {(start, (-1, 0)): 0}
    prev = {(start, (-1, 0)): None}
    queue = [(0, start, (-1, 0))]
    end = None
    while queue:
        cost, cur, facing = heapq.heappop(queue)
        if cur[0] == 0:
            end = (cur, facing)
            break
        for d in dirs:
            nxt = (cur[0] + d[0], cur[1] + d[1])
            if nxt not in free or nxt in avoid:
                continue
            jump = JUMPS.get(free[nxt])
            if jump is not None:
                if jump != d:
                    continue
                nxt = (nxt[0] + d[0], nxt[1] + d[1])
                if nxt not in free:
                    continue
            c = cost + 1 + (3 if d != facing else 0)
            if c < best.get((nxt, d), 1 << 30):
                best[(nxt, d)] = c
                prev[(nxt, d)] = (cur, facing)
                heapq.heappush(queue, (c, nxt, d))
    if end is None:
        return None
    path = [end]
    while prev[path[-1]] is not None:
        path.append(prev[path[-1]])
    path.reverse()
    # A corner: where the direction changes, and every landing after a jump.
    corners = [path[i][0] for i in range(1, len(path) - 1) if path[i + 1][1] != path[i][1]]
    return [p for p, _ in path], corners + [path[-1][0]]


def seed(port, out):
    import sqlite3
    import time
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    week = int(time.time() // (7 * 86400))
    db.executemany("INSERT INTO online_race_times VALUES (?, 0, ?, ?, ?)",
                   [(week, "Mia", 777, 3105), (week, "Rex", 31337, 3390)])
    db.commit()

    free = route_grid("Route102", ["primary/general", "secondary/petalburg"])
    first, corners1 = plan(free, LANES[0])
    second, corners2 = plan(free, LANES[1], avoid=frozenset(first[2:])) or plan(free, LANES[1])
    with open(os.path.join(out, "race_path.lua"), "w") as f:
        f.write("RACE_PATH = {\n")
        for corners in (corners1, corners2):
            f.write("    {" + ", ".join(f"{{{x}, {y}}}" for x, y in corners) + "},\n")
        f.write("}\n")
