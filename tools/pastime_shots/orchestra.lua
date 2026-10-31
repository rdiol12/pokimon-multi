-- Pokémon orchestra in Verdanturf: TEST1 plays the quartet's tunes back
-- up to 8 notes, then hits a wrong note. Then R > PASTIMES > ORCHESTRA.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local O = D.SYM.gOrchestra -- struct Orchestra (src/pastime_orchestra.c)
local LENGTH, INDEX, STATE = 12, 13, 14
local STATE_PLAY_NOTE, STATE_INPUT, STATE_END = 1, 3, 6
local PAD_KEYS = {[0] = K.UP, K.LEFT, K.RIGHT, K.DOWN}

D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_VERDANTURF_TOWN, 17, 11)
    D.wait(30)
    D.shot("conductor")
    D.talk(K.UP, "welcome", 1)
    D.waitFor("the tune", 3000, function() return D.read8(O + STATE) == STATE_PLAY_NOTE end, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    D.wait(8)
    D.shot("listen")
    local shots = 0
    while true do
        D.waitFor("your turn", 3000, function()
            local s = D.read8(O + STATE)
            return s == STATE_INPUT or s == STATE_END
        end)
        local length = D.read8(O + LENGTH)
        if D.read8(O + STATE) == STATE_END then break end
        local index = D.read8(O + INDEX)
        local note = D.read8(O + index)
        if length >= 8 and index == length - 1 then
            note = (note + 1) % 4 -- a wrong note
        end
        D.press(PAD_KEYS[note], 4)
        if shots < 6 and index == length - 1 then
            shots = shots + 1
            D.shot("play-" .. length)
        end
        D.wait(4)
    end
    for i = 1, 8 do
        D.wait(50)
        D.shot("end-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(17)
    D.wait(30)
    D.shot("orchestra-list")
    D.press(K.B, 60)
end)
