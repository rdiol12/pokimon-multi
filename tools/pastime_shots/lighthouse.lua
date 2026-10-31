-- Lighthouse keeper. By day the Dewford keeper asks the player to come back
-- after dark; at 21:00 the player keeps the light for a shift, swinging the
-- beam onto each ship coming in (a perfect shift), gets paid and the Big
-- Pearl. Then R > PASTIMES > LIGHTHOUSE.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local SHIPS, BEAM, LAUNCHED, END_TIMER = 0, 24, 25, 30 -- struct Lighthouse offsets
local SHIP_SIZE = 8

-- Keeps the beam on the sailing ship closest to the rocks.
local function keepLight()
    local g = D.SYM.gLighthouse
    local shots = 0
    D.waitFor("the shift", 6000, function()
        return D.read8(g + LAUNCHED) == 8 and D.read8(g + END_TIMER) ~= 0
    end, function(i)
        local target, bestY = nil, -1
        for s = 0, 2 do
            local ship = g + SHIPS + s * SHIP_SIZE
            if D.read8(ship) == 1 and D.read16(ship + 4) > bestY then
                target, bestY = D.read8(ship + 1), D.read16(ship + 4)
            end
        end
        local beam = D.read8(g + BEAM)
        local key = 0
        if target and i % 6 == 0 then
            if target < beam then key = K.LEFT elseif target > beam then key = K.RIGHT end
        end
        D.setKeys(key)
        if i % 110 == 60 and shots < 6 then
            shots = shots + 1
            D.shot("shift-" .. shots)
        end
    end)
    D.setKeys(0)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_DEWFORD_TOWN, 17, 11)
    D.wait(200)
    D.shot("keeper-day")
    D.talk(K.UP, "day", 2)
    for _ = 1, 3 do D.press(K.B, 60) end
    D.scene(21, MAPS.MAP_DEWFORD_TOWN, 17, 11)
    D.wait(200)
    D.talk(K.UP, "offer", 2)
    D.press(K.A, 40) -- YES
    D.wait(40)
    D.shot("howto")
    for _ = 1, 3 do D.press(K.A, 60) end
    keepLight()
    for i = 1, 8 do
        D.wait(70)
        D.shot("paid-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 60) end
    D.pastimeMenu(25)
    D.wait(30)
    D.shot("lighthouse")
    D.press(K.B, 60)
end)
