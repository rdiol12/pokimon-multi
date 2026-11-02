-- The Hoenn multiplayer fixes (hoenn_multi.py places the stand-in players).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver
local K = D.KEY

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)

    -- 1. Walking through other players: ASH stands in the aisle, MISTY at the
    -- nurse, BROCK by the door. We walk up through all of them and heal.
    D.scene(12, MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_1F, 7, 8, {})
    D.waitFor("the other players", 900, function() return D.remoteCount() >= 3 end)
    D.wait(300)
    D.shot("01-pokecenter-crowd")
    D.hold(K.UP, 16); D.wait(4)
    D.shot("02-walk-through-brock")
    D.hold(K.UP, 32); D.wait(4)
    D.shot("03-walk-through-ash")
    D.hold(K.UP, 16); D.wait(10)
    local x, y = D.pos()
    D.log(string.format("at %d,%d after walking up", x, y))
    if y ~= 4 then D.fail("expected to reach the counter through the other players") end
    D.shot("04-at-the-nurse")
    D.press(K.A, 90)
    D.shot("05-nurse-talks")
    D.press(K.B, 30); D.press(K.B, 30); D.press(K.B, 30)
    D.wait(300)

    -- 2. Private places: GARY is in the same bedroom, DAWN in the same gym, but
    -- neither is drawn there.
    D.scene(12, MAPS.MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F, 4, 5, {})
    D.wait(200)
    D.log("bedroom: players on this map in the mailbox: " .. D.remoteCount())
    D.shot("06-bedroom-alone")
    D.scene(12, MAPS.MAP_PETALBURG_CITY_GYM, 5, 108, {})
    D.wait(200)
    D.log("gym: players on this map in the mailbox: " .. D.remoteCount())
    D.shot("07-gym-alone")

    -- 3. Friend travel: IRIS is on Route 104, where TEST1 has never been.
    D.scene(12, MAPS.MAP_OLDALE_TOWN, 6, 17, {})
    local listSeq = D.mb8(0x43C)
    D.press(K.START, 40)
    for _ = 1, 4 do D.press(K.DOWN, 10) end
    D.shot("08-start-online")
    D.press(K.A, 30) -- ONLINE
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 10)
    D.press(K.A, 10) -- FRIENDS
    D.waitFor("the friend list", 600, function() return D.mb8(0x43C) ~= listSeq end)
    D.wait(30)
    D.shot("09-friends")
    D.press(K.A, 40) -- TRAVEL
    D.shot("10-not-been-there")
    D.wait(30)
    if D.mb8(0x016) ~= MAPS.MAP_OLDALE_TOWN[1] or D.mb8(0x017) ~= MAPS.MAP_OLDALE_TOWN[2] then
        D.fail("friend travel took us somewhere we haven't been")
    end
    D.result("ok")
end)
