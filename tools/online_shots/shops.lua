-- Player shops: TEST2 stocks a shelf from the bag; TEST1 browses the shops,
-- opens TEST2's and buys; TEST2 hears of the sale and collects the money.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- Presses A until the field is free, screenshotting each page.
local function pages(name)
    local page = 1
    while D.fieldLocked() and page < 6 do
        D.press(K.A, 90)
        page = page + 1
        if D.fieldLocked() then D.shot(name .. "-" .. page) end
    end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_LILYCOVE_CITY, P1 and 26 or 28, 16)
    if P1 then
        D.wait(3600) -- TEST2 stocks a shelf first
        D.openHub(4, "hub")
        D.wait(20)
        D.shot("shops-menu")
        D.press(K.A, 150) -- VISIT A SHOP
        D.shot("shop-list")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.shot("shop-list-pick")
        D.press(K.A, 150) -- TEST2'S SHOP
        D.shot("welcome")
        D.press(K.A, 60)
        D.shot("board")
        D.press(K.A, 150)
        D.shot("items")
        D.press(K.A, 60)
        D.shot("how-many")
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- 5
        D.shot("bought")
        D.press(K.A, 150)
        D.shot("anything-else")
        D.press(K.B, 60)
        D.waitFor("the field", 600, function() return not D.fieldLocked() end, function(i)
            if i % 30 == 0 then D.press(K.B, 0) end
        end)
    else
        D.giveItem("SUPER_POTION", 8)
        D.giveItem("GREAT_BALL", 10)
        D.openHub(4)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- MY SHOP
        D.shot("my-shop")
        D.press(K.A, 60) -- STOCK AN ITEM
        D.shot("bag-list")
        D.press(K.A, 60)
        D.shot("how-many-sell")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- ALL
        D.shot("prices")
        D.press(K.A, 150) -- the lowest
        D.shot("stocked")
        pages("stocked")
        D.waitNotice("sale-notice", 20000)
        D.wait(30)
        D.openHub(4)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- MY SHOP
        D.shot("money-waiting")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- COLLECT MONEY
        D.shot("collected")
        pages("collected")
    end
    D.wait(60)
end)
