-- The website's live map and shiny gallery with two players: TEST1 catches
-- a shiny Absol on Route 120, TEST2 (in Oldale Town) gets the notice. The
-- seed's browser then screenshots /map and /shinies while both are online;
-- it writes web_done when it has them.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    if P1 then
        D.scene(12, MAPS.MAP_ROUTE120, 20, 30, { item = "MASTER_BALL" })
        D.wildBattle("ABSOL", 30, true)
        D.waitFor("the battle", 1200, D.inBattle)
        D.wait(420)
        D.shot("shiny-absol")
        D.catchThis()
    else
        D.waitNotice("shiny-notice", 30000)
    end
    D.waitFor("the web screenshots", 100000, function()
        local f = io.open(OUT_DIR .. "/web_done", "r")
        if f then f:close() return true end
        return false
    end)
end)
