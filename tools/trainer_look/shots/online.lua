-- Two players online, each with their own look: each sees the other's
-- colours and accessories, and a change shows up for the other player.
-- Then TEST1 surfs on the Petalburg pond in their cap.
dofile(LOOK_LIB)
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local BRENDAN, MAY = 0, 1

-- {skin, hat/hair, bandana, clothes, dark, hairstyle, headwear, face}
local LOOK1 = LOOK.pack({2, 4, 5, 8, 3, 0, 1, 1})   -- tan, red hat, blue, purple; cap and glasses
local LOOK2 = LOOK.pack({4, 3, 7, 2, 4, 2, 3, 0})   -- dark, blonde, pink, orange; long hair, sun hat
local LOOK2B = LOOK.pack({1, 6, 1, 5, 6, 3, 2, 2})  -- light, blue, red, blue; spiky, beanie, shades

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    LOOK.set(P1 and LOOK1 or LOOK2, P1 and BRENDAN or MAY)
    D.wait(200)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 6 or 9, 16)
    D.hold(K.DOWN, 18)
    D.wait(20)
    D.hold(P1 and K.RIGHT or K.LEFT, 4)
    D.wait(400)
    D.shot("both")
    if P1 then
        D.wait(500)
        D.shot("other-changed")
    else
        LOOK.set(LOOK2B)
        D.wait(300)
        D.shot("changed")
        D.wait(200)
    end
    -- Running: a shot mid-stride.
    D.setKeys(K.B | (P1 and K.LEFT or K.RIGHT))
    D.wait(10)
    D.shot("running")
    D.wait(14)
    D.setKeys(0)
    D.wait(120)
    if P1 then
        D.scene(13, MAPS.MAP_PETALBURG_CITY, 22, 6, {give = "LAPRAS", level = 30, lead = true, surf = true})
        D.wait(60)
        D.hold(K.UP, 16)
        D.wait(200)
        D.shot("surfing")
        D.wait(300)
    else
        D.scene(13, MAPS.MAP_PETALBURG_CITY, 19, 11)
        D.wait(400)
        D.shot("sees-surfing")
    end
    D.wait(120)
end)
