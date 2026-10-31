-- Mt. Frostcrown: the ranger in Lilycove (badge check), the base camp,
-- the trail shelves and the snowy pines, a stranded climber, Snowdrift
-- Village, the Mountain Rescue chief and a ride down the sled run.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local T, V = MAPS.MAP_MT_FROSTCROWN, MAPS.MAP_SNOWDRIFT_VILLAGE

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function view(map, x, y, name)
    D.scene(12, map, x, y)
    D.wait(300)
    D.shot(name)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 20, 17, { give = "LUCARIO", level = 50, lead = true })
    D.wait(240)
    D.talk(K.UP, "01-ranger-needs-badges", 3)
    close(1)
    D.flag("FLAG_BADGE06_GET", true)
    D.talk(K.UP, "02-ranger", 3)
    for _ = 1, 6 do D.press(K.A, 60) end
    D.wait(200)
    D.wait(400)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(300)
    D.shot("03-arrival")
    view(T, 22, 81, "03-base-camp")
    view(T, 14, 69, "04-first-shelf")
    view(T, 20, 54, "05-second-shelf")
    view(T, 22, 34, "06-snowy-pines")
    D.scene(12, T, 5, 32)
    D.wait(300)
    D.talk(K.UP, "07-stranded-climber", 2)
    D.press(K.A, 120)
    D.wait(120)
    D.shot("08-climber-heads-down")
    close(1)
    view(T, 22, 16, "09-top-shelf")
    view(V, 22, 22, "10-snowdrift-village")
    view(V, 26, 10, "11-buried-summit-path")
    D.scene(12, MAPS.MAP_SNOWDRIFT_VILLAGE_LODGE, 6, 4)
    D.wait(200)
    D.talk(K.UP, "12-rescue-chief", 4)
    close(3)
    view(V, 43, 17, "13-sled-run-top")
    D.hold(K.DOWN, 20)
    D.wait(40)
    D.shot("14-sledding")
    D.wait(120)
    D.shot("15-sledding-down")
    D.wait(1100)
    D.shot("16-sled-run-bottom")
    D.result("ok")
end)
