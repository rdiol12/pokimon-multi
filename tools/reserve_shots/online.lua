-- Haven Reserve, two players online: TEST1 goes in through the Verdanturf
-- guard and leaves a Zigzagoon with the warden; it runs off into the
-- meadow. TEST2 comes in later, finds it roaming, battles it and catches
-- it, then knocks out a stray. TEST1 reads the notice board.
--   python3 tools/reserve_shots/run.py tools/reserve_shots/online.lua OUT 60000 2
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local function enterThroughGate(prefix)
    D.scene(12, MAPS.MAP_VERDANTURF_TOWN, 14, 10)
    D.talk(K.UP, prefix .. "gate-guard", 4) -- the last A is YES
    D.wait(50)
    D.shot(prefix .. "gate-guard-check")
    D.press(K.A, 10)
    D.wait(400)
    D.waitFor("the reserve", 1200, D.inOverworld)
    D.wait(150)
    D.shot(prefix .. "arrive")
end

local function battleEnds()
    D.waitFor("the battle to end", 6000, function() return D.inOverworld() end, function(i)
        if i % 60 == 0 then D.press(K.R, 0) -- throws the Master Ball at the menu
        elseif i % 60 == 30 then D.press(K.B, 0) end
    end)
end

D.run(function()
    D.boot()
    D.summer()
    D.waitFor("the server", 600, D.online)
    if P1 then
        D.showcase({ level = 30, badges = 4, party = { "SCEPTILE", "ZIGZAGOON", "MARILL" } })
        enterThroughGate("")
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 17, 24)
        D.shot("gate-yard")
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 13, 25)
        D.talk(K.UP, "warden", 4) -- YES
        D.wait(40)
        D.shot("warden-which")
        D.press(K.A, 90)
        D.press(K.DOWN, 20)
        D.shot("party")
        D.press(K.A, 60)
        D.shot("warden-sure-1")
        D.press(K.A, 60)
        D.shot("warden-sure-2")
        D.press(K.A, 10) -- YES
        D.wait(150)
        D.shot("warden-left-1")
        D.press(K.A, 60)
        D.shot("warden-left-2")
        D.press(K.A, 40)
        D.wait(90)
        D.shot("zigzagoon-runs-off")
        -- Wait for TEST2's visit, then read the board.
        D.wait(9000)
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 21, 24)
        D.talk(K.UP, "board-intro")
        D.wait(120)
        D.shot("board-news")
        D.press(K.B, 60)
    else
        D.showcase({ level = 50, badges = 4, party = { "BLAZIKEN" } })
        D.wait(3300)
        D.scene(12, MAPS.MAP_VERDANTURF_TOWN, 14, 10, { item = "MASTER_BALL" })
        enterThroughGate("p2-")
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 11, 8)
        D.wait(60)
        D.shot("p2-meadow")
        -- The shared ones come first: TEST1's Zigzagoon is object 1.
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 6, 9)
        D.approach(1)
        D.wait(130)
        D.shot("p2-zigzagoon")
        D.press(K.A, 110)
        D.shot("p2-challenge")
        D.waitFor("the battle", 1200, D.inBattle, function(i)
            if i % 60 == 0 then D.press(K.A, 0) end -- YES
        end)
        D.wait(600)
        D.press(K.B, 300)
        D.shot("p2-battle")
        D.press(K.R, 330)
        D.shot("p2-caught")
        battleEnds()
        D.wait(140)
        D.shot("p2-caught-after")
        D.press(K.A, 60)
        -- A stray (object 2, starts at 12,4): knocked out, it leaves.
        D.scene(12, MAPS.MAP_HAVEN_RESERVE, 12, 6)
        D.approach(2)
        D.wait(130)
        D.shot("p2-stray")
        D.press(K.A, 60)
        D.press(K.A, 10) -- YES
        D.winBattle("p2-stray-battle")
        D.wait(140)
        D.shot("p2-stray-gone")
        D.press(K.A, 60)
    end
end)
