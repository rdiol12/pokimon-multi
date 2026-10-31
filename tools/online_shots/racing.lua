-- Mount racing: TEST1 and TEST2, each with a Pokémon to ride, join the same
-- PETALBURG RUN heat, count down at the start line on Route 102 and race to
-- Petalburg City; both hear the results, and TEST1 looks at the best times.
-- The ride follows race_path.lua (racing_seed.py plans it from the map).
dofile(OUT_DIR .. "/lib.lua")
dofile(OUT_DIR .. "/race_path.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- Rides to each corner of the path in turn, holding the way there.
local function ride(path, shotEvery)
    local shots, frame = 0, 0
    for _, corner in ipairs(path) do
        local tx, ty = corner[1], corner[2]
        D.waitFor("the corner " .. tx .. "," .. ty, 2400, function()
            local x, y = D.pos()
            return (x == tx and y == ty) or D.fieldLocked()
        end, function()
            local x, y = D.pos()
            local key = x > tx and K.LEFT or x < tx and K.RIGHT or y > ty and K.UP or K.DOWN
            D.setKeys(key)
            frame = frame + 1
            if frame % shotEvery == 0 and shots < 4 then
                shots = shots + 1
                D.shot("racing")
            end
        end)
        if D.fieldLocked() then break end
    end
    -- Over the edge into Petalburg City: the finish.
    D.waitFor("the finish", 600, D.fieldLocked, function() D.setKeys(K.LEFT) end)
    D.setKeys(0)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 5 or 7, 16, {give = P1 and "RAPIDASH" or "ARCANINE", level = 40})
    if not P1 then D.wait(240) end
    D.openHub(13, P1 and "hub" or nil)
    D.wait(150)
    D.shot("courses")
    D.press(K.A, 60)
    D.shot("course-menu")
    D.press(K.A, 150) -- ENTER THE RACE
    D.shot("joined")
    D.press(K.A, 30)
    -- The countdown (the field is locked), then GO!
    D.waitFor("the start line", 1200, function() return D.fieldLocked() and D.inOverworld() end)
    D.wait(120)
    D.shot("countdown")
    D.waitFor("GO!", 2400, function() return not D.fieldLocked() end, function(i)
        if i == 600 then D.shot("countdown-2") end
    end)
    D.shot("go")
    ride(RACE_PATH[INSTANCE], 150)
    D.wait(60)
    D.shot("finish")
    D.wait(240)
    D.press(K.A, 240)
    D.shot("result")
    -- Back where they joined (notices from the race come up then).
    local oldale = MAPS.MAP_OLDALE_TOWN
    D.waitFor("the way back", 900, function() return D.mb8(0x016) == oldale[1] and D.mb8(0x017) == oldale[2] end,
        function(i) if i % 60 == 0 and D.mb8(0x017) ~= oldale[2] then D.press(K.A, 0) end end)
    D.wait(60)
    D.notices("results", 1500)
    if P1 then
        D.openHub(13)
        D.wait(150)
        D.press(K.A, 60)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- BEST TIMES
        D.shot("times-message")
        D.press(K.A, 60)
        D.shot("times")
        D.press(K.A, 60)
        D.notices("results-later", 4000)
    end
    D.wait(120)
end)
