-- Offline: the player talks to Rue (a Water-type fan), says no to a battle,
-- and trades Mudkip for one of Rue's Pokémon.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function film(name, count, every, key)
    for i = 1, count do
        D.wait(every)
        D.shot(name .. "-" .. i)
        if key then D.press(key, 0) end
    end
end

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {40}, 18, 3, true, 1)
    D.answer(false, "ask-battle")
    D.answer(true, "ask-trade")
    film("pick", 2, 60, K.A)
    D.waitFor("the trade", 600, function() return not D.inOverworld() end)
    D.wait(200) -- past the party menu's fade
    local n = 0
    D.waitFor("the overworld", 9000, D.inOverworld, function(i)
        if i % 90 == 0 and n < 60 then n = n + 1; D.shot("trade-" .. n) end
        if i % 100 == 0 then D.press(K.A, 0) end
    end)
    film("after", 4, 60, K.A)
    D.result("ok")
end)
