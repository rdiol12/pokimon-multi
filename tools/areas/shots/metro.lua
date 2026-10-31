-- Hoenn Metro: the attendant in Mauville (badge check), Mauville Central,
-- the station master's stamp rally, a stamp, and a train ride to Slateport.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local M = MAPS.MAP_METRO_MAUVILLE

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
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, 25, 7, { give = "PIKACHU", level = 30, lead = true })
    D.wait(240)
    D.talk(K.UP, "01-attendant-closed", 3)
    close(1)
    D.flag("FLAG_BADGE03_GET", true)
    D.talk(K.UP, "02-attendant", 3)
    for _ = 1, 4 do D.press(K.A, 60) end
    D.wait(400)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(300)
    D.shot("03-mauville-central")
    view(M, 16, 5, "04-concourse")
    view(M, 16, 11, "05-platform")
    D.scene(12, M, 28, 5)
    D.wait(300)
    D.talk(K.UP, "06-station-master", 6)
    close(4)
    D.scene(12, M, 18, 10)
    D.wait(300)
    D.talk(K.UP, "07-stamp", 2)
    close(1)
    D.scene(12, M, 13, 13)
    D.wait(300)
    D.talk(K.DOWN, "08-train-doors", 1)
    D.wait(60)
    D.shot("09-where-to")
    D.press(K.A, 60)
    D.wait(60)
    D.shot("10-all-aboard")
    for _ = 1, 3 do D.press(K.A, 60) end
    D.wait(600)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(900)
    D.shot("11-arrive-slateport")
    view(MAPS.MAP_METRO_SLATEPORT, 15, 5, "12-slateport-concourse")
    view(MAPS.MAP_METRO_VERDANTURF, 15, 9, "13-verdanturf-platform")
    D.result("ok")
end)
