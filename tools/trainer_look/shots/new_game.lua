-- A new game: Professor Birch, then the Look screen. Changes a few parts,
-- rolls random looks, picks May, and goes on into the game.
dofile(LOOK_LIB)
local D, K = Driver, Driver.KEY

D.run(function()
    LOOK.newGameToLookScreen()
    D.shot("look-screen")
    LOOK.change(ROW.SKIN, 3)
    LOOK.change(ROW.HAT, 3)
    LOOK.change(ROW.BAND, 5)
    LOOK.change(ROW.CLOTHES, 2)
    D.wait(30)
    D.shot("colours")
    LOOK.change(ROW.HAIRSTYLE, 3)  -- SPIKY
    LOOK.change(ROW.HEADWEAR, 1)   -- CAP
    LOOK.change(ROW.FACE, 2)       -- SHADES
    D.wait(20)
    D.shot("accessories")
    D.wait(70)
    D.shot("accessories-west")
    D.wait(70)
    D.shot("accessories-north")
    for n = 1, 4 do
        D.press(K.SELECT, 40)
        D.shot("random-" .. n)
    end
    LOOK.change(ROW.BODY, 1)       -- MAY
    D.wait(30)
    D.shot("may")
    LOOK.change(ROW.HEADWEAR, 3)   -- BOW
    LOOK.change(ROW.HAIRSTYLE, 1)  -- PONYTAIL
    D.wait(30)
    D.shot("may-bow")
    D.press(K.SELECT, 40)
    D.shot("may-random")
    D.press(K.START, 30)
    D.waitFor("the overworld", 6000, D.inOverworld, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
    D.wait(200)
    D.shot("truck")
end)
