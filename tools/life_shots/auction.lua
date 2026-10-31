-- Auction house, two players online. Run from tools/netplay_bridge (so the
-- bridge loads its extensions), with the server's clock skew file set:
--   LIFE_AUCTION_SKEW_FILE=<out>/clock_skew python3 ../life_shots/run.py \
--       ../life_shots/auction.lua <out> 60000 2
-- TEST1 puts a shiny Dratini up at the Lilycove Department Store, TEST2
-- bids on it from START > ONLINE > AUCTION, the auction runs out, TEST2
-- collects the Dratini and TEST1 collects the money.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local STORE = MAPS.MAP_LILYCOVE_CITY_DEPARTMENT_STORE_1F

-- Both games wait here until the other one gets here too.
local function sync(name)
    local mine = string.format("%s/sync-%s-%d", OUT_DIR, name, INSTANCE)
    local other = string.format("%s/sync-%s-%d", OUT_DIR, name, 3 - INSTANCE)
    io.open(mine, "w"):close()
    D.waitFor("the other game at " .. name, 60000, function()
        local f = io.open(other, "r")
        if f then f:close() return true end
        return false
    end)
end

local function money(thousands)
    local s = D.SYM.gLifeShowcase
    D.write8(s + 10, thousands)
    D.write8(s, 1)
    D.wait(10)
end

-- Presses a key that sends a request to the server, and waits until the
-- answer is in and the next message has printed.
local function request(key)
    local mb = D.SYM.gNetplayExtMailbox
    local seq = D.read8(mb + 9)
    -- The title badges use the same channel in the background, so look
    -- for an AUCTION request.
    local function isAuction()
        local word = ""
        for i = 0, 6 do word = word .. string.char(D.read8(mb + 0x10 + i)) end
        return word == "AUCTION"
    end
    D.press(key, 0)
    D.waitFor("the request", 600, function() return D.read8(mb + 9) ~= seq and isAuction() end)
    D.waitFor("the answer", 3000, function() return D.read8(mb + 10) == D.read8(mb + 9) end)
    D.wait(300) -- most answers save the game

end

-- A message box: lets it print, screenshots it and presses A.
local function msg(name)
    D.wait(160)
    if name then D.shot(name) end
    D.press(K.A, 30)
end

-- Talks to the auctioneer from below: the welcome, then COLLECT.
local function auctioneer(name)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 0)
    D.wait(160)
    D.shot(name)
    request(K.A)
end

-- From the AUCTION HOUSE menu: BROWSE.
local function browse(name)
    D.wait(30)
    request(K.A)
    D.shot(name)
end

local function leave()
    D.press(K.B, 60) -- closes the board
    D.press(K.B, 60) -- LEAVE
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)

    if P1 then
        D.scene(12, STORE, 13, 3, { give = "DRATINI", level = 32, shiny = true })
        D.wait(60)
        D.shot("store")
        auctioneer("auctioneer")
        D.shot("menu")
        D.wait(30)
        D.press(K.DOWN, 30)
        D.press(K.A, 0) -- SELL
        msg("sell-which")
        D.wait(120)
        D.press(K.DOWN, 20)
        D.press(K.DOWN, 20) -- the Dratini
        D.shot("sell-party")
        D.press(K.A, 120)
        D.shot("sell-price")
        for _ = 1, 5 do D.press(K.UP, 12) end -- 1500
        D.wait(20)
        D.shot("sell-price-1500")
        D.press(K.A, 120)
        D.shot("sell-length")
        D.press(K.A, 120) -- 1 HOUR
        D.shot("sell-confirm")
        request(K.A) -- YES
        msg("sell-listed")
        browse("board-mine")
        leave()
    end
    sync("listed")

    if not P1 then
        D.scene(12, STORE, 8, 6)
        money(3)
        D.press(K.START, 40)
        for _ = 1, 4 do D.press(K.DOWN, 10) end
        D.press(K.A, 40) -- ONLINE
        for _ = 1, 5 do D.press(K.DOWN, 10) end
        D.shot("online-menu")
        request(K.A) -- AUCTION
        D.shot("online-auction-menu")
        browse("board")
        D.press(K.A, 30)
        D.shot("bid-amount")
        D.press(K.RIGHT, 30) -- 2500
        D.shot("bid-amount-2500")
        D.press(K.A, 0)
        D.wait(160)
        D.shot("bid-confirm")
        request(K.A) -- YES
        D.shot("bid-done")
        request(K.A) -- back to the board
        D.shot("board-winning")
        leave()
    end
    sync("bid")

    if P1 then
        auctioneer("auctioneer-2")
        browse("board-bids-in")
        leave()
        -- An hour later.
        local f = io.open(OUT_DIR .. "/clock_skew", "w")
        f:write("3700")
        f:close()
    end
    sync("ended")

    if not P1 then
        D.press(K.START, 40)
        D.press(K.A, 40) -- ONLINE (the cursor is still there)
        request(K.A) -- AUCTION
        D.wait(200)
        D.shot("won")
        request(K.A) -- the next COLLECT: nothing more
        D.shot("won-menu")
        D.press(K.B, 60)
        D.press(K.START, 40)
        for _ = 1, 4 do D.press(K.UP, 10) end
        D.press(K.A, 120) -- POKéMON
        D.shot("won-party")
        D.press(K.B, 120)
        D.press(K.B, 40)
    else
        auctioneer("auctioneer-3")
        D.wait(200)
        D.shot("sold")
        request(K.A)
        D.shot("sold-menu")
        D.press(K.B, 60)
    end
    sync("done")
end)
