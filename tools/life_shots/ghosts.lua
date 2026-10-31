-- Ghost trainers, two players online (run from tools/netplay_bridge so the
-- bridge loads its extensions):
--   python3 ../life_shots/run.py ../life_shots/ghosts.lua <out> 60000 2
-- TEST1's team (a shiny Garchomp) goes up to the server. TEST2 meets it on
-- Route 101 as a ghost trainer and beats it, and TEST1 hears about it.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- Both games wait here until the other one gets here too.
local function sync(name)
    local mine = string.format("%s/sync-%s-%d", OUT_DIR, name, INSTANCE)
    local other = string.format("%s/sync-%s-%d", OUT_DIR, name, 3 - INSTANCE)
    io.open(mine, "w"):close()
    D.waitFor("the other game at " .. name, 60000, function()
        local f = io.open(other, "r")
        if f then f:close() return true end
        return false
    end)
end

local PP, HP, MAXHP, STATUS = 0x25, 0x2A, 0x2E, 0x50 -- struct BattlePokemon
local function fight(name)
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(300)
    D.shot(name .. "-battle")
    local shots = 0
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
            if i % 400 == 0 and shots < 3 then
                shots = shots + 1
                D.shot(name .. "-battle")
            end
            D.press(K.A, 0)
        else
            D.press(K.B, 0) -- out of the party menu
        end
    end)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)

    if P1 then
        D.scene(12, MAPS.MAP_PETALBURG_CITY, 15, 10, { give = "GARCHOMP", level = 70, lead = true, shiny = true })
        D.wait(600) -- the team goes up when it changes
        D.shot("owner")
    else
        D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "METAGROSS", level = 60, lead = true })
        D.wait(300)
    end
    sync("uploaded")

    if not P1 then
        D.hold(K.LEFT, 2) -- the ghost turns up three steps ahead
        D.wait(20)
        D.write8(D.SYM.gLifeShowcase + 11, 1)
        D.wait(240)
        D.shot("ghost-appears")
        D.hold(K.LEFT, 32)
        D.wait(30)
        D.shot("ghost-close")
        D.press(K.A, 0)
        for p = 1, 3 do
            D.wait(150)
            D.shot("ghost-intro-" .. p)
            D.press(K.A, 0)
        end
        D.wait(120)
        D.shot("ghost-ask")
        D.press(K.A, 120) -- YES
        D.shot("ghost-go")
        D.press(K.A, 0)
        fight("ghost")
        D.wait(200)
        D.shot("ghost-after")
        D.press(K.A, 120)
        D.wait(60)
        D.shot("ghost-gone")
    end
    sync("battled")

    if P1 then
        -- A new team member: the team goes up again, and the ghost's
        -- record comes back.
        D.scene(12, MAPS.MAP_PETALBURG_CITY, 15, 10, { give = "PIKACHU", level = 30 })
        for i = 1, 8 do
            D.wait(90)
            D.shot("owner-hears")
        end
    end
    sync("done")
end)
