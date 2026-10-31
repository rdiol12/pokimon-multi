-- Pacing: with eight badges the Lilycove sailor sails to Starfall Isle, the
-- guide lists it as new, and Jirachi and Celebi wait for the Champion.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function clear()
    for i = 1, 8 do D.press(K.B, 20) end
    D.wait(60)
end

D.run(function()
    D.boot()
    D.wait(60)
    for _, f in ipairs({"FLAG_BADGE01_GET", "FLAG_BADGE02_GET", "FLAG_BADGE03_GET", "FLAG_BADGE04_GET",
                        "FLAG_BADGE05_GET", "FLAG_BADGE06_GET", "FLAG_BADGE07_GET"}) do D.flag(f, true) end
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 14, 34)
    D.wait(200); clear()
    D.talk(K.UP, "01-sailor-seven-badges", 2)
    clear()
    D.flag("FLAG_BADGE08_GET", true)
    D.wait(200)
    D.shot("02-new-in-guide")
    clear()
    D.wait(200); clear()
    D.talk(K.UP, "03-sailor-eight-badges", 1)
    clear()
    D.press(K.R, 40); D.press(K.A, 40); D.press(K.A, 60)
    D.shot("04-guide-list")
    D.press(K.A, 120)
    D.shot("05-guide-map-lilycove")
    D.press(K.A, 30); D.shot("06-guide-map-lilycove-2")
    D.press(K.B, 120); D.press(K.B, 60)
    clear()
    D.scene(12, MAPS.MAP_STARFALL_ISLE_CAVE, 19, 4)
    D.wait(200); clear()
    D.talk(K.UP, "07-jirachi-asleep", 2)
    clear()
    D.scene(12, MAPS.MAP_AMBERWOOD_HOLLOW, 19, 7)
    D.wait(200); clear()
    D.talk(K.UP, "08-celebi-shy", 2)
    D.result("ok")
end)
