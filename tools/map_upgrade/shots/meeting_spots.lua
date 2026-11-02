-- Meeting spots (tools/map_upgrade/meeting_spots.py): the guide and the
-- matchmaker in every new-area and Solmara town.
--   python3 tools/areas/shots/run.py tools/map_upgrade/shots/meeting_spots.lua OUT 300000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- name, map, where to stand (below and between the two), hour
local TOWNS = {
    { "Cobblerock", MAPS.MAP_COBBLEROCK, 27, 21, 12 },
    { "EmberSprings", MAPS.MAP_EMBER_SPRINGS, 21, 24, 12 },
    { "Hollowmere", MAPS.MAP_HOLLOWMERE, 21, 21, 22 },
    { "LaurelCity", MAPS.MAP_LAUREL_CITY, 25, 28, 12 },
    { "MirageOasis", MAPS.MAP_MIRAGE_OASIS, 23, 20, 12 },
    { "NimbusCity", MAPS.MAP_NIMBUS_CITY, 23, 17, 12 },
    { "SnowdriftVillage", MAPS.MAP_SNOWDRIFT_VILLAGE, 27, 17, 12 },
    { "SunfairCarnival", MAPS.MAP_SUNFAIR_CARNIVAL, 29, 24, 12 },
    { "WonderwildPark", MAPS.MAP_WONDERWILD_PARK, 27, 20, 12 },
    { "PortSolace", MAPS.MAP_PORT_SOLACE, 22, 20, 12 },
    { "Gearholt", MAPS.MAP_GEARHOLT, 19, 18, 12 },
    { "Ashcombe", MAPS.MAP_ASHCOMBE, 20, 22, 12 },
    { "Silkwood", MAPS.MAP_SILKWOOD, 19, 18, 12 },
    { "Rosemead", MAPS.MAP_ROSEMEAD, 19, 18, 12 },
    { "Duskmere", MAPS.MAP_DUSKMERE, 19, 18, 12 },
    { "Frostholm", MAPS.MAP_FROSTHOLM, 19, 23, 12 },
    { "Drakespire", MAPS.MAP_DRAKESPIRE, 19, 18, 12 },
}

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    for i, t in ipairs(TOWNS) do
        D.scene(t[5], t[2], t[3], t[4])
        D.wait(300)
        D.shot(string.format("%02d-%s", i, t[1]))
    end
end)
