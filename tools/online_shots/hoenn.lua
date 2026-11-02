-- Original Hoenn for many players (docs/hoenn_multi.md), with hoenn_seed.py:
-- milestone notices, gym record boards and WATCH LIVE at a gym statue,
-- HUB > HOENN RECORDS, the wider Pokémon Center and its second nurse, the
-- quick-match desk upstairs, and TEAM UP with a partner in a legendary battle.
dofile(INSTANCE_DIR .. "/../lib.lua")
local D, K = Driver, Driver.KEY

local HUB_RECORDS, HUB_TEAMUP = 17, 18

local function clearField()
    D.waitFor("the field", 1200, function() return not D.fieldLocked() end, function(i)
        if i % 30 == 0 then D.press(K.B, 0) end
    end)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if INSTANCE == 2 then
        -- TEST2 hears about TEST1's badge.
        D.scene(12, MAPS.MAP_OLDALE_TOWN, 8, 17, {})
        D.waitNotice("p2-milestone-notice", 20000)
        D.wait(20000)
        D.result("ok")
        return
    end
    D.scene(12, MAPS.MAP_RUSTBORO_CITY_GYM, 2, 19, {})
    D.wait(200) -- the first report after joining: no badges yet
    D.notices("00-other-notice", 900) -- the guide's news, if any
    D.badges(1)
    -- MAY earns two badges a moment later; everyone online hears it.
    D.notices("01-notice", 2400)
    clearField()

    -- The left statue: its text, then the online screen.
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 90)
    D.shot("02-statue")
    D.press(K.A, 60)
    D.press(K.A, 90)
    D.shot("03-statue-menu")
    D.press(K.A, 200) -- RECORDS
    D.shot("04-records-message")
    D.press(K.A, 120)
    D.shot("05-gym-records")
    D.press(K.B, 60)
    clearField()

    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 90)
    D.press(K.A, 60)
    D.press(K.A, 90)
    D.press(K.DOWN, 10)
    D.press(K.A, 200) -- WATCH LIVE
    D.shot("06-watch-list")
    D.press(K.A, 200)
    D.shot("07-watching-lance")
    D.wait(400)
    D.shot("08-watching-lance-2")
    D.press(K.B, 60)
    D.press(K.B, 60)
    clearField()

    D.openHub(HUB_RECORDS)
    D.wait(200)
    D.shot("09-hoenn-records")
    D.press(K.B, 60)
    clearField()

    -- The wider Pokémon Center and its second nurse.
    D.scene(12, MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_1F, 13, 3, {})
    D.shot("10-pokecenter-east-wing")
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.shot("11-second-nurse")
    D.press(K.A, 60) -- YES
    D.wait(200)
    D.shot("12-second-nurse-heals")
    for _ = 1, 4 do D.press(K.A, 60) end
    clearField()

    -- The quick-match desk upstairs.
    D.scene(12, MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_2F, 2, 4, {})
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 90)
    D.shot("13-desk")
    D.press(K.A, 200) -- A BATTLE
    D.shot("14-desk-found-gary")
    D.press(K.A, 60)
    D.wait(30)
    D.shot("15-desk-challenge")
    D.wait(6000)
    D.shot("16-desk-gary-declined")
    D.press(K.A, 60)
    clearField()

    -- TEAM UP: IRIS lent her team. Bring it along, then meet GROUDON.
    D.scene(12, MAPS.MAP_ROUTE101, 10, 10, {})
    D.openHub(HUB_TEAMUP)
    D.wait(250)
    D.shot("17-teamup-view")
    for _ = 1, 3 do D.press(K.A, 120) end
    D.shot("18-teamup-got")
    D.press(K.A, 60)
    clearField()
    D.wildBattle("GROUDON", 45)
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(300)
    D.shot("19-groudon-with-iris")
    for i = 1, 6 do
        D.press(K.A, 200)
        D.shot("20-groudon-" .. i)
    end
    D.result("ok")
end)
