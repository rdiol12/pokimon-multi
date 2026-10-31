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

world_run.SYMBOLS += ["gMechanicsShowcase", "gBattlerControllerFuncs"]
world_run.HELPERS += """
-- Runs a request through gMechanicsShowcase (src/mechanics.c):
-- moves = {"FLAMETHROWER", ...} sets the lead's moves;
-- species = "ZIGZAGOON", level = 10, variant = 1 (COLOR_*), wild = true
-- makes a Pokémon with that color variant: the new lead, or a wild battle;
-- persona = 0-7 (PERSONA_*) picks its personality too;
-- fixMoves = true gives party Pokémon without moves their level-up moves.
function Driver.mech(t)
    local s = Driver.SYM.gMechanicsShowcase
    E.write8(s + 1, t.fixMoves and 1 or 0)
    local sp = t.species and SPECIES_ID[t.species] or 0
    E.write16(s + 10, sp)
    E.write8(s + 12, t.level or 30)
    E.write8(s + 13, t.variant or 0)
    E.write8(s + 14, t.wild and 1 or 0)
    E.write8(s + 15, t.persona and t.persona + 1 or 0)
    E.write8(s + 16, t.badges or 0)
    E.write8(s + 17, t.hurt and 1 or 0)
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
    Driver.waitFor("the battle", 1200, Driver.inBattle, function(i)
        if i % 60 == 0 then Driver.press(Driver.KEY.A, 0) end
    end)
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

-- Battles choosing the first move with a battle style each turn: styles
-- is a list of how many SELECT presses per turn (1 AGILE, 2 STRONG), used
-- in turn. Takes a shot of the move menu and every shotEvery frames.
function Driver.styleFight(name, styles, shotEvery, maxShots)
    local n, turn, cooldown = 0, 0, 0
    local function controller()
        return E.read32(Driver.SYM.gBattlerControllerFuncs) & ~1
    end
    Driver.waitFor("the battle", 1200, Driver.inBattle, function(i)
        if i % 60 == 0 then Driver.press(Driver.KEY.A, 0) end
    end)
    Driver.waitFor("the end of the battle", 40000, Driver.inOverworld, function(i)
        if cooldown > 0 then
            cooldown = cooldown - 1
        elseif controller() == STATIC_HandleInputChooseMove then
            turn = turn + 1
            Driver.wait(20)
            for _ = 1, styles[(turn - 1) % #styles + 1] do
                Driver.press(Driver.KEY.SELECT, 20)
            end
            Driver.shot(name .. "-menu")
            Driver.press(Driver.KEY.A, 0)
            cooldown = 30
        elseif controller() == STATIC_HandleInputChooseAction then
            Driver.press(Driver.KEY.A, 0)
            cooldown = 30
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
_statics = {"CB2_UpdatePartyMenu": 0, "HandleInputChooseAction": 0, "HandleInputChooseMove": 0}
_all = {}
for _line in os.popen("arm-none-eabi-nm " + os.path.join(here, "..", "..", "pokeemerald.elf")):
    _parts = _line.split()
    if len(_parts) == 3 and _parts[2] in _statics:
        _all.setdefault(_parts[2], []).append(int(_parts[0], 16) & ~1)
# Several controllers have a static HandleInputChooseAction: take the
# player's, the one next to HandleInputChooseMove.
for _k, _v in _all.items():
    _statics[_k] = min(_v, key=lambda a: abs(a - _all["HandleInputChooseMove"][0]))
world_run.HELPERS += "".join(f"STATIC_{k} = 0x{v:08X}\n" for k, v in _statics.items())

if __name__ == "__main__":
    world_run.main()
