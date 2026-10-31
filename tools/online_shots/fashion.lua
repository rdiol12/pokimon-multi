-- Pokémon fashion online: TEST1's follower wears the GOLD CROWN and TEST2's
-- the STRAW HAT (src/pastime_fashion.c); each sees what the other's wears.
-- Then TEST2 changes to SUNGLASSES, and TEST1 sees that too.
dofile(OUT_DIR .. "/lib.lua")
dofile(OUT_DIR .. "/fashion_addr.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
-- struct PastimeSaveData (include/pastime_save.h): who wears what.
local PERSONALITY, WORN = 244, 249
local CROWN, STRAW_HAT, SUNGLASSES = 6, 2, 5 -- accessory + 1

local function wear(accessory)
    local save = D.read32(PASTIME_SAVE_PTR)
    local personality = D.read32(PLAYER_PARTY) -- the lead's
    for i = 0, 3 do D.write8(save + PERSONALITY + i, (personality >> (i * 8)) & 0xFF) end
    D.write8(save + WORN, accessory)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.wait(300)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 5 or 9, 16,
        {give = P1 and "PIKACHU" or "EEVEE", level = 20, lead = true})
    wear(P1 and CROWN or STRAW_HAT)
    -- A step, so the follower comes out beside the player.
    D.hold(K.DOWN, 18)
    D.wait(30)
    D.hold(P1 and K.RIGHT or K.LEFT, 18)
    D.wait(400)
    D.shot("both")
    if P1 then
        D.wait(600)
        D.shot("changed")
    else
        wear(SUNGLASSES)
        D.hold(K.DOWN, 18)
        D.wait(30)
        D.hold(K.UP, 18)
        D.wait(300)
        D.shot("sunglasses")
        D.wait(300)
    end
    D.wait(120)
end)
