-- Museum donations: earlier donations already stand in the hall; the
-- player donates a Root Fossil, shows a Rayquaza, gets a reward, looks at
-- the models and opens R > PASTIMES > MUSEUM.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- struct PastimeSaveData offsets (include/pastime_save.h).
local FOSSILS, PHOTOS, RARE, REWARDS = 4, 6, 8, 32

D.run(function()
    D.boot()
    D.wait(60)
    D.pastime16(FOSSILS, 0x1C)            -- Helix, Dome, Old Amber
    D.pastime16(PHOTOS, 15)
    D.pastime16(RARE, SPECIES_ID.LATIOS)
    D.pastime8(REWARDS, 3)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 19, 5,
        { give = "RAYQUAZA", level = 70, lead = true, item = "ROOT_FOSSIL" })
    D.wait(60)
    D.shot("hall")
    D.talk(K.RIGHT, "curator", 2)
    D.wait(40)
    D.shot("menu")
    D.press(K.A, 80)
    D.shot("donate-fossil")
    D.press(K.A, 200)
    D.shot("fossil-donated")
    D.press(K.A, 80)
    D.shot("fossil-model")
    D.press(K.A, 80)
    D.press(K.DOWN, 10)
    D.press(K.A, 80)
    D.press(K.A, 60)
    D.press(K.A, 120)
    D.shot("party")
    D.press(K.A, 150)
    D.shot("rare-recorded")
    D.press(K.A, 200)
    D.shot("rare-model")
    D.press(K.A, 80)
    D.shot("reward")
    D.press(K.A, 120)
    D.shot("reward-item")
    for _ = 1, 3 do D.press(K.A, 60) end
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.wait(60)
    D.shot("hall-after")
    -- Look at the Root Fossil model (first fossil, at 16,6) from above.
    D.scene(12, MAPS.MAP_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F, 16, 5)
    D.wait(40)
    D.talk(K.DOWN, "model", 3)
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(1)
    D.wait(30)
    D.shot("list")
    for _ = 1, 15 do D.press(K.DOWN, 6) end
    D.wait(10)
    D.shot("list-rare")
    D.press(K.B, 40)
end)
