-- Living ecosystem with two players. Others have caught seven of one species
-- on Route 102 (seed); TEST1 catches the eighth, and the rest flee to a
-- nearby route. TEST1 reads HUB > WILDLIFE REPORT there and then where
-- they fled, gets the arrival notice, and meets one in the grass.
dofile(OUT_DIR .. "/lib.lua")
dofile(OUT_DIR .. "/eco_target.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    if not P1 then
        D.wait(12000)
        return
    end
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "SCEPTILE", level = 60, lead = true, item = "MASTER_BALL" })
    D.wildBattle(ECO.species, 4)
    D.catchThis()
    D.notices("flee-notice", 1200)
    D.openHub(15)
    D.waitAnswer()
    D.shot("report-route102")
    D.press(K.A, 30)
    D.waitAnswer()
    D.shot("migrations")
    D.press(K.A, 60)
    D.scene(12, ECO.map, ECO.x, ECO.y)
    D.waitNotice("arrival-notice", 1500)
    D.wait(60)
    D.openHub(15)
    D.waitAnswer()
    D.shot("report-there")
    D.press(K.A, 30)
    D.waitAnswer()
    D.press(K.A, 60)
    D.wait(60)
    -- The weather can swap in its own species (snow: Swinub); run from those.
    for _ = 1, 8 do
        local species = D.findBattle(K.LEFT, K.RIGHT)
        if species == SPECIES_ID[ECO.species] then break end
        D.runAway()
        D.wait(60)
    end
    D.wait(250)
    D.shot("migrant-battle")
    for _ = 1, 4 do
        if not D.inBattle() then break end
        D.press(K.R, 30)
        D.press(K.A, 600)
    end
    D.waitFor("the overworld", 3000, D.inOverworld, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    for i = 1, 6 do
        D.wait(100)
        D.shot("after-" .. i)
    end
end)
