-- Offline (run.py ... 1): a rampaging Pokémon from the offline quest board
-- (Route 104 on the harness's day) charges, the player beats it, and the
-- Oldale board pays the reward and lists the job as cleared by the player.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 4, 18, { give = "MUDKIP", level = 45, lead = true })
    D.rovers(1, {3}, 14, 2, true) -- rovers on (the test boot keeps them quiet otherwise)
    D.wait(60)
    D.scene(12, MAPS.MAP_ROUTE104, 22, 2, { give = "MUDKIP", level = 45, lead = true })
    local shots = 0
    D.waitFor("the rampaging Pokémon", 3000, D.inBattle, function(i)
        if i % 90 == 0 and shots < 4 then shots = shots + 1; D.shot("01-route-" .. shots) end
        if i % 90 == 45 and shots >= 2 then D.press(K.A, 0) end
    end)
    D.wait(200)
    D.shot("02-battle")
    D.waitFor("the battle's end", 6000, D.inOverworld, function(i)
        -- A through the menus; now and then a direction, so every move gets tried.
        if i % 40 == 0 then D.press(K.A, 0) end
        if i % 160 == 20 then D.press(({K.RIGHT, K.DOWN, K.LEFT, K.UP})[(i // 160) % 4 + 1], 0) end
        if i == 400 then D.shot("03-battle-2") end
    end)
    D.wait(120)
    D.shot("04-job-done")
    local money = D.money()
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 4, 18)
    D.wait(120)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 0)
    D.answer(false, "05-board")
    D.wait(60)
    D.log("money before " .. money .. " after " .. D.money())
    D.result(D.money() > money and "ok" or "no reward")
end)
