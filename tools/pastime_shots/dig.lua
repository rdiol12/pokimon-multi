-- Archaeology digs. The archaeologist in Fallarbor Town, then the Route 114
-- dig site: a few empty squares glow with how close the finds are, then the
-- finds come out one by one. Digging there again the same day, the
-- archaeologist's first reward, and R > PASTIMES > DIG SITES.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local COLS, ROWS = 8, 6
local CURSOR_X, CURSOR_Y = 74, 75 -- struct Dig offsets

local function dig(x, y)
    D.write8(D.SYM.gDig + CURSOR_X, x)
    D.write8(D.SYM.gDig + CURSOR_Y, y)
    D.press(K.A, 12)
end

local function cell(x, y) return D.read8(D.SYM.gDig + y * COLS + x) end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(11, MAPS.MAP_FALLARBOR_TOWN, 12, 14)
    D.wait(200)
    D.shot("archaeologist")
    D.talk(K.UP, "welcome", 4)
    for _ = 1, 5 do D.press(K.B, 60) end
    D.scene(11, MAPS.MAP_ROUTE114, 22, 12)
    D.wait(200)
    D.shot("site")
    D.hold(K.UP, 18)
    D.wait(20)
    D.talk(K.UP, "site", 2)
    D.press(K.A, 60) -- YES
    D.press(K.A, 60)
    D.press(K.A, 60)
    D.wait(30)
    D.shot("dig-start")
    -- Three misses: empty squares a little away from the finds.
    local misses = 0
    for y = 0, ROWS - 1, 2 do
        for x = 0, COLS - 1, 3 do
            if misses < 3 and cell(x, y) == 0 then
                dig(x, y); misses = misses + 1
            end
        end
    end
    D.wait(20)
    D.shot("dig-heat")
    -- Then every square of every find.
    local n = 0
    for y = 0, ROWS - 1 do
        for x = 0, COLS - 1 do
            if cell(x, y) & 0x7F ~= 0 and cell(x, y) & 0x80 == 0 then
                dig(x, y)
                n = n + 1
                if n == 2 then D.shot("dig-part") end
            end
        end
    end
    D.wait(10)
    D.shot("dig-done")
    for i = 1, 4 do
        D.wait(70)
        D.shot("found-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 60) end
    D.talk(K.UP, "dug-today", 1)
    for _ = 1, 3 do D.press(K.B, 60) end
    D.scene(11, MAPS.MAP_FALLARBOR_TOWN, 12, 14)
    D.wait(200)
    D.talk(K.UP, "reward", 6)
    for _ = 1, 5 do D.press(K.B, 60) end
    D.pastimeMenu(23)
    D.wait(30)
    D.shot("dig-sites")
    D.press(K.B, 60)
end)
