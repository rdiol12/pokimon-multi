-- The TREASURE HUNTERS board with two players online (run.py ... 2): both
-- dig up today's treasure in Oldale, one after the other, then TEST2 asks the treasure hunter
-- in Dewford for the board.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    -- TEST1 digs first and leaves, then TEST2 takes its turn.
    if not P1 then D.wait(900) end
    D.scene(11, MAPS.MAP_OLDALE_TOWN, 10, 11)
    -- After the scene: setting the clock can turn the day over.
    D.adventure({ treasure = 0 })
    D.wait(60)
    D.talk(K.UP, "dig", 6)
    D.wait(120)
    if P1 then D.scene(11, MAPS.MAP_PETALBURG_CITY, 16, 14) end
    D.wait(180)
    if not P1 then
        D.scene(11, MAPS.MAP_DEWFORD_TOWN, 4, 15)
        D.wait(90)
        D.talk(K.UP, "hunter-done", 2)
        D.wait(180)
        D.shot("board-online")
        D.press(K.B, 40)
    else
        D.wait(900)
    end
end)
