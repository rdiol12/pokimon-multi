-- Two players meet in Oldale Town. Player 1 changes into Red on the Look
-- screen (START > STYLE) and player 2 sees it; player 1 sends an emote and a quick phrase. They battle (player 1 forfeits), trade their
-- lead Pokémon, then make a deal: player 2 gives both its Pokémon for player
-- 1's lead Pokémon and ¥1,000. Player 2 opens the PvP ladder and finds
-- itself in first place, then adds player 1 as a friend, and after player 1
-- walks off to Route 101, travels to it from the friend list. Player 2 claims
-- a ladder season reward. Last, both go to the Oldale Poké Mart: player 1
-- puts a Pokémon on the market board, player 2 buys it, and player 1 collects
-- the money, and the two team up for a co-op battle against two Battle
-- Frontier trainers. Player 3 starts inside the Pokémon Center and watches the battle
-- from START > ONLINE > WATCH.
-- Run with run_e2e.py.

dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver
local K = D.KEY

-- Holds a direction until pred() is true.
local function walk(desc, key, pred)
    D.waitFor(desc, 900, pred, function() D.setKeys(key) end)
    D.setKeys(0)
    D.wait(20)
end

local function remoteAt(x, y)
    for slot = 0, 7 do
        local off = 0x020 + slot * 20
        if D.mb16(off) ~= 0 and D.mb16(off + 4) == x and D.mb16(off + 6) == y then return true end
    end
    return false
end

local OLDALE, OLDALE_MART = 10, 4 -- map numbers (groups 0 and 2)

-- From Route 101 just south of Oldale Town to the market board (x=5, y=1)
-- in its Poké Mart, then opens the board.
local function goToMarketBoard(inside)
    walk("Oldale Town", D.KEY.UP, function() return D.mb8(0x017) == OLDALE end)
    -- The night-only storyteller stands at (9, 12), so turn east below her.
    walk("the street to the Mart", D.KEY.UP, function() return select(2, D.pos()) <= 13 end)
    walk("the Mart's column", D.KEY.RIGHT, function() return D.pos() >= 14 end)
    walk("the Mart's street", D.KEY.UP, function() return select(2, D.pos()) <= 8 end)
    walk("the Mart", D.KEY.UP, function() return D.mb8(0x016) == 2 and D.mb8(0x017) == OLDALE_MART end)
    D.waitFor("the Mart", 300, D.inOverworld)
    D.wait(60)
    if inside then inside() end
    walk("the aisle", D.KEY.RIGHT, function() return D.pos() >= 4 end)
    walk("the back wall", D.KEY.UP, function() return select(2, D.pos()) <= 2 end)
    walk("the spot by the board", D.KEY.RIGHT, function() return D.pos() >= 5 end)
    D.press(D.KEY.UP, 20)
end

local function openMarketBoard()
    local seq = D.mb8(0x518)
    D.press(D.KEY.A, 20)
    D.waitFor("the market board", 600, function() return D.mb8(0x518) ~= seq end)
    D.wait(30)
end

-- After a sale, purchase or collect the board saves, then reloads its page.
local function waitForMarketReload()
    local seq = D.mb8(0x518)
    D.waitFor("the board to reload", 1200, function() return D.mb8(0x518) ~= seq end)
    D.wait(30)
end

-- Rows of the menu when talking to another player (data/scripts/netplay.inc).
local TALK = { BATTLE = 1, TRADE = 2, DEAL = 3, FRIEND = 4, COOP = 5, DOUBLE = 6, MULTI = 7 }

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

-- Player 3: waits on the live battle list for the battle, then watches it.
local function spectate()
    D.waitFor("the server", 600, D.online)
    D.wait(60)
    D.press(K.START, 40)
    for _ = 1, 4 do D.press(K.DOWN, 10) end
    D.press(K.A, 30) -- ONLINE
    for _ = 1, 3 do D.press(K.DOWN, 10) end
    D.press(K.A, 60) -- WATCH
    D.shot("watch-none")
    D.waitFor("a live battle", 6000, function() return D.mb8(0x63D) > 0 end)
    D.wait(20)
    D.shot("watch-list")
    D.press(K.A, 10)
    -- The battle is listed once both players accept; the first report comes
    -- after the link-up and the battle intro, which can take over 10 seconds.
    D.waitFor("the battle view", 1800, function() return D.mb16(0x6A8) ~= 0 end)
    D.wait(10)
    D.shot("watching")
    local lastSeq, shots = D.mb8(0x6A4), 0
    D.waitFor("the battle to end", 20000, function() return D.mb8(0x63E) == 1 end, function()
        if D.mb8(0x6A4) ~= lastSeq and shots < 6 then
            lastSeq = D.mb8(0x6A4)
            shots = shots + 1
            D.shot("watching")
        end
    end)
    D.wait(10)
    D.shot("watch-ended")
    D.log(string.format("watched %d updates", shots))
    D.press(K.B, 30)
    D.press(K.B, 30)
end

D.run(function()
    D.boot()
    if INSTANCE == 3 then return spectate() end
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
        D.press(K.A, 40) -- STYLE: the Look screen, cursor on BODY
        D.waitFor("the Look screen", 600, function() return D.callback2() ~= D.SYM.CB2_Overworld end)
        D.wait(60)
        D.press(K.RIGHT, 15) -- MAY
        D.press(K.RIGHT, 30) -- RED
        D.shot("style-menu")
        D.press(K.START, 0) -- save and go back to the start menu
        D.waitFor("the overworld", 600, D.inOverworld)
        -- Close the start menu. It takes input once the field has faded
        -- back in, which takes longer the more people are on the map.
        D.wait(40)
        for _ = 1, 20 do
            D.press(K.B, 20)
            if emu:read8(D.SYM.sLockFieldControls) == 0 then break end
        end
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

    -- Emotes: player 1 sends LOVE, then the phrase GG!; player 2 sees both.
    if INSTANCE == 1 then
        D.wait(30)
        D.press(K.L, 30)
        D.press(K.RIGHT, 20) -- LOVE
        D.shot("emote-menu")
        D.press(K.A, 16)
        D.shot("emote-love")
        D.wait(120)
        D.press(K.L, 30)
        D.press(K.RIGHT, 12)
        for _ = 1, 4 do D.press(K.DOWN, 12) end -- GG!
        D.press(K.A, 20)
        D.shot("chat-gg")
        D.wait(200)
    else
        D.waitFor("an emote", 900, function() return D.mb8(0x0C9) == 6 and D.mb8(0x0CE) == 1 end)
        D.wait(16) -- the bubble has bounced into place
        D.shot("sees-love")
        D.waitFor("a phrase", 900, function() return D.mb8(0x0C9) == 6 and D.mb8(0x0CE) == 9 end)
        D.wait(20)
        D.shot("sees-gg")
        D.wait(150)
    end

    -- Battle -------------------------------------------------------------
    local seq = D.mb8(0x0C8)
    if INSTANCE == 2 then
        D.waitFor("a step west", 120, function() return D.pos() == 7 end, function() D.setKeys(K.LEFT) end)
        D.setKeys(0)
        D.wait(30)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        talkToOtherPlayer(TALK.BATTLE)
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
        talkToOtherPlayer(TALK.TRADE)
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
        talkToOtherPlayer(TALK.DEAL)
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
    -- START: POKéMON, BAG, PLAYER, STYLE, ONLINE...; ONLINE: LADDER, SEASON, FRIENDS.
    if INSTANCE == 2 then
        local ladderSeq = D.mb8(0x388)
        D.wait(60)
        D.press(K.START, 40)
        for _ = 1, 4 do D.press(K.DOWN, 10) end
        D.press(K.A, 30) -- ONLINE
        D.shot("online-menu")
        D.press(K.A, 10) -- LADDER
        D.waitFor("the ladder", 600, function() return D.mb8(0x388) ~= ladderSeq end)
        D.wait(30)
        D.shot("ladder")
        local rank, count = D.mb16(0x38A), D.mb8(0x389)
        D.log(string.format("ladder: %d players, my rank %d, rating %d", count, rank, D.mb16(0x38C + 8)))
        if rank ~= 1 or count ~= 2 then D.fail("expected first of two on the ladder") end
        D.press(K.B, 30)
    end

    -- Friends: player 2 adds player 1, player 1 walks to Route 101, and
    -- player 2 finds it on the friend list and travels there. ---------------
    if INSTANCE == 2 then
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        talkToOtherPlayer(TALK.FRIEND)
        D.wait(60)
        D.shot("friend-added")
        D.waitFor("player 1 to leave", 3000, function() return D.remoteCount() == 0 end)
        -- Two pages of text; a spare A then faces empty grass.
        for _ = 1, 3 do D.press(K.A, 40) end
        local listSeq = D.mb8(0x43C)
        D.wait(120)
        D.press(K.START, 40) -- the cursor is still on ONLINE
        D.shot("start-online")
        D.press(K.A, 30)
        D.press(K.DOWN, 10)
        D.press(K.DOWN, 10)
        D.press(K.A, 10) -- FRIENDS
        D.waitFor("the friend list", 600, function() return D.mb8(0x43C) ~= listSeq end)
        D.wait(30)
        D.shot("friends")
        if D.mb8(0x43D) ~= 1 or D.mb16(0x440 + 8) == 0 then D.fail("expected one online friend") end
        local group, num = D.mb8(0x440 + 10), D.mb8(0x440 + 11)
        D.press(K.A, 10) -- TRAVEL
        D.waitFor("the trip", 600, function() return D.mb8(0x016) == group and D.mb8(0x017) == num and D.inOverworld() end)
        D.waitFor("player 1", 600, function() return D.remoteCount() > 0 end)
        D.wait(60)
        D.shot("traveled")

        -- Season: claim the reward run_e2e.py left for an earlier season.
        local seasonSeq, count = D.mb8(0x508), D.partyCount()
        D.press(K.START, 40)
        D.press(K.A, 30) -- ONLINE
        D.press(K.UP, 10)
        D.press(K.A, 10) -- SEASON
        D.waitFor("the season", 600, function() return D.mb8(0x508) ~= seasonSeq end)
        D.wait(30)
        D.shot("season")
        D.log(string.format("season %d, %d days left, reward tier %d", D.mb16(0x50A), D.mb16(0x50C), D.mb8(0x509)))
        if D.mb8(0x509) ~= 2 then D.fail("expected a GOLD reward to claim") end
        D.press(K.A, 10) -- CLAIM
        D.waitFor("the reward", 600, function() return D.partyCount() ~= count end)
        D.wait(10)
        D.shot("season-saving")
        D.wait(900)
        D.shot("season-claimed")
        D.press(K.B, 30)
        D.waitFor("the overworld after the season", 600, D.inOverworld)
    else
        D.wait(400)
        local num = D.mb8(0x017)
        -- Player 2 is to the east and trees to the south: step down, then
        -- take the path at x = 9.
        D.press(K.DOWN, 30)
        D.waitFor("the path south", 600, function() return D.pos() >= 9 end, function() D.setKeys(K.RIGHT) end)
        D.setKeys(0)
        D.wait(20)
        D.waitFor("Route 101", 1200, function() return D.mb8(0x017) ~= num end, function() D.setKeys(K.DOWN) end)
        D.setKeys(0)
        D.press(K.DOWN, 30)
        D.press(K.DOWN, 30)
        D.log(string.format("on map %d.%d", D.mb8(0x016), D.mb8(0x017)))
        D.waitFor("player 2", 3000, function() return D.remoteCount() > 0 end)
        D.wait(90)
        D.shot("friend-arrived")
    end

    -- Market board: player 1 lists its second Pokémon for ¥1000, player 2
    -- buys it, and player 1 collects the money. ---------------------------
    if INSTANCE == 1 then
        D.wait(200)
        goToMarketBoard()
        D.shot("market-mart")
        openMarketBoard()
        D.shot("market-board")
        local count = D.partyCount()
        D.press(K.SELECT, 150) -- party menu
        D.press(K.DOWN, 20)
        D.press(K.A, 200) -- back on the board, setting a price
        D.shot("market-price")
        D.press(K.A, 10) -- list it for ¥1000
        D.waitFor("the listing", 600, function() return D.partyCount() ~= count end)
        waitForMarketReload()
        D.shot("market-listed")
        D.press(K.B, 30)
        -- Out of player 2's way, and wait for it to buy and step back. Our
        -- follower trails one tile behind, so step down by the counter to
        -- pull it out of the aisle, and player 2 steps back two tiles so its
        -- follower clears the board.
        walk("out of the way", K.LEFT, function() return D.pos() <= 3 end)
        walk("out of the aisle", K.DOWN, function() return select(2, D.pos()) >= 3 end)
        D.waitFor("player 2 at the board", 4000, function() return remoteAt(5, 2) end)
        D.waitFor("player 2 to step back", 3000, function() return remoteAt(5, 4) end)
        local money = D.money()
        walk("back to the aisle", K.UP, function() return select(2, D.pos()) <= 2 end)
        walk("back to the board", K.RIGHT, function() return D.pos() >= 5 end)
        D.press(K.UP, 20)
        openMarketBoard() -- collects first, then shows the board
        D.shot("market-collected")
        D.log(string.format("collected: ¥%d -> ¥%d", money, D.money()))
        if D.money() ~= money + 1000 then D.fail("expected ¥1000 from the sale") end
        D.press(K.B, 30)
    else
        D.waitFor("player 1 to go into the Mart", 3000, function() return D.remoteCount() == 0 end)
        D.wait(300)
        goToMarketBoard(function()
            D.waitFor("player 1 out of the way", 3000, function() return remoteAt(3, 3) end)
        end)
        openMarketBoard()
        D.shot("market-for-sale")
        if D.mb8(0x519) < 1 then D.fail("expected a Pokémon for sale") end
        local money, count = D.money(), D.partyCount()
        D.press(K.A, 30) -- the newest listing
        D.shot("market-confirm")
        D.press(K.A, 10) -- buy
        D.waitFor("the purchase", 600, function() return D.partyCount() ~= count end)
        waitForMarketReload()
        D.shot("market-bought")
        D.log(string.format("bought: ¥%d -> ¥%d, %d -> %d Pokémon", money, D.money(), count, D.partyCount()))
        if D.money() ~= money - 1000 then D.fail("expected to pay ¥1000") end
        D.press(K.B, 30)
        walk("away from the board", K.DOWN, function() return select(2, D.pos()) >= 4 end)
        D.wait(600)
    end

    -- Co-op: player 2 (below player 1, by the board) teams up with player 1
    -- against two trainers; both pick the first move and target each turn.
    seq = D.mb8(0x0C8)
    if INSTANCE == 2 then
        D.wait(200)
        -- Already facing player 1; another UP would walk onto their tile now
        -- that players don't block each other.
        walk("back below player 1", K.UP, function() return select(2, D.pos()) <= 3 end)
        D.wait(20)
        talkToOtherPlayer(TALK.COOP)
    else
        acceptInvite(seq)
    end
    D.waitFor("the co-op battle", 3000, D.inBattle)
    D.log(string.format("co-op battle started; opponents' levels %d and %d",
        D.partyLevel(1, 0), D.partyLevel(3, 0)))
    D.wait(400)
    D.shot("coop-battle")
    local turns = 0
    D.waitFor("the end of the co-op battle", 40000, D.inOverworld, function(i)
        if i % 30 ~= 0 then return end
        -- A picks the first move and target. After a faint the game leaves
        -- the battle screen for the party menu: step down to the next
        -- Pokémon, pick it, SHIFT, and back out if it can't fight.
        if not D.inBattle() then
            local cycle = { K.DOWN, K.A, K.A, K.B }
            D.press(cycle[(i // 30) % #cycle + 1], 0)
        else
            D.press(K.A, 0)
        end
        if i % 900 == 0 and turns < 4 then
            turns = turns + 1
            D.shot("coop-battle")
        end
    end)
    D.log("co-op battle over")
    D.wait(120)
    D.shot("after-coop")
end)
