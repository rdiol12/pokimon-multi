-- Treasure hunts: the old treasure hunter in Dewford hands out today's map
-- (a picture of the spot and a riddle); R > ADVENTURE > TREASURE MAP shows
-- it again; pressing A on the X in Oldale digs up the treasure.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(11, MAPS.MAP_DEWFORD_TOWN, 4, 15)
    D.wait(60)
    D.shot("dewford")
    D.talk(K.UP, "hunter", 9)
    D.wait(60)
    -- Today's map is Oldale's, for the dig below.
    D.adventure({ treasure = 0 })
    D.press(K.R, 40)
    for _ = 1, 7 do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
    for _ = 1, 4 do D.press(K.DOWN, 8) end
    D.press(K.A, 90)
    D.shot("treasure-map")
    D.press(K.A, 60)
    D.scene(11, MAPS.MAP_OLDALE_TOWN, 10, 11)
    D.wait(60)
    D.shot("oldale")
    D.talk(K.UP, "dig", 5)
    D.wait(60)
    D.scene(11, MAPS.MAP_DEWFORD_TOWN, 4, 15)
    D.wait(60)
    D.talk(K.UP, "hunter-done", 2)
    D.press(K.B, 40) -- NO

end)
