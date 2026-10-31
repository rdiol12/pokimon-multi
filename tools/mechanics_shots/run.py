#!/usr/bin/env python3
"""Headless in-game screenshots of the battle and Pokémon mechanics
(docs/mechanics.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py (Driver.scene, Driver.talk, two
players with a server, ...).
"""
import os
import re
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gMechanicsShowcase"]
world_run.HELPERS += """
-- Runs a request through gMechanicsShowcase (src/mechanics.c):
-- moves = {"FLAMETHROWER", ...} sets the lead's moves;
-- fixMoves = true gives party Pokémon without moves their level-up moves.
function Driver.mech(t)
    local s = Driver.SYM.gMechanicsShowcase
    E.write8(s + 1, t.fixMoves and 1 or 0)
    for i = 1, 4 do
        local m = t.moves and MOVE_ID[t.moves[i]] or 0
        E.write16(s + 2 + (i - 1) * 2, m)
    end
    E.write8(s, 1)
    Driver.waitFor("the mechanics showcase", 600, function() return E.read8(s) == 0 end)
end

-- Battles by picking the first move each turn (and the next healthy
-- Pokémon when one faints) until the overworld is back. shotEvery > 0
-- takes a screenshot every that many frames, up to maxShots.
function Driver.fight(name, shotEvery, maxShots)
    local n, t = 0, 0
    Driver.waitFor("the battle", 1200, Driver.inBattle)
    Driver.waitFor("the end of the battle", 40000, Driver.inOverworld, function(i)
        if Driver.callback2() == STATIC_CB2_UpdatePartyMenu then
            t = t + 1
            if t % 80 == 1 then Driver.press(Driver.KEY.B, 0)
            elseif t % 80 == 20 then Driver.press(Driver.KEY.DOWN, 0)
            elseif t % 80 == 40 then Driver.press(Driver.KEY.A, 0)
            elseif t % 80 == 60 then Driver.press(Driver.KEY.A, 0) end
        elseif i % 30 == 0 then
            Driver.press(Driver.KEY.A, 0)
        end
        if shotEvery and i % shotEvery == shotEvery // 2 and n < (maxShots or 8) then
            n = n + 1
            Driver.shot(name)
        end
    end)
    Driver.wait(30)
end

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

_moves = re.findall(r"^\s+MOVE_(\w+) = (\d+),", open(os.path.join(here, "..", "..", "include", "constants", "moves.h")).read(), re.M)
world_run.HELPERS += "MOVE_ID = {\n" + "".join(f"    {n} = {v},\n" for n, v in _moves) + "}\n"

# Static functions aren't in pokeemerald.map; take them from the ELF.
_statics = {"CB2_UpdatePartyMenu": 0}
for _line in os.popen("arm-none-eabi-nm " + os.path.join(here, "..", "..", "pokeemerald.elf")):
    _parts = _line.split()
    if len(_parts) == 3 and _parts[2] in _statics:
        _statics[_parts[2]] = int(_parts[0], 16) & ~1
world_run.HELPERS += "".join(f"STATIC_{k} = 0x{v:08X}\n" for k, v in _statics.items())

if __name__ == "__main__":
    world_run.main()
