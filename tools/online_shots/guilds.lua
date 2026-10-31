-- Guild halls, clan wars and base raids: TEST2 goes to the Aces' hall, puts
-- items in the guild chest and guards the base; TEST1 follows, looks at the
-- members, takes from the chest, claims last week's clan war prize and raids
-- the Star Squad's base.
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local HALL = MAPS.MAP_ONLINE_GUILD_HALL

-- Stands at x, y facing dir and talks to whoever is there.
local function talk(x, y, dir, name)
    D.scene(13, HALL, x, y)
    D.press(dir, 20)
    D.press(K.A, 60)
    if name then D.shot(name) end
end

local function closeAll()
    D.waitFor("the field", 900, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
    D.wait(30)
end

local function goToHall(shotName)
    D.openHub(12, shotName)
    D.wait(150)
    D.shot("hall-welcome")
    D.press(K.A, 30)
    D.waitFor("the hall", 900, function() return not D.fieldLocked() and D.inOverworld() end)
    D.wait(120)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    if P1 then
        D.wait(2400) -- TEST2 goes first
        goToHall("hub")
        D.shot("hall")
        -- Clerk: the members, then the chest.
        talk(11, 6, K.UP, "clerk")
        D.press(K.A, 150) -- MEMBERS
        D.shot("members-message")
        D.press(K.A, 60)
        D.shot("members")
        closeAll()
        talk(11, 6, K.UP)
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- TAKE FROM CHEST
        D.shot("chest")
        D.press(K.A, 200)
        D.shot("took")
        closeAll()
        -- War officer: the board and last week's prize.
        talk(2, 9, K.LEFT)
        D.wait(90)
        D.shot("war-message")
        D.press(K.A, 60)
        D.shot("war-board")
        D.press(K.A, 60)
        D.wait(30)
        D.shot("war-prize")
        D.press(K.A, 200)
        D.shot("war-claimed")
        closeAll()
        -- Raid captain: raid the Star Squad.
        talk(9, 10, K.RIGHT, "raid-menu")
        D.press(K.A, 150) -- RAID A BASE
        D.shot("raid-list")
        D.press(K.A, 150)
        D.shot("raid-base")
        D.waitFor("the battle", 1200, D.inBattle, function(i)
            if i % 60 == 0 then D.press(K.A, 0) end
        end)
        D.wait(400)
        D.shot("raid-battle")
        local step, shots = 0, 1
        D.waitFor("the end of the raid", 30000, function() return D.inOverworld() and D.fieldLocked() end,
            function(i)
                if i % 40 ~= 0 then return end
                step = step + 1
                if step % 10 == 0 and shots < 3 then
                    shots = shots + 1
                    D.shot("raid-battle-" .. shots)
                end
                -- B backs out of the party menu if A opened it.
                D.press(step % 4 == 0 and K.B or K.A, 0)
            end)
        D.wait(200)
        D.shot("raid-result")
        closeAll()
        talk(2, 9, K.LEFT)
        D.wait(90)
        D.press(K.A, 60)
        D.shot("war-board-after")
        closeAll()
    else
        D.giveItem("SUPER_POTION", 8)
        goToHall()
        D.scene(13, HALL, 5, 7)
        talk(11, 6, K.UP)
        D.press(K.DOWN, 8)
        D.press(K.A, 60) -- PUT IN CHEST
        D.shot("bag-list")
        D.press(K.A, 60)
        D.shot("how-many")
        D.press(K.DOWN, 8)
        D.press(K.DOWN, 8)
        D.press(K.A, 150) -- ALL
        D.shot("put")
        closeAll()
        talk(9, 10, K.RIGHT)
        D.press(K.DOWN, 8)
        D.press(K.A, 90) -- GUARD OUR BASE
        D.shot("guard-sure")
        D.press(K.A, 200) -- YES
        D.shot("guarding")
        closeAll()
        D.scene(13, HALL, 7, 8)
        D.wait(9000)
    end
end)
