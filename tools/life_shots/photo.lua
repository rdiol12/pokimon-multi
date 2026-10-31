-- Photo safari: rent the camera from the photographer at the Safari Zone
-- entrance, take posed photos in the Safari Zone, check R > ALBUM, then
-- use up the film and collect the prize.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- gLifeShowcase: photoSpecies at 4, photoDistance 6, photoFacing 7 (DIR_*), photoCount 8.
local DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST = 1, 2, 3, 4
local function subject(species, distance, facing, count)
    local s = D.SYM.gLifeShowcase
    local sp = SPECIES_ID[species]
    D.write8(s + 6, distance)
    D.write8(s + 7, facing or 0)
    D.write8(s + 8, count or 1)
    D.write8(s + 5, sp >> 8)
    D.write8(s + 4, sp & 0xFF)
    D.wait(30)
end

local function snap(name)
    D.press(K.R, 100)
    D.shot(name)
    D.press(K.A, 60)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(14, MAPS.MAP_ROUTE121_SAFARI_ZONE_ENTRANCE, 16, 4)
    D.hold(K.UP, 2)
    D.wait(10)
    D.shot("ranger")
    D.press(K.A, 60)
    D.shot("ranger-intro")
    D.press(K.A, 60)
    D.shot("ranger-ask")
    D.press(K.A, 60) -- YES
    D.shot("ranger-howto")
    for _ = 1, 6 do D.press(K.A, 40) end
    D.wait(400)
    D.shot("ranger-howto-2")
    -- Some open grass in the Safari Zone.
    D.scene(14, MAPS.MAP_SAFARI_ZONE_SOUTH, 26, 26)
    D.wait(400)
    D.shot("safari-roaming")
    D.hold(K.LEFT, 2)
    D.wait(10)
    subject("GIRAFARIG", 2)
    D.shot("subject-girafarig")
    snap("photo-girafarig")
    subject("DODUO", 3, 0, 3)
    D.shot("subject-doduo")
    snap("photo-doduo-group")
    subject("PIKACHU", 4, DIR_WEST)
    snap("photo-pikachu-behind")
    subject("WOBBUFFET", 1, DIR_SOUTH)
    snap("photo-wobbuffet-side")
    -- Use up the rest of the film.
    for i = 1, 8 do
        D.press(K.R, 100)
        D.press(K.A, 60)
    end
    D.wait(40)
    D.shot("out-of-film")
    D.press(K.A, 30)
    D.press(K.A, 30)
    D.wait(400)
    D.shot("back-at-entrance")
    D.scene(14, MAPS.MAP_ROUTE121_SAFARI_ZONE_ENTRANCE, 16, 4)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.press(K.A, 60)
    D.shot("payout")
    D.press(K.A, 200)
    D.shot("prize")
    for _ = 1, 4 do D.press(K.A, 40) end
    D.wait(30)
    -- The best shots, from the R menu (RIDE, BOND, JOBS, ALBUM).
    D.press(K.R, 60)
    D.shot("menu")
    for _ = 1, 3 do D.press(K.DOWN, 8) end
    D.press(K.A, 60)
    D.shot("album")
end)
