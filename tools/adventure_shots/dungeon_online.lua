-- The DEEPEST DIVERS board with two players online (run.py ... 2): TEST1
-- climbs down to B12F and TEST2 to B8F (each new floor gives trainer XP),
-- then TEST2 asks the spelunker for the board.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.adventure({ dungeonFloor = P1 and 11 or 7 })
    local x, y = D.dungeonLadder()
    D.scene(11, MAPS.MAP_ADVENTURE_DUNGEON, x, y + 1)
    D.wait(30)
    D.talk(K.UP, "ladder", 1)
    D.wait(20)
    D.press(K.A, 40) -- YES
    D.wait(200)
    D.shot(P1 and "b12f-online" or "b8f-online")
    D.wait(300)
    if not P1 then
        D.scene(11, MAPS.MAP_ROUTE114, 10, 65)
        D.wait(90)
        D.talk(K.UP, "spelunker-online", 1)
        D.wait(40)
        D.press(K.DOWN, 10)
        D.press(K.A, 180) -- DEEPEST DIVERS
        D.shot("board-online")
        D.press(K.B, 40)
    else
        D.wait(900)
    end
end)
