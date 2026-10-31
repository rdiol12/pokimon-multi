-- Daily quests: the Petalburg quest giver asks for three catches; TEST1
-- takes it, catches three on Route 101, gets the "quest done" notice and
-- turns it in. Also the Slateport and Fortree givers' requests and
-- R > ADVENTURE > DAILY QUESTS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function adventureMenu(index)
    D.press(K.R, 40)
    for _ = 1, 7 do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
    for _ = 1, index do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
end

-- Talks to the person above and screenshots each message page.
local function talk(name, pages)
    D.talk(K.UP, name, pages)
end

local function catchOne()
    D.findBattle(K.LEFT, K.RIGHT)
    D.wait(300)
    for _ = 1, 4 do
        if not D.inBattle() then break end
        D.press(K.R, 30)
        D.press(K.A, 600)
    end
    D.waitFor("the overworld", 3000, D.inOverworld, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    D.wait(40)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(10, MAPS.MAP_PETALBURG_CITY, 23, 18, { give = "MAGIKARP", level = 2, item = "ULTRA_BALL" })
    talk("petalburg-offer", 3)
    D.press(K.A, 60) -- YES
    D.shot("petalburg-accepted")
    D.press(K.A, 40)
    D.press(K.A, 40)
    adventureMenu(2)
    D.wait(20)
    D.shot("quest-list")
    D.press(K.B, 40)

    D.scene(10, MAPS.MAP_ROUTE101, 4, 4)
    for n = 1, 3 do
        catchOne()
        if n == 3 then
            D.wait(60)
            D.shot("quest-done-notice")
        end
    end
    D.wait(200)
    adventureMenu(2)
    D.wait(20)
    D.shot("quest-list-ready")
    D.press(K.B, 40)

    D.scene(10, MAPS.MAP_PETALBURG_CITY, 23, 18)
    talk("petalburg-reward", 3)
    D.wait(200)
    D.press(K.A, 60)
    D.press(K.B, 60)
    adventureMenu(2)
    D.wait(20)
    D.shot("quest-list-done")
    D.press(K.B, 40)

    D.scene(10, MAPS.MAP_SLATEPORT_CITY, 17, 21)
    talk("slateport-offer", 2)
    D.press(K.B, 60)
    D.press(K.A, 40)
    D.scene(10, MAPS.MAP_FORTREE_CITY, 3, 8)
    talk("fortree-offer", 2)
end)
