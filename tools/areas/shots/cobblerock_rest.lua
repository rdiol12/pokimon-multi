-- Cobblerock: the miners' rest, where the cook heals your team.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_COBBLEROCK_MINERS_REST, 7, 3)
    D.wait(150)
    D.shot("01-miners-rest")
    D.talk(K.UP, "02-cook", 2)
    D.wait(200)
    D.shot("03-healed")
    D.press(K.A, 40)
    D.result("ok")
end)
