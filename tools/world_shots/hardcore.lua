-- Hardcore mode: the Challenge Clerk in Oldale; a level 1 Magikarp leads,
-- faints and is gone after the battle; a second Pokémon in the same area
-- can't be caught.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 15, 17)
    D.hold(K.RIGHT, 2)
    D.wait(10)
    D.press(K.A, 60)
    for i = 1, 6 do
        D.wait(40)
        if i == 2 or i == 6 then D.shot("clerk-rules-" .. i) end
        D.press(K.A, 20)
    end
    D.wait(60)
    D.shot("clerk-ask")
    D.press(K.A, 80)   -- YES
    D.press(K.A, 60)   -- message
    D.press(K.A, 80)   -- sure? YES
    D.press(K.A, 80)
    D.wait(30)
    D.shot("clerk-started")
    D.press(K.A, 200)

    D.scene(13, MAPS.MAP_ROUTE101, 4, 4, { give = "MAGIKARP", level = 1, lead = true, item = "ULTRA_BALL" })
    D.findBattle(K.LEFT, K.RIGHT)
    D.wait(300)
    local shotFaint = false
    D.waitFor("the overworld", 20000, D.inOverworld, function(i)
        if i % 40 ~= 0 then return end
        if D.inBattle() then
            D.press(K.A, 0)
        else
            -- the party menu after Magikarp fainted: send out the next one
            if not shotFaint then D.shot("magikarp-fainted"); shotFaint = true end
            D.press(K.DOWN, 6); D.press(K.A, 20); D.press(K.A, 0)
        end
    end)
    D.wait(60)
    D.shot("lost-magikarp")
    D.press(K.A, 60)
    D.shot("lost-magikarp-2")
    D.press(K.A, 60)
    D.press(K.A, 60)

    D.findBattle(K.LEFT, K.RIGHT)
    D.wait(300)
    D.press(K.A, 150)
    D.press(K.RIGHT, 10)
    D.press(K.A, 90)
    D.shot("bag-0")
    D.press(K.RIGHT, 40)
    D.shot("bag-1")
    D.press(K.A, 40)
    D.press(K.A, 40)
    for i = 1, 3 do D.wait(30); D.shot("second-encounter-blocked-" .. i) end
    D.press(K.A, 40); D.press(K.B, 40); D.press(K.B, 40); D.press(K.B, 60)
    D.press(K.A, 60)
    D.runAway()
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 15, 17)
    D.talk(K.RIGHT, "clerk-status", 2)
end)
