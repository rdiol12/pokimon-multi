-- Offline: a rover whose lead is a fusion it bred at the Day Care (Wingull
-- + Pikachu = Wingchu). The player talks to it, sees its team and battles it.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.summer()
    D.scene(12, MAPS.MAP_ROUTE101, 10, 9, { give = "MUDKIP", level = 30, lead = true })
    D.roverLead("WINGULL", "PIKACHU")
    D.rovers(1, {12}, 20, 2, true, 1)
    D.answer(true, "fusion-talk")
    D.press(K.A, 90)
    D.waitFor("the battle", 1500, D.inBattle, function(i)
        if i % 40 == 0 then D.press(K.A, 0) end
    end)
    for i = 1, 14 do
        D.wait(40)
        D.shot("fusion-battle-" .. i)
        if i > 6 then D.press(K.A, 0) end
    end
    D.result("ok")
end)
