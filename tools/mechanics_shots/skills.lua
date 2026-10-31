-- Trainer skills: the teacher in the Rustboro Pokémon School, picking
-- FIELD MEDIC, and Potions used in a wild battle: the first stays in the Bag.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_RUSTBORO_CITY_POKEMON_SCHOOL, 11, 4, { give = "MUDKIP", level = 20, lead = true, item = "POTION" })
    D.fastText()
    D.mech({ fixMoves = true, badges = 4 })
    D.wait(10)
    D.shot("school")
    D.hold(K.UP, 2)
    D.wait(10)
    for _ = 1, 4 do D.step(K.A, "intro") end
    D.step(K.A, "current")
    D.step(K.A, "menu")
    D.wait(30)
    D.step(K.DOWN, "menu-2")
    D.step(K.A, "help")
    D.step(K.B, "help-b")
    D.step(K.UP, "menu-up")
    D.step(K.A, "fm-help")
    D.step(K.A, "fm-help")
    D.step(K.A, "fm-use")
    D.step(K.A, "fm-on")
    D.step(K.A, "done")
    -- A wild battle with the lead hurt.
    D.wait(60)
    D.mech({ hurt = true, species = "ZIGZAGOON", level = 5, wild = true })
    D.waitFor("the battle", 1200, D.inBattle)
    local function actionMenu()
        return (D.read32(D.SYM.gBattlerControllerFuncs) & ~1) == STATIC_HandleInputChooseAction
    end
    for round = 1, 2 do
        D.waitFor("the action menu", 3000, actionMenu, function(i) if i % 40 == 0 then D.press(K.B, 0) end end)
        D.wait(20)
        D.step(K.RIGHT, "action" .. round, 30)
        D.step(K.A, "bag" .. round, 120)
        D.step(K.A, "potion" .. round, 60)
        D.step(K.A, "use" .. round, 90)
        D.step(K.A, "party" .. round, 120)
        D.step(K.A, "healed" .. round, 120)
    end
    D.waitFor("the action menu", 3000, actionMenu, function(i) if i % 40 == 0 then D.press(K.B, 0) end end)
    D.wait(20)
    D.step(K.RIGHT, "action3", 30)
    D.step(K.A, "bag3", 120)
    D.result("ok")
end)
