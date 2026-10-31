-- Carnival games at the Sunfair Carnival: a game of bowling (strikes and a
-- spare), three holes of mini golf (with a splash in the pond), a soccer
-- shootout and a Ferris wheel ride. Then R > PASTIMES > CARNIVAL.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local M = D.SYM.gWorldTimingMeter -- struct TimingMeter (src/world_meter.c)
local G = D.SYM.gCarnival         -- struct Carnival (src/pastime_carnival.c)

-- The carnival map comes with the area branches, whose R menu has the
-- BOND, JOBS and ALBUM rows too.
D.PASTIMES_ROW = 11

local function s16(addr) local v = D.read16(addr); return v >= 0x8000 and v - 0x10000 or v end
local function meterRunning() return D.read8(M + 20) == 0xFF end

-- Stops the meter: "perfect" in the middle of the zone, "good" near its
-- edge, "miss" not at all.
local function playMeter(mode)
    D.waitFor("the meter", 3000, function() return not meterRunning() end, function()
        if mode == "miss" or D.read8(M + 13) ~= 0 then D.setKeys(0); return end
        local needle, zone, tiles = s16(M + 16), s16(M + 18), D.read8(M + 7)
        local target = mode == "perfect" and zone + tiles * 4 or zone + 3
        D.setKeys(math.abs(needle - target) <= 1 and K.A or 0)
    end)
    D.setKeys(0)
end

-- Plays a game: presses A through the messages and plays each meter with
-- the next mode, until done() says the game is over.
local function playGame(name, modes, done)
    local n = 0
    while true do
        D.waitFor(name .. " meter", 6000, function() return meterRunning() or done() end, function(i)
            if i % 40 == 0 then D.press(K.A, 0) end
        end)
        if done() then break end
        n = n + 1
        D.wait(10)
        D.shot(name .. "-aim-" .. n)
        playMeter(modes[(n - 1) % #modes + 1])
        D.wait(30)
        D.shot(name .. "-" .. n)
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_SUNFAIR_CARNIVAL, 9, 14)
    D.wait(30)
    D.shot("lanes")
    D.talk(K.UP, "bowling", 1)
    playGame("bowl", {"perfect", "good", "good", "perfect", "miss", "good", "perfect", "good", "good"},
        function() return D.read8(G) >= 5 end)
    for i = 1, 6 do
        D.wait(60)
        D.shot("bowl-end-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end

    D.scene(12, MAPS.MAP_SUNFAIR_CARNIVAL, 39, 27)
    D.write8(G, 0) -- the round counter is still bowling's
    D.talk(K.UP, "golf", 1)
    playGame("golf", {"perfect", "good", "good", "perfect", "good", "miss", "good", "perfect"},
        function() return D.read8(G) >= 3 end)
    for i = 1, 6 do
        D.wait(60)
        D.shot("golf-end-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end

    D.scene(12, MAPS.MAP_SUNFAIR_CARNIVAL, 12, 27)
    D.write8(G, 0)
    D.talk(K.UP, "soccer", 1)
    local k = 0
    D.waitFor("the shootout", 9000, function() return D.read8(G) >= 5 end, function(i)
        if meterRunning() then
            k = k + 1
            D.shot("soccer-aim-" .. k)
            playMeter(k % 3 == 0 and "good" or "perfect")
            D.wait(30)
            D.shot("soccer-shot-" .. k)
        elseif i % 40 == 0 then
            if i % 120 == 0 then D.shot("soccer-" .. i // 120) end
            D.press(K.A, 0)
        end
    end)
    for i = 1, 6 do
        D.wait(60)
        D.shot("soccer-end-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end

    D.scene(20, MAPS.MAP_SUNFAIR_CARNIVAL, 29, 13)
    D.talk(K.UP, "ferris", 1)
    for i = 1, 22 do
        D.wait(45)
        D.shot("ride-" .. i)
        if i % 2 == 0 then D.press(K.A, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(120)
    D.shot("after-ride")
    D.pastimeMenu(16)
    D.wait(30)
    D.shot("carnival-list")
    D.press(K.B, 60)
end)
