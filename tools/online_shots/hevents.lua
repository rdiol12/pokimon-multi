-- Player-hosted events: TEST1 looks at the events board and hosts a catch
-- hunt for ZIGZAGOON with money and items from the bag as the prize. TEST2
-- hears of it, joins, catches a ZIGZAGOON and picks up the prize; TEST1
-- hears who won. TEST2 also looks at a tournament's bracket.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local HUB_EVENTS = 19

local function closeMessage()
    D.waitFor("the message to close", 600, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
end

-- HUB > PLAYER EVENTS, then row `row` of its menu.
local function openEvents(row, shotName)
    D.openHub(HUB_EVENTS)
    D.wait(150)
    if shotName then D.shot(shotName) end
    for _ = 1, row do D.press(K.DOWN, 8) end
    D.press(K.A, 150)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    if P1 then
        D.scene(13, MAPS.MAP_PETALBURG_CITY, 15, 12, {item = "RARE_CANDY"})
        openEvents(0, "menu") -- EVENTS BOARD
        D.shot("board")
        D.press(K.A, 30)
        closeMessage()
        openEvents(2) -- HOST AN EVENT
        D.shot("kind")
        D.press(K.A, 150) -- CATCH HUNT
        D.shot("targets")
        D.press(K.A, 150) -- ZIGZAGOON
        D.shot("time")
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- 1 HOUR
        D.shot("money")
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- $1000
        D.shot("add-item")
        D.press(K.A, 60) -- YES
        D.shot("bag")
        D.press(K.A, 60)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- 5
        D.shot("summary")
        D.press(K.A, 60)
        D.shot("summary-2")
        D.press(K.A, 60)
        D.shot("post")
        D.press(K.A, 200) -- YES
        D.shot("posted")
        closeMessage()
        D.waitNotice("joined-notice", 20000)
        D.waitNotice("won-notice", 40000)
        openEvents(0)
        D.shot("board-after")
        D.press(K.A, 30)
        closeMessage()
    else
        D.scene(13, MAPS.MAP_ROUTE101, 10, 10, {item = "MASTER_BALL"})
        D.waitNotice("hosting-notice", 30000)
        openEvents(1) -- JOIN AN EVENT
        D.shot("list")
        for _ = 1, 3 do D.press(K.DOWN, 8) end
        D.press(K.A, 150) -- TEST1's hunt
        D.shot("details")
        D.press(K.A, 60)
        D.shot("details-2")
        D.press(K.A, 60)
        D.shot("details-3")
        D.press(K.A, 60)
        D.shot("join")
        D.press(K.A, 150) -- YES
        D.shot("joined")
        closeMessage()
        D.wildBattle("ZIGZAGOON", 3)
        D.waitFor("the battle", 1200, D.inBattle)
        D.wait(400)
        D.shot("battle")
        D.catchThis()
        D.wait(400)
        openEvents(0)
        D.wait(200)
        D.shot("prize")
        closeMessage()
        openEvents(1) -- the cup under way
        for _ = 1, 2 do D.press(K.DOWN, 8) end
        D.press(K.A, 150) -- Ivy's cup
        D.shot("cup-details")
        D.press(K.A, 60)
        D.shot("cup-details-2")
        D.press(K.A, 60)
        D.shot("cup-details-3")
        D.press(K.A, 60)
        D.wait(30)
        D.shot("bracket")
        D.press(K.A, 30)
        closeMessage()
    end
    D.wait(60)
end)
