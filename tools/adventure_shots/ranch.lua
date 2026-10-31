-- Ranching: the ranch hand by the Route 117 Day Care leads the way; the
-- rancher explains; six Pokémon in the RANCH box roam the pasture; a day
-- later the rancher hands over what they found.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(10, MAPS.MAP_ROUTE117, 55, 7)
    D.talk(K.UP, "ranch-hand", 2)
    D.press(K.A, 60) -- YES
    D.press(K.A, 40)
    D.waitFor("the ranch", 600, function() return D.inOverworld() end)
    D.wait(150)
    D.shot("arrive")
    -- The rancher stands at (7, 8); walk up the path to face him.
    D.scene(10, MAPS.MAP_ADVENTURE_RANCH, 6, 8)
    D.talk(K.RIGHT, "rancher-first", 5)
    for _ = 1, 4 do D.press(K.A, 40) end
    D.adventure({ ranch = { "MARILL", "SHUCKLE", "ODDISH", "GEODUDE", "ZIGZAGOON", "SKITTY" }, ranchHoursAgo = 24 })
    D.scene(10, MAPS.MAP_ADVENTURE_RANCH, 14, 13)
    D.wait(200)
    D.shot("pasture")
    D.wait(200)
    D.shot("pasture-2")
    D.scene(10, MAPS.MAP_ADVENTURE_RANCH, 6, 8)
    D.talk(K.RIGHT, "rancher-haul", 1)
    D.wait(60)
    D.shot("haul")
    for _ = 1, 3 do D.press(K.DOWN, 8) end
    D.shot("haul-2")
    D.press(K.B, 40)
    D.wait(30)
    D.shot("haul-done")
    D.press(K.A, 60)
    -- Talk to a Pokémon in the pasture.
    D.scene(10, MAPS.MAP_ADVENTURE_RANCH, 14, 13)
    D.wait(30)
end)
