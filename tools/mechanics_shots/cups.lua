-- The tournament host in the Verdanturf Battle Tent: the cup list, the
-- RISING CUP (level 30) with a level 60 lead brought down, three rounds,
-- and the prize; then the LITTLE CUP turning away evolved Pokémon.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "GARDEVOIR", level = 55, lead = true })
    D.mech({ fixMoves = true, moves = { "PSYCHIC", "MOONBLAST", "THUNDERBOLT", "CALM_MIND" } })
    D.scene(13, MAPS.MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "SALAMENCE", level = 58, lead = true })
    D.mech({ moves = { "DRAGON_CLAW", "FLAMETHROWER", "AERIAL_ACE", "CRUNCH" } })
    D.scene(13, MAPS.MAP_VERDANTURF_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "SWAMPERT", level = 60, lead = true })
    D.fastText()
    D.mech({ fixMoves = true, moves = { "SURF", "EARTHQUAKE", "ICE_BEAM", "ROCK_SLIDE" } })
    D.shot("lobby")
    D.hold(K.UP, 2)
    D.wait(10)
    for _ = 1, 6 do D.step(K.A, "intro") end
    D.step(K.DOWN, "menu")
    D.step(K.A, "help")
    D.step(K.A, "ready")
    D.step(K.A, "begin")
    for r = 1, 3 do
        D.step(K.A, "round-" .. r)
        D.fight("r" .. r .. "-battle", 90, 400)
        if not D.inOverworld() then break end
        D.shot("r" .. r .. "-after")
        D.step(K.A, "r" .. r .. "-next")
    end
    for _ = 1, 6 do D.step(K.A, "end") end
    -- The real levels are back.
    D.step(K.START, "start-menu")
    D.step(K.A, "party", 120)
    D.result("ok")
end)
