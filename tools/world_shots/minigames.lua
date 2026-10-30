-- Fishing and berry minigames: reel a Pokémon in with the meter, land it,
-- see its size; pick berries with the meter; then R > RECORDS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE103, 21, 9, { item = "OLD_ROD" })
    D.hold(K.RIGHT, 2)
    D.wait(10)
    D.scene(13, MAPS.MAP_ROUTE103, 21, 9, { item = "ULTRA_BALL" })
    D.hold(K.RIGHT, 2)
    D.wait(20)
    local caught = false
    for attempt = 1, 12 do
        D.press(K.SELECT, 10)
        -- wait for a bite (the meter) or for "Not even a nibble"
        local bite = false
        for _ = 1, 600 do
            if D.read8(D.SYM.gWorldTimingMeter + 20) == 0xFF then bite = true; break end
            if D.inBattle() then break end
            D.wait(1)
        end
        if bite then
            D.playMeter(false, not caught and "reel-meter" or nil)
            caught = true
            D.wait(10)
        end
        if D.read8(D.SYM.gWorldTimingMeter + 20) == 0xFF then D.playMeter(false) end
        for _ = 1, 200 do
            if D.inBattle() then break end
            D.wait(1)
        end
        if D.inBattle() then
            D.wait(300)
            D.press(K.A, 150)
            -- R throws the ball; B clears the text and says no to a nickname.
            for i = 1, 120 do
                if not D.inBattle() then break end
                D.press(i % 4 == 0 and K.R or K.B, 30)
            end
            D.waitFor("the overworld", 4000, D.inOverworld, function(i)
                if i % 30 == 0 then D.press(K.B, 0) end
            end)
            D.wait(30)
            D.shot("landed-ach")
            D.wait(230)
            D.shot("landed")
            D.wait(240)
            break
        else
            D.press(K.A, 30); D.press(K.A, 30); D.press(K.B, 30)
            D.wait(60)
        end
    end

    D.scene(13, MAPS.MAP_ROUTE102, 24, 3, { ripen = true })
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.shot("berry-tree")
    D.press(K.A, 60)
    D.press(K.A, 60) -- YES, pick
    D.wait(30)
    D.playMeter(false, "berry-meter")
    D.wait(30)
    D.shot("berry-picked")
    D.wait(60)
    D.shot("berry-picked-2")
    for _ = 1, 8 do D.press(K.B, 40) end
    D.wait(60)
    D.wait(120)
    D.press(K.R, 60)
    D.press(K.DOWN, 8)
    D.press(K.DOWN, 8)
    D.press(K.A, 60)
    D.shot("records")
end)
