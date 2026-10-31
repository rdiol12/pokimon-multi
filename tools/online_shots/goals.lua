-- Server-wide catch goals with two players. Other players (seed) have
-- nearly filled the week's goals; TEST1 looks at the board, catches the
-- Pokémon that fills the first goal, and both players get the notice and
-- collect the reward (TEST2 caught one earlier this week, in the seed).
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    if P1 then
        D.openHub(0, "hub")
        D.waitAnswer()
        D.shot("goals-board")
        D.press(K.A, 40)
        D.scene(12, MAPS.MAP_ROUTE101, 4, 4, { item = "MASTER_BALL" })
        D.catchWild()
        D.waitNotice("goal-reached", 900)
    else
        D.waitNotice("goal-reached-other-player", 20000)
    end
    D.wait(60)
    D.openHub(0)
    D.waitAnswer()
    D.shot("goals-board-reached")
    D.press(K.A, 60)
    D.shot("collect-ask")
    D.press(K.A, 200) -- YES
    D.shot("collected")
    D.press(K.A, 40)
    D.press(K.A, 40)
end)
