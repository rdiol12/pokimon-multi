-- Delivery jobs: take a parcel at the Slateport desk, check the timer in
-- R > PASTIMES > DELIVERY, deliver it (the fifth delivery, so with a bonus
-- item), then take another and let it run late.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- struct PastimeSaveData offsets (include/pastime_save.h).
local TO, START, DELIVERIES = 37, 40, 46

-- Each desk's map and the tile below it (src/pastime_delivery.c).
local DESKS = {
    [0] = { MAPS.MAP_OLDALE_TOWN, 8, 18 },
    [1] = { MAPS.MAP_PETALBURG_CITY, 18, 18 },
    [2] = { MAPS.MAP_RUSTBORO_CITY, 18, 40 },
    [3] = { MAPS.MAP_SLATEPORT_CITY, 17, 21 },
    [4] = { MAPS.MAP_MAUVILLE_CITY, 24, 7 },
    [5] = { MAPS.MAP_FALLARBOR_TOWN, 16, 9 },
    [6] = { MAPS.MAP_FORTREE_CITY, 7, 9 },
    [7] = { MAPS.MAP_LILYCOVE_CITY, 26, 16 },
}

local function goToDesk(town)
    local d = DESKS[town]
    D.scene(12, d[1], d[2], d[3])
    D.wait(120)
end

D.run(function()
    D.boot()
    D.wait(60)
    goToDesk(4) -- visit Mauville first, so there's somewhere to send parcels
    goToDesk(3)
    D.shot("slateport-desk")
    D.talk(K.UP, "offer", 3)
    D.press(K.A, 200) -- yes
    D.shot("got-parcel")
    D.press(K.A, 60)
    D.shot("hurry")
    for _ = 1, 3 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(2)
    D.wait(30)
    D.shot("timer")
    D.press(K.B, 60)

    D.pastime16(DELIVERIES, 4)
    goToDesk(D.pastimeRead8(TO))
    D.talk(K.UP, "deliver", 3)
    D.wait(30)
    D.shot("deliver-4")
    D.press(K.A, 60)
    D.shot("bonus")
    D.press(K.A, 200)
    D.shot("bonus-item")
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)

    -- Another parcel, then pretend half an hour went by.
    D.talk(K.UP, "offer-2", 3)
    D.press(K.A, 200)
    for _ = 1, 5 do D.press(K.B, 40) end
    D.wait(30)
    D.pastime32(START, D.pastimeRead32(START) - 1800)
    D.wait(30)
    D.shot("late-notice")
    D.wait(30)
    D.pastimeMenu(2)
    D.wait(30)
    D.shot("timer-late")
    D.press(K.B, 40)
end)
