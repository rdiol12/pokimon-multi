#!/usr/bin/env python3
"""Headless in-game screenshots of the battle and Pokémon mechanics
(docs/mechanics.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py (Driver.scene, Driver.talk, two
players with a server, ...).
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.HELPERS += """
-- Sets the text speed to FAST, so each A press turns one page.
function Driver.fastText()
    local sb2 = E.read32(Driver.SYM.gSaveBlock2Ptr)
    E.write8(sb2 + 0x14, (E.read8(sb2 + 0x14) & 0xF8) | 2)
end

-- Presses key, waits for the screen to settle, and takes a screenshot.
function Driver.step(key, name, frames)
    Driver.press(key, frames or 90)
    if name then Driver.shot(name) end
end
"""

if __name__ == "__main__":
    world_run.main()
