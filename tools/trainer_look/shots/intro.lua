-- A new game: after the Look screen, Professor Birch shows the new look.
dofile(LOOK_LIB)
local D, K = Driver, Driver.KEY

D.run(function()
    LOOK.newGameToLookScreen()
    LOOK.change(ROW.SKIN, 3)
    LOOK.change(ROW.HAT, 3)
    LOOK.change(ROW.BAND, 5)
    LOOK.change(ROW.CLOTHES, 2)
    D.wait(30)
    D.shot("look")
    D.press(K.START, 0)
    for n = 1, 6 do
        D.wait(30)
        D.shot("birch-" .. n)
    end
end)
