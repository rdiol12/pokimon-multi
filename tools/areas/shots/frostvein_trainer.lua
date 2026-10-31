-- Frostvein Cave: COOLTRAINER GLEN in the entry hall spots the player.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_FROSTVEIN_CAVE_1F, 27, 25)
    D.wait(200)
    D.shot("01-glen-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-glen-battle")
    D.result("ok")
end)
