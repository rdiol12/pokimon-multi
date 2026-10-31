-- Cobblerock Mine: Hiker GRANT spots you in the main gallery.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_COBBLEROCK_MINE_1F, 18, 11)
    D.wait(200)
    D.hold(K.DOWN, 16)
    D.wait(200)
    D.shot("01-grant-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-grant-battle")
    D.result("ok")
end)
