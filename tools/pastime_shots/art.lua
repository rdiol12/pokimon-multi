-- Art studio at the Lilycove Museum: TEST1 paints the POKé BALL by numbers
-- (a few strokes by hand, then the driver fills in most of the canvas),
-- gets it hung and paid for, and, with the other four already hung, wins
-- the gallery prize. Then R > PASTIMES > ART STUDIO.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local A = D.SYM.gArtStudio -- struct ArtStudio (src/pastime_art.c)
local COLOR_COUNT = 152
local BEST = 172 -- struct PastimeSaveData offset of artBest[]

D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

-- sPaintings[0], with the TEXT_COLOR_* of each cell.
local BALL = {
    "....KKKK....", "..KKRRRRKK..", ".KRRrRRRRRK.", ".KRrRRRRRRK.",
    "KRRRRKKRRRRK", "KKKKK..KKKKK", "KKKKK..KKKKK", "K....KK....K",
    ".K........K.", ".K........K.", "..KK....KK..", "....KKKK....",
}
local COLORS = {["."] = 1, K = 2, R = 4, r = 5}

-- Paints rows first..last right, except the cells in skip.
local function fill(first, last, skip)
    for y = first, last do
        for x = 0, 11 do
            if not (skip and skip[y * 12 + x]) then
                D.write8(A + y * 12 + x, COLORS[BALL[y + 1]:sub(x + 1, x + 1)])
            end
        end
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 5, 9)
    for i = 1, 4 do D.pastime8(BEST + i, 95) end
    D.wait(30)
    D.shot("painter")
    D.talk(K.UP, "welcome", 1)
    D.press(K.A, 60)
    D.shot("which")
    D.press(K.A, 60) -- POKé BALL
    D.waitFor("the canvas", 3000, function() return D.read8(A + COLOR_COUNT) ~= 0 end, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    D.wait(30)
    D.shot("canvas")
    -- A few strokes: R picks red, then paint across the top half.
    D.press(K.R, 10)
    D.shot("color")
    D.press(K.LEFT, 6); D.press(K.LEFT, 6); D.press(K.LEFT, 6)
    D.press(K.UP, 6)
    D.press(K.A, 6)
    for _ = 1, 4 do D.hold(K.A | K.RIGHT, 6); D.wait(4) end
    D.wait(20)
    D.shot("strokes")
    fill(0, 5)
    D.press(K.DOWN, 10) -- redraw
    D.shot("half")
    fill(6, 11, {[8 * 12 + 1] = true, [9 * 12 + 10] = true})
    D.press(K.UP, 10)
    D.shot("nearly")
    D.press(K.START, 30)
    for i = 1, 12 do
        D.wait(50)
        D.shot("judged-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(18)
    D.wait(30)
    D.shot("art-list")
    D.press(K.B, 60)
end)
