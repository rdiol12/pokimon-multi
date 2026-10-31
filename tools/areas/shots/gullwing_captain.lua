-- Gullwing Point: CAPTAIN BARNACLE at the helm.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.flag("FLAG_GULL_BEAM_SOLVED", true)
    D.scene(12, MAPS.MAP_GULLWING_POINT_PIRATE_SHIP, 15, 5)
    D.wait(120)
    D.talk(K.RIGHT, "01-captain", 2)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(400)
    D.shot("02-captain-battle")
    D.result("ok")
end)
