-- Star map and meteor shower nights. At 21:00 the Lilycove stargazer shows
-- TEST1 THE SKY WHALE through the telescope (the second constellation on
-- the STAR MAP). Four days later it's a meteor shower night on Route 114:
-- shooting stars, and a meteorite to pick up. Then the astronomer at the
-- Mossdeep Space Center gives the first STAR MAP prize, and R > PASTIMES >
-- STAR MAP.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local SEEN, FOUND = 152, 158 -- struct PastimeSaveData offsets

-- Moves the clock on whole days: the local time is the RTC minus
-- localTimeOffset (SaveBlock2), so the offset's days go down.
local function addDays(n)
    local addr = D.read32(D.SYM.gSaveBlock2Ptr) + 0x98
    local days = (D.read16(addr) - n) & 0xFFFF
    D.write8(addr, days & 0xFF); D.write8(addr + 1, days >> 8)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(21, MAPS.MAP_LILYCOVE_CITY, 30, 22)
    D.pastime16(SEEN, 0x02)
    D.wait(30)
    D.shot("stargazer")
    D.talk(K.UP, "offer", 1)
    D.press(K.A, 60)
    D.wait(30)
    D.shot("chart")
    D.press(K.A, 90)
    D.shot("looks-like")
    D.press(K.A, 120)
    D.shot("logged")
    D.press(K.A, 60)
    for _ = 1, 3 do D.press(K.B, 40) end
    -- A meteor shower night, by the Route 114 meteorite.
    D.scene(21, MAPS.MAP_ROUTE114, 24, 52)
    addDays(4)
    D.wait(40)
    for i = 1, 8 do
        D.wait(12)
        D.shot("shower-" .. i)
    end
    D.talk(K.UP, "meteorite", 1)
    for i = 1, 6 do
        D.wait(50)
        D.shot("meteorite-" .. i)
        D.press(K.A, 0)
    end
    D.waitFor("the find", 600, function() return D.pastimeRead8(FOUND) > 0 end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(30)
    D.shot("meteorite-gone")
    D.scene(14, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 8, 8)
    D.talk(K.UP, "astronomer", 1)
    for i = 1, 10 do
        D.wait(60)
        D.shot("astronomer-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 6 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(15)
    D.wait(30)
    D.shot("star-map")
    D.press(K.B, 60)
end)
