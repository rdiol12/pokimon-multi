-- Battle dojo: the master outside Dewford Gym. TEST1 trains (all five
-- drills, which with 16 points banked earns the YELLOW belt), then teaches
-- MUDKIP AQUA TAIL (any Water type can learn it at the dojo), and opens
-- R > PASTIMES > BATTLE DOJO.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local POINTS, EARNED = 114, 116 -- struct PastimeSaveData offsets

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_DEWFORD_TOWN, 13, 17)
    D.pastime16(POINTS, 6); D.pastime16(EARNED, 16)
    D.wait(30)
    D.shot("master")
    D.talk(K.UP, "welcome", 2)
    D.wait(60)
    D.shot("menu")
    D.press(K.A, 90) -- TRAIN
    D.shot("rules")
    D.press(K.A, 60)
    D.playMeter(false, "drill")
    D.wait(60)
    D.shot("points")
    D.press(K.A, 240)
    D.shot("belt")
    D.press(K.A, 90)
    D.wait(30)
    D.press(K.DOWN, 10)
    D.press(K.A, 90) -- LEARN A MOVE
    D.wait(30)
    D.shot("moves")
    for _ = 1, 5 do D.press(K.DOWN, 8) end
    D.shot("moves-aqua-tail")
    D.press(K.A, 90)
    D.shot("teach")
    -- Through the message to the tutor's box.
    D.waitFor("the box", 900, function() return not D.inOverworld() end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    D.wait(90)
    D.shot("box")
    -- Up to PARTY POKéMON, then MUDKIP and SELECT.
    D.press(K.UP, 40); D.press(K.UP, 40); D.press(K.A, 90)
    D.shot("party-box")
    D.press(K.A, 60)
    D.press(K.A, 120) -- MUDKIP
    for i = 1, 10 do
        D.wait(50)
        D.shot("learn-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 6 do D.press(K.B, 60) end
    D.wait(60)
    D.pastimeMenu(10)
    D.wait(30)
    D.shot("dojo-list")
    for _ = 1, 10 do D.press(K.DOWN, 8) end
    D.shot("dojo-list-moves")
    D.press(K.B, 60)
end)
