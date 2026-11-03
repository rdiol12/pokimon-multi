-- The brand-new Pokémon in the game (docs/new_species.md).
-- Run: python3 tools/world_shots/run.py tools/new_species/shots.lua <out dir> 120000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local NAME = {}
for k, v in pairs(SPECIES_ID) do NAME[v] = k end
local LINE = { JELLUME = true, LUMEDUSA = true, AURELLUMA = true }
local LEVEL = { JELLUME = 34, LUMEDUSA = 46, AURELLUMA = 58 }

local function walk(dir, frames)
    D.hold(dir, frames); D.wait(20)
end

-- A wild Pokémon on land can start a battle while we walk; leave it.
local function leaveBattle()
    if not D.inOverworld() then
        D.wait(200)
        D.runAway()
    end
end

D.run(function()
    D.boot()
    D.wait(60)
    -- Following the player on land, normal and shiny.
    for _, mon in ipairs({ "JELLUME", "LUMEDUSA", "AURELLUMA" }) do
        for _, shiny in ipairs({ false, true }) do
            leaveBattle()
            D.scene(13, MAPS.MAP_ROUTE208, 12, 10, { give = mon, level = LEVEL[mon], lead = true, shiny = shiny })
            walk(K.RIGHT, 48)
            walk(K.LEFT, 16)
            D.shot("follow-" .. mon .. (shiny and "-shiny" or ""))
        end
    end
    -- Wild ones on Route 208's water, battled with each of the line.
    local seen = {}
    for _, lead in ipairs({ "JELLUME", "LUMEDUSA", "AURELLUMA" }) do
        leaveBattle()
        D.scene(19, MAPS.MAP_ROUTE208, 44, 24, { give = lead, level = 75, lead = true, surf = true, dex = true })
        local got = false
        for _ = 1, 60 do
            local sp = D.findBattle(K.LEFT, K.RIGHT)
            local name = NAME[sp] or tostring(sp)
            D.log(lead .. " met " .. name)
            local want = lead == "AURELLUMA" and name == "AURELLUMA" or lead ~= "AURELLUMA" and LINE[name]
            if want and not got then
                D.wait(260)
                D.shot("battle-" .. lead .. "-vs-" .. name)
                D.press(K.A, 0)
                D.wait(300)
                D.shot("battle-" .. lead .. "-vs-" .. name .. "-2")
                got = true
                seen[name] = true
            end
            D.runAway()
            if got then break end
            D.shot("water-" .. lead)
        end
    end
    -- The Pokédex, the party and the summary.
    leaveBattle()
    D.press(K.START, 60)
    D.shot("menu")
    D.press(K.A, 200)
    for _ = 1, 160 do D.press(K.RIGHT, 8) end -- a page at a time, to the end
    D.wait(60)
    D.shot("dex-list")
    D.press(K.A, 150)
    D.shot("dex-entry")
    D.press(K.UP, 120)
    D.shot("dex-entry-up")
    D.press(K.UP, 120)
    D.shot("dex-entry-up2")
    D.waitFor("the overworld", 3000, D.inOverworld, function(i)
        if i % 60 == 0 then D.press(K.B, 0) end
    end)
    D.scene(13, MAPS.MAP_ROUTE208, 12, 10) -- a fresh map, so START opens the menu again
    D.press(K.START, 80)
    D.shot("menu-2")
    D.press(K.DOWN, 20)
    D.press(K.A, 120)
    D.shot("party")
    D.press(K.A, 40)
    D.press(K.A, 150)
    for p = 1, 3 do
        D.shot("summary-" .. p)
        D.press(K.RIGHT, 60)
    end
end)
