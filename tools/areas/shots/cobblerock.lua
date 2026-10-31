-- Cobblerock: the guide in Rustboro (badge check), the town and ore yard,
-- the mine's two floors and three ore veins, the assayer's reward, the
-- boulder moved off the old shaft, and the Carbink inside.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local T = MAPS.MAP_COBBLEROCK
local M1 = MAPS.MAP_COBBLEROCK_MINE_1F
local M2 = MAPS.MAP_COBBLEROCK_MINE_B1F

local function say(dir, name, pages)
    D.talk(dir, name, pages)
    D.wait(30)
end

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function view(map, x, y, name)
    D.scene(12, map, x, y)
    D.wait(90)
    D.shot(name)
end

local function chip(map, x, y, dir, name)
    D.scene(12, map, x, y)
    D.wait(60)
    D.talk(dir, name .. "-ask", 2)
    D.press(K.A, 90)        -- yes
    D.wait(60)
    D.shot(name .. "-chipped")
    close(1)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_RUSTBORO_CITY, 13, 41, { give = "GYARADOS", level = 60, lead = true })
    D.wait(240)
    say(K.LEFT, "01-guide-needs-badge", 2)
    close(1)
    D.flag("FLAG_BADGE01_GET", true)
    say(K.LEFT, "02-guide", 2)
    D.press(K.A, 40)
    D.press(K.A, 40)
    D.wait(200)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(150)
    D.shot("03-arrive")
    view(T, 20, 24, "04-ore-yard")
    view(T, 13, 12, "05-mine-entrance")
    D.scene(12, T, 13, 10); say(K.RIGHT, "06-foreman", 3); close(1)
    D.scene(12, T, 30, 11); say(K.UP, "07-boulder", 1); close(1)
    D.scene(12, T, 33, 21); say(K.UP, "08-old-miner", 3); close(1)
    view(M1, 20, 26, "09-mine-entrance")
    view(M1, 20, 14, "10-main-gallery")
    chip(M1, 4, 19, K.LEFT, "11-vein-1")
    view(M2, 22, 15, "12-deep-gallery")
    chip(M2, 9, 4, K.LEFT, "13-vein-2")
    chip(M2, 35, 27, K.RIGHT, "14-vein-3")
    D.scene(12, MAPS.MAP_COBBLEROCK_ASSAY_OFFICE, 6, 5)
    D.wait(90)
    D.shot("15-assay-office")
    D.talk(K.UP, "16-assayer", 4)
    D.wait(120)
    D.shot("17-got-rolycoly")
    D.press(K.A, 60)
    D.wait(200)
    D.shot("18-shaft-news")
    close(3)
    view(T, 30, 12, "19-boulder-moved")
    view(MAPS.MAP_COBBLEROCK_MINE_OLD_SHAFT, 12, 12, "20-old-shaft")
    D.scene(12, MAPS.MAP_COBBLEROCK_MINE_OLD_SHAFT, 13, 6); say(K.UP, "21-carbink", 2)
    D.waitFor("the battle", 1500, D.inBattle, function(i) if i % 40 == 0 then D.press(K.A, 0) end end)
    D.wait(300)
    D.shot("22-carbink-battle")
    D.result("ok")
end)
