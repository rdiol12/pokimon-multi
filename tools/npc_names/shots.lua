-- Everyone's own name, and wild Pokémon that stay home (docs/npc_names.md).
-- Run: python3 tools/world_shots/run.py tools/npc_names/shots.lua <out dir> 80000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local OBJ_SIZE, OFFSET = 0x24, 7

local function objAt(localId)
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * OBJ_SIZE
        if (D.read8(o) & 1) == 1 and D.read8(o + 8) == localId then
            return D.read16(o + 0x10) - OFFSET, D.read16(o + 0x12) - OFFSET
        end
    end
end

local function walkNextTo(tx, ty)
    for _ = 1, 60 do
        local x, y = objAt(0xFF)
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
        local nx, ny = objAt(0xFF)
        if nx == x and ny == y then
            local side = (dir == K.LEFT or dir == K.RIGHT) and (dy >= 0 and K.DOWN or K.UP) or (dx >= 0 and K.RIGHT or K.LEFT)
            D.hold(side, 16); D.wait(4)
        end
    end
    return false
end

local function finishTalk()
    D.waitFor("the talk to end", 1500, function() return D.inOverworld() end, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    D.wait(30)
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
    finishTalk()
end

-- Wild Pokémon on the map now: {species name, ...} by object slot.
local function wildOnMap()
    local out = {}
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * OBJ_SIZE
        if (D.read8(o) & 1) == 1 and D.read8(o + 7) == 0xFF then
            local g = D.read16(o + 4)
            out[i] = SPECIES_NAME[g & 0xFFF] or tostring(g & 0xFFF)
        end
    end
    return out
end

-- Stands still for `frames` and lists every wild Pokémon that turns up.
local function leaveBattle()
    if D.inBattle() then D.runAway() end
    D.waitFor("the overworld", 3000, D.inOverworld, function(i) if i % 30 == 0 then D.press(K.B, 0) end end)
end

local function watchSpawns(label, frames, shots)
    local last, seen, counts = {}, {}, {}
    for f = 1, frames do
        D.wait(1)
        if D.inBattle() then leaveBattle() end
        if f % 10 == 0 then
            local now = wildOnMap()
            for slot, name in pairs(now) do
                if last[slot] ~= name then
                    seen[#seen + 1] = name
                    counts[name] = (counts[name] or 0) + 1
                end
            end
            last = now
        end
        if shots and f % math.floor(frames / shots) == 0 then
            D.shot(string.format("%s-%d", label, f // math.floor(frames / shots)))
        end
    end
    local parts = {}
    for name, n in pairs(counts) do parts[#parts + 1] = name .. " x" .. n end
    table.sort(parts)
    D.log("SPAWNS " .. label .. ": " .. #seen .. " (" .. table.concat(parts, ", ") .. ")")
end

D.run(function()
    D.boot()
    D.wait(60)
    D.write8(D.SYM.gAliveShowcase + 2, 1) -- the living world (visible wild Pokémon) on the test boot

    -- Names: townsfolk, a Pokémon Center, a route.
    D.scene(12, MAPS.MAP_LITTLEROOT_TOWN, 13, 15)
    D.wait(400) -- the season notice
    talkTo(3, "01-littleroot-boy", 1)
    D.scene(12, MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_1F, 4, 6)
    talkTo(2, "02-center-gentleman", 1)
    talkTo(4, "03-center-girl", 1)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 10, 10)
    talkTo(3, "04-oldale-footprints-man", 1)
    talkTo(1, "05-oldale-girl", 1)
    D.scene(12, MAPS.MAP_ROUTE102, 18, 13)
    talkTo(1, "07-route102-little-boy", 1)
    -- Wild Pokémon in a rainstorm on Route 101 and 102: rain visitors used to
    -- take a third of the spawns anywhere; now only the route's own appear.
    D.scene(10, MAPS.MAP_ROUTE101, 10, 13, {storm = "rain"})
    watchSpawns("10-route101-rain", 15000, 3)
    leaveBattle()
    D.scene(10, MAPS.MAP_ROUTE102, 22, 10, {storm = "rain"})
    watchSpawns("11-route102-rain", 15000, 3)
    leaveBattle()
    D.scene(10, MAPS.MAP_ROUTE101, 10, 13)
    watchSpawns("12-route101-clear", 8000, 2)
    -- Birds in Littleroot come from Route 101 now.
    leaveBattle()
    D.scene(14, MAPS.MAP_LITTLEROOT_TOWN, 10, 12)
    D.wait(300)
    D.shot("13-littleroot-birds")

    -- A route trainer spots the player: the name the battle uses, plus a
    -- surname (there's more than one CALVIN).
    D.scene(12, MAPS.MAP_ROUTE102, 31, 17)
    D.hold(K.RIGHT, 16)
    D.hold(K.RIGHT, 16)
    D.wait(150)
    D.shot("08-route102-trainer-spots")
    D.press(K.A, 60)
    D.shot("08-route102-trainer-spots-2")
    finishTalk()
    D.waitFor("the battle", 3000, D.inBattle, function(i) if i % 20 == 0 then D.press(K.A, 0) end end)
    D.wait(200)
    D.shot("09-route102-trainer-battle")
end)
