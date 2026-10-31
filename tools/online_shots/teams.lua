-- 2v2 team battle: TEST2 (team with TEST4) challenges TEST1 (team with
-- TEST3) by talking to them; the three others accept, all four games link
-- up for a multi battle, everyone hears who won, and all look at the team
-- ladder.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local HOST = INSTANCE == 2

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("another player", 900, function() return D.remoteCount() > 0 end)
    D.wait(240)
    local seq = D.mb8(0x0C8)
    if HOST then
        D.waitFor("a step west", 120, function() return D.pos() == 7 end, function() D.setKeys(K.LEFT) end)
        D.setKeys(0)
        D.wait(30)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        D.press(K.A, 150)
        D.press(K.DOWN, 15)
        D.press(K.DOWN, 15)
        D.shot("menu")
        D.press(K.A, 30) -- 2v2 team battle
        D.wait(60)
        D.shot("waiting")
    else
        D.waitFor("an invite", 3000, function() return D.mb8(0x0C8) ~= seq and D.mb8(0x0C9) == 1 end)
        D.wait(90 + 40 * INSTANCE)
        D.shot("invite")
        D.press(K.A, 40) -- YES
    end
    D.waitFor("the battle", 4000, D.inBattle)
    D.wait(500)
    D.shot("battle")
    local step, shots = 0, 1
    D.waitFor("the overworld after the battle", 90000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        step = step + 1
        if step % 15 == 0 and shots < 4 then
            shots = shots + 1
            D.shot("battle-" .. shots)
        end
        if INSTANCE == 1 and step > 14 then
            -- TEST1 gives up for their team after a few turns: Run.
            local cycle = { K.B, K.B, K.RIGHT, K.DOWN, K.A }
            D.press(cycle[(step - 1) % #cycle + 1], 0)
        else
            -- Mostly A; now and then down and back, so a fainted Pokémon's
            -- switch menu moves on to one that can still fight.
            local cycle = { K.A, K.A, K.A, K.A, K.A, K.DOWN, K.A, K.UP }
            D.press(cycle[(step - 1) % #cycle + 1], 0)
        end
    end)
    D.wait(60)
    D.shot("after-battle")
    D.notices("notice", 2000)
    D.openHub(11)
    D.wait(150)
    D.shot("team-message")
    D.press(K.A, 60)
    D.press(K.A, 60) -- TEAM LADDER
    D.wait(120)
    D.wait(90)
    D.shot("ladder-message")
    D.press(K.A, 90)
    D.shot("ladder")
    D.press(K.A, 60)
    D.wait(60)
end)
