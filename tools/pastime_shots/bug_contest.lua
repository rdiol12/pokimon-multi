-- Bug-catching contest, with two players online (run.py ... 2): TEST1 enters
-- at the Route 119 officer, catches Bugs in the grass with Sport Balls, runs
-- the clock down, and is judged. Then R > PASTIMES > BUG CONTEST and the
-- online board. TEST2 waits.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- struct BugContest (src/pastime_bug_contest.c)
local function contest16(off) return D.read16(D.SYM.gBugContest + off) end
local function setTicks(v)
    D.write8(D.SYM.gBugContest + 2, v & 0xFF); D.write8(D.SYM.gBugContest + 3, v >> 8)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if not P1 then
        D.scene(12, MAPS.MAP_ROUTE119, 26, 112)
        D.wait(12000)
        return
    end
    D.scene(12, MAPS.MAP_ROUTE119, 29, 112, { give = "BLAZIKEN", level = 40, lead = true })
    D.wait(30)
    D.shot("officer")
    D.talk(K.UP, "welcome", 3)
    D.waitFor("the contest", 600, function() return D.read8(D.SYM.gBugContest) == 1 end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    D.wait(200)
    D.shot("balls")
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(8)
    D.wait(20)
    D.press(K.A, 40)
    D.wait(20)
    D.shot("clock")
    D.press(K.B, 60)
    -- Catch Bugs until one scores.
    D.scene(12, MAPS.MAP_ROUTE119, 30, 115)
    for b = 1, 5 do
        if contest16(6) ~= 0 then break end
        local sp = D.findBattle(K.LEFT, K.RIGHT)
        D.log("wild " .. (SPECIES_NAME[sp] or sp))
        D.wait(300)
        D.shot("wild-" .. b)
        -- R throws the last Ball used (the Sport Ball); B backs out of a
        -- menu if a press landed in one.
        for _ = 1, 8 do
            for _, key in ipairs({K.B, K.R, K.A}) do
                if D.inBattle() then D.press(key, key == K.A and 500 or 30) end
            end
        end
        if D.inBattle() then D.runAway() end
        D.waitFor("the overworld", 3000, D.inOverworld, function(i)
            if i % 30 == 0 then D.press(K.B, 0) end
        end)
        D.wait(40)
        D.shot("after-" .. b)
        -- The catch's banners (an achievement, then the contest's) take turns.
        for i = 1, 6 do D.wait(60); D.shot("banners-" .. b .. "-" .. i) end
    end
    -- Run the clock down.
    setTicks(8)
    D.wait(100)
    D.shot("times-up")
    D.wait(60)
    D.scene(12, MAPS.MAP_ROUTE119, 29, 112)
    D.talk(K.UP, "judging", 1)
    for i = 1, 16 do
        D.wait(60)
        D.shot("results-" .. i)
        if i % 2 == 0 then D.press(K.B, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(8)
    D.wait(20)
    D.press(K.A, 40)
    D.wait(20)
    D.shot("records")
    D.press(K.B, 60)
    D.wait(30)
    D.pastimeMenu(8)
    D.wait(20)
    D.press(K.DOWN, 10)
    D.press(K.A, 40)
    D.wait(120)
    D.shot("bug-board")
    D.press(K.B, 40)
end)
