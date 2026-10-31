-- Rare color variants: the same species in its normal colors and in each
-- variant, in a wild battle and on the summary screen.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function wild(species, level, variant, name, sentOut)
    D.mech({ species = species, level = level, variant = variant, wild = true })
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(400)
    if sentOut then D.press(K.A, 300) end
    D.shot(name)
    D.runAway()
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_ROUTE119, 26, 40)
    D.fastText()
    for _, sp in ipairs({ "ZIGZAGOON", "ALTARIA", "GARDEVOIR" }) do
        for v = 0, 3 do wild(sp, 5, v, sp:lower() .. "-" .. v) end
    end
    -- One of your own: a DUSK Gardevoir, in battle and on the summary screen.
    D.mech({ species = "GARDEVOIR", level = 40, variant = 2 })
    D.wait(60)
    wild("ZIGZAGOON", 5, 0, "own-dusk-gardevoir", true)
    D.wait(120)
    D.step(K.START, "start", 60)
    D.step(K.A, "party", 120)
    D.step(K.A, "party-menu", 30)
    D.step(K.A, "summary", 150)
    D.result("ok")
end)
