-- The server boss: two players bring it down (NETPLAY_BOSS_HP=400) and claim rewards.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.showcase({ level = 90, party = P1 and { "ELECTIVIRE", "SCEPTILE", "GARCHOMP" } or { "LUDICOLO", "MAGNEZONE" }, badges = 8 })
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 6 or 8, 12)
    if not P1 then D.wait(1500) end
    local before = D.answers()
    D.community(2, "menu")
    D.waitAnswer(before)
    D.wait(40)
    D.shot("status")
    D.press(K.A, 10)
    D.winBattle("battle")
    D.wait(60)
    D.shot("damage")
    D.press(K.B, 60) -- B closes messages without talking to the follower
    -- Until it falls (gCommunityMailbox.events[COMMUNITY_EVENT_BOSS]: seq, id, alive...).
    D.waitFor("the boss to fall", 8000, function() return D.cmb8(0x3BC + 0xC0 + 2) == 0 end)
    D.wait(90)
    D.shot("later")
    D.press(K.B, 60) D.shot("later2")
    D.press(K.B, 60)
    D.wait(120)
    if not P1 then return end
    before = D.answers()
    D.community(2)
    D.waitAnswer(before)
    D.wait(40)
    D.shot("reward-status")
    D.press(K.A, 90) D.shot("reward1")
    D.press(K.A, 90) D.shot("reward2")
    D.press(K.A, 120) D.shot("reward3")
    D.press(K.A, 120) D.shot("reward4")
    D.press(K.A, 120) D.shot("reward5")
end)
