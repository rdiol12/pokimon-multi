-- Online (run.py ... 2): player 1 reads the quest board in Oldale Town,
-- then talks to a rover on Route 102 and battles it. Player 2 just waits.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function film(name, count, every, key)
    for i = 1, count do
        D.wait(every)
        D.shot(name .. "-" .. i)
        if key then D.press(key, 0) end
    end
end

D.run(function()
    D.boot()
    D.summer()
    if INSTANCE ~= 1 then
        D.scene(12, MAPS.MAP_LITTLEROOT_TOWN, 10, 10)
        D.wait(9000)
        D.result("ok")
        return
    end
    D.waitFor("online", 3000, D.online)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 4, 18, { give = "MUDKIP", level = 20, lead = true })
    D.wait(120)
    D.shot("board-town")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 30)
    film("board", 24, 50, K.A)
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "MUDKIP", level = 20, lead = true })
    D.rovers(1, {3}, 14, 2, false, 1)
    film("talk", 40, 50, K.A)
    D.result("ok")
end)
