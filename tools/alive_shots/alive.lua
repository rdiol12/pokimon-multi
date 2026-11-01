-- The living world (docs/alive.md): one screenshot set per feature.
-- Run: python3 tools/world_shots/run.py tools/alive_shots/alive.lua <out dir> 60000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local OBJ_SIZE, OFFSET = 0x24, 7
local SHOW = { NONE = 0, INCIDENT = 1, CHATTER = 2, CRITTERS = 3, PROJECT = 4, PACK = 5, REMARK = 6, DRY = 7 }
local WEATHER = { NONE = 0, RAIN = 3, SNOW = 4, DROUGHT = 12 }

-- Sets what the living world forces on the next map (gAliveShowcase).
local function alive(scene, arg, weather, species)
    local s = D.SYM.gAliveShowcase
    D.write8(s, scene or 0)
    D.write8(s + 1, arg or 0)
    D.write8(s + 2, 1)
    D.write8(s + 3, weather or 0)
    local sp = species and SPECIES_ID[species] or 0
    D.write8(s + 4, sp & 0xFF); D.write8(s + 5, sp >> 8)
end

-- Map coords (no offset) of the object with this local id, or nil.
local function objAt(localId)
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * OBJ_SIZE
        if (D.read8(o) & 1) == 1 and D.read8(o + 8) == localId then
            local x, y = D.read16(o + 0x10), D.read16(o + 0x12)
            return x - OFFSET, y - OFFSET
        end
    end
end

local function playerAt() return objAt(0xFF) end

-- Walks (greedy, with sidesteps) next to the tile, then faces it.
local function walkNextTo(tx, ty)
    for _ = 1, 60 do
        local x, y = playerAt()
        if x == nil then return false end
        local dx, dy = tx - x, ty - y
        if math.abs(dx) + math.abs(dy) == 1 then
            local dir = dx == 1 and K.RIGHT or dx == -1 and K.LEFT or dy == 1 and K.DOWN or K.UP
            D.hold(dir, 2); D.wait(10)
            return true
        end
        local dir
        if math.abs(dx) > math.abs(dy) then dir = dx > 0 and K.RIGHT or K.LEFT
        else dir = dy > 0 and K.DOWN or K.UP end
        D.hold(dir, 16); D.wait(4)
        local nx, ny = playerAt()
        if nx == x and ny == y then
            -- Blocked: step around.
            local side = (dir == K.LEFT or dir == K.RIGHT) and (dy >= 0 and K.DOWN or K.UP) or (dx >= 0 and K.RIGHT or K.LEFT)
            D.hold(side, 16); D.wait(4)
        end
    end
    return false
end

local function talkTo(localId, name, pages)
    local x, y = objAt(localId)
    if x == nil then D.log("no object " .. localId .. " for " .. name); return end
    if not walkNextTo(x, y) then D.log("couldn't reach " .. name); return end
    D.press(K.A, 40)
    for p = 1, pages or 1 do
        D.wait(70)
        D.shot(name .. "-" .. p)
        D.press(K.A, 20)
    end
    D.wait(30)
end

local function finishTalk()
    D.waitFor("the talk to end", 1500, function() return D.inOverworld() end, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
end

D.run(function()
    D.boot()
    D.wait(60)

    -- 1. Visible wild Pokémon: a pack by day, then sleepers at night.
    alive(SHOW.PACK, 0, 0, "POOCHYENA")
    D.scene(10, MAPS.MAP_ROUTE101, 10, 10)
    D.wait(500)
    D.shot("01-wild-pack")
    alive(SHOW.NONE)
    D.scene(23, MAPS.MAP_ROUTE101, 10, 10)
    D.wait(700)
    D.shot("01-wild-asleep")

    -- 2. Townsfolk head home in the evening; the Mart closes at night.
    alive(SHOW.NONE)
    D.scene(20, MAPS.MAP_OLDALE_TOWN, 10, 14)
    D.wait(200)
    D.shot("02-evening-commute-a")
    D.wait(400)
    D.shot("02-evening-commute-b")
    D.scene(23, MAPS.MAP_OLDALE_TOWN_MART, 3, 3)
    D.talk(K.LEFT, "02-shop-closed", 2)
    finishTalk()

    -- 3. A townsperson remembers the player.
    alive(SHOW.REMARK)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 13, 18)
    D.wait(60)
    D.talk(K.UP, "03-remark", 3)
    finishTalk()

    -- 4. The town crier shouts the news and hands out the paper.
    alive(SHOW.NONE)
    D.scene(11, MAPS.MAP_OLDALE_TOWN, 6, 18)
    D.wait(500)
    D.shot("04-crier-shout")
    talkTo(56, "04-crier", 1)
    D.wait(90)
    D.shot("04-paper")
    finishTalk()

    -- 9. Town Pokémon around Oldale and Rustboro.
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 9, 15)
    D.wait(300)
    D.shot("09-town-mons-oldale")
    talkTo(52, "09-town-mon", 2)
    finishTalk()
    D.scene(12, MAPS.MAP_RUSTBORO_CITY, 16, 40)
    D.wait(300)
    D.shot("09-town-mons-rustboro")

    -- 10. Two townsfolk chat.
    alive(SHOW.CHATTER)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 14, 14)
    for i = 1, 8 do
        D.wait(60)
        D.shot("10-chatter-" .. i)
    end

    -- 5. Birds hop about and fly off; a fish jumps.
    alive(SHOW.CRITTERS)
    D.scene(14, MAPS.MAP_ROUTE102, 20, 10)
    D.wait(120)
    D.shot("05-critters")
    D.wait(400)
    D.shot("05-critters-later")

    -- 6. A town project: being built, then finished.
    alive(SHOW.NONE)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 9, 15)
    D.wait(120)
    D.shot("06-project-site")
    talkTo(57, "06-foreman", 3)
    finishTalk()
    alive(SHOW.PROJECT, 0)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 9, 15)
    D.wait(120)
    D.shot("06-project-built")
    talkTo(57, "06-foreman-built", 2)
    finishTalk()

    -- 7. Small surprise events.
    local incidents = { "stampede", "lost-child", "stuck-mon", "crate", "hurt-mon" }
    local places = { {MAPS.MAP_ROUTE101, 10, 10}, {MAPS.MAP_ROUTE102, 20, 10}, {MAPS.MAP_ROUTE102, 30, 10},
                     {MAPS.MAP_ROUTE103, 20, 12}, {MAPS.MAP_ROUTE101, 10, 10} }
    for k = 1, 5 do
        alive(SHOW.INCIDENT, k)
        local p = places[k]
        D.scene(11, p[1], p[2], p[3])
        D.wait(40)
        D.shot("07-" .. incidents[k] .. "-a")
        D.wait(80)
        D.shot("07-" .. incidents[k] .. "-b")
    end
    -- The hurt Pokémon: talk to it.
    talkTo(64, "07-hurt-mon-talk", 2)
    finishTalk()

    -- 8. Weather changes the map.
    alive(SHOW.NONE, 0, WEATHER.RAIN)
    D.scene(11, MAPS.MAP_ROUTE102, 20, 10)
    D.wait(200)
    D.shot("08-rain-puddles")
    alive(SHOW.NONE, 0, WEATHER.SNOW)
    D.scene(11, MAPS.MAP_ROUTE101, 6, 3)
    D.wait(150)
    D.shot("08-snow")
    for _ = 1, 4 do D.hold(K.RIGHT, 16) end
    D.hold(K.DOWN, 16)
    D.wait(10)
    D.shot("08-snow-footprints")
    D.wait(30)
    alive(SHOW.DRY, 0, 2)
    D.scene(11, MAPS.MAP_ROUTE102, 40, 8)
    D.wait(150)
    D.shot("08-dry-pond")
end)
