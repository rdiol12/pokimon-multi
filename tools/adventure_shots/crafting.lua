-- Crafting: the Rustboro crafter teaches it and gives Apricorns, then
-- R > ADVENTURE > CRAFTING makes a Level Ball, Potions and a Fire Stone,
-- and the crafter's Apricorn shop.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function adventureMenu(index)
    D.press(K.R, 40)
    for _ = 1, 7 do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
    for _ = 1, index do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_RUSTBORO_CITY, 14, 18)
    D.wait(100)
    D.talk(K.UP, "crafter", 4)
    for _ = 1, 6 do D.press(K.A, 50) end
    D.wait(60)
    D.adventure({ items = { "ORAN_BERRY", "SITRUS_BERRY", "TINY_MUSHROOM", "RED_SHARD", "BLUE_APRICORN", "PECHA_BERRY" } })
    adventureMenu(3)
    D.wait(30)
    D.shot("crafting-list")
    D.press(K.A, 30)
    D.shot("crafted-level-ball")
    for _ = 1, 10 do D.press(K.DOWN, 6) end
    D.wait(10)
    D.shot("potion-recipe")
    D.press(K.A, 30)
    D.shot("crafted-potions")
    for _ = 1, 9 do D.press(K.DOWN, 6) end
    D.wait(10)
    D.shot("fire-stone-recipe")
    D.press(K.A, 30)
    D.shot("crafted-fire-stone")
    D.press(K.DOWN, 20)
    D.shot("missing-shards")
    D.press(K.B, 40)
    D.talk(K.UP, "crafter-shop", 1)
    D.wait(60)
    D.shot("apricorn-shop")
end)
