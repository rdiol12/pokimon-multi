-- Co-op fishing with two players on Route 103: TEST1 faces the water and
-- asks TEST2 for help, TEST2 joins, both play the reeling meter, and both
-- land the giant. Then the giant catch board.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local M = function() return D.SYM.gWorldTimingMeter end

local function meterOn() return D.read8(M() + 20) == 0xFF end

local function reel(name)
    D.waitFor("the reeling meter", 6000, meterOn, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    D.playMeter(false, name)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_ROUTE103, 21, P1 and 9 or 11)
    D.hold(K.RIGHT, 8)
    D.wait(120)
    D.hold(K.RIGHT, 8)
    D.wait(30)
    if P1 then
        D.openHub(3, "hub")
        D.wait(20)
        D.shot("cofish-menu")
        D.press(K.A, 150) -- FISH TOGETHER
        D.shot("pick-partner")
        D.press(K.A, 40)  -- TEST2
        D.wait(30)
        D.shot("waiting")
    else
        D.waitFor("the invitation", 20000, D.fieldLocked)
        D.wait(90)
        D.shot("invitation")
        D.press(K.A, 60) -- YES
    end
    D.wait(60)
    D.shot("bite")
    reel(P1 and "reeling" or "reeling-p2")
    D.waitFor("the result", 6000, function() return D.read16(D.SYM.gSpecialVar_0x800B) == 1 end)
    D.wait(150)
    D.shot("landed")
    local page = 1
    while D.fieldLocked() and page < 6 do
        D.press(K.A, 90)
        page = page + 1
        if D.fieldLocked() then D.shot("landed-" .. page) end
    end
    D.wait(60)
    if P1 then
        D.openHub(3)
        D.press(K.DOWN, 8)
        D.press(K.A, 150)
        D.shot("giant-board")
        D.press(K.A, 40)
    end
end)
