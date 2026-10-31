-- Alpha taming. The Route 116 alpha Loudred, drawn bigger than other
-- Pokémon: calmed on the meter (every stop good), it joins the party. The
-- Route 114 alpha Zangoose: a missed stop, and it attacks (a wild battle).
-- Then R > PASTIMES > ALPHAS.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local function approach(map, x, y, name)
    D.scene(13, map, x, y + 3)
    D.wait(200)
    D.shot(name)
    D.hold(K.UP, 34)
    D.wait(30)
end

D.run(function()
    D.boot()
    D.wait(60)
    approach(MAPS.MAP_ROUTE116, 43, 10, "loudred")
    D.shot("loudred-near")
    D.talk(K.UP, "loudred", 2)
    D.press(K.A, 40) -- YES
    D.wait(40)
    D.shot("howto")
    D.press(K.A, 40)
    D.playMeter(false, "calm-meter")
    for i = 1, 4 do
        D.wait(80)
        D.shot("tamed-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 60) end
    D.wait(30)
    D.shot("loudred-gone")
    approach(MAPS.MAP_ROUTE114, 32, 13, "zangoose")
    D.talk(K.UP, "zangoose", 2)
    D.press(K.A, 40)
    D.wait(40)
    D.press(K.A, 40)
    D.playMeter(true)
    D.wait(60)
    D.shot("attacks")
    D.press(K.A, 40)
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(400)
    D.shot("battle")
    D.runAway()
    D.wait(60)
    D.pastimeMenu(24)
    D.wait(30)
    D.shot("alphas")
    D.press(K.B, 60)
end)
