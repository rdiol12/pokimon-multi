-- Spectator betting: Gary and Mia battle; both games hear of it, TEST1
-- bets 100 COINS on Gary and TEST2 50 on Mia; Gary wins, TEST1 collects
-- the pool, and the best bettors board.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_MAUVILLE_CITY, P1 and 20 or 22, 9)
    D.waitNotice("battle-notice", 40000)
    D.openHub(9, P1 and "hub" or nil)
    D.wait(20)
    D.shot("betting-menu")
    D.press(K.A, 200) -- BET ON A BATTLE
    D.shot("battles")
    D.press(K.A, 200) -- Gary vs Mia
    D.shot("sides")
    if not P1 then D.press(K.DOWN, 8) end
    D.press(K.A, 100)
    D.shot("how-much")
    D.press(K.DOWN, 8)
    if P1 then D.press(K.DOWN, 8) end
    D.press(K.A, 200)
    D.shot("placed")
    D.press(K.A, 60)
    D.notices("notice", 12000)
    if P1 then
        D.openHub(9)
        D.press(K.DOWN, 8)
        D.press(K.A, 250) -- COLLECT WINNINGS
        D.shot("collected")
        D.press(K.A, 60)
    end
    D.openHub(9)
    D.press(K.DOWN, 8)
    D.press(K.DOWN, 8)
    D.press(K.A, 150) -- BEST BETTORS
    D.shot("board")
    D.press(K.A, 60)
    D.wait(60)
end)
