-- Cooking contest, with two players online (run.py ... 2): TEST1 enters the
-- Lilycove COOK-OFF with a Spicy Curry, plays the timing meter perfectly and
-- wins the NORMAL rank, then (the next day) misses every step in the GREAT
-- rank and loses. Then R > PASTIMES > COOK-OFF and the online board.
-- TEST2 waits.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local COOK_DAY = 84 -- struct PastimeSaveData offset of cookDay

-- FIRST_BERRY_INDEX hides the Cheri Berry from run.py; the Oran Berry is six after it.
ITEM_ID.CHERI_BERRY = ITEM_ID.ORAN_BERRY - 6

-- Talks to the judge and cooks the Spicy Curry (the first dish).
local function enter(prefix, miss)
    D.pastime8(COOK_DAY, 0); D.pastime8(COOK_DAY + 1, 0)
    D.talk(K.UP, prefix .. "welcome", 3)
    -- YES, then the first dish; the menus can miss a press while they open.
    D.waitFor("the entry", 900, function() return D.pastimeRead8(COOK_DAY) ~= 0 end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    D.wait(90)
    D.shot(prefix .. "start")
    D.press(K.A, 0)
    D.playMeter(miss, prefix .. "meter")
    for i = 1, 14 do
        D.wait(50)
        D.shot(prefix .. "verdict-" .. i)
        if i % 2 == 0 then D.press(K.B, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if not P1 then
        D.scene(12, MAPS.MAP_LILYCOVE_CITY, 33, 28)
        D.wait(9000)
        return
    end
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 40, 27, { item = "ORAN_BERRY" })
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 40, 27, { item = "CHERI_BERRY" })
    D.wait(30)
    D.shot("judge")
    enter("normal-", false)
    D.wait(60)
    enter("great-", true)
    D.wait(60)
    D.pastimeMenu(7)
    D.wait(20)
    D.press(K.A, 40)
    D.wait(20)
    D.shot("records")
    D.press(K.B, 60)
    D.wait(30)
    D.pastimeMenu(7)
    D.wait(20)
    D.press(K.DOWN, 10)
    D.press(K.A, 40)
    D.wait(120)
    D.shot("cook-off-champs")
    D.press(K.B, 40)
end)
