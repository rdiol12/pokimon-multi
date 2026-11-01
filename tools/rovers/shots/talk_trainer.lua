-- A rover battles Route 102's own trainer; the player talks to that trainer
-- and watches the battle.
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


-- The map's trainer in the duel: the trainer next to the rover's foe Pokémon (122).
local function foeTrainer()
    for i = 0, N - 1 do
        local o = D.SYM.gObjectEvents + i * OBJ
        local id = E.read8(o + 8)
        if (E.read8(o) & 1) == 1 and (E.read8(o + 2) & 1) == 0 and E.read8(o + 7) ~= 0 and id < 70 then return o end
    end
end

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(3, {61}, 12, 2)
    D.waitFor("the duel", 1200, function() return findObj(122) ~= nil end)
    D.wait(30)
    D.shot("duel")
    bringAbove(foeTrainer())
    D.wait(10)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.shot("talk-trainer")
    D.press(K.A, 60)
    D.shot("ask")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(200)
    D.shot("battle")
    D.waitFor("the overworld", 20000, D.inOverworld, function(i) if i % 120 == 0 then D.press(K.A, 0) end end)
    D.wait(60)
    D.shot("after")
    D.result("ok")
end)
