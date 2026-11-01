-- The Market Hall (Slateport) and the Auction House (Lilycove), two players
-- online. Run from tools/netplay_bridge (so the bridge loads its extensions):
--   python3 ../markets/run.py ../markets/shots.lua <out> 60000 2
-- TEST1 walks into the Market Hall, talks to the MARKET MASTER and lists a
-- Pokémon on the market board. TEST2 hears from the old department store
-- auctioneer that the auctions moved, walks into the Auction House, talks
-- to GAVIN at the podium and puts a Pokémon up for auction.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local function onMap(name)
    local m = MAPS[name]
    return function() return D.mb8(0x016) == m[1] and D.mb8(0x017) == m[2] end
end


-- Holds a direction until pred() is true.
local function walk(desc, key, pred)
    D.waitFor(desc, 900, pred, function() D.setKeys(key) end)
    D.setKeys(0)
    D.wait(20)
end

-- Closes the season notice that pops up outdoors, then steps up.
local function settle(y)
    D.wait(60)
    for _ = 1, 3 do D.press(K.B, 40) end
    D.waitFor("a step up", 300, function() return select(2, D.pos()) <= y end, function() D.setKeys(K.UP) end)
    D.setKeys(0)
    D.hold(K.UP, 2) -- face the door
    D.wait(120)
    D.press(K.B, 60) -- a rover's news can pop up meanwhile
end

-- Presses A through `pages` message pages, screenshotting each.
local function pages(name, n)
    for p = 1, n do
        D.wait(70)
        D.shot(string.format("%s-%d", name, p))
        D.press(K.A, 20)
    end
end

-- Presses a key that sends an auction request, and waits for the answer.
local function auctionRequest(key)
    local mb = D.SYM.gNetplayExtMailbox
    local seq = D.read8(mb + 9)
    local function isAuction()
        local word = ""
        for i = 0, 6 do word = word .. string.char(D.read8(mb + 0x10 + i)) end
        return word == "AUCTION"
    end
    D.press(key, 0)
    D.waitFor("the request", 600, function() return D.read8(mb + 9) ~= seq and isAuction() end)
    D.waitFor("the answer", 3000, function() return D.read8(mb + 10) == D.read8(mb + 9) end)
    D.wait(300)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.season(1) -- summer

    if P1 then
        -- Slateport: the hall stands beside the Pokémon Center.
        D.scene(12, MAPS.MAP_SLATEPORT_CITY, 14, 20, { give = "LOTAD", level = 12 })
        settle(20)
        D.shot("01-slateport-market-hall")
        walk("the Market Hall", K.UP, onMap("MAP_SLATEPORT_CITY_MARKET_HALL"))
        D.waitFor("the hall", 300, D.inOverworld)
        D.wait(200)
        walk("the middle of the hall", K.UP, function() return select(2, D.pos()) <= 7 end)
        D.wait(60)
        D.shot("02-market-hall-inside")
        walk("the aisle", K.UP, function() return select(2, D.pos()) <= 5 end)
        D.shot("03-market-hall-counter")
        walk("the counter", K.UP, function() return select(2, D.pos()) <= 4 end)
        -- The MARKET MASTER, then the board.
        local seq = D.mb8(0x518)
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 0)
        pages("04-market-master", 4)
        D.waitFor("the market board", 900, function() return D.mb8(0x518) ~= seq end)
        D.wait(60)
        D.shot("05-market-board")
        local count = D.partyCount()
        D.press(K.SELECT, 150) -- party menu
        D.press(K.DOWN, 20)
        D.press(K.A, 200)
        D.shot("06-market-price")
        seq = D.mb8(0x518)
        D.press(K.A, 10) -- list it
        D.waitFor("the listing", 600, function() return D.partyCount() ~= count end)
        D.waitFor("the board to reload", 1200, function() return D.mb8(0x518) ~= seq end)
        D.wait(60)
        D.shot("07-market-listed")
        D.press(K.B, 60)
        -- The others in the hall.
        walk("the stalls", K.LEFT, function() return D.pos() <= 6 end)
        walk("the shelves", K.DOWN, function() return select(2, D.pos()) >= 9 end)
        D.wait(30)
        D.shot("08-market-hall-floor")
    else
        -- The department store's old auctioneer points the way.
        D.scene(12, MAPS.MAP_LILYCOVE_CITY_DEPARTMENT_STORE_1F, 13, 3, { give = "DRATINI", level = 30, shiny = true })
        D.talk(K.UP, "01-dept-store-moved", 2)
        -- Lilycove: east of the Contest Hall.
        D.scene(12, MAPS.MAP_LILYCOVE_CITY, 47, 25)
        settle(24)
        D.shot("02-lilycove-auction-house")
        walk("the Auction House", K.UP, onMap("MAP_LILYCOVE_CITY_AUCTION_HOUSE"))
        D.waitFor("the house", 300, D.inOverworld)
        D.wait(200)
        walk("the sale room", K.UP, function() return select(2, D.pos()) <= 15 end)
        D.wait(60)
        D.shot("03-auction-house-inside")
        walk("the stage", K.UP, function() return select(2, D.pos()) <= 10 end)
        D.wait(30)
        D.shot("04-auction-stage")
        -- GAVIN, then the auction.
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 0)
        for p = 1, 3 do
            D.wait(70)
            D.shot(string.format("05-auctioneer-%d", p))
            D.press(K.A, 20)
        end
        D.wait(70)
        D.shot("05-auctioneer-4")
        auctionRequest(K.A) -- the last page closes: COLLECT
        D.shot("06-auction-menu")
        D.wait(30)
        D.press(K.DOWN, 30)
        D.press(K.A, 0) -- SELL
        D.wait(160)
        D.press(K.A, 30)
        D.wait(120)
        D.press(K.DOWN, 20)
        D.press(K.DOWN, 20)
        D.shot("07-auction-sell-party")
        D.press(K.A, 120)
        for _ = 1, 5 do D.press(K.UP, 12) end
        D.wait(20)
        D.shot("08-auction-price")
        D.press(K.A, 120)
        D.press(K.A, 120) -- 1 HOUR
        D.shot("09-auction-confirm")
        auctionRequest(K.A) -- YES
        D.wait(160)
        D.shot("10-auction-listed")
        D.press(K.A, 30)
        D.wait(30)
        auctionRequest(K.A) -- BROWSE
        D.shot("11-auction-board")
        D.press(K.B, 60)
        D.press(K.B, 60)
        walk("the floor", K.DOWN, function() return select(2, D.pos()) >= 15 end)
        D.shot("12-auction-floor")
    end
end)
