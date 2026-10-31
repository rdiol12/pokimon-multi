-- Verdant River: the canoe guide on Route 119 (badge check), the stilt
-- village, the elder's story, the gorge full of rapids, pulling the sluice
-- lever on the lookout, surfing up the calm gorge, the whirl round the
-- temple island sweeping you out west, the beach lever turning it, riding
-- in to the island dock, the temple and Keldeo.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local R = MAPS.MAP_VERDANT_RIVER

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function at(map, x, y)
    D.scene(12, map, x, y)
    D.wait(300)
end

local function walk(dir, n)
    for _ = 1, n do D.hold(dir, 16) end
    D.wait(30)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_ROUTE119, 13, 132, { give = "LUCARIO", level = 45, lead = true, surf = true })
    D.wait(300)
    D.talk(K.LEFT, "01-guide-needs-badges", 2)
    close(2)
    D.wait(60)
    D.flag("FLAG_BADGE05_GET", true)
    D.talk(K.LEFT, "02-guide", 3)
    for _ = 1, 4 do D.press(K.A, 60) end
    D.wait(400)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(200)
    D.shot("03-village-arrive")
    at(R, 10, 49)
    D.talk(K.UP, "04-elder", 6)
    close(1)
    at(R, 28, 45)
    D.shot("05-gorge-rapids")
    at(R, 15, 31)
    D.talk(K.UP, "06-sluice-lever", 3)
    D.wait(200)
    D.shot("07-shake")
    D.wait(120)
    D.shot("08-gorge-calms")
    close(2)
    at(R, 28, 40)
    walk(K.UP, 3)
    D.shot("09-surfing-up-the-gorge")
    at(R, 30, 28)
    D.shot("10-the-whirl")
    walk(K.UP, 3)
    D.shot("11-swept-away")
    D.wait(200)
    D.shot("12-thrown-out-west")
    at(R, 8, 17)
    D.talk(K.UP, "13-beach-lever", 3)
    D.wait(320)
    D.shot("14-whirl-turns")
    close(2)
    at(R, 28, 27)
    walk(K.UP, 3)
    D.wait(120)
    D.shot("15-into-the-dock")
    walk(K.UP, 2)
    D.shot("16-temple-island")
    walk(K.UP, 6)
    D.wait(300)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(120)
    D.shot("17-temple")
    walk(K.UP, 9)
    walk(K.RIGHT, 1)
    walk(K.UP, 1)
    D.shot("18-keldeo")
    D.talk(K.UP, "19-keldeo", 2)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("20-keldeo-battle")
    D.result("ok")
end)
