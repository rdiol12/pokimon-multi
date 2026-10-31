-- A fishing tournament with two players on Route 103. It starts as they
-- arrive (both get the notice); each lands Pokémon (a third are the
-- tournament's rare ones), checks the live board, and when it ends both get
-- the results and collect their prizes.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    local y = P1 and 9 or 11
    D.scene(13, MAPS.MAP_ROUTE103, 21, y, { item = "OLD_ROD" })
    D.scene(13, MAPS.MAP_ROUTE103, 21, y, { item = "MASTER_BALL" })
    D.waitNotice(P1 and "tourney-started" or "tourney-started-p2", 100000)
    local landed = 0
    for n = 1, 3 do
        if D.fishOne(K.RIGHT, n == 1 and "hooked" or nil) then
            landed = landed + 1
            D.wait(60)
            -- "+N points" (and maybe another notice)
            if D.fieldLocked() or n == 1 then D.waitNotice("points-" .. n, 600) end
        end
    end
    D.log("landed " .. landed)
    D.wait(200)
    D.openHub(2)
    D.wait(150)
    D.shot("live-message")
    D.press(K.A, 60)
    D.shot("live-board")
    D.press(K.A, 40)
    D.waitFor("the tournament to end", 600000, function() return D.seedSays("ended") end, function(i) if i % 600 == 0 then D.wait(0) end end)
    D.waitNotice(P1 and "results-notice" or "results-notice-p2", 3000)
    D.openHub(2)
    D.wait(150)
    D.press(K.A, 60)
    D.shot("results-board")
    D.press(K.A, 60)
    D.shot("prize-message")
    D.press(K.A, 60)
    D.wait(150)
    D.shot("prize")
    D.press(K.A, 40)
    D.press(K.A, 40)
end)
