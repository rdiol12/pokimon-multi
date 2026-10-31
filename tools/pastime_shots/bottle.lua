-- Bottle treasure maps. Today's bottle bobs on the Route 104 beach: Captain
-- Marlow's first letter and the first piece of his first map. Then (with
-- the next two pieces already found) the last piece makes the map whole and
-- shows its riddle; A on the X on Route 109 digs up the Pearl String. Then
-- R > PASTIMES > BOTTLE MAPS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local DAY, PIECES = 236, 238 -- struct PastimeSaveData offsets

local function readBottle(name, pages)
    D.press(K.A, 40)
    for i = 1, pages do
        D.wait(70)
        D.shot(name .. "-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 50) end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE104, 17, 64)
    D.wait(200)
    D.shot("beach")
    D.hold(K.LEFT, 34)
    D.wait(20)
    D.shot("bottle")
    readBottle("first", 5)
    D.wait(30)
    D.shot("bottle-gone")
    -- Pieces 2 and 3 found on other days; today's bottle again.
    D.pastime16(PIECES, 0x7)
    D.pastime16(DAY, 0)
    D.wait(30)
    D.hold(K.LEFT, 2)
    readBottle("last", 11)
    D.scene(13, MAPS.MAP_ROUTE109, 31, 10)
    D.wait(200)
    D.hold(K.RIGHT, 2)
    D.wait(20)
    D.shot("x-spot")
    D.press(K.A, 40)
    for i = 1, 6 do
        D.wait(80)
        D.shot("dig-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 50) end
    D.pastimeMenu(26)
    D.wait(30)
    D.shot("bottle-maps")
    D.press(K.B, 60)
end)
