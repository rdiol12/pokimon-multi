-- Hoenn Metro: Hex Maniac ODETTE on the old line.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_METRO_OLD_LINE, 31, 10)
    D.wait(300)
    D.hold(K.UP, 16)
    D.wait(200)
    D.shot("01-odette-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-odette-battle")
    D.result("ok")
end)
