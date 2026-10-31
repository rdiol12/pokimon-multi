-- Online: Laurel City, the Rover League's city (tools/areas/laurel.py), on
-- a Monday (run with ROVERS_TIME_OFFSET, see docs/rovers.md). Player 1
-- takes the league ferry from Slateport, looks around the city with league
-- rovers in it, enters the league at the stadium's desk, visits the nurse,
-- the Hall of Champions, the Rovers' Lodge and the League Shop. Player 2
-- only keeps the server company.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.waitFor("the server", 600, D.online)
    if INSTANCE == 2 then
        D.waitFor("player 1", 90000, function()
            local f = io.open(OUT_DIR .. "/done", "r")
            if f then f:close() return true end
            return false
        end)
        D.result("ok")
        return
    end
    os.execute(string.format('python3 "%s/seed_hall.py" "%s/league.sqlite3"', ROVERS_SHOTS, OUT_DIR))
    -- The league ferry in Slateport.
    D.scene(12, MAPS.MAP_SLATEPORT_CITY, 33, 14, { give = "SWAMPERT", level = 70, lead = true })
    D.hold(K.RIGHT, 2)
    D.wait(10)
    D.press(K.A, 30)
    D.answer(true, "ferry")
    D.press(K.A, 30)
    D.wait(300)
    D.waitFor("Laurel City", 1200, function() return D.inOverworld() end)
    D.wait(120)
    D.shot("arrival")
    -- League rovers around town.
    D.scene(12, MAPS.MAP_LAUREL_CITY, 25, 30)
    D.rovers(1, {0, 3, 7, 21}, 40, 3, true, 0)
    D.wait(150)
    D.shot("avenue")
    D.scene(12, MAPS.MAP_LAUREL_CITY, 23, 20)
    D.wait(60)
    D.shot("stadium")
    D.scene(12, MAPS.MAP_LAUREL_CITY, 8, 40)
    D.wait(60)
    D.shot("hall-lodge")
    D.scene(12, MAPS.MAP_LAUREL_CITY, 36, 40)
    D.wait(60)
    D.shot("shop")
    -- The stadium: the league desk.
    D.scene(12, MAPS.MAP_LAUREL_CITY_STADIUM, 11, 14)
    D.wait(60)
    D.shot("lobby")
    D.scene(12, MAPS.MAP_LAUREL_CITY_STADIUM, 5, 11)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 30)
    D.answer(true, "desk")       -- enter
    D.answer(false, "entered")   -- not now
    D.answer(false, "watch")     -- no watching
    D.wait(60)
    D.shot("desk-bye")
    D.press(K.A, 60)
    D.scene(12, MAPS.MAP_LAUREL_CITY_STADIUM, 17, 11)
    D.talk(K.UP, "nurse", 3)
    D.press(K.A, 60)
    -- The Hall of Champions.
    D.scene(12, MAPS.MAP_LAUREL_CITY_HALL_OF_CHAMPIONS, 8, 5)
    D.talk(K.UP, "hall", 4)
    D.press(K.A, 60)
    D.scene(12, MAPS.MAP_LAUREL_CITY_ROVERS_LODGE, 4, 7)
    D.wait(60)
    D.shot("lodge")
    D.scene(12, MAPS.MAP_LAUREL_CITY_LEAGUE_SHOP, 2, 3)
    D.talk(K.LEFT, "shop-clerk", 2)
    D.wait(60)
    D.shot("shop-menu")
    io.open(OUT_DIR .. "/done", "w"):close()
    D.result("ok")
end)
