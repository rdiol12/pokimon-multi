-- Duo quests: TEST1 asks TEST2 to team up, TEST2 says yes, both see the
-- quest board, each catches one Pokémon, which finishes the quest; both
-- collect the reward and the next quest starts.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local function catchOne(name)
    D.wildBattle("ZIGZAGOON", 2)
    D.wait(200)
    D.shot(name)
    D.catchThis()
end

local function board(prefix)
    D.openHub(6)
    D.wait(150)
    D.shot(prefix .. "-message")
    D.press(K.A, 60)
    D.shot(prefix)
    D.press(K.A, 60)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_ROUTE103, 21, P1 and 9 or 11, {item = "ULTRA_BALL"})
    if P1 then
        D.openHub(6, "hub")
        D.wait(150)
        D.shot("no-partner")
        D.press(K.A, 60)
        D.shot("find")
        D.press(K.A, 150) -- YES
        D.shot("who")
        D.press(K.A, 150) -- TEST2
        D.shot("asked")
        D.press(K.A, 60)
        D.waitNotice("teamed-notice", 20000)
        D.wait(300)
        board("board")
        D.shot("keep-going")
        D.press(K.A, 60) -- KEEP GOING
        catchOne("battle")
    else
        D.waitNotice("invite-notice", 20000)
        D.openHub(6)
        D.wait(150)
        D.shot("invitation")
        D.press(K.A, 200) -- YES
        D.shot("teamed")
        D.press(K.A, 60)
        D.wait(600)
        board("board")
        D.press(K.A, 60) -- KEEP GOING
        D.wait(900)
        catchOne("battle")
    end
    D.notices("notice", 3000)
    board("reward")
    D.shot("reward")
    D.press(K.A, 250)
    D.shot("reward-claimed")
    D.press(K.A, 60)
    D.wait(60)
end)
