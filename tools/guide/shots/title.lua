-- The title menu with JOIN AND INVITE FRIENDS: save, reset, open the menu.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.run(function()
    D.boot()
    D.wait(200)
    D.press(K.A, 60)
    D.press(K.START, 40)
    for i = 1, 5 do D.press(K.DOWN, 10) end
    D.press(K.A, 90)       -- SAVE
    D.press(K.A, 300)      -- yes
    D.press(K.A, 120)
    D.press(K.B, 60)
    D.hold(K.A + K.B + K.START + K.SELECT, 10)
    D.wait(200)
    for i = 1, 12 do D.press(K.START, 80) end  -- intro, title screen
    D.wait(100)
    D.shot("title-menu")
    for i = 1, 3 do D.press(K.DOWN, 20) end
    D.wait(30)
    D.shot("title-menu-invite")
    D.result("ok")
end)
