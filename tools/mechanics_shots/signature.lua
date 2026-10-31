-- Starter signature moves: Birch's aide starts the quest, the statues of
-- the three starters, the guardian battle in Petalburg Woods, and the aide
-- teaching CANOPY EDGE.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    -- Past the lab's early-game scenes.
    D.mech({ var = { 0x4084, 5 } })
    D.wait(10)
    D.scene(13, MAPS.MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 8, 10, { give = "GROVYLE", level = 30, lead = true })
    D.fastText()
    D.mech({ fixMoves = true, badges = 3 })
    D.wait(30)
    D.shot("lab")
    D.hold(K.RIGHT, 2)
    D.wait(10)
    for _ = 1, 9 do D.step(K.A, "aide") end
    -- The other two statues, with their starters in front.
    D.scene(13, MAPS.MAP_FIERY_PATH, 28, 10, { give = "COMBUSKEN", level = 30, lead = true })
    D.wait(30)
    D.hold(K.UP, 4)
    D.wait(10)
    D.hold(K.UP, 16)
    D.wait(30)
    D.shot("fiery-statue")
    D.step(K.A, "fiery-talk")
    D.step(K.A, "fiery-talk")
    D.step(K.B, nil)
    D.step(K.B, nil)
    D.scene(13, MAPS.MAP_ROUTE117, 18, 12, { give = "MARSHTOMP", level = 30, lead = true })
    D.wait(30)
    D.hold(K.UP, 2)
    D.wait(30)
    D.shot("r117-statue")
    -- Grovyle's statue (a strong Grovyle, with a Fire move for the duel).
    D.scene(13, MAPS.MAP_PETALBURG_WOODS, 14, 36, { give = "GROVYLE", level = 70, lead = true })
    D.mech({ fixMoves = true, moves = { "FIRE_BLAST", "AERIAL_ACE", "DRAGON_CLAW", "LEAF_BLADE" } })
    D.wait(30)
    D.hold(K.UP, 16)
    D.wait(30)
    D.shot("woods-statue")
    for _ = 1, 4 do D.step(K.A, "woods-talk") end
    D.fight("guardian", 60, 6)
    for _ = 1, 4 do D.step(K.A, "beaten") end
    -- Back to the aide.
    D.scene(13, MAPS.MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 8, 10)
    D.fastText()
    D.hold(K.RIGHT, 2)
    D.wait(10)
    for _ = 1, 3 do D.step(K.A, "teach") end
    D.step(K.A, "party", 150)
    D.step(K.A, "chosen", 120)
    for _ = 1, 12 do D.step(K.A, "learn") end
    D.result("ok")
end)
