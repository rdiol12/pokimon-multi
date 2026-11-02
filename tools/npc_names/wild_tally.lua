-- Counts the wild Pokémon met in the grass of the starting routes in a
-- rainstorm (weather and season visitors used to swap in anywhere).
-- Run: python3 tools/world_shots/run.py tools/npc_names/wild_tally.lua <out dir> 200000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function tally(label, map, x, y, battles)
    D.scene(10, map, x, y, {storm = "rain"})
    local counts, order = {}, {}
    for i = 1, battles do
        local sp = D.findBattle(K.LEFT, K.RIGHT, 1)
        local name = SPECIES_NAME[sp] or tostring(sp)
        if not counts[name] then order[#order + 1] = name end
        counts[name] = (counts[name] or 0) + 1
        D.wait(260)
        D.shot(string.format("%s-%02d-%s", label, i, name))
        D.runAway()
    end
    table.sort(order)
    local parts = {}
    for _, name in ipairs(order) do parts[#parts + 1] = name .. " x" .. counts[name] end
    D.log("TALLY " .. label .. ": " .. table.concat(parts, ", "))
end

D.run(function()
    D.boot()
    D.wait(60)
    tally("route101", MAPS.MAP_ROUTE101, 14, 4, 30)
    tally("route102", MAPS.MAP_ROUTE102, 5, 4, 30)
end)
