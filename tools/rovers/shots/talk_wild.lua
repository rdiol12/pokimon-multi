-- A rover fights a wild Pokémon; the player talks to that wild Pokémon
-- (and to the thrown Poké Ball). Before the fix this ran a script from a
-- null pointer: "Incorrect StringVar index" crash screen.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local OBJ, N = 36, 64
local E = { read8 = Driver.read8, read16 = function(a) return Driver.read8(a) | (Driver.read8(a + 1) << 8) end,
    write16 = function(a, v) Driver.write8(a, v & 0xFF); Driver.write8(a + 1, (v >> 8) & 0xFF) end }

local function findObj(localId)
    for i = 0, N - 1 do
        local o = D.SYM.gObjectEvents + i * OBJ
        if (E.read8(o) & 1) == 1 and E.read8(o + 8) == localId then return o end
    end
end

local function player()
    for i = 0, N - 1 do
        local o = D.SYM.gObjectEvents + i * OBJ
        if (E.read8(o) & 1) == 1 and (E.read8(o + 2) & 1) == 1 then return o end
    end
end

-- Puts the object right above the player, so facing up and pressing A talks to it.
local function bringAbove(o)
    local p = player()
    local x, y = E.read16(p + 0x10), E.read16(p + 0x10 + 2)
    E.write16(o + 0x10, x); E.write16(o + 0x12, y - 1)
    E.write16(o + 0x14, x); E.write16(o + 0x16, y - 1)
end

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(4, {44}, 14, 3)
    local wild
    D.waitFor("the wild Pokémon", 1200, function() wild = findObj(120); return wild ~= nil end)
    D.wait(30)
    D.shot("rover-vs-wild")
    bringAbove(wild)
    D.wait(10)
    D.shot("next-to-wild")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.shot("talk-wild-1")
    D.press(K.A, 60)
    D.shot("talk-wild-2")
    D.press(K.A, 60)
    D.shot("talk-wild-3")
    D.wait(120)
    D.shot("after")
    D.result(D.inOverworld() and "ok" or "not in overworld")
end)
