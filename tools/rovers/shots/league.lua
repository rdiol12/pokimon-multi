-- Online: the weekly Rover League on Oldale's quest board, on a Monday
-- (run with ROVERS_TIME_OFFSET set, see docs/rovers.md). Player 1 looks at
-- the bracket, enters, plays its quarterfinal, then watches a rover match
-- (what it sees is the result). Player 2 only keeps the server company.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function battle(name)
    D.waitFor("the battle", 1500, D.inBattle, function(i)
        if i % 40 == 0 then D.shot(name .. "-start"); D.press(K.A, 0) end
    end)
    local n = 0
    D.waitFor("the overworld", 30000, D.inOverworld, function(i)
        if i % 120 == 0 and n < 40 then n = n + 1; D.shot(name .. "-" .. n) end
        if i % 50 == 0 then D.press(K.A, 0) end
    end)
    D.wait(150)
    D.shot(name .. "-after")
    D.press(K.A, 60)
end

local function board()
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 30)
end

D.run(function()
    D.boot()
    D.summer()
    D.waitFor("the server", 600, D.online)
    if INSTANCE == 2 then
        D.waitFor("player 1", 60000, function()
            local f = io.open(OUT_DIR .. "/done", "r")
            if f then f:close() return true end
            return false
        end)
        D.result("ok")
        return
    end
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 4, 18, { give = "SWAMPERT", level = 70, lead = true })
    D.rovers(1, {}, 20, 1, true, 0) -- no rovers here, just the board
    D.wait(120)
    board()
    D.answer(true, "jobs")       -- the rankings
    D.answer(true, "rankings")   -- the league
    D.answer(true, "bracket")    -- enter
    D.answer(true, "entered")    -- play the match now
    battle("match")
    D.shot("match-result")
    D.press(K.A, 60)
    D.wait(60)
    board()
    D.answer(true, "jobs2")
    D.answer(true, "rankings2")
    D.answer(true, "bracket2")   -- watch a match
    battle("watch")
    D.shot("watched")
    D.press(K.A, 60)
    io.open(OUT_DIR .. "/done", "w"):close()
    D.result("ok")
end)
