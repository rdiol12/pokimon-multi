#!/usr/bin/env python3
"""Headless in-game screenshots of Pokémon injuries (src/injury.c).

Usage: run.py <out dir>

Uses the world_shots runner (tools/world_shots/run.py) with tools/injury/shots.lua,
plus Driver.flee(), which picks RUN as soon as the battle menu is up.
"""
import os
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gBattlerControllerFuncs"]
# HandleInputChooseAction is static, so it is not in pokeemerald.map.
_nm = subprocess.run(["arm-none-eabi-nm", os.path.join(world_run.root, "pokeemerald.elf")],
                     capture_output=True, text=True).stdout
# Several battle controllers have one; any of them means the menu is up.
_menus = [int(l.split()[0], 16) & ~1 for l in _nm.splitlines() if l.endswith(" HandleInputChooseAction")]
_menu_check = " or ".join(f"f == 0x{a:08X}" for a in _menus)
world_run.HELPERS += f"""
-- Runs from a wild battle: whenever the action menu is up, moves the
-- cursor to RUN (DOWN then RIGHT works from any cursor spot) and picks it.
function Driver.flee()
    Driver.waitFor("the overworld", 4000, Driver.inOverworld, function(i)
        local f = Driver.read32(Driver.SYM.gBattlerControllerFuncs) & ~1
        if {_menu_check} then
            Driver.press(Driver.KEY.DOWN, 8)
            Driver.press(Driver.KEY.RIGHT, 8)
            Driver.press(Driver.KEY.A, 30)
        elseif i % 40 == 0 then
            Driver.press(Driver.KEY.B, 0)
        end
    end)
    Driver.wait(30)
end
"""

if __name__ == "__main__":
    sys.argv = [sys.argv[0], os.path.join(here, "shots.lua"), sys.argv[1]] + sys.argv[2:]
    world_run.main()
