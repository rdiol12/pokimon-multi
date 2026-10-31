-- Second online batch, end to end. Two players in Oldale Town go through
-- the Online Plaza: guilds, titles, Wonder Trade, mail, bounties, secret
-- bases, the Battle Tower, a draft battle, its replay and the Pokédex race.
-- Run with run_ext_e2e.py.

dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver
local K = D.KEY
local EXT = D.SYM.gNetplayExtMailbox
local SYNC = INSTANCE_DIR .. "/../sync-"

local function ext8(off) return D.read8(EXT + off) end

-- struct BattlePokemon: its size and where hp sits (checked with offsetof).
local BATTLE_MON_SIZE = 140
local BATTLE_MON_HP = 42

-- The two games only meet through files next to their folders.
local function signal(name)
    local f = io.open(SYNC .. name, "w")
    f:write("1")
    f:close()
end

local function waitSignal(name, timeout)
    D.waitFor("signal " .. name, timeout or 30000, function()
        local f = io.open(SYNC .. name, "r")
        if f then f:close() return true end
        return false
    end)
end

local function idle()
    return D.inOverworld() and ext8(0xA52) == 0
end

local function waitIdle(timeout)
    D.waitFor("the field to be free", timeout or 3000, idle)
    D.wait(20)
end

-- Presses A through messages until the script ends.
local function closeText(timeout)
    D.waitFor("the messages to end", timeout or 1500, idle, function(i)
        if i % 40 == 20 then D.press(K.A, 0) end
    end)
    D.wait(20)
end

-- Presses A through messages until a full-screen menu opens.
local function untilMenu(timeout)
    D.waitFor("a menu", timeout or 900, function() return not D.inOverworld() end, function(i)
        if i % 40 == 20 then D.press(K.A, 0) end
    end)
    D.wait(90)
end

local function openPlaza(feature)
    waitIdle()
    D.write8(EXT + 0xA51, 1)
    D.waitFor("the plaza", 600, function() return not D.inOverworld() end)
    D.wait(90)
    -- Feature n sits at column (n-1)%5, row (n-1)//5.
    local col, row = (feature - 1) % 5, (feature - 1) // 5
    for _ = 1, col do D.press(K.RIGHT, 12) end
    for _ = 1, row do D.press(K.DOWN, 12) end
    D.wait(20)
end

local function pick()
    D.press(K.A, 10)
    D.waitFor("the field", 600, D.inOverworld)
end

-- Waits for the request channel to go quiet (answer in), then a bit for text.
local function waitAnswer(after)
    D.waitFor("an answer", 900, function() return ext8(0x009) == ext8(0x00A) end)
    D.wait(after or 60)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 600, function() return D.remoteCount() > 0 end)
    D.write8(EXT + 0xA51, 0xEE) -- test data: a secret base, Pokédex entries
    D.wait(30)

    -- Plaza ------------------------------------------------------------
    openPlaza(INSTANCE == 1 and 1 or 6)
    D.shot("plaza")
    D.press(K.B, 10)
    D.waitFor("the field", 600, D.inOverworld)
    waitIdle()
    signal("plaza" .. INSTANCE)
    waitSignal("plaza" .. (3 - INSTANCE))

    -- Guilds -------------------------------------------------------------
    if INSTANCE == 1 then
        openPlaza(1)
        pick()
        waitAnswer(150)
        D.shot("guild-none")
        D.press(K.A, 90)            -- Found a guild
        D.press(K.A, 90)            -- "First pick your tag..."
        D.press(K.A, 60)            -- "Then give your guild a name."
        D.waitFor("the naming screen", 600, function() return not D.inOverworld() end)
        D.wait(80)
        D.press(K.A, 30)            -- A (then the keyboard turns lowercase)
        D.press(K.LEFT, 16)         -- back from b to a
        D.press(K.RIGHT, 16) D.press(K.RIGHT, 16) D.press(K.A, 16)  -- C
        D.press(K.RIGHT, 16) D.press(K.RIGHT, 16) D.press(K.A, 16)  -- E
        D.shot("guild-tag")
        D.press(K.START, 12) D.press(K.A, 10)
        D.wait(120)
        D.waitFor("the second naming screen", 600, function() return not D.inOverworld() end)
        D.wait(80)
        -- The name: "Ace" again.
        D.press(K.A, 30)
        D.press(K.LEFT, 16)
        D.press(K.RIGHT, 16) D.press(K.RIGHT, 16) D.press(K.A, 16)
        D.press(K.RIGHT, 16) D.press(K.RIGHT, 16) D.press(K.A, 16)
        D.shot("guild-name")
        D.press(K.START, 12) D.press(K.A, 10)
        D.waitFor("the field", 600, D.inOverworld)
        waitAnswer(200)
        D.shot("guild-founded")
        D.wait(200)
        D.press(K.A, 40)
        waitIdle()
        signal("guild-made")
        waitSignal("guild-joined")
        openPlaza(1)
        pick()
        waitAnswer(60)
        D.wait(30)
        D.shot("guild-board")
        D.press(K.A, 120)           -- close the board
        D.shot("guild-menu")
        D.press(K.A, 30)            -- Guild ladder
        waitAnswer(90)
        D.shot("guild-ladder")
        D.press(K.A, 30)
        waitIdle()
    else
        waitSignal("guild-made")
        openPlaza(1)
        pick()
        waitAnswer(150)
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- Join a guild
        waitAnswer(150)
        D.shot("guild-pick")
        D.press(K.A, 120)           -- the first guild
        D.shot("guild-join-confirm")
        D.press(K.A, 30)            -- YES
        waitAnswer(150)
        D.shot("guild-joined")
        D.wait(200)
        D.press(K.A, 40)
        waitIdle()
        signal("guild-joined")
    end

    -- Titles -------------------------------------------------------------
    if INSTANCE == 1 then
        openPlaza(6)
        pick()
        waitAnswer(150)
        D.shot("title-list")
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- FOUNDER
        waitAnswer(120)
        D.shot("title-set")
        D.press(K.A, 30)
        waitIdle()
        D.wait(400)
        D.shot("own-badge")
        signal("title-set")
    else
        waitSignal("title-set")
        D.wait(400)
        D.shot("sees-badge")
    end
    signal("titles" .. INSTANCE)
    waitSignal("titles" .. (3 - INSTANCE))

    -- Wonder Trade -------------------------------------------------------
    -- Player 1 sends Mudkip and waits; player 2 sends Zigzagoon and gets
    -- the Mudkip at once; player 1 picks up the Zigzagoon on its next visit.
    local function wonderSend(slot)
        openPlaza(2)
        pick()
        waitAnswer(240)
        D.shot("wonder-intro")
        untilMenu()                 -- YES: the party menu
        for _ = 1, slot do D.press(K.DOWN, 20) end
        D.shot("wonder-party")
        D.press(K.A, 150)
        D.shot("wonder-confirm")
        D.press(K.A, 30)            -- YES
        waitAnswer(270)
    end
    if INSTANCE == 1 then
        wonderSend(0)
        D.shot("wonder-sent")
        closeText()
        signal("wonder-waiting")
        waitSignal("wonder-done")
        openPlaza(2)
        pick()
        waitAnswer(370)
        D.shot("wonder-received")
        closeText()
        D.log("wonder trade got: " .. D.partyMon(1).nickname)
    else
        waitSignal("wonder-waiting")
        wonderSend(1)
        D.shot("wonder-sent-for")
        D.press(K.A, 30)
        D.wait(250)
        D.shot("wonder-received")
        closeText()
        D.log("wonder trade got: " .. D.partyMon(1).nickname)
        signal("wonder-done")
    end
    signal("wonder" .. INSTANCE)
    waitSignal("wonder" .. (3 - INSTANCE))

    -- Bounties -----------------------------------------------------------
    -- Gary (seeded by the runner) wants a Poochyena: player 1 hands one in.
    -- Player 2 posts a bounty of its own.
    if INSTANCE == 1 then
        local money = D.money()
        openPlaza(5)
        pick()
        waitAnswer(240)
        D.shot("bounty-menu")
        D.press(K.A, 30)            -- See the board
        waitAnswer(270)
        D.shot("bounty-board")
        D.press(K.A, 120)           -- Gary's Poochyena
        D.shot("bounty-hand-in")
        untilMenu()                 -- YES: the party menu
        D.press(K.A, 150)           -- Poochyena, now first
        waitAnswer(270)
        D.shot("bounty-paid")
        closeText()
        D.log(string.format("bounty paid: ¥%d -> ¥%d", money, D.money()))
        if D.money() ~= money + 1500 then D.fail("the bounty didn't pay") end
        signal("bounty-filled")
    else
        waitSignal("bounty-filled")
        openPlaza(5)
        pick()
        waitAnswer(300)
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- Post a bounty
        D.wait(60)
        D.shot("bounty-species")
        D.press(K.A, 60)            -- the first species seen
        D.shot("bounty-reward")
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- ¥1,000
        waitAnswer(270)
        D.shot("bounty-posted")
        closeText()
        openPlaza(5)
        pick()
        waitAnswer(240)
        D.press(K.A, 30)
        waitAnswer(270)
        D.shot("bounty-board")
        D.press(K.B, 40)
        waitIdle()
    end
    signal("bounty" .. INSTANCE)
    waitSignal("bounty" .. (3 - INSTANCE))

    -- Mail and gifts -----------------------------------------------------
    -- Player 2 mails player 1 with ¥500 attached.
    if INSTANCE == 2 then
        openPlaza(9)
        pick()
        D.wait(150)
        D.shot("mail-menu")
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- Send mail
        waitAnswer(270)
        D.shot("mail-who")
        D.press(K.A, 120)           -- TEST1
        D.shot("mail-phrase")
        D.press(K.A, 60)            -- the first phrase
        D.shot("mail-gift")
        D.press(K.DOWN, 12)
        D.press(K.A, 60)            -- Money
        D.press(K.A, 30)            -- ¥500
        waitAnswer(370)
        D.shot("mail-sent")
        closeText()
        signal("mail-sent")
    else
        waitSignal("mail-sent")
        local money = D.money()
        D.wait(200)
        D.shot("mail-new")
        openPlaza(9)
        pick()
        D.wait(150)
        D.press(K.A, 30)            -- Read mail
        waitAnswer(270)
        D.shot("mail-inbox")
        D.press(K.A, 30)            -- the letter
        waitAnswer(270)
        D.wait(60)
        D.shot("mail-read")
        D.press(K.A, 60)
        D.wait(200)
        D.shot("mail-gift-got")
        closeText()
        D.log(string.format("mail gift: ¥%d -> ¥%d", money, D.money()))
        if D.money() ~= money + 500 then D.fail("the gift didn't arrive") end
    end
    signal("mail" .. INSTANCE)
    waitSignal("mail" .. (3 - INSTANCE))

    -- Secret bases -------------------------------------------------------
    -- Both share their base; player 2 battles player 1's team. Player 1
    -- goes inside player 2's base at the very end, since that leaves town.
    local function shareBase()
        openPlaza(3)
        pick()
        D.wait(150)
        D.press(K.A, 30)            -- Share my base
        waitAnswer(250)
        D.shot("base-shared")
        closeText()
    end
    local function visitBase(how)
        openPlaza(3)
        pick()
        D.wait(150)
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- Visit a base
        waitAnswer(250)
        D.shot("base-list")
        D.press(K.A, 30)            -- the first (only) one
        waitAnswer(250)
        D.shot("base-how")
        for _ = 1, how do D.press(K.DOWN, 12) end
        D.press(K.A, 30)
    end
    local function fight(name)
        D.waitFor("the battle", 1500, D.inBattle, function(i)
            if i % 40 == 20 then D.press(K.A, 0) end
        end)
        D.wait(400)
        D.shot(name)
        -- In battle the cursors are set straight in memory: FIGHT, then the
        -- second move mostly, sometimes the first (the first can be Rest,
        -- which stalls). The party menu after a faint: B, RIGHT, A, A sends out the next.
        local partyKeys = { K.B, K.RIGHT, K.A, K.A }
        local step = 0
        D.waitFor("the end of the battle", 60000, D.inOverworld, function(i)
            if i % 30 ~= 0 then return end
            step = step + 1
            if D.callback2() == D.SYM.CB2_BagMenuRun then
                D.press(K.B, 0)
            elseif D.inBattle() then
                -- The test only checks that these battles start and end, so
                -- the opponent's Pokémon go down in one hit (gBattleMons[1].hp).
                -- (Never 0 -> 1: that would revive a fainted one.)
                local hp = D.SYM.gBattleMons + BATTLE_MON_SIZE + BATTLE_MON_HP
                if D.read8(hp) > 1 or D.read8(hp + 1) > 0 then
                    D.write8(hp, 1)
                    D.write8(hp + 1, 0)
                end
                D.write8(D.SYM.gActionSelectionCursor, 0)
                D.write8(D.SYM.gMoveSelectionCursor, step % 3 == 0 and 0 or 1)
                D.press(K.A, 0)
            else
                D.press(partyKeys[(step - 1) % #partyKeys + 1], 0)
            end
        end)
        D.wait(90)
    end
    -- After a battle: B reads on and answers NO to "Keep climbing?".
    local function closeTextNo()
        D.waitFor("the messages to end", 1500, idle, function(i)
            if i % 40 == 20 then D.press(K.B, 0) end
        end)
        D.wait(20)
    end
    if INSTANCE == 1 then
        shareBase()
        signal("base-shared")
    else
        waitSignal("base-shared")
        shareBase()
        visitBase(1)                -- Battle now
        fight("base-battle")
        D.shot("base-battle-over")
        closeTextNo()
    end
    signal("bases" .. INSTANCE)
    waitSignal("bases" .. (3 - INSTANCE))

    -- Battle Tower -------------------------------------------------------
    -- Player 2 enters first: its team is stored, but nobody else's is yet.
    -- Then player 1 climbs and meets player 2's team on floor 1.
    local function tower()
        openPlaza(4)
        pick()
        waitAnswer(200)
        D.shot("tower-welcome")
        for _ = 1, 3 do D.press(K.A, 90) end
        D.wait(60)
        D.shot("tower-menu")
        D.press(K.A, 30)            -- Climb
        D.wait(90)
        D.press(K.A, 30)            -- "Your current party will be your team"
        waitAnswer(250)
    end
    if INSTANCE == 2 then
        tower()
        D.shot("tower-no-teams")
        closeText()
        signal("tower-entered")
    else
        waitSignal("tower-entered")
        tower()
        D.shot("tower-opponent")
        fight("tower-battle")
        D.shot("tower-result")
        closeTextNo()
        openPlaza(4)
        pick()
        waitAnswer(200)
        for _ = 1, 3 do D.press(K.A, 90) end
        D.wait(60)
        D.press(K.DOWN, 12)
        D.press(K.A, 30)            -- Records
        waitAnswer(250)
        D.shot("tower-records")
        closeText()
    end
    signal("tower" .. INSTANCE)
    waitSignal("tower" .. (3 - INSTANCE))

    -- Draft battle -------------------------------------------------------
    -- Both draft three from today's pool and get matched. Player 1 forfeits.
    local party = D.partyCount()
    openPlaza(8)
    pick()
    waitAnswer(250)
    D.shot("draft-pool")
    D.press(K.A, 60)                -- close the pool
    D.wait(60)
    D.press(K.A, 30)                -- YES, start
    for n = 1, 3 do
        D.wait(120)
        if n == 1 then D.shot("draft-pick") end
        D.press(K.A, 30)
    end
    D.wait(120)
    D.shot("draft-waiting")
    D.waitFor("the battle", 3000, D.inBattle)
    D.wait(300)
    D.shot("draft-battle")
    local step = 0
    D.waitFor("the end of the draft battle", 30000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        step = step + 1
        if INSTANCE == 1 then
            local cycle = { K.B, K.B, K.RIGHT, K.DOWN, K.A }
            D.press(cycle[(step - 1) % #cycle + 1], 0)
        else
            D.press(K.A, 0)
        end
    end)
    D.waitFor("the session to end", 900, function() return D.mb8(0x0D0) == D.mb8(0x0C8) end)
    D.wait(120)
    closeText()
    D.wait(200)
    if D.partyCount() ~= party then D.fail("the draft team wasn't swapped back") end
    signal("draft" .. INSTANCE)
    waitSignal("draft" .. (3 - INSTANCE))

    -- Replays ------------------------------------------------------------
    -- The draft battle uploads in the background; player 2 watches it.
    D.wait(900)
    if INSTANCE == 2 then
        openPlaza(7)
        pick()
        waitAnswer(250)
        D.shot("replay-list")
        D.press(K.A, 30)
        D.waitFor("the replay", 3000, D.inBattle)
        D.wait(400)
        D.shot("replay-playing")
        D.waitFor("the end of the replay", 30000, D.inOverworld)
        D.wait(90)
        D.shot("replay-over")
        closeText()
    end
    signal("replay" .. INSTANCE)
    waitSignal("replay" .. (3 - INSTANCE), 40000)

    -- Pokédex race -------------------------------------------------------
    openPlaza(10)
    pick()
    waitAnswer(250)
    D.wait(60)
    D.shot("dex-board")
    closeText()
    signal("dex" .. INSTANCE)
    waitSignal("dex" .. (3 - INSTANCE))

    -- Into player 2's secret base ----------------------------------------
    if INSTANCE == 1 then
        visitBase(0)                -- Go inside (a warp: still the overworld)
        D.wait(400)
        D.waitFor("the base", 900, idle)
        D.shot("base-inside")
    end
end)
