-- Hoenn Metro: three stamps, the shutter opens, the old line, the Ghost
-- Station's conductor and stamp, and the station master's prize.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local M, O = MAPS.MAP_METRO_MAUVILLE, MAPS.MAP_METRO_OLD_LINE

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function view(map, x, y, name)
    D.scene(12, map, x, y)
    D.wait(300)
    D.shot(name)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.flag("FLAG_METRO_STAMP_1", true)
    D.flag("FLAG_METRO_STAMP_2", true)
    D.flag("FLAG_METRO_STAMP_3", true)
    D.scene(12, M, 28, 5, { give = "PIKACHU", level = 35, lead = true })
    D.wait(300)
    D.talk(K.UP, "01-master-three-stamps", 4)
    for _ = 1, 3 do D.press(K.A, 80) end
    D.wait(60)
    D.shot("02-shutter-opens")
    close(3)
    view(M, 31, 10, "03-open-shutter")
    D.hold(K.RIGHT, 120)
    D.wait(400)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(200)
    D.shot("04-old-line")
    view(O, 12, 4, "05-signal-room")
    view(O, 18, 3, "06-depot-office")
    view(O, 28, 10, "07-rusted-train")
    view(O, 43, 7, "08-ghost-station")
    D.scene(12, O, 44, 10)
    D.wait(300)
    D.talk(K.UP, "09-conductor", 4)
    D.press(K.A, 200)
    D.wait(120)
    D.shot("10-conductor-gone")
    close(1)
    D.scene(12, O, 46, 6)
    D.wait(300)
    D.talk(K.UP, "11-ghost-stamp", 2)
    close(1)
    D.scene(12, M, 28, 5)
    D.wait(300)
    D.talk(K.UP, "12-master-prize", 3)
    D.wait(150)
    D.shot("13-got-klink")
    D.result("ok")
end)
