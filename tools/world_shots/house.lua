-- Player housing: the new house on Starfall Isle, the empty room, the
-- furniture catalog, placing pieces, and the furnished room.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local HOUSE = MAPS.MAP_STARFALL_ISLE_PLAYER_HOUSE
-- Catalog order (data/maps/StarfallIsle_PlayerHouse/scripts.inc).
local ITEMS = { plant = 1, bookcase = 2, table = 3, stool = 4, bed = 5, tv = 6,
                lamp = 7, aquarium = 8, cushion = 9, trophies = 10, telescope = 11 }

local function place(item, x, y, face, shotName)
    D.scene(12, HOUSE, 1, 2)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)  -- "It's a furniture catalog."
    D.press(K.A, 40)
    for _ = 2, ITEMS[item] do D.press(K.DOWN, 8) end
    if shotName then D.shot(shotName) end
    D.press(K.A, 60)
    if shotName then D.shot(shotName .. "-howto") end
    D.press(K.A, 40)  -- "Face an empty spot…"
    D.scene(12, HOUSE, x, y)
    D.hold(face, 2)
    D.wait(20)
    if shotName then
        -- the piece blinks on the tile in front while it is being placed
        D.shot(shotName .. "-ghost")
        D.wait(16)
        D.shot(shotName .. "-ghost-2")
    end
    D.press(K.A, 40)
    if shotName then D.shot(shotName .. "-placed") end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_STARFALL_ISLE, 39, 34)
    D.shot("outside")
    D.hold(K.UP, 40)
    D.wait(150)
    D.shot("inside-empty")

    place("bookcase", 2, 3, K.UP, "catalog")
    place("plant", 0, 3, K.UP)
    place("tv", 4, 3, K.UP)
    place("bed", 11, 4, K.UP)
    place("aquarium", 7, 4, K.UP)
    place("table", 5, 8, K.UP)
    place("stool", 4, 8, K.UP)
    place("lamp", 12, 3, K.UP)
    D.scene(12, HOUSE, 7, 5)
    D.hold(K.LEFT, 2)
    D.wait(60)
    D.shot("furnished")
    D.wait(200)
    D.shot("furnished-2")
end)
