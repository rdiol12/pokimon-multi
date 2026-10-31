-- Weather lab at the Route 119 Weather Institute: the researcher signs TEST1
-- up and shows the forecast; out in a thunderstorm on Route 110 the
-- POKéNAV takes a reading; reporting it (with 8 points banked) reaches the
-- ASSISTANT rank. Then R > PASTIMES > WEATHER LAB.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local READINGS, SEEN, POINTS = 144, 146, 148 -- struct PastimeSaveData offsets

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_ROUTE119_WEATHER_INSTITUTE_1F, 12, 11)
    D.pastime16(POINTS, 8)
    D.wait(30)
    D.shot("researcher")
    D.talk(K.UP, "welcome", 2)
    D.waitFor("the yes/no", 900, function() return D.pastimeRead8(SEEN) ~= 0 end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
        if i == 300 then D.shot("welcome-end") end
    end)
    D.wait(30)
    D.shot("joined")
    for i = 1, 5 do
        D.wait(60)
        D.shot("joined-" .. i)
        D.press(K.A, 0)
    end
    D.wait(60)
    D.shot("menu")
    D.press(K.A, 120) -- FORECAST
    D.shot("forecast")
    D.press(K.B, 90)
    D.shot("menu-again")
    for _ = 1, 4 do D.press(K.B, 40) end
    -- Out under the thunderstorm.
    D.scene(13, MAPS.MAP_ROUTE110, 14, 95, {storm = "thunder"})
    D.waitFor("a reading", 600, function() return D.pastimeRead8(READINGS) > 0 end)
    for i = 1, 4 do
        D.wait(40)
        D.shot("reading-" .. i)
    end
    D.wait(60)
    D.pastime8(SEEN, D.pastimeRead8(SEEN) | 0x0D) -- rain, sand and snow logged before
    D.scene(12, MAPS.MAP_ROUTE119_WEATHER_INSTITUTE_1F, 12, 11)
    D.talk(K.UP, "hello", 1)
    D.wait(60)
    D.shot("menu-2")
    D.press(K.DOWN, 10)
    D.press(K.A, 90) -- REPORT READINGS
    D.shot("reported")
    for i = 1, 10 do
        D.wait(60)
        D.shot("after-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 6 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(14)
    D.wait(30)
    D.shot("weather-list")
    D.press(K.B, 60)
end)
