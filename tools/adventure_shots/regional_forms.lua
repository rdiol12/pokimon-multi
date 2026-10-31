-- Regional forms: each one following the player at home (Starfall Isle or
-- Amberwood Hollow), its summary, then in a wild battle there against another of its
-- kind, the player's one seen from behind.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local FORMS = {
    { "WINGULL_STARFALL", MAPS.MAP_STARFALL_ISLE, 40, 28, 19 },
    { "PELIPPER_STARFALL", MAPS.MAP_STARFALL_ISLE, 40, 28, 19 },
    { "LUVDISC_STARFALL", MAPS.MAP_STARFALL_ISLE, 40, 28, 19 },
    { "TEDDIURSA_AMBERWOOD", MAPS.MAP_AMBERWOOD_HOLLOW, 33, 10, 16 },
    { "URSARING_AMBERWOOD", MAPS.MAP_AMBERWOOD_HOLLOW, 33, 10, 16 },
}

D.run(function()
    D.boot()
    D.wait(60)
    for _, f in ipairs(FORMS) do
        local name, map, x, y, hour = f[1], f[2], f[3], f[4], f[5]
        local tag = name:lower():gsub("_", "-")
        D.adventure({ wild = name })
        D.scene(hour, map, x, y, { give = name, level = 40, lead = true })
        D.wait(90)
        D.hold(K.LEFT, 16)
        D.wait(40)
        D.shot(tag .. "-follower")
        -- Its summary: types, then stats.
        D.press(K.START, 40)
        D.press(K.A, 90)  -- POKéMON
        D.press(K.A, 40)  -- the lead
        D.press(K.A, 120) -- SUMMARY
        D.shot(tag .. "-summary")
        D.press(K.RIGHT, 60)
        D.shot(tag .. "-stats")
        D.press(K.B, 120) -- back to the party menu
        D.press(K.B, 40)
        D.press(K.B, 150) -- the START menu
        D.press(K.B, 60)
        D.findBattle(K.RIGHT, K.LEFT)
        D.wait(260)
        D.shot(tag .. "-battle")
        D.press(K.A, 240) -- Go! ...
        D.shot(tag .. "-battle-2")
        D.runAway()
    end
    D.adventure({ wild = "" })
end)
