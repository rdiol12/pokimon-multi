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

run.SYMBOLS += ["gSaveBlock2Ptr", "gAdventureShowcase", "gRoverShowcase", "gNetplayExtMailbox", "gBattleOutcome", "gLifeShowcase", "gSpecialVar_Result"]
run.HELPERS += 'ROVERS_SHOTS = "%s"\n' % here.replace("\\", "/")
run.HELPERS += """
-- The Rift Below's next floor is this one (src/adventure.c showcase).
function Driver.riftFloor(n)
    local s = Driver.SYM.gAdventureShowcase
    E.write8(s + 37, n)
    E.write8(s, 1)
    Driver.waitFor("the adventure scene", 1800, function() return E.read8(s) == 0 end)
    Driver.wait(10)
end
-- Summer (the seasons feature would otherwise follow the clock).
function Driver.summer() E.write8(Driver.SYM.gLifeShowcase + 2, 2) end

-- Presses A through messages until a YES/NO box is open (VAR_RESULT is
-- 0xFF while it waits), then answers it.
function Driver.answer(yes, name)
    local n = 0
    Driver.waitFor("a YES/NO box", 3000, function() return E.read16(Driver.SYM.gSpecialVar_Result) == 0xFF end, function(i)
        if i % 60 == 50 and name then n = n + 1; Driver.shot(name .. "-msg" .. n) end
        if i % 60 == 55 then Driver.press(Driver.KEY.A, 0) end
    end)
    Driver.wait(30)
    if name then Driver.shot(name) end
    if not yes then Driver.press(Driver.KEY.DOWN, 15) end
    Driver.press(Driver.KEY.A, 40)
end

-- Puts rovers on the current map (src/rovers.c RunShowcase).
-- scene: 1 wander, 2 duel, 3 vs the map's trainer, 4 wild, 5 charger,
-- 6 rampaging Pokémon, 7 catch.
-- The next Driver.rovers' first rover leads with a fusion (or 0, 0).
function Driver.roverLead(species, fusion)
    local s = Driver.SYM.gRoverShowcase
    local a, b = SPECIES_ID[species] or 0, SPECIES_ID[fusion] or 0
    E.write8(s + 20, a & 0xFF); E.write8(s + 21, a >> 8)
    E.write8(s + 22, b & 0xFF); E.write8(s + 23, b >> 8)
end

function Driver.rovers(scene, ids, level, teamSize, offline, talk, levelB, bond)
    local s = Driver.SYM.gRoverShowcase
    local MAX = 10 -- ROVERS_MAX_HERE (include/constants/rovers.h)
    E.write8(s + 1, scene)
    for i = 1, MAX do E.write8(s + 1 + i, ids[i] or 0) end
    local o = s + 2 + MAX
    E.write8(o, #ids)
    E.write8(o + 1, level or 20)
    E.write8(o + 2, teamSize or 3)
    E.write8(o + 3, offline == false and 0 or 1)
    E.write8(o + 4, talk or 0)
    E.write8(o + 5, levelB or 0)
    E.write8(o + 6, bond or 0) -- 1 rival, 2 friend
    E.write8(s, 1)
    Driver.wait(2)
end
"""

if __name__ == "__main__":
    run.main()
