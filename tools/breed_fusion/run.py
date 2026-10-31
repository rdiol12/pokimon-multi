#!/usr/bin/env python3
"""Headless in-game screenshots of fusion breeding (docs/breed_fusion.md).

Usage: run.py <out dir> [scenario.lua]

Runs two players with the netplay server (so the server builds the
fusions), on the same runner as tools/mechanics_shots/run.py, with
helpers that edit a Pokémon's encrypted data (Fusion.*).
"""
import importlib.util
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("mech_run", os.path.join(here, "..", "mechanics_shots", "run.py"))
mech_run = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(mech_run)

world_run = mech_run.world_run
world_run.SYMBOLS += ["gSaveBlock3Ptr", "gCommunityMailbox"]
world_run.HELPERS += r"""
-- A Pokémon's encrypted part, as plain bytes (src/pokemon.c).
Fusion = {}
local SUB = {
    {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3},
    {1, 1, 2, 3, 2, 3, 0, 0, 0, 0, 0, 0, 2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2},
    {2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2, 0, 0, 0, 0, 0, 0, 3, 2, 3, 2, 1, 1},
    {3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0},
}
local function r32(a) return E.read32(a) end
local function w32(a, v)
    for i = 0, 3 do E.write8(a + i, (v >> (8 * i)) & 0xFF) end
end
-- {sub = {[0..3] = 12 plain bytes each}} of the Pokémon at addr.
function Fusion.open(addr)
    local p, ot = r32(addr), r32(addr + 4)
    local key = p ~ ot
    local plain = {}
    for i = 0, 11 do
        local w = (r32(addr + 32 + i * 4) ~ key) & 0xFFFFFFFF
        for b = 0, 3 do plain[i * 4 + b] = (w >> (8 * b)) & 0xFF end
    end
    local mon = { addr = addr, p = p, ot = ot, sub = {} }
    for t = 0, 3 do
        local at = SUB[t + 1][p % 24 + 1] * 12
        mon.sub[t] = {}
        for b = 0, 11 do mon.sub[t][b] = plain[at + b] end
    end
    return mon
end
function Fusion.close(mon)
    local plain = {}
    for t = 0, 3 do
        local at = SUB[t + 1][mon.p % 24 + 1] * 12
        for b = 0, 11 do plain[at + b] = mon.sub[t][b] end
    end
    local key, sum = mon.p ~ mon.ot, 0
    for i = 0, 11 do
        local w = plain[i * 4] | (plain[i * 4 + 1] << 8) | (plain[i * 4 + 2] << 16) | (plain[i * 4 + 3] << 24)
        sum = sum + (w & 0xFFFF) + (w >> 16)
        w32(mon.addr + 32 + i * 4, (w ~ key) & 0xFFFFFFFF)
    end
    w32(mon.addr, mon.p)
    E.write8(mon.addr + 28, sum & 0xFF)
    E.write8(mon.addr + 29, (sum >> 8) & 0xFF)
end
function Fusion.edit(addr, fn) local m = Fusion.open(addr); fn(m); Fusion.close(m) end
function Fusion.partyAddr(slot) return Driver.SYM.gParties + slot * 100 end
-- MON_DATA_FUSION_PARTNER's bits (src/pokemon.c).
function Fusion.setPartner(addr, partner)
    Fusion.edit(addr, function(m)
        local s3 = m.sub[3]
        s3[10] = (s3[10] & 0x0F) | ((partner & 0x0F) << 4)          -- bits 20-23 of the word at 8
        s3[11] = (s3[11] & 0xE8) | ((partner >> 4) & 0x07) | (((partner >> 9) & 1) << 4) -- 24-26, 28
        local s0 = m.sub[0]
        s0[11] = (s0[11] & 0x3F) | (((partner >> 7) & 3) << 6)       -- bits 14-15 of the u16 at 10
    end)
    local h = E.read8(addr + 19)
    E.write8(addr + 19, (h & 0x7F) | (((partner >> 10) & 1) << 7))
end
function Fusion.setFriendship(addr, v) Fusion.edit(addr, function(m) m.sub[0][9] = v end) end
-- Female (low personality byte 0) or male (0xFE); keeps the data readable.
function Fusion.setGender(addr, female)
    Fusion.edit(addr, function(m) m.p = (m.p & 0xFFFFFF00) | (female and 0x00 or 0xFE) end)
end
function Fusion.setNickname(addr, text)
    for i = 0, 9 do
        local c = text:byte(i + 1)
        local g = 0xFF
        if c then
            if c >= 65 and c <= 90 then g = 0xBB + c - 65
            elseif c >= 97 and c <= 122 then g = 0xD5 + c - 97
            else g = 0x00 end
        end
        E.write8(addr + 8 + i, g)
    end
end
function Fusion.copy(from, to, n) for i = 0, n - 1 do E.write8(to + i, E.read8(from + i)) end end
function Fusion.clear(addr, n) for i = 0, n - 1 do E.write8(addr + i, 0) end end
-- Recipes the server sent: SaveBlock3 breedFusion is at `offset`, 24 bytes
-- per recipe (structs are word-aligned).
function Fusion.recipeCount(offset)
    local base, n = E.read32(Driver.SYM.gSaveBlock3Ptr) + offset, 0
    for i = 0, 11 do if E.read8(base + i * 24 + 4) ~= 0 then n = n + 1 end end
    return n
end
"""


def main():
    out = os.path.abspath(sys.argv[1])
    scenario = sys.argv[2] if len(sys.argv) > 2 else os.path.join(here, "shots.lua")
    sys.argv = [sys.argv[0], scenario, out, "300000", "2"]
    world_run.main()


if __name__ == "__main__":
    main()
