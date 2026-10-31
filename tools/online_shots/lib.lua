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
    local s = D.SYM.gOnlineShowcase
    local id = SPECIES_ID[species]
    D.write8(s + 1, level)
    D.write8(s + 2, id & 0xFF)
    D.write8(s + 3, id >> 8)
    D.write8(s, 1)
end
