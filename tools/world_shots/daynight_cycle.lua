-- Day and night cycle: the same town in the morning, by day, in the
-- evening and at night (windows light up), lit windows in other towns, and
-- which wild Pokémon are out by day and by night on Route 121.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function encounters(label, hour, n)
    D.scene(hour, MAPS.MAP_ROUTE121, 57, 6, { give = "SWAMPERT", level = 90, lead = true })
    local seen = {}
    for i = 1, n do
        -- B first: a message (an injury note, say) may still be open.
        for _ = 1, 4 do D.press(K.B, 15) end
        if i > 1 then D.scene(hour, MAPS.MAP_ROUTE121, 57, 6) end
        local sp = D.findBattle(K.LEFT, K.RIGHT, 2)
        D.wait(200)
        local name = tostring(SPECIES_NAME[sp])
        if not seen[name] then
            seen[name] = true
            D.shot(label .. "-wild-" .. name:lower())
        end
        D.log(label .. " wild " .. name)
        D.runAway()
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    for _, t in ipairs({ { 7, "morning" }, { 13, "day" }, { 20, "evening" }, { 23, "night" } }) do
        D.scene(t[1], MAPS.MAP_PETALBURG_CITY, 7, 8)
        D.wait(420) -- let the map name and any news banner go
        D.shot("petalburg-" .. t[2])
    end
    D.scene(22, MAPS.MAP_LITTLEROOT_TOWN, 10, 10)
    D.shot("littleroot-night")
    D.scene(22, MAPS.MAP_RUSTBORO_CITY, 13, 33)
    D.shot("rustboro-night")
    D.scene(22, MAPS.MAP_MAUVILLE_CITY, 22, 9)
    D.shot("mauville-night")
    D.scene(22, MAPS.MAP_FALLARBOR_TOWN, 7, 13)
    D.shot("fallarbor-night")
    D.scene(13, MAPS.MAP_ROUTE121, 57, 6)
    D.shot("route121-day")
    D.scene(23, MAPS.MAP_ROUTE121, 57, 6)
    D.shot("route121-night")
    encounters("day", 13, 10)
    encounters("night", 23, 10)
end)
