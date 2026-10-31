-- The Hall of Fame wall with two players (run.py ... 2): TEST1 enters the
-- Hall of Fame twice (a new team the second time), TEST2 once; then TEST2
-- asks the curator in the POKéMON LEAGUE and TEST1 opens R > ADVENTURE >
-- HALL OF FAME WALL.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local MAP = MAPS.MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F

-- Puts these Pokémon at the front of the party, first one leading.
local function team(mons)
    for i = #mons, 1, -1 do
        local m = mons[i]
        D.scene(14, MAP, P1 and 12 or 5, P1 and 5 or 4, { give = m[1], level = m[2], shiny = m[3], lead = true })
    end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if P1 then
        team({ { "SWAMPERT", 58 }, { "PELIPPER_STARFALL", 55 }, { "GARDEVOIR", 56 },
               { "AGGRON", 55 }, { "FLYGON", 57, true }, { "LUVDISC_STARFALL", 52 } })
        D.adventure({ hofEnter = true })
        D.wait(600)
        team({ { "RAYQUAZA", 70 } })
        D.adventure({ hofEnter = true })
        D.wait(600)
    else
        D.wait(300)
        team({ { "SCEPTILE", 61, true }, { "URSARING_AMBERWOOD", 58 }, { "SALAMENCE", 60 },
               { "MILOTIC", 59 }, { "METAGROSS", 62 }, { "WINGULL_STARFALL", 50 } })
        D.adventure({ hofEnter = true })
        D.wait(900)
    end
    D.wait(600)
    if P1 then
        D.shot("league")
        D.press(K.R, 40)
        for _ = 1, 7 do D.press(K.DOWN, 8) end
        D.press(K.A, 40)
        for _ = 1, 6 do D.press(K.DOWN, 8) end
        D.shot("menu")
        D.press(K.A, 240)
        D.shot("wall")
        D.press(K.DOWN, 30)
        D.shot("wall-2")
        D.press(K.DOWN, 30)
        D.shot("wall-3")
        D.press(K.B, 60)
        D.wait(600)
        return
    end
    D.wait(1200)
    D.shot("league")
    D.talk(K.RIGHT, "curator", 3)
    D.wait(240)
    D.shot("wall")
    D.press(K.DOWN, 30)
    D.shot("wall-2")
    D.press(K.DOWN, 30)
    D.shot("wall-3")
    D.press(K.B, 60)
    D.shot("closed")
end)
