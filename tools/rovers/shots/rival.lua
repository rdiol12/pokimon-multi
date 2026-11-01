-- Offline: a rival the player beat (Hal) spots them on Route 101, walks up
-- and challenges them; then a friend (Rue) says hello.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {12}, 18, 2, true, 0, 0, 1)
    local n = 0
    D.waitFor("the rival's challenge", 2400, function() return not D.inOverworld() end, function(i)
        if i % 30 == 0 and n < 40 then n = n + 1; D.shot("rival-" .. n) end
        if i % 60 == 59 then D.press(K.A, 0) end
    end)
    D.wait(400)
    D.shot("rival-battle")
    D.result("ok")
end)
