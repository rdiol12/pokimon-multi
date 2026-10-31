-- The online Battle Frontier with two players (run.py ... 2): both have
-- streaks at the Factory, Arena and Pyramid; TEST2 asks the records keeper
-- outside the Ranking Hall for each board.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.scene(13, MAPS.MAP_BATTLE_FRONTIER_OUTSIDE_EAST, P1 and 33 or 38, 16)
    D.adventure({ frontier = P1 and { 45, 30, 12 } or { 28, 41, 21 } })
    D.wait(240)
    if P1 then
        D.wait(2400)
        return
    end
    D.hold(K.UP, 16)
    D.wait(30)
    D.shot("keeper")
    local boards = { "factory", "arena", "pyramid" }
    for i, name in ipairs(boards) do
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 120)
        if i == 1 then
            D.shot("keeper-talk")
        end
        D.press(K.A, 120)
        D.press(K.A, 120)
        if i == 1 then D.shot("keeper-menu") end
        for _ = 2, i do D.press(K.DOWN, 10) end
        D.press(K.A, 180)
        D.shot("board-" .. name)
        D.press(K.B, 60)
        D.wait(30)
    end
end)
