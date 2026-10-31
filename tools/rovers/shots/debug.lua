-- Quick look: rovers wandering and meeting wild Pokémon on Route 102.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(4, {0, 5}, 18, 3)
    for i = 1, 12 do
        D.wait(45)
        D.shot("wild-" .. i)
    end
    D.rovers(2, {12, 40}, 22, 3)
    for i = 1, 10 do
        D.wait(60)
        D.shot("duel-" .. i)
    end
    D.result("ok")
end)
