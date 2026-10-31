-- Online contests: one Cute contest win (P1) and one loss (P2).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.showcase({ level = 45, badges = 6, party = P1 and { "SKITTY", "MILOTIC" } or { "ZIGZAGOON", "ALTARIA" } })
    if P1 then D.setCondition(0, 2, 230, 120) end
    D.scene(13, MAPS.MAP_LILYCOVE_CITY, P1 and 27 or 29, 9)
    if not P1 then D.wait(1400) end
    D.community(4)
    D.wait(40) D.shot("which")
    D.press(K.A, 30)
    D.wait(20) D.shot("categories")
    for _ = 1, P1 and 2 or 1 do D.press(K.DOWN, 8) end
    D.press(K.A, 30)
    D.wait(30) D.shot("whichmon")
    D.press(K.A, 120)
    D.shot("party")
    local before = D.answers()
    D.waitFor("the results", 900, function() return D.answers() ~= before end, function(i)
        if i % 60 == 30 then D.press(K.A, 0) end
    end)
    D.wait(30) D.shot("results-1")
    D.wait(60) D.shot("results-2")
    D.wait(80) D.shot("results")
    D.press(K.A, 60)
    D.wait(60) D.shot("after")
    D.press(K.A, 60)
    D.wait(200)
end)
