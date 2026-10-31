-- Every overworld ability, one scene each (docs/field_abilities.md).
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- The world spots spawn a moment after the map loads (the online update
-- runs first): give them time before walking or talking.
local scene = D.scene
D.scene = function(...)
    scene(...)
    D.wait(120)
end

-- Presses A n times, a screenshot after each.
local function pages(name, n, after)
    for i = 1, n do D.step(K.A, name .. "-" .. i, after or 70) end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.fastText()

    -- Living lantern: Granite Cave B1F is pitch dark with Mudkip, lit by Torchic.
    D.scene(12, MAPS.MAP_GRANITE_CAVE_B1F, 7, 8, { give = "MUDKIP", level = 20, lead = true })
    D.shot("dark-cave")
    D.scene(12, MAPS.MAP_GRANITE_CAVE_B1F, 7, 8, { give = "TORCHIC", level = 20, lead = true })
    D.wait(30)
    D.shot("lantern")
    D.press(K.A, 60)
    D.shot("lantern-lit")

    -- Power up: the generator next to the player, with an Electrike.
    D.scene(12, MAPS.MAP_GRANITE_CAVE_B1F, 7, 8, { give = "ELECTRIKE", level = 20, lead = true })
    D.face(K.LEFT)
    D.press(K.A, 60)
    D.shot("generator")
    pages("generator", 8, 90)
    D.wait(120)
    D.shot("generator-lit")
    D.face(K.LEFT)
    D.press(K.A, 60)
    D.shot("generator-hums")
    D.press(K.A, 30)

    -- Ice melter: a block in the Shoal Cave ice room.
    D.scene(12, MAPS.MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, 11, 13, { give = "TORCHIC", level = 20, lead = true })
    D.face(K.DOWN)
    D.shot("ice")
    D.press(K.A, 60)
    pages("ice", 6, 90)

    -- Dig spots: sparkling sand in the Route 111 desert, with a Sandshrew.
    D.scene(12, MAPS.MAP_ROUTE111, 16, 51, { give = "SANDSHREW", level = 20, lead = true })
    D.face(K.DOWN)
    D.shot("dig")
    D.press(K.A, 60)
    pages("dig", 6, 90)

    -- Strong arms: a Machop shoves the Granite Cave boulder; there's a prize under it.
    D.scene(12, MAPS.MAP_GRANITE_CAVE_1F, 7, 9, { give = "MACHOP", level = 20, lead = true })
    D.face(K.UP)
    D.shot("boulder")
    D.press(K.A, 60)
    pages("boulder", 6, 90)
    D.wait(30)
    D.hold(K.UP, 20)
    D.wait(60)
    D.shot("boulder-pushed")
    pages("boulder-prize", 3, 90)

    -- Sixth sense: Ralts on the Route 104 beach, near two hidden items.
    D.scene(12, MAPS.MAP_ROUTE104, 17, 63, { give = "RALTS", level = 20, lead = true })
    for i = 1, 12 do
        D.wait(10)
        D.shot("sixth-sense")
    end

    -- Harvest: an Oddish lead picks one more Oran Berry.
    D.scene(12, MAPS.MAP_ROUTE102, 24, 3, { give = "ODDISH", level = 20, lead = true, ripen = true })
    D.face(K.UP)
    D.press(K.A, 60)
    D.shot("harvest")
    pages("harvest", 4, 90)

    -- Natural Cure: Swablu soothes a poisoned teammate on the next step.
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "SWABLU", level = 20, lead = true })
    D.fa(1, 0)
    D.fa(2)
    D.hold(K.LEFT, 16)
    D.wait(60)
    D.shot("healer")
    D.press(K.A, 60)
    D.shot("healer-2")
    D.press(K.A, 30)

    -- Frisk: Shuppet pockets the wild Zigzagoon's Nugget.
    D.scene(12, MAPS.MAP_ROUTE102, 20, 8, { give = "SHUPPET", level = 30, lead = true })
    D.fa(1, 1)
    D.mech({ fixMoves = true })
    D.fa(3)
    D.mech({ species = "ZIGZAGOON", level = 2, wild = true })
    D.fight("frisk-battle", 400, 2)
    D.wait(60)
    D.shot("frisk")
    D.result("ok")
end)
