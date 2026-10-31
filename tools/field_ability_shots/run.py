#!/usr/bin/env python3
"""Headless in-game screenshots of the overworld abilities
(docs/field_abilities.md).

Usage: run.py <scenario.lua> <out dir> [frames]

The mechanics runner (tools/mechanics_shots/run.py: Driver.scene, Driver.mech,
Driver.fight, ...) plus Driver.fa(action, slot), which writes a request to
gFieldAbilityShowcase (src/field_ability.c): 1 = the lead gets ability slot
`slot`, 2 = poison the last party member and cure it on the next step,
3 = the next wild Pokémon holds a Nugget.
"""
import os
import sys

import importlib.util

here = os.path.dirname(os.path.abspath(__file__))
# Both runners are called run.py: load the mechanics one under its own name
# (it imports the world one as "run" itself).
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
_spec = importlib.util.spec_from_file_location("mech_run", os.path.join(here, "..", "mechanics_shots", "run.py"))
mech_run = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(mech_run)

world_run = mech_run.world_run
world_run.SYMBOLS += ["gFieldAbilityShowcase"]
world_run.HELPERS += """
function Driver.fa(action, slot)
    local s = Driver.SYM.gFieldAbilityShowcase
    E.write8(s + 1, slot or 0)
    E.write8(s, action)
    Driver.wait(10)
end

-- Faces dir (a Driver.KEY) without walking: a short tap.
function Driver.face(dir)
    Driver.hold(dir, 2)
    Driver.wait(20)
end
"""

if __name__ == "__main__":
    world_run.main()
