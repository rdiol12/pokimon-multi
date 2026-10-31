-- Battle styles: the style master in the Fallarbor Battle Tent, the
-- practice battle (AGILE and STRONG on the move menu, the turn announcement),
-- and learning them.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_FALLARBOR_TOWN_BATTLE_TENT_LOBBY, 4, 7, { give = "SCEPTILE", level = 50, lead = true })
    D.fastText()
    D.mech({ fixMoves = true, moves = { "LEAF_BLADE", "DRAGON_CLAW", "EARTHQUAKE", "X_SCISSOR" } })
    D.shot("lobby")
    D.hold(K.UP, 2)
    D.wait(10)
    for _ = 1, 4 do D.step(K.A, "intro") end
    for _ = 1, 6 do D.step(K.A, "explain") end
    D.step(K.A, "ready")
    D.step(K.A, "begin")
    D.step(K.A, "start", 200)
    D.styleFight("battle", { 1, 2 }, 45, 30)
    for _ = 1, 8 do D.step(K.A, "after") end
    D.result("ok")
end)
