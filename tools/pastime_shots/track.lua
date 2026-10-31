-- Monthly reward track: with 40 points banked this month (tiers 1 and 2
-- paid), the day's first visit adds 5 and pays tier 3 with a toast. Then
-- R > PASTIMES > REWARD TRACK.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local MONTH, POINTS, TIER, DAY = 184, 186, 188, 190 -- struct PastimeSaveData offsets

D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_PETALBURG_CITY, 15, 14)
    -- The scene's clock is day 0, month 0 (stored + 1). Yesterday's visit.
    D.pastime16(MONTH, 1); D.pastime16(POINTS, 40); D.pastime8(TIER, 2); D.pastime16(DAY, 0)
    D.waitFor("the daily points", 300, function() return D.pastimeRead8(TIER) == 3 end)
    for i = 1, 4 do
        D.wait(20)
        D.shot("toast-" .. i)
    end
    D.wait(200)
    D.pastimeMenu(20)
    D.wait(30)
    D.shot("track")
    for _ = 1, 9 do D.press(K.DOWN, 8) end
    D.shot("track-top")
    D.press(K.B, 60)
end)
