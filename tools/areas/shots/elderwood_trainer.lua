-- Elderwood Forest: an area trainer (BUG MANIAC HAL) spots the player.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local EW = MAPS.MAP_ELDERWOOD_FOREST

D.run(function()
    D.boot()
    D.wait(60)
    -- the bug maniac spots you
    D.scene(12, EW, 20, 29)
    D.wait(200)
    D.shot("01-hal-spots")
    D.press(K.A, 120)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("02-hal-battle")
    D.result("ok")
end)
