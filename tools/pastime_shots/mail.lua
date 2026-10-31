-- Mail courier routes: with a PIDGEOTTO in the party, take today's letters
-- from the postmaster (the bird flies one over), check R > PASTIMES >
-- MAIL ROUTE, hand out the other two and get paid (third route: a stamp).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local ROUTES = 52 -- struct PastimeSaveData offset of mailRoutes

-- Per town (src/pastime_mail.c): the postmaster, then the recipients.
local TOWNS = {
    [0] = { MAPS.MAP_RUSTBORO_CITY, { 20, 35 }, { 24, 39 }, { 29, 23 }, { 14, 46 } },
    [1] = { MAPS.MAP_SLATEPORT_CITY, { 19, 12 }, { 6, 15 }, { 18, 38 }, { 4, 45 } },
    [2] = { MAPS.MAP_MAUVILLE_CITY, { 26, 11 }, { 32, 16 }, { 16, 7 }, { 11, 8 } },
    [3] = { MAPS.MAP_LILYCOVE_CITY, { 23, 16 }, { 34, 21 }, { 15, 21 }, { 43, 26 } },
}

-- Stands below spot n of the town and faces it.
local function visit(town, n, extra)
    local t = TOWNS[town]
    D.scene(12, t[1], t[n][1], t[n][2] + 1, extra)
    D.wait(100)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 6, 18) -- sets the clock
    D.wait(60)
    local days = D.read16(D.SYM.gLocalTime)
    local town = days % 4
    D.log("today's town " .. town)
    D.pastime16(ROUTES, 2)
    visit(town, 2, { give = "PIDGEOTTO", level = 25, lead = true })
    D.shot("postmaster")
    D.talk(K.UP, "hello", 2)
    D.press(K.A, 200) -- yes
    D.shot("got-letters")
    D.press(K.A, 60)
    D.shot("bird")
    D.press(K.A, 60)
    D.shot("bird-2")
    D.press(K.A, 60)
    D.press(K.A, 60)
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(3)
    D.wait(30)
    D.shot("route-list")
    D.press(K.B, 60)

    visit(town, 3)
    D.shot("recipient")
    D.talk(K.UP, "letter-1", 2)
    for _ = 1, 3 do D.press(K.B, 40) end
    visit(town, 4)
    D.talk(K.UP, "letter-2", 3)
    for _ = 1, 3 do D.press(K.B, 40) end

    visit(town, 2)
    D.talk(K.UP, "complete", 2)
    D.wait(30)
    D.shot("stamp")
    D.press(K.A, 200)
    D.shot("stamp-item")
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(3)
    D.wait(30)
    D.shot("route-list-done")
    D.press(K.B, 60)
end)
