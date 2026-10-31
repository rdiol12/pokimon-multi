-- Pokémon cafe: the chef at CAFÉ SOLEIL in Lilycove cooks a Spicy Curry from
-- Cheri and Oran Berries; the next wild battle opens with an Attack boost.
-- Then R > PASTIMES > CAFE.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- FIRST_BERRY_INDEX hides the Cheri Berry from run.py; the Oran Berry is six after it.
ITEM_ID.CHERI_BERRY = ITEM_ID.ORAN_BERRY - 6

local DISH = 80 -- struct PastimeSaveData offset of cafeDish

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 38, 28, { item = "ORAN_BERRY" })
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 38, 28, { item = "CHERI_BERRY" })
    D.wait(30)
    D.shot("chef")
    D.talk(K.UP, "welcome", 2)
    D.wait(60)
    D.shot("dishes")
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 20)
    D.shot("dishes-feast")
    D.press(K.A, 60)
    D.shot("feast-recipe")
    D.press(K.A, 60)
    D.shot("feast-missing")
    D.press(K.A, 60)
    D.wait(60)
    for _ = 1, 5 do D.press(K.UP, 10) end
    D.press(K.A, 60)
    D.shot("curry-recipe")
    D.press(K.A, 60)
    D.shot("curry-recipe-2")
    D.press(K.A, 60)
    D.wait(90)
    D.shot("cook")
    -- YES; the menu can miss a press while it opens.
    D.waitFor("the dish", 600, function() return D.pastimeRead8(DISH) == 1 end, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    -- The fanfare's length varies, so take a shot every 40 frames and
    -- close the messages with B (A would talk to the chef again).
    for i = 1, 16 do
        D.wait(40)
        D.shot("served-" .. i)
        if i >= 8 and i % 2 == 0 then D.press(K.B, 0) end
    end
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(30)
    D.pastimeMenu(6)
    D.wait(30)
    D.shot("cafe-list")
    D.press(K.B, 60)
    D.scene(12, MAPS.MAP_ROUTE101, 4, 4)
    D.findBattle(K.LEFT, K.RIGHT)
    for i = 1, 8 do
        D.wait(50)
        D.shot("battle-" .. i)
        D.press(K.A, 10)
    end
end)
