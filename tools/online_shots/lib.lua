-- Shared helpers for the sixth-batch screenshot scenarios (run.py).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- START > ONLINE > HUB, then row `row` of the hub (0-based). Both menus
-- remember their cursor, so it only moves down the first time.
local hubOpened = false
function Driver.openHub(row, shotName)
    D.press(K.START, 40)
    if not hubOpened then for _ = 1, 4 do D.press(K.DOWN, 8) end end
    D.press(K.A, 30) -- ONLINE
    if not hubOpened then for _ = 1, 5 do D.press(K.DOWN, 8) end end
    hubOpened = true
    D.press(K.A, 40) -- HUB
    if shotName then D.shot(shotName) end
    for _ = 1, row or 0 do D.press(K.DOWN, 8) end
    D.press(K.A, 30)
end

-- A script or menu has the player (gNetplayExtMailbox.fieldLocked).
function Driver.fieldLocked()
    return D.read8(D.SYM.gNetplayExtMailbox + 0xA52) ~= 0
end

-- Waits for the server's answer to the hub's request to be on screen.
function Driver.waitAnswer(frames)
    D.wait(frames or 150)
end

-- Catches the first wild Pokémon found walking left and right (R throws
-- the last used Ball), then waits for the overworld.
function Driver.catchWild()
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
end

-- Waits until a notice (or any script) takes the player, then screenshots it.
function Driver.waitNotice(name, timeout)
    D.waitFor("a notice", timeout or 3000, D.fieldLocked)
    D.wait(110)
    D.shot(name)
    D.press(K.A, 30)
    D.waitFor("the notice to close", 600, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
end

-- Starts a battle with a roaming Pokémon (gOnlineShowcase in src/online.c).
function Driver.roamer(species, level)
    Driver.wildBattle(species, level, false, 1)
end

-- Starts a wild battle with species at level, shiny if asked.
function Driver.wildBattle(species, level, shiny, kind)
    local s = D.SYM.gOnlineShowcase
    local id = SPECIES_ID[species]
    D.write8(s + 1, level)
    D.write8(s + 2, id & 0xFF)
    D.write8(s + 3, id >> 8)
    D.write8(s + 4, shiny and 1 or 0)
    D.write8(s, kind or 2)
end

-- Puts count of an item (ITEM_ID name) in the bag.
function Driver.giveItem(item, count)
    local s = D.SYM.gOnlineShowcase
    local id = ITEM_ID[item]
    D.write8(s + 1, count)
    D.write8(s + 2, id & 0xFF)
    D.write8(s + 3, id >> 8)
    D.write8(s, 3)
    D.waitFor("the item", 300, function() return D.read8(s) == 0 end)
end

-- Throws the last used Ball until the battle is over (Master Ball: once).
function Driver.catchThis()
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(400)
    for _ = 1, 4 do
        if not D.inBattle() then break end
        D.press(K.R, 30)
        D.press(K.A, 600)
    end
    D.waitFor("the overworld", 3000, D.inOverworld, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
end

-- Fishes facing dir with the registered rod (SELECT), plays the reel
-- meter, and throws the last used Ball at whatever bites, until one Pokémon
-- is landed. shotName: a screenshot once the Pokémon is in battle.
function Driver.fishOne(dir, shotName)
    D.hold(dir, 2)
    D.wait(20)
    for _ = 1, 15 do
        D.press(K.SELECT, 10)
        local bite = false
        for _ = 1, 600 do
            if D.read8(D.SYM.gWorldTimingMeter + 20) == 0xFF then bite = true; break end
            if D.inBattle() then break end
            D.wait(1)
        end
        if bite then D.playMeter(false) end
        if D.read8(D.SYM.gWorldTimingMeter + 20) == 0xFF then D.playMeter(false) end
        for _ = 1, 200 do
            if D.inBattle() then break end
            D.wait(1)
        end
        if D.inBattle() then
            D.wait(380)
            if shotName then D.shot(shotName) end
            D.press(K.A, 150)
            for i = 1, 120 do
                if not D.inBattle() then break end
                D.press(i % 4 == 0 and K.R or K.B, 30)
            end
            D.waitFor("the overworld", 4000, D.inOverworld, function(i)
                if i % 30 == 0 then D.press(K.B, 0) end
            end)
            D.wait(30)
            return true
        end
        D.press(K.A, 30); D.press(K.A, 30); D.press(K.B, 30)
        D.wait(60)
    end
    return false
end

-- True once a file the seed writes exists in the output folder.
function Driver.seedSays(name)
    local f = io.open(OUT_DIR .. "/" .. name, "r")
    if f then f:close() return true end
    return false
end
