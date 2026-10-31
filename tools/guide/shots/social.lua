-- Social: TEST1 (no badges) is paired with a mentor on joining; TEST2 (seven
-- badges) hears it can mentor, finds TEST1 in PLAYERS NEARBY and adds them;
-- TEST1 invites friends and TEST2 hears of it; TEST2 walks near TEST1.
-- Run: tools/online_shots/run.py tools/guide/shots/social.lua OUT 30000 2 tools/guide/shots/social_seed.py
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- R > GUIDE, then row `row`.
local function guide(row, shotName)
    D.waitFor("the field", 1200, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    for _ = 1, 6 do
        D.press(K.R, 40)
        if D.fieldLocked() then break end
        D.wait(60)
    end
    D.press(K.A, 40)
    if shotName then D.shot(shotName) end
    for _ = 1, row do D.press(K.DOWN, 8) end
    D.press(K.A, 30)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    if P1 then
        D.scene(12, MAPS.MAP_PETALBURG_CITY, 15, 15)
        D.badges(0)
        D.notices("p1-joined", 900)        -- the mentor notice
        D.wait(1400)                       -- TEST2 adds TEST1
        D.notices("p1-friend", 600)
        guide(3, "p1-guide-menu")          -- INVITE FRIENDS
        D.wait(60)
        D.shot("p1-invite-ask")
        D.press(K.A, 200)                  -- YES
        D.shot("p1-invited")
        D.press(K.A, 60)
        D.press(K.A, 60)
        D.notices("p1-nearby", 2400)       -- TEST2 walks up
    else
        D.scene(12, MAPS.MAP_LILYCOVE_CITY, 26, 16)
        D.badges(7)
        D.notices("p2-can-mentor", 900)
        guide(2)                           -- PLAYERS NEARBY
        D.wait(200)
        D.shot("p2-nearby")
        D.press(K.A, 60)
        D.wait(40)
        D.shot("p2-nearby-list")
        D.press(K.A, 200)                  -- TEST1
        D.shot("p2-added")
        D.press(K.A, 60)
        D.press(K.A, 60)
        D.notices("p2-invite", 1600)       -- TEST1's invitation
        D.scene(12, MAPS.MAP_ROUTE104, 12, 70)
        D.wait(1600)
    end
    D.wait(60)
end)
