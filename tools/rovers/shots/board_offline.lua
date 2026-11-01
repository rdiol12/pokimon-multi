-- Offline: read the quest board in Oldale Town, then walk away.
-- Before the fix the player stayed frozen once the first message closed.
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
    local n = 0
    -- Press A through the messages until the script ends (or 15 tries).
    for i = 1, 15 do
        D.wait(60)
        n = n + 1
        D.shot(string.format("02-page-%d", n))
        if D.read8(D.SYM.sLockFieldControls) == 0 then break end
        D.press(K.A, 0)
    end
    D.wait(60)
    D.log("locked after board: " .. D.read8(D.SYM.sLockFieldControls))
    D.shot("03-after")
    D.hold(K.DOWN, 40)
    D.wait(10)
    D.hold(K.RIGHT, 50)
    D.wait(30)
    D.shot("04-walked")
    local locked = D.read8(D.SYM.sLockFieldControls)
    D.log("locked after walking: " .. locked)
    D.result(locked == 0 and "ok" or "frozen")
end)
