-- Legend sightings with two players. Others (seed) met Raikou and Latias
-- earlier; TEST1 runs into a roaming Latios on Route 117, TEST2 gets the
-- notice in Oldale Town and opens HUB > LEGEND SIGHTINGS.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    if P1 then
        D.scene(12, MAPS.MAP_ROUTE117, 30, 10)
        D.roamer("LATIOS", 5)
        D.waitFor("the battle", 1200, D.inBattle)
        D.wait(400)
        D.shot("latios")
        D.runAway()
        D.wait(400)
    else
        D.waitNotice("sighting-notice", 20000)
    end
    D.openHub(1)
    D.waitAnswer()
    D.shot("sightings-board")
    D.press(K.A, 40)
end)
