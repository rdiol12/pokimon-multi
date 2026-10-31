-- The guide: R menu, the list, the map with markers, and the notice when a
-- badge opens something new. Run with tools/areas/shots/run.py.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.flag("FLAG_BADGE01_GET", true)
    D.flag("FLAG_BADGE02_GET", true)
    D.flag("FLAG_BADGE03_GET", true)
    D.flag("FLAG_ADVENTURE_CRAFTING", true)
    D.scene(12, MAPS.MAP_PETALBURG_CITY, 15, 15)
    D.wait(200)
    D.shot("00-new-in-guide-at-start")
    D.press(K.A, 60)
    D.wait(200)
    D.press(K.R, 40); D.shot("01-r-menu")
    D.press(K.A, 40); D.shot("02-guide-menu")
    D.press(K.A, 60); D.shot("03-list")
    for i = 1, 3 do D.press(K.DOWN, 10) end
    D.wait(20); D.shot("04-list-detail")
    for i = 1, 14 do D.press(K.DOWN, 8) end
    D.wait(20); D.shot("05-list-locked")
    for i = 1, 17 do D.press(K.UP, 8) end
    D.press(K.A, 120); D.shot("06-map")
    D.press(K.A, 30); D.shot("07-map-next")
    for i = 1, 6 do D.press(K.RIGHT, 12) end
    D.wait(30); D.shot("08-map-moved")
    D.press(K.B, 120); D.shot("09-back-to-list")
    D.press(K.B, 60)
    D.wait(60)
    -- A badge opens Ember Springs and Cinder Caverns.
    D.flag("FLAG_BADGE04_GET", true)
    D.wait(150)
    D.shot("10-new-in-guide")
    D.press(K.A, 60)
    D.wait(400)
    for i = 1, 6 do D.press(K.B, 30) end
    D.wait(100)
    D.press(K.R, 40); D.press(K.A, 40); D.press(K.A, 60)
    D.shot("11-list-after-badge")
    D.press(K.A, 120)
    D.shot("12-map-after-badge")
    D.press(K.B, 120)
    D.result("ok")
end)
