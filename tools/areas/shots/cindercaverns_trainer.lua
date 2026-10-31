-- Cinder Caverns: HIKER BRUNO on the lake shore spots the player.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_CINDER_CAVERNS_B1F, 37, 27)
    D.wait(200)
    D.shot("01-bruno-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-bruno-battle")
    D.result("ok")
end)
