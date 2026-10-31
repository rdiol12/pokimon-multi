-- Pokémon pageant: TEST2 enters a shiny Gardevoir; TEST1 looks through the
-- entries (pictures and all) and votes for two; TEST2 hears of the vote,
-- checks the standings and collects last week's prize.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_LILYCOVE_CITY, P1 and 26 or 28, 16,
            not P1 and {give = "GARDEVOIR", level = 40, lead = true, shiny = true} or nil)
    if P1 then
        D.wait(3000) -- TEST2 enters first
        D.openHub(5, "hub")
        D.wait(20)
        D.shot("pageant-menu")
        D.press(K.A, 200) -- VOTE
        D.shot("entry-1")
        for i = 2, 4 do
            D.press(K.DOWN, 8)
            D.press(K.A, 200) -- NEXT
            D.shot("entry-" .. i)
        end
        D.press(K.A, 200) -- VOTE FOR IT (TEST2's)
        D.shot("voted")
        D.press(K.A, 200)
        D.shot("entry-1-again")
        D.press(K.A, 200) -- VOTE FOR IT (Gary's)
        D.shot("voted-2")
        D.press(K.A, 200)
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 60) -- CLOSE
        D.openHub(5)
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- STANDINGS
        D.shot("standings-message")
        D.press(K.A, 60)
        D.shot("standings")
        D.press(K.A, 60)
    else
        D.openHub(5)
        D.press(K.DOWN, 8)
        D.press(K.A, 60) -- ENTER A POKéMON
        D.shot("which")
        D.press(K.A, 120)
        D.shot("party")
        D.press(K.A, 200)
        D.shot("entered")
        D.press(K.A, 60)
        D.waitFor("the field", 600, function() return not D.fieldLocked() end, function(i)
            if i % 30 == 0 then D.press(K.A, 0) end
        end)
        D.waitNotice("vote-notice", 20000)
        D.wait(30)
        D.openHub(5)
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- STANDINGS
        D.shot("standings-message")
        D.press(K.A, 60)
        D.shot("standings")
        D.press(K.A, 60)
        D.shot("prize")
        D.press(K.A, 200)
        D.shot("prize-won")
    end
    D.wait(60)
end)
