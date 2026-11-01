-- Fights on the map show real moves: the move's name at the bottom of the
-- screen, its sound and an effect on the target (a rover vs a wild Pokémon,
-- then two rovers).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function film(name, count, every)
    for i = 1, count do
        D.wait(every)
        D.shot(name .. "-" .. i)
    end
end

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(4, {44}, 14, 3)
    film("wild", 24, 10)
    D.rovers(2, {12, 40}, 22, 3)
    film("duel", 30, 10)
    D.result("ok")
end)
