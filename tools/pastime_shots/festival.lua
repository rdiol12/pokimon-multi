-- Seasonal festivals. Day 5 of the week is a festival day: in spring the
-- BLOSSOM FESTIVAL host stands in Lilycove with petals drifting down. A stall
-- stamps the card; with a full card the host gives the Leaf Stone. Then the
-- summer FIREWORKS FESTIVAL at night, autumn leaves, the winter host, and
-- R > PASTIMES > FESTIVALS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local STAMPS, DONE = 198, 200 -- struct PastimeSaveData offsets

local function addDays(n)
    local addr = D.read32(D.SYM.gSaveBlock2Ptr) + 0x98
    local days = (D.read16(addr) - n) & 0xFFFF
    D.write8(addr, days & 0xFF); D.write8(addr + 1, days >> 8)
end

local function season(s)
    D.write8(D.SYM.gLifeShowcase + 2, s + 1)
end

local function festival(s, hour, x, y)
    season(s)
    D.scene(hour, MAPS.MAP_LILYCOVE_CITY, x, y)
    addDays(5)
    D.pastime16(196, 0) -- a new festival (the test's seasons share a week)
    D.wait(400) -- past the season and map name popups
end

D.run(function()
    D.boot()
    D.wait(60)
    festival(0, 12, 31, 16)
    D.shot("spring-host")
    D.talk(K.UP, "spring-welcome", 4)
    for _ = 1, 3 do D.press(K.B, 30) end
    -- The stall by the department store.
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 37, 18)
    addDays(5)
    D.wait(60)
    D.shot("stall")
    D.talk(K.UP, "stall", 2)
    for _ = 1, 3 do D.press(K.B, 30) end
    -- The other two stamps, then the host's prize.
    D.pastime8(STAMPS, 7)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 31, 16)
    addDays(5)
    D.wait(60)
    D.talk(K.UP, "prize", 1)
    for i = 1, 6 do
        D.wait(60)
        D.shot("prize-" .. i)
        D.press(K.A, 0)
    end
    D.waitFor("the prize", 600, function() return D.pastimeRead8(DONE) > 0 end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    for _ = 1, 4 do D.press(K.B, 40) end
    -- Summer night: fireworks.
    festival(1, 21, 31, 16)
    for i = 1, 8 do
        D.wait(15)
        D.shot("fireworks-" .. i)
    end
    D.talk(K.UP, "summer-host", 1)
    for _ = 1, 5 do D.press(K.B, 60) end
    -- Autumn leaves.
    festival(2, 14, 31, 16)
    D.wait(40)
    D.shot("autumn")
    D.talk(K.UP, "autumn-host", 1)
    for _ = 1, 5 do D.press(K.B, 60) end
    festival(3, 17, 31, 16)
    D.talk(K.UP, "winter-host", 1)
    for _ = 1, 5 do D.press(K.B, 60) end
    D.wait(60)
    D.pastimeMenu(22)
    D.wait(30)
    D.shot("festivals")
    D.press(K.B, 60)
end)
