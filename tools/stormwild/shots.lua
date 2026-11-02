-- The Stormwild (tools/stormwild/shots_seed.py runs a short, fixed run):
-- TEST1 and TEST2 talk to the Storm Warden in Mauville, pack a pouch and go
-- in together. TEST1 opens a supply cache and gets out at the first beacon
-- to light; TEST2 catches a Pokémon in the tall grass and is still inside
-- when the storm closes in, so loses the pouch and the catch.
dofile(OUT_DIR .. "/lib.lua")
dofile(OUT_DIR .. "/stormwild_path.lua")
local D, K = Driver, Driver.KEY
local ONE = INSTANCE == 1
local IDLE, LOBBY, ENTERING, LIVE, SCRIPT, AFTER = 0, 1, 2, 3, 4, 5

local function phase() return D.read8(STORM_STATE) end
local function shot(name) if ONE or name:find("^p2") then D.shot(name) end end

-- Runs to each corner in turn.
local function walk(path)
    for _, corner in ipairs(path) do
        local tx, ty = corner[1], corner[2]
        D.waitFor("the corner " .. tx .. "," .. ty, 4000, function()
            local x, y = D.pos()
            return x == tx and y == ty
        end, function(i)
            if D.fieldLocked() then
                D.setKeys(0)
                if i % 30 == 0 then D.press(K.A, 0) end
                return
            end
            local x, y = D.pos()
            local key = x > tx and K.LEFT or x < tx and K.RIGHT or y > ty and K.UP or K.DOWN
            D.setKeys(key | K.B)
        end)
    end
    D.setKeys(0)
end

-- Only three Pokémon may go in: the test save's others stay behind.
local function trimParty()
    for slot = 3, 5 do
        for i = 0, 99 do D.write8(D.SYM.gParties + slot * 100 + i, 0) end
    end
    if D.partyCount() > 3 then D.write8(D.SYM.gPartiesCount, 3) end
end

local function dumpObjs()
    if os.getenv("STORM_DUMP") then
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * 0x24
        if D.read8(o) & 1 == 1 then
            D.log(string.format("obj %d b1 %02x sprite %d elev %02x gfx %d local %d at %d,%d", i, D.read8(o + 1), D.read8(o + 0x23), D.read8(o + 0xB), D.read16(o + 4), D.read8(o + 8),
                D.read16(o + 0x10) - 7, D.read16(o + 0x12) - 7))
        end
    end
    local flags = D.read32(D.SYM.gSaveBlock1Ptr) + 0x1270
    D.log(string.format("temp flags %08x", D.read32(flags)))
    end
end

local function readAll(name, n)
    for i = 1, n do
        D.wait(70)
        if name then shot(name .. "-" .. i) end
        D.press(K.A, 20)
    end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.wait(300)
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, ONE and 20 or 19, ONE and 7 or 6)
    D.log("party at the start: " .. D.partyCount())
    trimParty()
    D.giveItem("SUPER_POTION", 12)
    D.giveItem(ONE and "ULTRA_BALL" or "MASTER_BALL", ONE and 15 or 1)
    D.giveItem("REVIVE", 3)
    if not ONE then D.wait(1600) end
    D.hold(ONE and K.UP or K.RIGHT, 2)
    D.wait(20)
    if ONE then D.shot("warden") end
    D.press(K.A, 40)
    readAll(ONE and "hello" or nil, 2)
    D.wait(40)
    shot("menu")
    if ONE then
        -- HOW IT WORKS first.
        D.wait(30)
        D.press(K.DOWN, 20); D.press(K.DOWN, 20)
        D.wait(20)
        D.shot("menu-rules")
        D.press(K.A, 40)
        for i = 1, 11 do D.wait(70); shot("rules-" .. i); D.press(K.A, 20) end
        D.wait(60)
        D.press(K.UP, 20); D.press(K.UP, 20)
    end
    D.press(K.A, 40) -- ENTER THE STORMWILD
    readAll(ONE and "pouch" or nil, 2)
    D.wait(90)
    shot("pack-list")
    -- Pack the first three things on the list.
    for i = 1, 3 do
        D.press(K.A, 90)
        if i == 2 then shot("pack-list-2") end
    end
    D.press(K.B, 90)
    shot("ready")
    D.press(K.A, 200) -- YES
    shot("joined")
    D.press(K.A, 30)
    D.waitFor("the lobby", 1200, function() return phase() == LOBBY end)
    D.wait(120)
    shot("gate")
    D.waitFor("the drop", 3600, function() return phase() == LIVE end, function(i)
        if i % 60 == 0 and D.fieldLocked() then D.press(K.A, 0) end
    end)
    D.wait(200)
    shot(ONE and "arrived" or "p2-arrived")
    local save = D.SYM.gSaveBlockExt + STORM_SAVE_OFFSET
    D.log(string.format("stormwild save: state %d run %d active %08x", D.read8(save), D.read16(save + 4), D.read32(save + 8)))
    dumpObjs()

    if ONE then
        -- The nearest cache.
        walk(STORM_PATH[1][1])
        D.wait(40)
        D.press(K.RIGHT, 20)
        D.press(K.RIGHT, 30)
        D.shot("cache")
        dumpObjs()
        D.press(K.A, 150)
        D.shot("cache-loot")
        D.press(K.A, 120)
        D.waitFor("the field", 600, function() return not D.fieldLocked() end, function(i)
            if i % 30 == 0 then D.press(K.A, 0) end
        end)
        D.wait(60)
        D.shot("cache-gone")
        -- The beacon; wait for it to light.
        walk(STORM_PATH[1][2])
        D.wait(30)
        D.shot("beacon-dark")
        D.waitFor("the beacon", 6000, function() return phase() == SCRIPT end, function(i)
            if i == 700 then D.shot("beacon-lit") end
            if i % 120 == 60 and phase() == LIVE then D.shot("beacon-hold-" .. (i // 120)) end
        end)
        D.wait(80)
        D.shot("pickup")
        D.press(K.A, 200)
        D.waitFor("the gate", 1500, function() return phase() == IDLE end, function(i)
            if i % 40 == 0 and D.fieldLocked() then D.press(K.A, 0) end
        end)
        D.wait(150)
        D.shot("out")
        D.press(K.A, 300)
        D.shot("out-2")
        D.press(K.A, 60)
        D.wait(60)
        D.shot("welcome")
        D.press(K.A, 60)
        D.waitFor("the save", 1200, function() return not D.fieldLocked() end, function(i)
            if i % 40 == 0 then D.press(K.A, 0) end
        end)
        D.wait(120)
        -- Best hauls.
        D.hold(K.UP, 2)
        D.wait(20)
        D.press(K.A, 40)
        readAll(nil, 2)
        D.wait(40)
        D.press(K.DOWN, 20)
        D.press(K.A, 60)
        D.wait(60)
        D.shot("record")
        D.press(K.A, 200)
        D.press(K.A, 120)
        D.shot("board")
        D.press(K.A, 60)
    else
        walk(STORM_PATH[2][1])
        -- Into the tall grass below and back until something jumps out.
        D.findBattle(K.DOWN, K.UP, 2)
        D.wait(250)
        D.shot("p2-wild")
        for _ = 1, 5 do
            if not D.inBattle() then break end
            D.press(K.R, 30)
            D.press(K.A, 600)
        end
        D.waitFor("the overworld", 3000, D.inOverworld, function(i)
            if i % 30 == 0 then D.press(K.B, 0) end
        end)
        D.wait(60)
        D.waitFor("the field", 600, function() return not D.fieldLocked() end, function(i)
            if i % 30 == 0 then D.press(K.A, 0) end
        end)
        D.wait(30)
        D.log("party after the catch: " .. D.partyCount() .. ", battle outcome " .. D.read8(D.SYM.gBattleOutcome)
            .. ", caught in the run " .. D.read8(D.SYM.gSaveBlockExt + STORM_SAVE_OFFSET + 2)
            .. string.format(" (%08x, %08x)", D.read32(D.SYM.gSaveBlockExt + STORM_SAVE_OFFSET + 428), D.read32(D.SYM.gParties + 200)))
        D.shot("p2-caught")
        local noticed = false
        D.waitFor("the storm", 20000, function() return phase() == SCRIPT end, function(i)
            if D.fieldLocked() and i % 30 == 0 then
                if not noticed then D.shot("p2-notice"); noticed = true end
                D.press(K.A, 0)
            end
            if i == 3000 then D.shot("p2-waiting") end
        end)
        D.wait(80)
        D.shot("p2-timeup")
        D.press(K.A, 200)
        D.waitFor("the gate", 2500, function() return phase() == IDLE end, function(i)
            if i % 40 == 0 and D.fieldLocked() then D.press(K.A, 0) end
        end)
        D.wait(150)
        D.shot("p2-lost")
        D.press(K.A, 80)
        D.shot("p2-rescued")
        D.press(K.A, 80)
        D.press(K.A, 80)
        D.wait(60)
        D.log("party after the storm: " .. D.partyCount())
        D.shot("p2-back")
    end
end)
