-- Two players meet in Oldale Town, battle (player 1 forfeits), then trade
-- their lead Pokémon. Run with run_e2e.py.

dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver
local K = D.KEY

local function talkToOtherPlayer(choice)
    -- Player 2 stands at x=7 facing west, next to player 1 at x=6.
    D.press(K.A, 150) -- "What would you like to do with TEST1?" + menu
    D.shot("menu")
    for _ = 1, choice - 1 do D.press(K.DOWN, 15) end
    D.press(K.A, 30)
    D.shot("waiting-for-answer")
end

local function acceptInvite(lastSeq)
    D.waitFor("an invite", 1800, function()
        return D.mb8(0x0C8) ~= lastSeq and D.mb8(0x0C9) == 1
    end)
    D.wait(90)
    D.shot("invite")
    D.press(K.A, 40) -- finish the text
    D.press(K.A, 40) -- YES
end

TRACE_LINK = true

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 600, function() return D.remoteCount() > 0 end)
    D.wait(30)
    D.shot("together")

    -- Battle -------------------------------------------------------------
    local seq = D.mb8(0x0C8)
    if INSTANCE == 2 then
        D.waitFor("a step west", 120, function() return D.pos() == 7 end, function() D.setKeys(K.LEFT) end)
        D.setKeys(0)
        D.wait(30)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        talkToOtherPlayer(1)
    else
        acceptInvite(seq)
    end
    D.waitFor("the battle", 3000, D.inBattle)
    D.log("battle started")
    D.wait(300)
    D.shot("battle")
    -- Player 1 runs (forfeits) from the action menu; player 2 picks moves.
    local step = 0
    -- The party menu or bag can open by mistake (a different cb2), so wait for
    -- the overworld; B backs out of any submenu.
    D.waitFor("the overworld after the battle", 20000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        step = step + 1
        if INSTANCE == 1 then
            local cycle = { K.B, K.B, K.RIGHT, K.DOWN, K.A }
            D.press(cycle[(step - 1) % #cycle + 1], 0)
        else
            D.press(K.A, 0)
        end
    end)
    D.log("battle over")
    D.wait(60)
    D.shot("after-battle")
    D.waitFor("the session to end", 600, function() return not D.online() or D.mb8(0x0D0) == D.mb8(0x0C8) end)

    -- Trade --------------------------------------------------------------
    local before = D.partyMon(0)
    D.log("lead before trade: " .. before.nickname .. " (OT " .. before.ot .. ")")
    seq = D.mb8(0x0C8)
    if INSTANCE == 2 then
        D.wait(120)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        talkToOtherPlayer(2)
    else
        acceptInvite(seq)
    end
    D.waitFor("the trade menu", 3000, function() return not D.inOverworld() end)
    D.wait(400)
    D.shot("trade-menu")
    D.press(K.A, 30) -- lead Pokémon
    D.press(K.DOWN, 15) -- TRADE
    D.press(K.A, 120)
    D.shot("trade-confirm")
    -- Confirm, then sit through the animation and the save.
    D.waitFor("the trade", 6000, function()
        local mon = D.partyMon(0)
        return mon.ot ~= before.ot
    end, function(i) if i % 60 == 0 then D.press(K.A, 0) end end)
    local after = D.partyMon(0)
    D.log("lead after trade: " .. after.nickname .. " (OT " .. after.ot .. ")")
    D.wait(1500)
    D.shot("after-trade")
    -- Leave the trade menu: move to Cancel and confirm.
    D.waitFor("the overworld after trading", 6000, D.inOverworld, function(i)
        local phase = (i // 60) % 4
        if i % 60 == 0 then
            D.press(phase == 3 and K.A or (phase == 0 and K.DOWN or (phase == 1 and K.DOWN or K.A)), 0)
        end
    end)
    D.wait(60)
    D.shot("done")
end)
