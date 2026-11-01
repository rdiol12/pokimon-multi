-- Offline (run.py ... 1): the quest board in Oldale Town without a server.
-- The game makes today's jobs, rankings and news itself; then the player
-- walks away (this used to freeze the game).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 4, 18, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {3}, 14, 2, true)
    D.wait(180)
    D.shot("01-board")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 0)
    -- Through the jobs to the "see the rankings?" question: yes.
    D.answer(true, "02-jobs")
    local n = 0
    for i = 1, 20 do
        D.wait(70)
        n = n + 1
        D.shot(string.format("03-rankings-%d", n))
        if D.read8(D.SYM.sLockFieldControls) == 0 then break end
        D.press(K.A, 0)
    end
    D.wait(60)
    D.log("locked after board: " .. D.read8(D.SYM.sLockFieldControls))
    D.hold(K.DOWN, 40)
    D.wait(10)
    D.hold(K.RIGHT, 50)
    D.wait(30)
    D.shot("04-walked")
    local locked = D.read8(D.SYM.sLockFieldControls)
    D.log("locked after walking: " .. locked)
    D.result(locked == 0 and "ok" or "frozen")
end)
