-- Mentors: TEST1 (eight badges) joins the mentor list; TEST2 (none) picks
-- TEST1; TEST1 cheers TEST2 on; TEST2 wins a badge and both collect gifts.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local function claimGifts(name)
    D.press(K.A, 250)
    D.shot(name)
    D.press(K.A, 60)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.scene(13, MAPS.MAP_LILYCOVE_CITY, P1 and 26 or 28, 16)
    if P1 then
        D.badges(8)
        D.openHub(7, "hub")
        D.wait(150)
        D.shot("mentor-message")
        D.press(K.A, 60)
        D.shot("apprentices-empty")
        D.press(K.A, 60)
        D.shot("mentor-menu")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- JOIN THE LIST
        D.shot("joined")
        D.press(K.A, 60)
        D.waitNotice("picked-notice", 20000)
        D.openHub(7)
        D.wait(150)
        D.press(K.A, 60)
        D.shot("apprentices")
        D.press(K.A, 60)
        D.press(K.A, 150) -- CHEER ON
        D.shot("cheer-who")
        D.press(K.A, 200) -- TEST2
        D.shot("cheered")
        D.press(K.A, 60)
        D.waitNotice("badge-notice", 20000)
        D.openHub(7)
        D.wait(150)
        D.press(K.A, 60)
        D.press(K.A, 60)
        D.press(K.DOWN, 8)
        D.shot("gifts-pick")
        claimGifts("gift")
    else
        D.wait(1800) -- TEST1 joins the list first
        D.openHub(7)
        D.wait(150)
        D.shot("new")
        D.press(K.A, 60)
        D.shot("find")
        D.press(K.A, 150) -- YES
        D.shot("mentor-list")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 250) -- TEST1
        D.shot("picked")
        D.press(K.A, 60)
        D.waitNotice("cheer-notice", 20000)
        D.badges(1)
        D.waitNotice("badge-notice", 20000)
        D.openHub(7)
        D.wait(150)
        D.shot("apprentice")
        claimGifts("gifts")
    end
    D.wait(60)
end)
