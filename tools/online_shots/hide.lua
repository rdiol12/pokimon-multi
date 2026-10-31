-- Hide and seek: TEST1 starts a game in Petalburg Woods and seeks; TEST2 and
-- TEST3 join and hide. TEST1 counts, walks up to TEST2 (who only shows up
-- close by) and finds them; TEST3 stays hidden till the time is up.
-- The walks follow hide_path.lua (hide_seed.py plans them from the map).
-- TEST1 leads with Kirlia (Psychic: senses hiders), TEST2 with Gastly
-- (Ghost: blends in).
dofile(OUT_DIR .. "/lib.lua")
dofile(OUT_DIR .. "/hide_path.lua")
local D, K = Driver, Driver.KEY
local SEEKER = INSTANCE == 1
local WAITING, PLAYING, ENDED = 2, 3, 4 -- sHide.state

local function state() return D.read8(HIDE_STATE) end
local function countdown() return D.read16(HIDE_STATE + 4) end
-- The game's clock (sHide.timer) is down to the seeking part (seekFrames).
local function seeking() return D.read16(HIDE_STATE + 6) <= D.read16(HIDE_STATE + 8) end

-- Walks (running) to each corner in turn; each(x, y) may take pictures.
local function walk(path, each)
    for _, corner in ipairs(path) do
        local tx, ty = corner[1], corner[2]
        D.waitFor("the corner " .. tx .. "," .. ty, 3000, function()
            local x, y = D.pos()
            return x == tx and y == ty
        end, function()
            local x, y = D.pos()
            local key = x > tx and K.LEFT or x < tx and K.RIGHT or y > ty and K.UP or K.DOWN
            D.setKeys(key | K.B)
            if each then each(x, y) end
        end)
    end
    D.setKeys(0)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.wait(300) -- everyone connects
    local lead = ({"KIRLIA", "GASTLY"})[INSTANCE]
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 4 + INSTANCE * 2, 16, lead and {give = lead, level = 30, lead = true} or nil)
    if not SEEKER then D.wait(INSTANCE == 2 and 300 or 500) end
    D.openHub(14, SEEKER and "hub" or nil)
    D.wait(150)
    if SEEKER then D.shot("arenas") end
    D.press(K.A, 60)
    if SEEKER then D.shot("menu") end
    D.press(K.A, 150) -- PLAY
    D.shot("joined")
    D.press(K.A, 30)
    D.waitFor("the lobby", 1200, function() return state() == WAITING end, function(i)
        if i % 60 == 0 and state() == 0 then D.press(K.A, 0) end
    end)
    D.wait(150)
    D.shot("lobby")
    -- The countdown's over: the server says who hides and who seeks.
    D.waitFor("the start", 3000, function() return state() == WAITING and countdown() == 0 end)
    D.wait(150)
    D.shot("go")
    D.press(K.A, 30)
    D.waitFor("the game", 600, function() return state() == PLAYING end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    if SEEKER then
        D.wait(300)
        D.shot("counting")
        D.waitFor("the end of the count", 3000, seeking)
        D.wait(120)
        D.shot("ready")
        D.waitFor("the field", 600, function() return not D.fieldLocked() end, function(i)
            if i % 60 == 0 then D.press(K.A, 0) end
        end)
        local shot = false
        walk(HIDE_PATH[1], function(x, y)
            if y == 27 and x == 13 and not shot then
                shot = true
                D.shot("searching")
            end
        end)
        -- TEST2 blends in: only seen from the next tile.
        D.wait(90)
        D.shot("spotted")
        D.waitFor("seeking", 3000, seeking)
        D.press(K.LEFT, 20)
        D.press(K.A, 200)
        D.shot("found")
        D.press(K.A, 60)
    else
        D.wait(60)
        D.shot("hiding")
        walk(HIDE_PATH[INSTANCE])
        D.shot("hidden")
        for n = 1, 4 do
            D.wait(900)
            if state() ~= PLAYING then break end
            D.shot("waiting-" .. n)
        end
    end
    D.waitFor("the end", 12000, function() return D.fieldLocked() and state() == ENDED end)
    D.wait(120)
    D.shot("times-up")
    D.press(K.A, 240)
    D.shot("result")
    D.waitFor("the way back", 900, function() return state() == 0 and D.inOverworld() and not D.fieldLocked() end,
        function(i) if i % 60 == 0 and state() ~= 0 then D.press(K.A, 0) end end)
    D.wait(60)
    if SEEKER then
        D.openHub(14)
        D.wait(150)
        D.press(K.A, 60)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- POINTS BOARD
        D.press(K.A, 60)
        D.shot("points")
        D.press(K.A, 60)
    end
    D.wait(120)
end)
