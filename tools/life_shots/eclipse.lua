-- Team Eclipse: the postgame chapter from the Space Center scientist,
-- through Meteor Falls and the Mt. Pyre summit, to Leader Nyx and the
-- reward. Battles are won by setting the opponent's HP to 1 and attacking
-- (and keeping the player's Pokémon healthy: HP, PP and no status).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- gLifeShowcase byte 9: game clear, chapter at step (value - 1).
local function chapter(step)
    local s = D.SYM.gLifeShowcase
    D.write8(s + 9, step + 1)
    D.write8(s, 1)
    D.wait(10)
end

local PP, HP, MAXHP, STATUS = 0x25, 0x2A, 0x2E, 0x50 -- struct BattlePokemon
local function fight(name)
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(300)
    D.shot(name .. "-battle")
    D.waitFor("the overworld", 30000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        if D.inBattle() then
            local hp = D.SYM.gBattleMons + D.SYM.BATTLEMON_SIZE + HP
            if D.read16(hp) > 1 then D.write8(hp, 1); D.write8(hp + 1, 0) end
            local mine = D.SYM.gBattleMons
            local max = D.read16(mine + MAXHP)
            D.write8(mine + HP, max & 0xFF); D.write8(mine + HP + 1, max >> 8)
            for m = 0, 3 do D.write8(mine + PP + m, 10) end
            for b = 0, 3 do D.write8(mine + STATUS + b, 0) end
            D.press(K.A, 0)
        else
            D.press(K.B, 0) -- out of the party menu
        end
    end)
    D.wait(60)
end

local function trainer(dir, name, introPages)
    D.talk(dir, name .. "-intro", introPages)
    D.press(K.A, 20)
    fight(name)
    D.shot(name .. "-after")
    for _ = 1, 6 do D.press(K.A, 40) end
    D.wait(120)
    D.shot(name .. "-gone")
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(20, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 11, 7, { give = "MEWTWO", level = 100, lead = true })
    chapter(0)
    D.scene(20, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 11, 7)
    D.wait(200)
    D.shot("scientist")
    D.talk(K.UP, "scientist-start", 6)
    D.wait(60)

    D.scene(20, MAPS.MAP_METEOR_FALLS_1F_1R, 13, 20)
    D.wait(150)
    D.shot("falls")
    trainer(K.LEFT, "falls-grunt-m", 2)
    D.scene(20, MAPS.MAP_METEOR_FALLS_1F_1R, 15, 21)
    D.wait(150)
    trainer(K.LEFT, "falls-grunt-f", 1)

    D.scene(20, MAPS.MAP_MT_PYRE_SUMMIT, 21, 16)
    D.wait(150)
    D.shot("pyre")
    trainer(K.UP, "pyre-grunt", 1)
    D.scene(20, MAPS.MAP_MT_PYRE_SUMMIT, 23, 9)
    D.wait(150)
    D.shot("pyre-vesper")
    trainer(K.UP, "vesper", 4)

    D.scene(20, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_2F, 12, 3)
    D.wait(150)
    D.shot("center")
    trainer(K.UP, "center-grunt", 1)
    D.scene(20, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_2F, 14, 3)
    D.wait(150)
    D.shot("center-nyx")
    trainer(K.UP, "nyx", 5)

    D.scene(20, MAPS.MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 11, 7)
    D.wait(150)
    D.talk(K.UP, "reward", 6)
    for _ = 1, 6 do D.press(K.A, 60) end
    D.shot("reward-done")
end)
