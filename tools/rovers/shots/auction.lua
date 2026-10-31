-- Online: rovers at the auction house. The player has just won Hal's
-- Makuhita (a rover's Pokémon arrives as a real one, Hal its trainer), and
-- Dax has an Aron up. Player 2 only keeps the server company.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function request(key)
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
    if INSTANCE == 2 then
        D.waitFor("player 1", 40000, function()
            local f = io.open(OUT_DIR .. "/done", "r")
            if f then f:close() return true end
            return false
        end)
        D.result("ok")
        return
    end
    local p = D.SYM.gSaveBlock2Ptr
    local sb2 = D.read8(p) | (D.read8(p + 1) << 8) | (D.read8(p + 2) << 16) | (D.read8(p + 3) << 24)
    local name = ""
    for i = 0, 6 do
        local c = D.read8(sb2 + i)
        if c == 0xFF then break end
        name = name .. (c >= 0xBB and c <= 0xD4 and string.char(c - 0xBB + 65) or c >= 0xA1 and c <= 0xAA and string.char(c - 0xA1 + 48) or "?")
    end
    local tid = D.read8(sb2 + 10) | (D.read8(sb2 + 11) << 8)
    os.execute(string.format('python3 "%s/seed_auction.py" "%s/league.sqlite3" %s %d', ROVERS_SHOTS, OUT_DIR, name, tid))
    D.scene(12, MAPS.MAP_LILYCOVE_CITY_DEPARTMENT_STORE_1F, 8, 6)
    D.press(K.START, 40)
    for _ = 1, 4 do D.press(K.DOWN, 10) end
    D.press(K.A, 40) -- ONLINE
    for _ = 1, 7 do D.press(K.DOWN, 10) end
    request(K.A) -- AUCTION: collects first
    D.wait(200)
    D.shot("won")
    request(K.A)
    D.shot("menu")
    D.wait(30)
    request(K.A) -- BROWSE
    D.shot("board")
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.wait(60)
    D.press(K.START, 40)
    for _ = 1, 4 do D.press(K.UP, 10) end
    D.press(K.A, 120) -- POKéMON
    D.shot("party")
    for _ = 1, 2 do D.press(K.DOWN, 12) end
    D.press(K.A, 30)
    D.press(K.A, 120) -- SUMMARY
    D.shot("summary")
    io.open(OUT_DIR .. "/done", "w"):close()
    D.result("ok")
end)
