#!/usr/bin/env python3
"""Headless in-game screenshots of the Haven Reserve (docs/reserve.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players] [VAR=value...]

The community screenshot runner (tools/community_shots/run.py: the world
runner plus the community mailbox and Driver.showcase) with
Driver.approach(localId), which walks up to a wandering object on the
current map and talks to it.
"""
import importlib.util
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("community_run", os.path.join(here, "..", "community_shots", "run.py"))
community_run = importlib.util.module_from_spec(spec)
spec.loader.exec_module(community_run)
world_run = community_run.world_run

world_run.SYMBOLS += ["gObjectEvents", "gLifeShowcase", "gSaveBlock3Ptr", "sShownCount"]
world_run.HELPERS += """
-- Summer, whatever the clock says (gLifeShowcase.season, src/life_seasons.c).
function Driver.summer() E.write8(Driver.SYM.gLifeShowcase + 2, 2) end

-- struct ObjectEvent is 0x24 bytes; coords are map coords + 7.
local function objAt(i)
    local o = Driver.SYM.gObjectEvents + i * 0x24
    return o, E.read8(o) & 1, E.read8(o + 8), E.read16(o + 0x10) - 7, E.read16(o + 0x12) - 7, E.read8(o + 2) & 1
end

function Driver.playerXY()
    for i = 0, 63 do -- OBJECT_EVENTS_COUNT
        local _, active, _, x, y, isPlayer = objAt(i)
        if active == 1 and isPlayer == 1 then return x, y end
    end
end

function Driver.objectXY(localId)
    for i = 0, 63 do -- OBJECT_EVENTS_COUNT
        local _, active, id, x, y, isPlayer = objAt(i)
        if active == 1 and isPlayer == 0 and id == localId then return x, y end
    end
end

-- Walks next to object `localId` (it may wander) and talks to it.
function Driver.approach(localId)
    local K = Driver.KEY
    for _ = 1, 80 do
        local px, py = Driver.playerXY()
        local mx, my = Driver.objectXY(localId)
        if not mx then Driver.fail("no object " .. localId) return end
        local dx, dy = mx - px, my - py
        if math.abs(dx) + math.abs(dy) == 1 then
            local dir = dx == 1 and K.RIGHT or dx == -1 and K.LEFT or dy == 1 and K.DOWN or K.UP
            -- The Pokémon's script sets VAR_0x8004 to its spot (0-19).
            local var = Driver.SYM.gSpecialVar_0x8004
            E.write8(var, 0xFF); E.write8(var + 1, 0xFF)
            Driver.hold(dir, 2)
            Driver.wait(4)
            Driver.press(K.A, 20)
            if E.read16(var) ~= 0xFFFF then return end
        end
        local dir
        if math.abs(dx) > 1 or (dy == 0) then dir = dx > 0 and K.RIGHT or K.LEFT
        else dir = dy > 0 and K.DOWN or K.UP end
        Driver.hold(dir, 16)
        Driver.wait(2)
    end
    Driver.fail("couldn't reach object " .. localId)
end
"""

if __name__ == "__main__":
    env = [a for a in sys.argv[1:] if "=" in a and not a.startswith("/")]
    for pair in env:
        k, v = pair.split("=", 1)
        os.environ[k] = v
    sys.argv = [sys.argv[0]] + [a for a in sys.argv[1:] if a not in env]
    world_run.main()
