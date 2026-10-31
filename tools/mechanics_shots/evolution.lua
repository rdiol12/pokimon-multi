-- Evolution quests: the sage in a Mauville City house. Onix needs a Metal
-- Coat, Kadabra's bond is too weak, and Machoke passes the trial and evolves.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function choose(downs, name)
    D.step(K.A, name .. "-which")
    D.step(K.A, name .. "-party", 150)
    for _ = 1, downs do D.press(K.DOWN, 15) end
    D.step(K.A, name .. "-chosen", 30)
    D.step(K.A, name .. "-select", 150)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_MAUVILLE_CITY_HOUSE1, 7, 6, { give = "KADABRA", level = 30, lead = true })
    D.mech({ species = "ONIX", level = 30 })
    D.wait(20)
    D.mech({ species = "MACHOKE", level = 40 })
    D.wait(20)
    D.mech({ fixMoves = true, friendship = 255, moves = { "CROSS_CHOP", "ROCK_SLIDE", "BULK_UP", "KNOCK_OFF" } })
    D.fastText()
    D.wait(20)
    D.shot("house")
    D.hold(K.UP, 16)
    D.wait(20)
    for _ = 1, 2 do D.step(K.A, "intro") end
    -- Onix (third).
    choose(1, "onix")
    D.step(K.A, "onix-item")
    D.step(K.A, "onix-item")
    -- Kadabra.
    choose(2, "kadabra")
    D.step(K.A, "kadabra-bond")
    D.step(K.A, "kadabra-bond")
    D.step(K.A, "kadabra-bond")
    -- Machoke (lead).
    choose(0, "machoke")
    D.step(K.A, "ready")
    D.step(K.A, "ready")
    D.step(K.A, "yes")
    D.step(K.A, "begin", 200)
    D.fight("trial", 80, 6)
    D.step(K.A, "passed")
    D.step(K.A, "passed", 200)
    for i = 1, 20 do
        D.press(K.A, 60)
        if i % 3 == 0 then D.shot("evolve") end
    end
    D.step(K.A, "after")
    D.result("ok")
end)
