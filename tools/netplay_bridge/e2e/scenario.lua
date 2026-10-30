-- Two players meet in Oldale Town. Player 1 changes into Red in the Style
-- menu and player 2 sees it. They battle (player 1 forfeits), trade their
-- lead Pokémon, then make a deal: player 2 gives both its Pokémon for player
-- 1's lead Pokémon and ¥1,000. Last, player 2 opens the PvP ladder and finds
-- itself in first place.
-- Run with run_e2e.py.

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
    -- Text is fast, so YES/NO is already up; a second A is harmless after a
    -- battle or trade invite but would press ADD on the deal screen.
    D.press(K.A, 40) -- YES
end

TRACE_LINK = true

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 600, function() return D.remoteCount() > 0 end)
    D.wait(30)
    D.shot("together")

    -- Style --------------------------------------------------------------
    -- START menu: POKéMON, BAG, PLAYER, STYLE, LADDER, SAVE, OPTION, EXIT.
    if INSTANCE == 1 then
        local gfx = D.mb16(0x01E)
        D.press(K.START, 40)
        D.shot("start-menu")
        for _ = 1, 3 do D.press(K.DOWN, 10) end
        D.press(K.A, 40) -- STYLE
        D.press(K.DOWN, 15) -- MAY
        D.press(K.DOWN, 30) -- RED
        D.shot("style-menu")
        D.press(K.A, 40)
        if D.mb16(0x01E) == gfx then D.fail("the style didn't change") end
        D.wait(30)
        D.shot("style-red")
    else
        local gfx = D.mb16(0x020 + 10)
        D.waitFor("player 1's new look", 900, function() return D.mb16(0x020 + 10) ~= gfx end)
        -- The menu previews each look in turn; wait for the one kept.
        local last, still = nil, 0
        D.waitFor("player 1 to pick", 900, function()
            local now = D.mb16(0x020 + 10)
            still = now == last and still + 1 or 0
            last = now
            return still >= 90
        end)
        D.shot("sees-red")
    end

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
    D.shot("after-trade-overworld")
    D.waitFor("the trade session to end", 600, function() return D.mb8(0x0D0) == D.mb8(0x0C8) end)

    -- Deal: player 2 offers both its Pokémon; player 1 offers its lead and
    -- ¥1,000. Both accept.
    local money, count = D.money(), D.partyCount()
    D.log(string.format("before the deal: ¥%d, %d Pokémon", money, count))
    seq = D.mb8(0x0C8)
    if INSTANCE == 2 then
        D.wait(120)
        -- The player must be able to move again after a trade.
        D.press(K.RIGHT, 40)
        if select(3, D.pos()) ~= D.DIR.EAST then D.fail("can't turn after the trade") end
        D.press(K.LEFT, 40)
        talkToOtherPlayer(3)
    else
        acceptInvite(seq)
    end
    D.wait(200)
    D.shot("deal-screen")
    local function addMon(downs)
        D.press(K.A, 150) -- ADD: party menu
        for _ = 1, downs do D.press(K.DOWN, 20) end
        D.press(K.A, 200) -- back on the deal screen
    end
    if INSTANCE == 2 then
        addMon(0)
        D.shot("deal-party")
        addMon(1)
        D.shot("deal-two-mons")
        D.wait(200)
        for _ = 1, 3 do D.press(K.RIGHT, 15) end
    else
        addMon(0)
        D.press(K.RIGHT, 15)
        D.press(K.RIGHT, 15)
        D.press(K.A, 30) -- MONEY
        D.press(K.UP, 20) -- +1000
        D.shot("deal-money")
        D.press(K.A, 30)
        D.wait(400)
        D.press(K.RIGHT, 15)
    end
    D.wait(100)
    D.shot("deal-offers")
    D.press(K.A, 60) -- ACCEPT
    D.shot("deal-accepted")
    D.waitFor("the deal", 1800, function() return D.partyCount() ~= count end)
    D.wait(500) -- the save, then the fanfare and "The deal is done!"
    D.shot("deal-done")
    D.press(K.A, 60)
    D.log(string.format("after the deal: ¥%d, %d Pokémon", D.money(), D.partyCount()))
    local wantMoney = INSTANCE == 1 and money - 1000 or money + 1000
    local wantCount = INSTANCE == 1 and count + 1 or count - 1
    if D.money() ~= wantMoney or D.partyCount() ~= wantCount then
        D.fail(string.format("expected ¥%d and %d Pokémon", wantMoney, wantCount))
    end
    if INSTANCE == 1 then
        local bought = D.partyMon(wantCount - 1)
        D.log("bought: " .. bought.nickname .. " (OT " .. bought.ot .. ")")
    end
    D.waitFor("the overworld after the deal", 600, D.inOverworld)
    D.shot("done")

    -- Ladder: player 2 won the battle, so it is first. -----------------------
    if INSTANCE == 2 then
        local ladderSeq = D.mb8(0x388)
        D.wait(60)
        D.press(K.START, 40)
        for _ = 1, 4 do D.press(K.DOWN, 10) end
        D.press(K.A, 10) -- LADDER
        D.waitFor("the ladder", 600, function() return D.mb8(0x388) ~= ladderSeq end)
        D.wait(30)
        D.shot("ladder")
        local rank, count = D.mb16(0x38A), D.mb8(0x389)
        D.log(string.format("ladder: %d players, my rank %d, rating %d", count, rank, D.mb16(0x38C + 8)))
        if rank ~= 1 or count ~= 2 then D.fail("expected first of two on the ladder") end
        D.press(K.B, 30)
    end
end)
