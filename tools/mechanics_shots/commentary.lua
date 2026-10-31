-- Battle commentary in a tournament: the RISING CUP's first round, with the
-- announcer opening the battle and calling super effective hits, KOs and
-- the last Pokémon standing.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "SWAMPERT", level = 60, lead = true })
    D.mech({ fixMoves = true, moves = { "SURF", "EARTHQUAKE", "ICE_BEAM", "ROCK_SLIDE" } })
    D.scene(13, MAPS.MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "JOLTEON", level = 55, lead = true })
    D.fastText()
    D.mech({ fixMoves = true, moves = { "THUNDERBOLT", "SHADOW_BALL", "PIN_MISSILE", "AGILITY" } })
    D.hold(K.UP, 2)
    D.wait(10)
    for _ = 1, 6 do D.step(K.A, "intro") end
    D.step(K.DOWN, "menu")
    D.step(K.A, "help")
    D.step(K.A, "ready")
    D.step(K.A, "begin")
    D.step(K.A, "round-1")
    D.fight("battle", 30, 400)
    D.result("ok")
end)
