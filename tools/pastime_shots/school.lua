-- Trainer school: the new teacher at the Rustboro Trainer's School. TEST1
-- (who has passed every lesson but STATUS) takes the STATUS test, gets one
-- wrong on purpose, passes with 4 of 5, and graduates. Then R > PASTIMES >
-- TRAINER SCHOOL.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local PASSED = 130 -- struct PastimeSaveData offset of schoolPassed

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_RUSTBORO_CITY_POKEMON_SCHOOL, 11, 4)
    D.pastime8(PASSED, 0x1E)
    D.wait(30)
    D.shot("teacher")
    D.talk(K.UP, "welcome", 2)
    D.wait(60)
    D.shot("lessons")
    D.press(K.A, 90) -- STATUS
    D.shot("start")
    D.press(K.A, 60)
    for q = 1, 5 do
        D.waitFor("question " .. q, 600, function() return D.read8(D.SYM.gSchoolTest + 1) == q end, function(i)
            if i % 60 == 0 then D.press(K.A, 0) end
        end)
        D.wait(120)
        local right = D.read8(D.SYM.gSchoolTest + 3)
        local pick = q == 3 and (right + 1) % 4 or right
        for _ = 1, pick do D.press(K.DOWN, 8) end
        if q <= 2 or q == 3 then D.shot("question-" .. q) end
        D.press(K.A, 60)
        if q <= 3 then D.shot("reply-" .. q) end
        D.press(K.A, 30)
    end
    for i = 1, 14 do
        D.wait(60)
        D.shot("result-" .. i)
        if i % 2 == 0 then D.press(K.A, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(12)
    D.wait(30)
    D.shot("school-list")
    D.press(K.B, 60)
end)
