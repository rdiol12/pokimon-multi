-- Offline: three rovers on Route 101; the player talks to Rue and teams up
-- with her against the other two.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 45, lead = true })
    D.rovers(1, {40, 12, 77}, 12, 1, true, 1)
    D.answer(true, "tag-ask")
    D.waitFor("the tag battle", 1500, D.inBattle, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    local n = 0
    D.waitFor("the end", 30000, D.inOverworld, function(i)
        if i % 90 == 0 and n < 150 then n = n + 1; D.shot("tag-" .. n) end
        if i % 50 == 0 then D.press(K.A, 0) end
    end)
    D.wait(60)
    D.shot("after")
    D.result("ok")
end)
