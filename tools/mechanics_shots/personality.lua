-- Pokémon personalities: a PLAYFUL, a SHY and a GRUMPY lead following the
-- player; talking to each, and what they do while the player stands still.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function follower(species, persona, name)
    D.scene(13, MAPS.MAP_PETALBURG_CITY, 14, 15)
    D.fastText()
    D.mech({ species = species, level = 30, persona = persona })
    D.wait(30)
    -- Walk so the follower comes out behind, then turn to face it.
    D.hold(K.RIGHT, 16)
    D.hold(K.RIGHT, 16)
    D.wait(30)
    D.hold(K.LEFT, 2)
    D.wait(30)
    D.shot(name .. "-follow")
    for i = 1, 6 do
        D.press(K.A, 150)
        D.shot(name .. "-talk")
        D.press(K.A, 40)
        D.press(K.B, 40)
    end
    -- Standing still.
    D.wait(250)
    D.shot(name .. "-idle")
    D.wait(30)
    D.shot(name .. "-idle")
end

D.run(function()
    D.boot()
    D.wait(60)
    follower("PIKACHU", 0, "playful")
    follower("RALTS", 1, "shy")
    follower("MAKUHITA", 5, "brave")
    follower("SNORLAX", 3, "lazy")
    follower("SABLEYE", 7, "grumpy")
    D.result("ok")
end)
