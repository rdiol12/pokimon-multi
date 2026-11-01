-- Shared by scars_shots.lua and scars_online.lua (scars.py puts it in place of
-- their "-- @common" line): writes injury bits into party slots.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local function w8(a, v) D.write8(a, v & 0xFF) end
local function w16(a, v) w8(a, v); w8(a + 1, v >> 8) end
local function w32(a, v) w16(a, v & 0xFFFF); w16(a + 2, (v >> 16) & 0xFFFF) end

local PART = { NONE = 0, EYE = 1, LIMB = 2, TAIL = 3, WING = 4, HORN = 5 }
local FLAG = { SCAR = 1, NOTIFY = 2, NEW_SCAR = 4 }
local function bit(p) return 1 << (p - 1) end

-- Substruct order by personality % 24 (sSubstructOffsets in src/pokemon.c).
local OFFS = {
    [0] = {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3},
    [1] = {1, 1, 2, 3, 2, 3, 0, 0, 0, 0, 0, 0, 2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2},
}

-- Writes the injury bits straight into party slot `slot`'s encrypted box data.
local function setInjury(slot, part, pct, lost, flags)
    local base = D.SYM.gParties + slot * 100
    local pers, otid = D.read32(base), D.read32(base + 4)
    local key = pers ~ otid
    local w = {}
    for i = 0, 11 do w[i] = D.read32(base + 32 + i * 4) ~ key end
    local s0 = OFFS[0][pers % 24 + 1] * 3
    local s1 = OFFS[1][pers % 24 + 1] * 3
    if part ~= 0 and pct >= 100 then lost = lost | bit(part) end
    -- substruct0: u16 @2 bits 10-15 injuryDamage; u32 @4 bits 29-31 injuryFlags
    w[s0] = (w[s0] & 0x03FFFFFF) | (((pct // 2) & 0x3F) << 26)
    w[s0 + 1] = (w[s0 + 1] & 0x1FFFFFFF) | ((flags & 7) << 29)
    -- substruct1: u16 @4 bits 11-15 lostParts; u16 @6 bits 11-13 injuredPart
    w[s1 + 1] = (w[s1 + 1] & ~((0x1F << 11) | (0x7 << 27))) | ((lost & 0x1F) << 11) | ((part & 7) << 27)
    local sum = 0
    for i = 0, 11 do sum = sum + (w[i] & 0xFFFF) + ((w[i] >> 16) & 0xFFFF) end
    for i = 0, 11 do w32(base + 32 + i * 4, (w[i] ~ key) & 0xFFFFFFFF) end
    w16(base + 28, sum & 0xFFFF)
end


