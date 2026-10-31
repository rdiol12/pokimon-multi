-- Bonding: R > BOND, pick the lead, rub it with the hand, then feed it a
-- berry it likes.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(14, MAPS.MAP_OLDALE_TOWN, 10, 10, { give = "EEVEE", level = 18, lead = true, item = "PECHA_BERRY" })
    -- Four hearts already: friendship 225.
    D.write8(D.SYM.gLifeShowcase + 1, 225)
    D.write8(D.SYM.gLifeShowcase, 1)
    D.wait(10)
    D.press(K.R, 40)
    D.shot("bond-menu")
    D.press(K.DOWN, 10)
    D.press(K.A, 60)
    D.press(K.A, 60)
    D.press(K.A, 120) -- the lead
    D.wait(60)
    D.shot("bond-screen")
    -- Bring the hand onto the Pokémon and rub.
    D.hold(K.RIGHT, 18)
    D.hold(K.UP, 10)
    for i = 1, 6 do
        D.hold(K.A | K.LEFT, 14)
        D.hold(K.A | K.RIGHT, 14)
    end
    D.hold(K.A | K.LEFT, 6)
    D.shot("bond-petting")
    for i = 1, 6 do
        D.hold(K.A | K.LEFT, 14)
        D.hold(K.A | K.RIGHT, 14)
    end
    D.wait(10)
    D.shot("bond-petting-2")
    D.press(K.SELECT, 20)
    D.shot("bond-treat-pick")
    D.press(K.A, 12)
    D.shot("bond-treat")
    D.wait(60)
    D.press(K.B, 60)
    D.shot("bond-back")
end)
