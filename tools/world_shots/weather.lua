-- Roaming weather: the rainstorm over Route 118 (3 pm), the sandstorm over
-- Route 117 and the snow squall over Route 116 (both 7 am), a weather-only
-- Pokémon in each, and the weather watcher's forecast in Oldale.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local WEATHER_MONS = {
    rain = { LOTAD = 1, LOMBRE = 1, PELIPPER = 1, PSYDUCK = 1, POLIWAG = 1, POLIWHIRL = 1, SURSKIT = 1,
             MASQUERAIN = 1, MARILL = 1, AZUMARILL = 1, WOOPER = 1, QUAGSIRE = 1, TYMPOLE = 1, GOOMY = 1, PALPITOAD = 1 },
    sand = { SANDSHREW = 1, TRAPINCH = 1, CACNEA = 1, BALTOY = 1, HIPPOPOTAS = 1, SILICOBRA = 1, DWEBBLE = 1, GIBLE = 1, LARVITAR = 1 },
    snow = { SNORUNT = 1, SNOVER = 1, SWINUB = 1, SPHEAL = 1, CUBCHOO = 1, VANILLITE = 1, SNOM = 1, SNEASEL = 1, VULPIX_ALOLA = 1 },
}

local function hunt(kind)
    for _ = 1, 25 do
        local sp = D.findBattle(K.LEFT, K.RIGHT)
        D.wait(250)
        local name = SPECIES_NAME[sp] or "?"
        D.log(kind .. " wild " .. name)
        if WEATHER_MONS[kind][name] then
            D.shot(kind .. "-wild-" .. name:lower())
            D.runAway()
            return
        end
        D.runAway()
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(15, MAPS.MAP_ROUTE118, 52, 10)
    D.wait(400)
    D.shot("route118-rainstorm")
    hunt("rain")

    D.scene(7, MAPS.MAP_ROUTE117, 30, 16)
    D.wait(400)
    D.shot("route117-sandstorm")
    hunt("sand")

    D.scene(7, MAPS.MAP_ROUTE116, 8, 15)
    D.wait(400)
    D.shot("route116-snow")
    hunt("snow")

    D.scene(15, MAPS.MAP_OLDALE_TOWN, 13, 18)
    D.talk(K.UP, "forecast", 5)
end)
