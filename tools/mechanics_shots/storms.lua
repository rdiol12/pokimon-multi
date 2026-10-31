-- The postgame storms: the researcher in the Weather Institute, the drought
-- on Route 114 with Groudon's echo, the thunderstorm on Route 118 with Kyogre's
-- echo, and the reward once both are calm.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local GAME_CLEAR = 0x864

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE119_WEATHER_INSTITUTE_2F, 17, 4, { give = "METAGROSS", level = 80, lead = true })
    D.fastText()
    D.mech({ fixMoves = true, badges = 8, var = { GAME_CLEAR, 1 }, moves = { "METEOR_MASH", "ZEN_HEADBUTT", "EARTHQUAKE", "BULLET_PUNCH" } })
    -- Let the achievement pop-ups go by.
    D.wait(600)
    D.hold(K.UP, 2)
    D.wait(10)
    D.shot("institute")
    D.step(K.A, "researcher", 60)
    for _ = 1, 5 do D.step(K.A, "researcher") end
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE114, 11, 33)
    D.wait(60)
    D.shot("drought")
    D.hold(K.LEFT, 16)
    D.wait(10)
    D.step(K.A, "groudon-intro", 120)
    D.step(K.A, "groudon-intro")
    D.step(K.A, "groudon-start", 200)
    D.fight("groudon-battle", 50, 30)
    for _ = 1, 5 do D.step(K.A, "groudon-after", 60) end
    D.scene(13, MAPS.MAP_ROUTE118, 32, 11, { give = "JOLTEON", level = 100, lead = true })
    D.mech({ fixMoves = true, moves = { "THUNDERBOLT", "SHADOW_BALL", "PIN_MISSILE", "AGILITY" } })
    D.wait(60)
    D.shot("thunderstorm")
    D.hold(K.LEFT, 16)
    D.wait(10)
    D.step(K.A, "kyogre-intro", 120)
    D.step(K.A, "kyogre-intro")
    D.step(K.A, "kyogre-start", 200)
    D.fight("kyogre-battle", 50, 30)
    for _ = 1, 5 do D.step(K.A, "kyogre-after", 60) end
    D.scene(13, MAPS.MAP_ROUTE119_WEATHER_INSTITUTE_2F, 17, 4)
    D.hold(K.UP, 2)
    D.wait(10)
    for _ = 1, 8 do D.step(K.A, "reward", 60) end
    D.result("ok")
end)
