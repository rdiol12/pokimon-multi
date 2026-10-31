-- Rovers out in the world (offline): wandering, a wild Pokémon caught,
-- one knocked out, a rover against Route 102's own trainer, a wild
-- Pokémon charging out of the grass, and a rampaging one.
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
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {3, 77, 120}, 16, 4)
    film("wander", 4, 50)
    D.rovers(7, {0}, 14, 3)
    film("catch", 60, 8)
    D.rovers(4, {44}, 14, 3)
    film("wild", 30, 10)
    D.rovers(3, {61}, 12, 2)
    film("vs-trainer", 10, 45)
    D.rovers(5, {}, 12, 2)
    film("charger", 12, 15)
    D.waitFor("the charge", 1800, function() return not D.inOverworld() end, function(i)
        if i % 100 == 0 then D.shot("charging") end
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    film("charge-battle", 4, 90)
    D.result("ok")
end)
