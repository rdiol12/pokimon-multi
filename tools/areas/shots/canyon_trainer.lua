-- Skyrope Canyon: HIKER HAMISH on the west trail spots the player.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_SKYROPE_CANYON, 8, 44)
    D.wait(200)
    D.shot("01-hamish-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-hamish-battle")
    D.result("ok")
end)
