-- Lost and found: Lucy in Petalburg lost her hair ribbon on Route 104.
-- The player hears about it, finds it, brings it back, and checks
-- R > PASTIMES > LOST & FOUND.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_PETALBURG_CITY, 22, 11)
    D.wait(120)
    D.shot("lucy")
    D.talk(K.UP, "lucy-plea", 2)
    D.wait(40)

    D.scene(12, MAPS.MAP_ROUTE104, 13, 45)
    D.wait(60)
    D.shot("ribbon-on-route")
    D.hold(K.UP, 2)
    D.wait(20)
    D.press(K.A, 10)
    D.wait(150)
    D.shot("found-ribbon")
    D.press(K.A, 40)
    D.shot("found-ribbon-2")
    D.press(K.A, 40)

    D.scene(12, MAPS.MAP_PETALBURG_CITY, 22, 11)
    D.wait(100)
    D.talk(K.UP, "lucy-thanks", 3)
    D.wait(160)
    D.shot("lucy-reward")
    for _ = 1, 4 do D.press(K.A, 40) end
    D.wait(60)
    D.talk(K.UP, "lucy-after", 1)
    D.press(K.B, 60)
    D.press(K.B, 60)

    -- Meet a few more owners and find some of their things, for the list.
    D.pastime8(2, 0x5F)  -- heard: all but Wren and Kai
    D.pastime8(0, D.pastimeRead8(0) | 0x12)  -- found: Evan's notebook and the glasses
    D.pastime8(1, D.pastimeRead8(1) | 0x02)  -- Evan has his notebook back
    D.wait(60)
    D.pastimeMenu(0)
    D.wait(30)
    D.shot("list")
    for _ = 1, 4 do D.press(K.DOWN, 10) end
    D.wait(10)
    D.shot("list-2")
    D.press(K.B, 40)
end)
