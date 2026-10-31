-- START > STYLE from the field, then a run of random looks for both bodies
-- (the preview is cropped into the overview), then B to cancel.
dofile(LOOK_LIB)
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(120)
    LOOK.openFromStartMenu()
    D.shot("style-screen")
    for n = 1, 12 do
        D.press(K.SELECT, 30)
        D.shot("brendan-" .. n)
    end
    LOOK.change(ROW.BODY, 1)
    for n = 1, 12 do
        D.press(K.SELECT, 30)
        D.shot("may-" .. n)
    end
    LOOK.change(ROW.BODY, 1)
    D.wait(30)
    D.shot("red")
    D.press(K.B, 0)
    D.waitFor("the overworld", 900, D.inOverworld)
    D.wait(90)
    D.shot("cancelled")
end)
