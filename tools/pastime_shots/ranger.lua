-- Ranger missions: TEST1 takes a RESCUE mission from the Fortree RANGER,
-- finds the lost SKITTY on Route 104, reports back (and is promoted), then
-- takes a CALM mission and wins the last of its three wild battles on
-- Route 119.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local POINTS, PROGRESS = 126, 124 -- struct PastimeSaveData offsets

-- Fights the current wild battle with the first move until it's over. The
-- wild Pokémon's HP is set to 1 first (found in gBattleMons[1] as hp, level,
-- friendship, maxHP), so a weak team still wins.
local function winBattle()
    local base = D.SYM.gBattleMons + D.SYM.BATTLEMON_SIZE
    D.waitFor("the wild Pokémon's HP", 600, function()
        for o = 0x20, 0x60, 2 do
            local hp, level = D.read16(base + o), D.read8(base + o + 2)
            if hp > 1 and hp == D.read16(base + o + 4) and level >= 1 and level <= 100 then
                D.write8(base + o, 1); D.write8(base + o + 1, 0)
                return true
            end
        end
        return false
    end)
    D.waitFor("the overworld", 6000, D.inOverworld, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    D.wait(30)
end

local function talkThrough(name, frames)
    for i = 1, frames // 40 do
        D.wait(40)
        if i % 2 == 0 then D.shot(name .. "-" .. (i // 2)) end
        if i % 2 == 0 then D.press(K.A, 0) end
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_FORTREE_CITY, 22, 14)
    D.pastime16(POINTS, 8)
    D.wait(30)
    D.shot("ranger")
    D.talk(K.RIGHT, "hello", 2)
    D.press(K.A, 90) -- yes
    D.shot("briefing")
    D.press(K.A, 90)
    D.shot("good-luck")
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(30)
    D.pastimeMenu(11)
    D.wait(30)
    D.shot("ranger-list")
    for _ = 1, 4 do D.press(K.DOWN, 8) end
    D.shot("ranger-list-mission")
    D.press(K.B, 60)
    D.scene(12, MAPS.MAP_ROUTE104, 23, 51)
    D.wait(30)
    D.shot("lost-skitty")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 0)
    talkThrough("rescue", 640)
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(60)
    D.shot("skitty-gone")
    D.scene(12, MAPS.MAP_FORTREE_CITY, 22, 14)
    D.hold(K.RIGHT, 2)
    D.wait(10)
    D.press(K.A, 0)
    talkThrough("report", 800)
    for _ = 1, 4 do D.press(K.B, 40) end
    -- The next mission: calm Route 119.
    D.wait(30)
    D.hold(K.RIGHT, 2)
    D.wait(10)
    D.press(K.A, 0)
    talkThrough("calm", 480)
    for _ = 1, 4 do D.press(K.B, 40) end
    -- Two wins are already in; the next one finishes the mission.
    D.pastime8(PROGRESS, 2)
    D.scene(12, MAPS.MAP_ROUTE119, 30, 115)
    for b = 1, 1 do
        D.findBattle(K.LEFT, K.RIGHT)
        D.shot("calm-battle-" .. b)
        winBattle()
        D.wait(40)
        D.shot("calm-after-" .. b)
        D.wait(100)
        D.shot("calm-toast-" .. b)
    end
    D.log("progress " .. D.pastimeRead8(PROGRESS))
end)
