-- Seasons: the same corner of Route 102 and Petalburg in spring, summer,
-- autumn and winter (gLifeShowcase.season overrides the month), the
-- new-season notice, and a winter-only wild Pokémon.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local NAMES = { "spring", "summer", "autumn", "winter" }

local function season(n) D.write8(D.SYM.gLifeShowcase + 2, n) end

D.run(function()
    D.boot()
    D.wait(60)
    for n = 1, 4 do
        season(n)
        D.scene(13, MAPS.MAP_ROUTE102, 30, 9)
        D.wait(30)
        D.shot("route102-" .. NAMES[n])
        D.scene(13, MAPS.MAP_PETALBURG_CITY, 15, 16)
        D.shot("petalburg-" .. NAMES[n])
    end
    -- Stepping outside in a new season shows a notice.
    season(4)
    D.scene(13, MAPS.MAP_ROUTE101, 4, 4)
    D.wait(90)
    D.shot("winter-notice")
    local seen = {}
    for i = 1, 25 do
        local sp = D.findBattle(K.LEFT, K.RIGHT)
        local name = SPECIES_NAME[sp] or tostring(sp)
        D.log("wild " .. name)
        if (name:find("DEERLING") or name == "SNOVER" or name == "CUBCHOO" or name == "BERGMITE" or name == "SNOM"
            or name == "DELIBIRD" or name == "FRIGIBAX") and not seen[name] then
            D.wait(150)
            D.shot("winter-wild-" .. name:lower())
            seen[name] = true
        end
        D.runAway()
        if seen.DEERLING_WINTER and (seen.SNOVER or seen.CUBCHOO or seen.BERGMITE or seen.SNOM) then break end
    end
end)
