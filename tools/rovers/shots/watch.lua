-- Two rovers battle next to the player; the player watches it as a real
-- battle with both sides played by the AI.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(2, {12, 40}, 22, 3, true, 1)
    D.wait(60)
    D.shot("duel")
    D.wait(120)
    D.shot("ask")
    D.press(K.A, 120) -- YES
    D.shot("watch")
    D.waitFor("the battle", 1500, D.inBattle, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    local n = 0
    D.waitFor("the overworld", 20000, D.inOverworld, function(i)
        if i % 60 == 0 and n < 200 then n = n + 1; D.shot("battle-" .. n) end
        if i % 120 == 0 then D.press(K.A, 0) end
    end)
    D.wait(60)
    D.shot("after")
    D.press(K.A, 60)
    D.wait(60)
    D.shot("after2")
    D.result("ok")
end)
