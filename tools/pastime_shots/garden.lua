-- Community garden at the north end of Route 104: the GARDENER gives starter
-- Berries; an Oran tree grown next to a Pecha tree is picked (with the
-- berry-picking meter) and gives a mutated Qualot Berry too; the Qualot is
-- today's GARDEN SHOW entry, which earns the BLOOM rank. Then R > PASTIMES >
-- GARDEN.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local POINTS = 138 -- struct PastimeSaveData offset of gardenPoints
local TREE_FIRST = 112
local BERRY_ORAN, BERRY_PECHA = 7, 3

-- struct BerryTree (include/global.berry.h), 8 bytes each from 0x169C of
-- SaveBlock1: berry, stage, then the mutation in the top bits.
local function setTree(plot, berry, stage, yield, mutation)
    local addr = D.read32(D.SYM.gSaveBlock1Ptr) + 0x169C + (TREE_FIRST + plot) * 8
    D.write8(addr, berry)
    D.write8(addr + 1, stage)
    D.write8(addr + 2, 0x10); D.write8(addr + 3, ((mutation or 0) & 3) << 6)
    D.write8(addr + 4, yield | (((mutation or 0) >> 2) << 6))
    D.write8(addr + 5, 0)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_ROUTE104, 26, 2)
    D.pastime8(POINTS, 6)
    D.wait(30)
    D.shot("garden")
    D.talk(K.RIGHT, "welcome", 2)
    for i = 1, 8 do
        D.wait(60)
        if i % 2 == 1 then D.shot("gift-" .. i) end
        D.press(K.A, 0)
    end
    D.wait(60)
    D.shot("mutations")
    for _ = 1, 3 do D.press(K.A, 60) end
    D.shot("show")
    D.press(K.DOWN, 10)
    D.press(K.A, 60) -- no, not yet
    for _ = 1, 3 do D.press(K.B, 40) end
    -- Grown overnight: an Oran tree next to a Pecha tree, with a mutation.
    setTree(0, BERRY_ORAN, 5, 4, 3)
    setTree(1, BERRY_PECHA, 5, 4, 0)
    setTree(4, BERRY_ORAN, 3, 0, 0)
    D.scene(12, MAPS.MAP_ROUTE104, 22, 2)
    D.wait(30)
    D.shot("plots")
    D.talk(K.UP, "tree", 1)
    -- Yes, pick them; then the berry-picking meter.
    D.waitFor("the meter", 900, function() return D.read8(D.SYM.gWorldTimingMeter + 20) == 0xFF end, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
    D.playMeter(false, "picking")
    for i = 1, 12 do
        D.wait(50)
        D.shot("picked-" .. i)
        if i % 2 == 0 then D.press(K.A, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.scene(12, MAPS.MAP_ROUTE104, 26, 2)
    D.talk(K.RIGHT, "show-again", 1)
    D.press(K.A, 90) -- yes
    D.shot("show-points")
    D.press(K.A, 240)
    D.shot("rank-up")
    D.press(K.A, 120)
    D.shot("prize")
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(13)
    D.wait(30)
    D.shot("garden-list")
    D.press(K.B, 60)
end)
