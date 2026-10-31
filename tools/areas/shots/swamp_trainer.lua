-- Mirebough Marsh: RANGER IVY spots the player near the south reeds.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_MIREBOUGH_MARSH, 36, 41)
    D.wait(30)
    D.hold(K.UP, 16)
    D.wait(200)
    D.shot("01-ivy-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-ivy-battle")
    D.result("ok")
end)
