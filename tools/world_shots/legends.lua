-- Legendary quests: the weather watcher's tale, Raikou in the thunderstorm
-- on Route 110 running off to Route 118, the battle there, Suicune in the
-- rain on Route 120, and R > QUESTS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 13, 18)
    D.talk(K.UP, "hint", 9)
    for _ = 1, 4 do D.press(K.B, 30) end

    -- The hour the thunderstorm is over Route 110.
    D.scene(13, MAPS.MAP_ROUTE110, 14, 95, { storm = "thunder" })
    D.hold(K.UP, 2)
    D.wait(300)
    D.shot("raikou-route110")
    D.press(K.A, 30)
    D.shot("raikou-cry")
    D.wait(150)
    D.shot("raikou-runs")
    D.wait(120)
    D.shot("raikou-fled")
    for _ = 1, 4 do D.press(K.B, 30) end

    -- Hours later the storm has moved on to Route 118, and so has Raikou.
    D.scene(15, MAPS.MAP_ROUTE118, 62, 11, { item = "MASTER_BALL", storm = "thunder" })
    D.hold(K.UP, 2)
    D.wait(300)
    D.shot("raikou-route118")
    D.press(K.A, 60)
    D.shot("raikou-stands")
    D.waitFor("the battle", 2000, D.inBattle, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    D.wait(400)
    D.shot("raikou-battle")
    for i = 1, 160 do
        if not D.inBattle() then break end
        D.press(i % 4 == 0 and K.R or K.B, 30)
    end
    D.waitFor("the overworld", 4000, D.inOverworld, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    D.wait(60)
    D.shot("raikou-caught")
    D.wait(500)
    D.shot("raikou-caught-2")
    D.wait(300)

    -- Suicune in the rain on Route 120 (that part of the route always has rain).
    D.scene(10, MAPS.MAP_ROUTE120, 5, 27)
    D.hold(K.UP, 2)
    D.wait(300)
    D.shot("suicune-route120")
    D.press(K.A, 200)
    D.shot("suicune-fled")
    for _ = 1, 4 do D.press(K.B, 30) end
    D.scene(18, MAPS.MAP_ROUTE119, 30, 18, { storm = "rain" })
    D.hold(K.UP, 2)
    D.wait(300)
    D.shot("suicune-route119")

    -- Entei: the stargazer's tale, then Route 113 at night.
    D.scene(23, MAPS.MAP_FALLARBOR_TOWN, 5, 11)
    D.talk(K.UP, "entei-hint", 9)
    for _ = 1, 4 do D.press(K.B, 30) end
    D.scene(23, MAPS.MAP_ROUTE113, 9, 11)
    D.hold(K.UP, 2)
    D.wait(200)
    D.shot("entei-route113")
    D.press(K.A, 200)
    D.shot("entei-fled")
    for _ = 1, 6 do D.press(K.B, 30) end
    D.scene(23, MAPS.MAP_ROUTE113, 40, 5)
    D.hold(K.UP, 2)
    D.wait(200)
    D.shot("entei-waits")

    -- Cresselia: the Night Market's tip, then Route 121 at night.
    D.scene(22, MAPS.MAP_LILYCOVE_CITY, 30, 27)
    D.hold(K.UP, 2)
    D.press(K.A, 80)
    for _ = 1, 3 do D.press(K.A, 40) end
    D.press(K.B, 60) -- leave the shop
    D.talk(K.UP, "cresselia-hint", 5)
    for _ = 1, 4 do D.press(K.B, 30) end
    D.scene(22, MAPS.MAP_ROUTE121, 12, 10)
    D.hold(K.UP, 2)
    D.wait(200)
    D.shot("cresselia-route121")

    D.press(K.R, 60)
    for _ = 1, 3 do D.press(K.DOWN, 8) end
    D.press(K.A, 60)
    D.shot("quests")
    D.press(K.DOWN, 20)
    D.shot("quests-suicune")
    D.press(K.DOWN, 20)
    D.shot("quests-entei")
end)
