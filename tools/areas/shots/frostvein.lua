-- Frostvein Cave: the boat from Mossdeep, the 1F ice field solved by
-- sliding, the ladder, B1F, Avalugg and the Ice Priestess's Frigibax.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local F1 = MAPS.MAP_FROSTVEIN_CAVE_1F
local B1 = MAPS.MAP_FROSTVEIN_CAVE_B1F
local DIR = { U = K.UP, D = K.DOWN, L = K.LEFT, R = K.RIGHT }

local function say(dir, name, pages)
    D.talk(dir, name, pages)
    D.wait(30)
end

-- Waits until the player has stopped sliding.
local function settle()
    local px, py = D.where()
    local still = 0
    while still < 3 do
        D.wait(10)
        local x, y = D.where()
        if x == px and y == py then still = still + 1 else still = 0 end
        px, py = x, y
    end
end

-- Plays a solution: the first move also steps onto the ice from the floor.
local function moves(seq, name)
    for i = 1, #seq do
        D.hold(DIR[seq:sub(i, i)], i == 1 and 34 or 24)
        settle()
        local x, y = D.where()
        D.log(seq:sub(i, i) .. " -> " .. x .. "," .. y)
        if name and i == 3 then D.shot(name) end
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.flag("FLAG_BADGE07_GET", true)
    D.scene(12, MAPS.MAP_MOSSDEEP_CITY, 30, 18)
    D.shot("01-mossdeep-sailor")
    say(K.RIGHT, "02-sailor", 2)
    D.press(K.A, 30)
    D.wait(200)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(120)
    D.shot("03-arrive")
    D.scene(12, F1, 13, 24); say(K.UP, "04-researcher", 3)
    D.scene(12, F1, 15, 21); D.wait(30); D.shot("05-ice-field")
    moves("ULDRULUU", "06-sliding")
    D.wait(30)
    D.shot("07-across")
    D.scene(12, F1, 20, 5)
    D.hold(K.UP, 20)
    D.wait(150)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(60)
    D.shot("08-b1f")
    D.scene(12, B1, 9, 26); D.wait(30); D.shot("09-big-ice-field")
    D.scene(12, B1, 20, 10); D.wait(30); D.shot("10-antechamber")
    D.flag("FLAG_FROSTVEIN_AVALUGG_DONE", true)
    D.scene(12, B1, 20, 3)
    say(K.UP, "11-priestess", 3)
    D.wait(120)
    D.shot("12-frigibax")
    D.press(K.A, 60)
    D.shot("13-after")
    D.press(K.A, 60)
    D.flag("FLAG_FROSTVEIN_AVALUGG_DONE", false)
    D.flag("FLAG_HIDE_FROSTVEIN_AVALUGG", false)
    D.scene(12, B1, 20, 6)
    D.talk(K.UP, "14-avalugg", 1)
    D.press(K.A, 30)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(400)
    D.shot("15-avalugg-battle")
    D.result("ok")
end)
