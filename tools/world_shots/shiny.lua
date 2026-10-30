-- Shiny hunting: defeat the same species in a row to build a chain; a shiny
-- lead follows you with its shiny colours.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE101, 4, 4)
    local shots = 0
    for _ = 1, 12 do
        D.findBattle(K.LEFT, K.RIGHT)
        D.wait(200)
        D.waitFor("the overworld", 20000, D.inOverworld, function(i)
            if i % 30 == 0 then D.press(K.A, 0) end
        end)
        D.wait(40)
        D.shot("after-battle")
        shots = shots + 1
        if shots >= 8 then break end
        D.wait(200)
    end
    D.scene(13, MAPS.MAP_ROUTE103, 10, 12, { give = "RAYQUAZA", level = 70, lead = true, shiny = true })
    D.hold(K.LEFT, 30)
    D.hold(K.DOWN, 20)
    D.hold(K.UP, 20)
    D.wait(40)
    D.shot("shiny-follower")
end)
