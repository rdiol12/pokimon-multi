-- Achievements: riding unlocks "Saddle Up"; catching a Pokémon at night
-- unlocks "First Catch" and "Night Owl"; then R > ACHIEVEMENTS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(22, MAPS.MAP_ROUTE101, 4, 4, { give = "RAPIDASH", level = 40, item = "ULTRA_BALL" })
    D.press(K.R, 40)
    D.press(K.A, 140)
    D.shot("banner-saddle-up")
    D.wait(200)
    D.press(K.R, 40)
    D.press(K.A, 60)
    D.wait(60)
    -- Catch the first wild Pokémon: R throws the last used Ball.
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
    D.wait(40)
    D.shot("banner-first-catch")
    D.wait(220)
    D.shot("banner-night-owl")
    D.wait(220)
    D.press(K.R, 40)
    D.press(K.DOWN, 10)
    D.press(K.A, 60)
    D.shot("achievement-list")
    for _ = 1, 7 do D.press(K.DOWN, 6) end
    D.wait(20)
    D.shot("achievement-list-night-owl")
end)
