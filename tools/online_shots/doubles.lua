-- Ranked doubles: TEST2 challenges TEST1 to a ranked double battle, TEST1
-- accepts, they battle (TEST1 runs after a few turns), everyone hears who
-- won, and both look at the doubles ladder.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    local seq = D.mb8(0x0C8)
    if P1 then
        D.waitFor("an invite", 3000, function() return D.mb8(0x0C8) ~= seq and D.mb8(0x0C9) == 1 end)
        D.wait(90)
        D.shot("invite")
        D.press(K.A, 40) -- YES
    else
        D.waitFor("a step west", 120, function() return D.pos() == 7 end, function() D.setKeys(K.LEFT) end)
        D.setKeys(0)
        D.wait(30)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        D.press(K.A, 150)
        for _ = 1, 5 do D.press(K.DOWN, 15) end
        D.shot("menu")
        D.press(K.A, 30) -- Ranked double battle
        D.shot("waiting")
    end
    D.waitFor("the battle", 3000, D.inBattle)
    D.wait(400)
    D.shot("battle")
    local step, shots = 0, 1
    D.waitFor("the overworld after the battle", 30000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        step = step + 1
        if step % 12 == 0 and shots < 4 then
            shots = shots + 1
            D.shot("battle-" .. shots)
        end
        if P1 and step > 40 then
            local cycle = { K.B, K.B, K.RIGHT, K.DOWN, K.A }
            D.press(cycle[(step - 1) % #cycle + 1], 0)
        else
            D.press(K.A, 0)
        end
    end)
    D.wait(60)
    D.shot("after-battle")
    D.notices("notice", 2000)
    D.openHub(10)
    D.wait(150)
    D.shot("ladder-message")
    D.press(K.A, 60)
    D.shot("ladder")
    D.press(K.A, 60)
    D.wait(60)
end)
