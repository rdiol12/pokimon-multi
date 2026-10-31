-- In-game screenshots of Pokémon injuries (src/injury.c).
-- Run: python3 tools/injury/run.py <out dir>
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

local function setMoney(v)
    local sb1, sb2 = D.read32(D.SYM.gSaveBlock1Ptr), D.read32(D.SYM.gSaveBlock2Ptr)
    w32(sb1 + 0x490, (v ~ D.read32(sb2 + 0xAC)) & 0xFFFFFFFF)
end

local function step(dir)
    D.hold(dir, 18)
    D.wait(10)
end

-- Takes one step so the injury message runs, and shoots each page.
local function message(name, pages)
    step(K.LEFT)
    for p = 1, pages do
        D.wait(140)
        D.shot(name .. "-" .. p)
        D.press(K.A, 20)
    end
    D.wait(60)
    D.press(K.B, 30)
end

local function summary(name)
    D.press(K.START, 60)
    D.press(K.A, 120)  -- POKéMON
    D.press(K.A, 40)   -- lead
    D.press(K.A, 160)  -- SUMMARY
    D.shot(name)
    D.press(K.B, 150)
    D.press(K.B, 120)
    D.waitFor("the overworld", 600, D.inOverworld, function(i)
        if i % 60 == 30 then D.press(K.B, 0) end
    end)
    D.wait(30)
    D.press(K.B, 60)
end

local function battle(name)
    D.findBattle(K.LEFT, K.RIGHT)
    D.wait(200)
    D.press(K.A, 10)  -- "You encountered a wild ..."
    D.wait(260)       -- the lead is sent out
    D.shot(name)
    D.flee()
    D.wait(60)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE101, 4, 4, { give = "PIKACHU", level = 30, lead = true })

    -- After a battle: the tail took damage.
    setInjury(0, PART.TAIL, 40, 0, FLAG.NOTIFY)
    message("01-tail-damaged", 2)
    summary("02-summary-tail-damaged")

    -- Left untreated it got worse and the tail was lost.
    setInjury(0, PART.TAIL, 100, 0, FLAG.NOTIFY | FLAG.NEW_SCAR | FLAG.SCAR)
    message("03-tail-lost", 3)
    message("04-scar", 1)
    summary("05-summary-tail-lost")
    battle("06-battle-tail-lost")

    -- Charizard that lost a wing and an eye.
    D.scene(13, MAPS.MAP_ROUTE101, 4, 4, { give = "CHARIZARD", level = 36, lead = true })
    setInjury(0, PART.NONE, 0, bit(PART.WING) | bit(PART.EYE), FLAG.SCAR | FLAG.NOTIFY)
    step(K.RIGHT)
    summary("07-summary-charizard")
    battle("08-battle-charizard")

    -- Mightyena with a badly hurt leg.
    D.scene(13, MAPS.MAP_ROUTE101, 4, 4, { give = "MIGHTYENA", level = 30, lead = true })
    setInjury(0, PART.LIMB, 60, bit(PART.HORN), FLAG.NOTIFY)
    message("09-leg-damaged", 2)
    summary("10-summary-mightyena")
    battle("11-battle-mightyena")

    -- The Pokémon Center clinic.
    D.scene(13, MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_1F, 7, 4)
    setMoney(20000)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    for p = 1, 13 do
        D.wait(90)
        D.shot(string.format("12-clinic-%02d", p))
        D.press(K.A, 20)
    end
    D.result("ok")
end)
