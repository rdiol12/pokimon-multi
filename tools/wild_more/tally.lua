-- Counts the kinds of wild Pokémon met on a few routes (tools/wild_more).
-- Run: python3 tools/world_shots/run.py tools/wild_more/tally.lua <out dir> 400000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function tally(label, map, x, y, battles)
    D.scene(12, map, x, y)
    local counts, order = {}, {}
    for i = 1, battles do
        local sp = D.findBattle(K.LEFT, K.RIGHT, 1)
        local name = SPECIES_NAME[sp] or tostring(sp)
        if not counts[name] then
            order[#order + 1] = name
            D.wait(260)
            D.shot(string.format("%s-%s", label, name))
        end
        counts[name] = (counts[name] or 0) + 1
        D.runAway()
    end
    table.sort(order)
    local parts = {}
    for _, name in ipairs(order) do parts[#parts + 1] = name .. " x" .. counts[name] end
    D.log("TALLY " .. label .. ": " .. #order .. " kinds: " .. table.concat(parts, ", "))
end

D.run(function()
    D.boot()
    D.wait(60)
    tally("route101", MAPS.MAP_ROUTE101, 14, 4, 40)
    tally("route104", MAPS.MAP_ROUTE104, 4, 10, 40)
    tally("route116", MAPS.MAP_ROUTE116, 7, 14, 40)
    tally("route110", MAPS.MAP_ROUTE110, 6, 40, 40)
end)
