-- Pokémon hotel: the bellhop on Starfall Isle takes the manager into the
-- five-star STARFALL HOTEL; STEVEN wants a Lava Cookie, another guest wants
-- something the player doesn't have; the desk collects the day's takings.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local STARS = 66 -- struct PastimeSaveData offset of hotelStars

D.run(function()
    D.boot()
    D.wait(60)
    D.pastime8(STARS, 5)
    D.scene(12, MAPS.MAP_STARFALL_ISLE, 35, 38, { item = "LAVA_COOKIE" })
    D.wait(60)
    D.shot("bellhop")
    D.talk(K.UP, "bellhop", 2)
    D.press(K.A, 60) -- yes
    D.press(K.A, 200)
    D.wait(60)
    D.shot("lobby")
    -- STEVEN, the five-star guest, sits at (6, 8).
    D.scene(12, MAPS.MAP_STARFALL_ISLE_HOTEL, 6, 7)
    D.wait(30)
    D.talk(K.DOWN, "steven", 3)
    D.press(K.A, 80) -- yes
    D.shot("tip")
    D.press(K.A, 60)
    D.shot("metal-coat")
    for _ = 1, 3 do D.press(K.B, 40) end
    D.scene(12, MAPS.MAP_STARFALL_ISLE_HOTEL, 2, 4)
    D.wait(30)
    D.talk(K.LEFT, "guest", 2)
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(30)
    D.pastimeMenu(5)
    D.wait(30)
    D.shot("guest-list")
    D.press(K.B, 60)
    D.scene(12, MAPS.MAP_STARFALL_ISLE_HOTEL, 4, 5)
    D.wait(30)
    D.talk(K.UP, "desk", 2)
    D.press(K.A, 80) -- yes, collect
    D.shot("takings")
    D.press(K.A, 60)
    D.shot("checked-out")
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.shot("empty-lobby")
end)
