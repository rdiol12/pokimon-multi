-- Wonderwild Park: all three mascots home, Tandemaus and the parade.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P = MAPS.MAP_WONDERWILD_PARK

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.flag("FLAG_PARK_MASCOT_1", true)
    D.flag("FLAG_PARK_MASCOT_2", true)
    D.flag("FLAG_PARK_MASCOT_3", true)
    D.scene(12, P, 28, 9, { give = "MARILL", level = 25, lead = true })
    D.wait(300)
    D.talk(K.UP, "01-director-thanks", 3)
    D.wait(150)
    D.shot("02-got-tandemaus")
    for _ = 1, 3 do D.press(K.A, 60) end
    D.wait(300)
    D.shot("03-parade")
    D.press(K.A, 60)
    D.wait(200)
    D.shot("04-parade-2")
    D.result("ok")
end)
