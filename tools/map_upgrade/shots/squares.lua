-- Town squares (tools/map_upgrade/squares.py): each square from the middle,
-- its notice board, and the guide and matchmaker in Littleroot.
--   python3 tools/areas/shots/run.py tools/map_upgrade/shots/squares.lua OUT 300000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- name, map, where to stand for the overview, the notice board
local TOWNS = {
    { "littleroot", MAPS.MAP_LITTLEROOT_TOWN, 10, 27, 5, 23 },
    { "oldale", MAPS.MAP_OLDALE_TOWN, 28, 10, 23, 5 },
    { "petalburg", MAPS.MAP_PETALBURG_CITY, 15, 37, 14, 32 },
    { "dewford", MAPS.MAP_DEWFORD_TOWN, 10, 26, 5, 22 },
    { "verdanturf", MAPS.MAP_VERDANTURF_TOWN, 10, 26, 4, 22 },
    { "mauville", MAPS.MAP_MAUVILLE_CITY, 25, 26, 19, 20 },
    { "fallarbor", MAPS.MAP_FALLARBOR_TOWN, 10, 26, 4, 22 },
    { "lavaridge", MAPS.MAP_LAVARIDGE_TOWN, 10, 26, 4, 22 },
    { "fortree", MAPS.MAP_FORTREE_CITY, 22, 27, 14, 23 },
}

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    for i, t in ipairs(TOWNS) do
        local n = string.format("%02d-%s", i, t[1])
        D.scene(12, t[2], t[3], t[4])
        D.wait(300)
        D.shot(n)
        D.scene(12, t[2], t[5], t[6] + 1)
        D.wait(200)
        D.talk(K.UP, n .. "-board", 2)
        D.press(K.A, 40)
        D.press(K.B, 40)
    end
    -- the guide and matchmaker (offline here; online they open the HUB/PLAZA
    -- and the quick match list)
    D.scene(12, MAPS.MAP_LITTLEROOT_TOWN, 8, 25)
    D.wait(300)
    D.talk(K.UP, "20-guide", 3)
    D.press(K.A, 40)
    D.press(K.B, 40)
    D.scene(12, MAPS.MAP_LITTLEROOT_TOWN, 11, 25)
    D.wait(300)
    D.talk(K.UP, "21-matchmaker", 2)
    D.press(K.A, 40)
    D.press(K.B, 40)
end)
