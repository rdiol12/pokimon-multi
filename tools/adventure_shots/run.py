#!/usr/bin/env python3
"""Headless in-game screenshots of the fourth-batch features
(docs/adventure.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py (Driver.scene, two players
with a server, ...), plus Driver.adventure{...}, which sets up a scene
through gAdventureShowcase (src/adventure.c):
    xp = trainer XP, outfit = OUTFIT_* number.
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gAdventureShowcase"]
world_run.HELPERS += """
-- Sets up an adventure scene (src/adventure.c, struct AdventureShowcase).
function Driver.adventure(t)
    local s = Driver.SYM.gAdventureShowcase
    if t.outfit then E.write8(s + 1, t.outfit + 1) end
    if t.xp then
        local v = t.xp + 1
        for i = 0, 3 do E.write8(s + 4 + i, (v >> (8 * i)) & 0xFF) end
    end
    E.write8(s, 1)
    Driver.waitFor("the adventure scene", 600, function() return E.read8(s) == 0 end)
    Driver.wait(10)
end
"""

if __name__ == "__main__":
    world_run.main()
