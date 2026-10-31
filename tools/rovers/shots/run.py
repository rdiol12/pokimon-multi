#!/usr/bin/env python3
"""Headless in-game screenshots of the roaming trainers (docs/rovers.md).

Usage (from tools/netplay_bridge, so online runs load the bridge's
extensions): python3 ../rovers/shots/run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py, with gRoverShowcase added.
Scenarios use Driver.rovers(scene, ids, level, teamSize, offline).
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "..", "world_shots"))
import run  # noqa: E402

run.SYMBOLS += ["gRoverShowcase", "gNetplayExtMailbox", "gBattleOutcome", "gLifeShowcase"]
run.HELPERS += """
-- Summer (the seasons feature would otherwise follow the clock).
function Driver.summer() E.write8(Driver.SYM.gLifeShowcase + 2, 2) end

-- Puts rovers on the current map (src/rovers.c RunShowcase).
-- scene: 1 wander, 2 duel, 3 vs the map's trainer, 4 wild, 5 charger,
-- 6 rampaging Pokémon, 7 catch.
function Driver.rovers(scene, ids, level, teamSize, offline, talk, levelB)
    local s = Driver.SYM.gRoverShowcase
    E.write8(s + 1, scene)
    for i = 1, 4 do E.write8(s + 1 + i, ids[i] or 0) end
    E.write8(s + 6, #ids)
    E.write8(s + 7, level or 20)
    E.write8(s + 8, teamSize or 3)
    E.write8(s + 9, offline == false and 0 or 1)
    E.write8(s + 10, talk or 0)
    E.write8(s + 11, levelB or 0)
    E.write8(s, 1)
    Driver.wait(2)
end
"""

if __name__ == "__main__":
    run.main()
