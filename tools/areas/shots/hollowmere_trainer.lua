-- Hollowmere: HEX MANIAC VESPER spots the player at night.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(22, MAPS.MAP_HOLLOWMERE, 12, 19)
    D.wait(200)
    D.shot("01-vesper-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-vesper-battle")
    D.result("ok")
end)
