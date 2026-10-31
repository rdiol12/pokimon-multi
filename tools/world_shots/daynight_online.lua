-- Day and night online (run.py ... 2): both players set their own clocks
-- to different hours (9 am and 3 pm), join, and meet in Petalburg. Online
-- the server's clock wins, so both see the same sky. Start the server with
-- NETPLAY_CLOCK_UTC_OFFSET set so that it is night there.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    local wld = Driver.SYM.gWorldMailbox
    D.waitFor("the server's clock", 1200, function() return D.read8(wld + 0x195) == 1 end)
    local minute = D.read16(wld + 0x196)
    D.log(string.format("server clock %02d:%02d", minute // 60, minute % 60))
    D.scene(P1 and 9 or 15, MAPS.MAP_PETALBURG_CITY, P1 and 7 or 9, 8)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(700) -- let the news banner go
    D.shot(P1 and "p1-own-clock-9am" or "p2-own-clock-3pm")
    D.result("ok")
end)
