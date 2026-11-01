#!/usr/bin/env python3
"""Screenshots of scars and lost parts outside battle (src/injury.c).

Usage: scars.py <out dir>          follower, party menu, summary and PC
       scars.py <out dir> online   two players: each sees the other's marked follower

Uses the world_shots runner (tools/world_shots/run.py). Needs a built ROM and
the mGBA fork (emulator/build.sh headless).
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gPokemonStoragePtr", "CB2_ReturnToPokeStorage"]

if __name__ == "__main__":
    online = len(sys.argv) > 2 and sys.argv[2] == "online"
    scenario = sys.argv[3] if len(sys.argv) > 3 else ("scars_online.lua" if online else "scars_shots.lua")
    out = os.path.abspath(sys.argv[1])
    common = open(os.path.join(here, "scars_common.lua")).read()
    combined = os.path.join(os.path.dirname(out), "." + os.path.basename(out) + "-" + scenario)
    with open(combined, "w") as f:
        f.write(open(os.path.join(here, scenario)).read().replace("-- @common\n", common, 1))
    sys.argv = [sys.argv[0], combined, out, "200000"] + (["2"] if online else [])
    world_run.main()
