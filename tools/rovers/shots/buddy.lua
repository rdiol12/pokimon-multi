-- Offline: a rover diving in the Rift Below explores B1F with the player,
-- fights the wild Pokémon beside them, and shares a find when they part.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.riftFloor(1)
    D.scene(11, MAPS.MAP_ADVENTURE_DUNGEON, 16, 27, { give = "MUDKIP", level = 30, lead = true })
    D.rovers(1, {12}, 16, 3, true, 1)
    D.answer(true, "buddy-ask")
    D.press(K.A, 60)
    D.press(K.A, 60)
    D.shot("buddy-joined")
    local n = 0
    D.hold(K.UP, 70)
    D.wait(10)
    D.shot("buddy-walk-0")
    for k = 1, 40 do
        if not D.inOverworld() then break end
        D.hold(k % 2 == 1 and K.UP or K.DOWN, 32)
        D.wait(4)
        if k <= 4 then n = n + 1; D.shot("buddy-walk-" .. n) end
    end
    D.waitFor("the battle", 900, D.inBattle)
    n = 0
    D.waitFor("the overworld", 30000, D.inOverworld, function(i)
        if i % 90 == 0 and n < 120 then n = n + 1; D.shot("buddy-battle-" .. n) end
        if i % 50 == 0 then D.press(K.A, 0) end
    end)
    D.wait(150)
    D.shot("buddy-back")
    D.hold(K.UP, 40)
    D.wait(20)
    D.shot("buddy-back2")
    D.scene(11, MAPS.MAP_ROUTE114, 10, 65)
    for i = 1, 8 do D.wait(30); D.shot("farewell-" .. i) end
    D.press(K.A, 60)
    D.shot("farewell-item")
    D.press(K.A, 60)
    D.result("ok")
end)
