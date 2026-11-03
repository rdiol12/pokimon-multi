"""Seed for tools/stormwild/shots.lua: a short, fixed run (the server's
NETPLAY_XRUN_* settings), a board with other trainers' hauls, and the walks
each player takes (stormwild_path.lua in the output folder)."""
import importlib.util
import os
import random
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")
ENV = {"NETPLAY_XRUN_LOBBY": 20, "NETPLAY_XRUN_LENGTH": 240, "NETPLAY_XRUN_CACHES": 14,
       "NETPLAY_XRUN_SEED": 7, "NETPLAY_FISHT_LENGTH": 0, "NETPLAY_FPS": 60}
# The server's clock says early afternoon, whenever the pictures are taken.
_utc = __import__("time").gmtime()
ENV["NETPLAY_CLOCK_UTC_OFFSET"] = 13 * 60 - (_utc.tm_hour * 60 + _utc.tm_min)
GRASS = {0x02, 0x03}
# The scenario checks how the wild battle ended.
if "run" in sys.modules and hasattr(sys.modules["run"], "SYMBOLS") and "gBattleOutcome" not in sys.modules["run"].SYMBOLS:
    sys.modules["run"].SYMBOLS.append("gBattleOutcome")


def seed(port, out):
    import sqlite3
    import time
    os.environ.update({k: str(v) for k, v in ENV.items()})
    sys.path.insert(0, os.path.join(ROOT, "server"))
    import online_xrun as xr
    rng = random.Random(int(ENV["NETPLAY_XRUN_SEED"]))
    caches = sorted(rng.sample(range(len(xr.CACHES)), xr.CACHES_PER_RUN))
    windows = xr.beacon_windows(rng)
    beacon = windows[0][1]

    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    week = int(time.time() // (7 * 86400))
    db.executemany("INSERT INTO online_xrun_hauls VALUES (?, ?, ?, ?, ?, ?)",
                   [(week, "Mia", 777, 48200, 6, 4), (week, "Rex", 31337, 22750, 9, 3),
                    (week, "Juno", 5150, 9300, 2, 1)])
    db.commit()

    spec = importlib.util.spec_from_file_location("racing_seed", os.path.join(ROOT, "tools/online_shots/racing_seed.py"))
    racing = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(racing)
    free = racing.route_grid("Stormwild", ["primary/general", "secondary/rustboro"])
    dry = {p: b for p, b in free.items() if b not in GRASS}
    cache = caches[0]
    cx, cy = xr.CACHES[cache]
    bx, by = xr.BEACONS[beacon]
    walks = [
        # TEST1: the first cache, then the first beacon to light.
        [(dry, (8, 8), (cx - 1, cy)), (dry, (cx - 1, cy), (bx, by + 1))],
        # TEST2: into the tall grass by the north spoke.
        [(dry, (32, 5), (32, 11))],
    ]
    nm = subprocess.run(["arm-none-eabi-nm", os.path.join(ROOT, "pokeemerald.elf")], capture_output=True, text=True).stdout
    storm = next(int(line.split()[0], 16) for line in nm.splitlines() if line.endswith(" sStorm"))
    off = next(int(line.split()[0], 16) for line in nm.splitlines() if line.endswith(" gStormwildSaveOffset"))
    dump = subprocess.run(["arm-none-eabi-objdump", "-s", f"--start-address=0x{off:x}", f"--stop-address=0x{off + 4:x}",
                           os.path.join(ROOT, "pokeemerald.elf")], capture_output=True, text=True).stdout
    offset = int.from_bytes(bytes.fromhex(dump.strip().splitlines()[-1].split()[1]), "little")
    with open(os.path.join(out, "stormwild_path.lua"), "w") as f:
        f.write(f"STORM_STATE = 0x{storm:08x}\nSTORM_SAVE_OFFSET = {offset}\nSTORM_BEACON_AT = {int(windows[0][0])}\n")
        f.write("STORM_PATH = {\n")
        for legs in walks:
            f.write("  {\n")
            for grid, start, goal in legs:
                corners = racing.plan(grid, start, goal=lambda p, g=goal: p == g)[1]
                f.write("    {" + ", ".join(f"{{{x}, {y}}}" for x, y in corners) + "},\n")
            f.write("  },\n")
        f.write("}\n")
