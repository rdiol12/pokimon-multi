-- Whisperstem Valley: the guide in Petalburg, the glades, the shrine gate,
-- the teahouse and its runaway Pancham, the koi pond bridge, the parted
-- bamboo and the secret grove's Mienfoo.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local WV = MAPS.MAP_WHISPERSTEM_VALLEY
local TEA = MAPS.MAP_WHISPERSTEM_VALLEY_TEAHOUSE

local function say(dir, name, pages)
    D.talk(dir, name, pages)
    D.wait(30)
end

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function view(x, y, name)
    D.scene(12, WV, x, y)
    D.wait(90)
    D.shot(name)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_PETALBURG_CITY, 7, 12, { give = "GYARADOS", level = 60, lead = true })
    D.wait(240)
    say(K.LEFT, "01-petalburg-guide", 2)
    D.press(K.A, 40)
    D.press(K.A, 40)
    D.wait(200)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(150)
    D.shot("02-arrive")
    view(28, 38, "03-shrine-gate")
    D.scene(12, WV, 26, 37); say(K.UP, "04-sign", 2); close(1)
    view(28, 27, "05-crossroads")
    view(27, 13, "06-teahouse-glade")
    D.scene(12, WV, 32, 12); say(K.RIGHT, "07-old-man-hint", 3); close(1)
    view(8, 8, "08-bamboo-maze")
    D.scene(12, WV, 3, 4); say(K.UP, "09-pancham", 2); close(1)
    D.wait(120)
    D.shot("10-pancham-ran-home")
    close(1)
    view(45, 29, "11-koi-pond")
    view(46, 20, "12-bridge")
    view(47, 12, "13-north-bank")
    D.scene(12, WV, 46, 10)
    D.wait(60)
    D.hold(K.UP, 16)
    D.wait(20)
    D.shot("14-slipping-through")
    D.hold(K.UP, 16)
    D.wait(20)
    D.hold(K.UP, 16)
    D.wait(60)
    D.shot("15-secret-grove")
    D.scene(12, WV, 47, 5); say(K.UP, "16-mienfoo", 2)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("17-mienfoo-battle")
    D.result("ok")
end)
