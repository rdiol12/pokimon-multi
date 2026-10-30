#!/usr/bin/env python3
"""Headless in-game screenshots of the world features.

Usage: run.py <scenario.lua> <out dir> [frames]

Boots the ROM through the netplay test boot (player TEST1 in Oldale Town),
then runs the scenario. Scenarios use the e2e Driver (driver_lib.lua) plus
Driver.scene(hour, map, x, y), which sets the clock and warps through
gWorldShowcase (src/world.c). MAPS has every MAP_* constant as {group, num}.
Needs a built pokeemerald.gba and libmgba-dev, like the netplay e2e test.
"""
import os
import re
import shutil
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))
e2e = os.path.join(root, "tools", "netplay_bridge", "e2e")

SYMBOLS = ["gNetplayMailbox", "gMain", "gParties", "gPartiesCount", "CB2_Overworld", "BattleMainCB2",
           "gLink", "gLinkStatus", "gReceivedRemoteLinkPlayers", "gLinkCallback", "gReadyToCloseLink",
           "gBattleMons", "gSaveBlock1Ptr", "gSaveBlock2Ptr", "gWorldShowcase", "gObjectEvents",
           "gSpecialVar_Result", "gWorldMailbox"]

HELPERS = """
-- Species of the wild Pokémon (gBattleMons[1]) in the current battle.
function Driver.enemySpecies() return E.read16(Driver.SYM.gBattleMons + Driver.SYM.BATTLEMON_SIZE) end

-- Walks back and forth (dir keys a, b) until a wild battle starts, then
-- returns the species once the battle screen has settled.
function Driver.findBattle(a, b, steps)
    local s, t = 0, 0
    Driver.waitFor("a wild battle", 30000, Driver.inBattle, function(i)
        if i % ((steps or 3) * 16) == 0 then s = s + 1 end
        Driver.setKeys(s % 2 == 0 and a or b)
    end)
    Driver.setKeys(0)
    Driver.wait(30)
    return Driver.enemySpecies()
end

-- Runs from the current wild battle and waits for the overworld.
function Driver.runAway()
    Driver.waitFor("the overworld", 3000, Driver.inOverworld, function(i)
        if i % 40 == 0 then Driver.press(Driver.KEY.B, 0)
        elseif i % 40 == 10 then Driver.press(Driver.KEY.DOWN, 0)
        elseif i % 40 == 20 then Driver.press(Driver.KEY.RIGHT, 0)
        elseif i % 40 == 30 then Driver.press(Driver.KEY.A, 0) end
    end)
    Driver.wait(30)
end

-- Faces dir and presses A, then screenshots each page of the message.
function Driver.talk(dir, name, pages)
    Driver.hold(dir, 2)
    Driver.wait(10)
    Driver.press(Driver.KEY.A, 40)
    for p = 1, pages or 1 do
        Driver.wait(60)
        Driver.shot(name .. (pages and pages > 1 and ("-" .. p) or ""))
        Driver.press(Driver.KEY.A, 20)
    end
end

function Driver.read8(a) return E.read8(a) end
function Driver.read16(a) return E.read16(a) end
function Driver.read32(a) return E.read32(a) end
function Driver.write8(a, v) return E.write8(a, v) end

-- Sets the clock to `hour` and warps to map {group, num} at x, y.
function Driver.scene(hour, map, x, y, extra)
    local s = Driver.SYM.gWorldShowcase
    E.write8(s + 1, hour)
    E.write8(s + 2, map[1])
    E.write8(s + 3, map[2])
    E.write8(s + 4, x & 0xFF); E.write8(s + 5, (x >> 8) & 0xFF)
    E.write8(s + 6, y & 0xFF); E.write8(s + 7, (y >> 8) & 0xFF)
    E.write8(s + 8, extra or 0)
    E.write8(s, 1)
    -- B closes whatever message is still open, so the warp can run.
    Driver.waitFor("the warp", 900, function() return E.read8(s) == 0 end, function(i)
        if i % 30 == 0 then Driver.press(Driver.KEY.B, 0) end
    end)
    Driver.wait(20)
    Driver.waitFor("the overworld", 1200, Driver.inOverworld)
    Driver.wait(90)
end
"""


def main():
    scenario, out = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    frames = sys.argv[3] if len(sys.argv) > 3 else "200000"
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(os.path.join(out, "p1"))
    harness = os.path.join(out, "harness")
    subprocess.check_call(["gcc", "-O1", "-o", harness, os.path.join(e2e, "harness.c"), "-lmgba"])

    syms = {}
    prev = None
    for line in open(os.path.join(root, "pokeemerald.map")):
        m = re.match(r"^\s+0x([0-9a-f]+)\s+(\w+)\s*$", line)
        if not m:
            continue
        if prev == "gBattleMons":
            # gBattleMons[MAX_BATTLERS_COUNT]; the next symbol ends it.
            syms["BATTLEMON_SIZE"] = (int(m.group(1), 16) - syms["gBattleMons"]) // 4
        prev = m.group(2)
        if m.group(2) in SYMBOLS:
            syms[m.group(2)] = int(m.group(1), 16)
    with open(os.path.join(out, "symbols.lua"), "w") as f:
        f.write("return {\n" + "".join(f"    {k} = 0x{v:08X},\n" for k, v in syms.items()) + "}\n")

    lib = open(os.path.join(e2e, "driver_lib.lua")).read()
    lib = lib.replace('"read8", "read16", "read32", "write8"', '"read8", "read16", "read32", "write8", "write16", "write32"')
    open(os.path.join(out, "driver_lib.lua"), "w").write(lib + HELPERS)

    maps = re.findall(r"^\s+(MAP_\w+)\s+= \((\d+) \| \((\d+) << 8\)\),", open(os.path.join(root, "include", "constants", "map_groups.h")).read(), re.M)
    prelude = "MAPS = {\n" + "".join(f"    {n} = {{{g}, {num}}},\n" for n, num, g in maps) + "}\n"
    species = re.findall(r"^\s+SPECIES_(\w+) = (\d+),", open(os.path.join(root, "include", "constants", "species.h")).read(), re.M)
    prelude += "SPECIES_NAME = {\n" + "".join(f'    [{v}] = "{n}",\n' for n, v in species) + "}\n"
    prelude += f'OUT_DIR = "{out}"\n'
    open(os.path.join(out, "p1", "driver.lua"), "w").write(prelude + open(scenario).read())
    subprocess.call([harness, os.path.join(root, "pokeemerald.gba"),
                     os.path.join(root, "tools", "netplay_bridge", "netplay_bridge.lua"), frames,
                     os.path.join(out, "p1")])
    print(open(os.path.join(out, "p1", "result.txt")).read().strip())


if __name__ == "__main__":
    main()
