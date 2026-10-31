-- Dungeon: the spelunker by Meteor Falls sends the player down into the
-- Rift Below; each floor is dug out anew, with item balls and a ladder
-- further down; deep floors have bigger caves and stronger Pokémon.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function toLadder(floor, name)
    D.adventure({ dungeonFloor = floor })
    local x, y = D.dungeonLadder()
    D.scene(11, MAPS.MAP_ADVENTURE_DUNGEON, x, y + 1)
    D.wait(30)
    D.shot(name)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(11, MAPS.MAP_ROUTE114, 10, 65)
    D.wait(60)
    D.shot("route114")
    D.talk(K.UP, "spelunker", 5)
    D.wait(40)
    D.shot("spelunker-menu")
    D.press(K.A, 60) -- CLIMB DOWN
    D.shot("spelunker-dive")
    D.press(K.A, 40)
    D.wait(200)
    D.waitFor("the rift", 600, function() return D.inOverworld() end)
    D.wait(60)
    D.shot("b1f")
    toLadder(1, "b1f-ladder")
    D.talk(K.UP, "ladder", 1)
    D.wait(20)
    D.press(K.A, 40) -- YES
    D.waitFor("B2F", 600, function() return D.inOverworld() end)
    D.wait(120)
    D.shot("b2f")
    for _, floor in ipairs({ 5, 12, 20 }) do
        D.adventure({ dungeonFloor = floor })
        D.scene(11, MAPS.MAP_ADVENTURE_DUNGEON, 16, 27)
        D.wait(30)
        D.shot("b" .. floor .. "f")
        D.dumpMap("b" .. floor .. "f")
        toLadder(floor, "b" .. floor .. "f-ladder")
    end
    -- Back up and ask about the board.
    D.scene(11, MAPS.MAP_ROUTE114, 10, 65)
    D.wait(90)
    D.talk(K.UP, "spelunker-again", 1)
    D.wait(40)
    D.press(K.DOWN, 10)
    D.press(K.A, 60) -- DEEPEST DIVERS
    D.wait(60)
    D.shot("board")
    D.press(K.B, 40)
end)
