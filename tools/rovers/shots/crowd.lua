-- Ten rovers on one map at once (the extended GBA's 64 objects; the GBA
-- fit 4): wandering Route 102 and Petalburg City with their Pokémon.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver

local function film(name, count, every)
    for i = 1, count do
        D.wait(every)
        D.shot(name .. "-" .. i)
    end
end

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {3, 77, 120, 5, 18, 33, 61, 90, 101, 140}, 16, 4)
    film("route102", 6, 60)
    D.scene(13, MAPS.MAP_PETALBURG_CITY, 15, 12, {})
    D.rovers(1, {8, 21, 44, 52, 66, 79, 88, 112, 125, 147}, 20, 4)
    film("petalburg", 6, 60)
    D.result("ok")
end)
