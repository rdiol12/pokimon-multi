-- Player-made quests: TEST1 hides 5 Rare Candies on a Lilycove path; TEST2
-- hears of it on Route 103, takes the quest, goes to Lilycove, is told
-- it's on this map, that something glints, and finds it on the spot.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local function closeMessage()
    D.waitFor("the message to close", 600, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    if P1 then
        D.scene(13, MAPS.MAP_LILYCOVE_CITY, 26, 16, {item = "RARE_CANDY"})
        D.openHub(8, "hub")
        D.wait(20)
        D.shot("quests-menu")
        D.press(K.DOWN, 8)
        D.press(K.A, 60) -- HIDE A TREASURE
        D.shot("what")
        D.press(K.A, 60)
        D.shot("how-many")
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- 5
        D.shot("hints")
        for _ = 1, 4 do D.press(K.DOWN, 8) end
        D.shot("hint-pick")
        D.press(K.A, 200) -- On the path
        D.shot("hidden")
        closeMessage()
        D.hold(K.LEFT, 64) -- out of the way
        D.wait(30)
        D.waitNotice("found-notice", 30000)
        D.openHub(8)
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- MY TREASURES
        D.shot("mine")
        closeMessage()
    else
        D.scene(13, MAPS.MAP_ROUTE103, 21, 11)
        D.waitNotice("hid-notice", 20000)
        D.openHub(8)
        D.press(K.A, 150) -- FIND A TREASURE
        D.shot("list")
        D.press(K.A, 150) -- TEST1's
        D.shot("details")
        D.press(K.A, 60)
        D.shot("look")
        D.press(K.A, 150) -- YES
        D.shot("taken")
        closeMessage()
        D.wait(120)
        D.scene(13, MAPS.MAP_LILYCOVE_CITY, 32, 16)
        D.waitFor("the map message", 1200, D.fieldLocked)
        D.wait(110)
        D.shot("on-map")
        closeMessage()
        local found = false
        for step = 1, 10 do
            D.hold(K.LEFT, 16)
            D.wait(30)
            if D.fieldLocked() then
                D.wait(110)
                if not found then
                    D.shot("glint")
                    found = true
                    closeMessage()
                else
                    D.wait(150)
                    D.shot("found")
                    closeMessage()
                    break
                end
            end
        end
    end
    D.wait(60)
end)
