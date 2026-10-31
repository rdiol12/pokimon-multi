-- Pokémon fashion. The stylist at Slateport's Fan Club: buy the RED BOW,
-- dress the lead Pokémon in it, and today's theme (CUTE) wins the prize.
-- Then the follower out in Slateport in every accessory, and R > PASTIMES >
-- FASHION.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local OWNED, WORN = 248, 249 -- struct PastimeSaveData offsets

local function pages(name, n)
    for i = 1, n do
        D.wait(60)
        D.shot(name .. "-" .. i)
        D.press(K.A, 0)
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_SLATEPORT_CITY_POKEMON_FAN_CLUB, 4, 5)
    D.wait(120)
    D.hold(K.UP, 18)
    D.wait(20)
    D.talk(K.UP, "welcome", 2)
    D.wait(40)
    D.shot("menu")
    D.press(K.A, 40) -- BUY
    D.wait(30)
    D.shot("buy")
    D.press(K.A, 40) -- RED BOW
    pages("bought", 1)
    D.wait(30)
    D.press(K.DOWN, 10)
    D.press(K.A, 40) -- DRESS UP
    D.wait(30)
    D.shot("dress")
    D.press(K.A, 40) -- RED BOW
    pages("dressed", 1)
    D.wait(30)
    D.press(K.DOWN, 10)
    D.press(K.DOWN, 10)
    D.press(K.A, 40) -- TODAY'S LOOK
    pages("judge", 6)
    for _ = 1, 4 do D.press(K.B, 50) end
    -- Out in Slateport, in every accessory.
    D.pastime8(OWNED, 0xFF)
    D.scene(13, MAPS.MAP_SLATEPORT_CITY, 19, 20)
    D.wait(200)
    D.hold(K.DOWN, 50)
    D.wait(30)
    D.hold(K.LEFT, 18) -- the follower steps up beside the player
    D.wait(30)
    for a = 1, 8 do
        D.pastime8(WORN, a)
        D.wait(20)
        D.shot("wear-" .. a)
    end
    D.pastime8(WORN, 6)
    D.hold(K.RIGHT, 34)
    D.wait(20)
    D.shot("walk-right")
    D.hold(K.UP, 34)
    D.wait(20)
    D.shot("walk-up")
    D.pastimeMenu(27)
    D.wait(30)
    D.shot("fashion")
    D.press(K.B, 60)
end)
