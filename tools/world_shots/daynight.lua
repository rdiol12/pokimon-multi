-- Day and night: Oldale by day and by night, the night storyteller, a night
-- encounter on Route 101, the Night Market and the Fallarbor stargazer.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local NIGHT = { HOOTHOOT = true, SPINARAK = true, MURKROW = true }

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 9, 13)
    D.shot("oldale-1pm")
    D.scene(22, MAPS.MAP_OLDALE_TOWN, 9, 13)
    D.shot("oldale-10pm")
    D.talk(K.UP, "storyteller", 5)
    D.wait(120)

    D.scene(22, MAPS.MAP_ROUTE101, 4, 4)
    D.shot("route101-night")
    for _ = 1, 20 do
        local sp = D.findBattle(K.LEFT, K.RIGHT)
        D.wait(250)
        D.log("wild " .. tostring(SPECIES_NAME[sp]))
        if NIGHT[SPECIES_NAME[sp]] then
            D.shot("night-wild-" .. SPECIES_NAME[sp]:lower())
            D.runAway()
            break
        end
        D.runAway()
    end

    D.scene(22, MAPS.MAP_LILYCOVE_CITY, 30, 27)
    D.shot("lilycove-night-market")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 90)
    D.shot("night-market-hello")
    D.press(K.A, 90)
    D.press(K.A, 120)
    D.shot("night-market-shop")
    D.wait(10)
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.press(K.A, 60)
    D.press(K.B, 60)
    D.wait(60)

    D.scene(23, MAPS.MAP_FALLARBOR_TOWN, 5, 11)
    D.shot("fallarbor-night")
    D.talk(K.UP, "stargazer", 4)
end)
