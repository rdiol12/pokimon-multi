-- Trainer levels with two players online (run.py ... 2): TEST1 reaches
-- level 8, opens R > ADVENTURE > TRAINER LEVEL and changes into the new
-- Starlight outfit; TEST2 plays MAY at level 30 in the Champion outfit.
-- Each sees the other's clothes.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- XP needed to reach a level (src/trainer_level.c: 40 + 20 * level each).
local function xpFor(level)
    local xp = 0
    for l = 1, level - 1 do xp = xp + 40 + 20 * l end
    return xp
end

-- R menu: RIDE, ACHIEVEMENTS, RECORDS, QUESTS, LEADERBOARDS, SAFARI HUNT,
-- PHOTO, ADVENTURE, CLOSE; ADVENTURE: TRAINER LEVEL, OUTFITS, CLOSE.
local function adventureMenu(index)
    D.press(K.R, 40)
    for _ = 1, 7 do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
    for _ = 1, index do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if P1 then
        D.scene(12, MAPS.MAP_OLDALE_TOWN, 6, 10)
    else
        local sb2 = D.read32(D.SYM.gSaveBlock2Ptr)
        D.write8(sb2 + 0x90, 2) -- START > STYLE: MAY
        D.scene(12, MAPS.MAP_OLDALE_TOWN, 9, 10)
    end
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(60)

    if P1 then
        D.wait(120) -- the town's name sign goes away
        D.adventure({ xp = xpFor(8) })
        D.wait(70)
        D.shot("level-up")
        D.wait(240)
        adventureMenu(0)
        D.wait(30)
        D.shot("trainer-level")
        for _ = 1, 8 do D.press(K.DOWN, 6) end
        D.wait(20)
        D.shot("trainer-level-perk")
        D.press(K.B, 40)
        adventureMenu(1)
        D.wait(20)
        D.shot("outfit-menu")
        D.press(K.DOWN, 20)
        D.shot("outfit-autumn")
        D.press(K.DOWN, 20)
        D.shot("outfit-starlight")
        D.press(K.A, 40)
        D.hold(K.DOWN, 2)
        D.wait(200)
        D.shot("both-outfits")
    else
        D.adventure({ xp = xpFor(30), outfit = 5 })
        D.wait(300)
        D.hold(K.DOWN, 2)
        D.wait(700)
        D.shot("sees-starlight")
    end
end)
