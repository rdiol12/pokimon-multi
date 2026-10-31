-- Trainer cards: the list and a card.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.showcase({ level = 62, badges = P1 and 8 or 5, party = P1 and { "SCEPTILE", "SWELLOW", "MANECTRIC", "GARDEVOIR", "METAGROSS", "MILOTIC" } or { "BLAZIKEN", "ALTARIA", "AGGRON" } })
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 6 or 8, 12)
    if not P1 then D.wait(800) end
    local before = D.answers()
    D.community(3)
    D.waitFor("the list", 900, function() return D.answers() - before >= 2 end)
    D.wait(40)
    D.shot("list")
    if P1 then D.press(K.DOWN, 10) end
    before = D.answers()
    D.press(K.A, 10)
    D.waitAnswer(before)
    D.wait(60)
    D.shot("card")
    D.press(K.B, 40)
    D.shot("back")
    D.wait(600)
end)
