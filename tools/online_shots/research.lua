-- Research Pokédex with two players. TEST1 already has some research (written
-- into the save below): catching a Zigzagoon completes its research (and
-- reaches research rank 3), beating a Linoone raises its level, then the
-- research screen (R > RESEARCH), a species page, and HUB > RESEARCH LEADERS
-- (the seed has other players' counts on the server).
dofile(OUT_DIR .. "/lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

-- offsetof(struct SaveBlockExt, research): changes when fields are added
-- before it (print it with a one-line C file, see docs/research.md).
local RESEARCH = D.SYM.gSaveBlockExt + 49640
local BIG, DAY, NIGHT, DONE, PERFECT = 1, 2, 4, 8, 16

local function set(nat, caught, defeated, evolved, flags)
    local e = RESEARCH + 4 + nat * 4
    D.write8(e, caught); D.write8(e + 1, defeated); D.write8(e + 2, evolved); D.write8(e + 3, flags)
end

local function seedResearch()
    for _, nat in ipairs({183, 261, 265, 270, 273, 276, 278, 293, 296}) do
        set(nat, 10, 15, 3, BIG | DAY | NIGHT | DONE | PERFECT)
    end
    D.write8(RESEARCH, 9)      -- completed
    D.write8(RESEARCH + 2, 2)  -- ranks given
    set(252, 1, 2, 0, DAY)     -- Treecko
    set(255, 2, 4, 0, DAY)     -- Torchic
    set(258, 0, 1, 0, 0)       -- Mudkip
    set(263, 6, 15, 0, BIG | NIGHT) -- Zigzagoon: level 9
    set(264, 0, 5, 0, 0)       -- Linoone
    set(280, 3, 6, 0, DAY | NIGHT) -- Ralts
    set(285, 7, 6, 1, BIG)     -- Shroomish
    set(300, 1, 0, 0, DAY)     -- Skitty
    set(304, 2, 10, 0, 0)      -- Aron
    set(309, 4, 8, 0, BIG)     -- Electrike
end

-- Fights the wild Pokémon with the first move until the battle ends.
local function fight()
    D.waitFor("the battle", 1200, D.inBattle)
    D.wait(400)
    D.waitFor("the overworld", 6000, D.inOverworld, function(i)
        if i % 60 == 0 then D.press(K.A, 0) end
    end)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(120)
    if not P1 then
        D.wait(9000)
        return
    end
    seedResearch()
    D.scene(12, MAPS.MAP_ROUTE101, 4, 4, { give = "SCEPTILE", level = 60, lead = true, item = "MASTER_BALL" })
    D.wildBattle("ZIGZAGOON", 4)
    D.catchThis()
    -- Achievement banners come first, then the research toasts.
    for i = 1, 14 do
        D.wait(100)
        D.shot("after-catch-" .. i)
    end
    D.wildBattle("LINOONE", 20)
    fight()
    for i = 1, 4 do
        D.wait(80)
        D.shot("after-defeat-" .. i)
    end
    D.wait(200)
    -- R > RESEARCH (the row before CLOSE).
    D.press(K.R, 40)
    for _ = 1, 13 do D.press(K.DOWN, 8) end
    D.shot("r-menu")
    D.press(K.A, 60)
    D.shot("list")
    D.wait(30)
    D.press(K.A, 40)
    D.shot("page-first")
    for i = 1, 12 do
        D.press(K.DOWN, 20)
        D.shot("page-" .. i)
    end
    D.press(K.B, 30)
    D.shot("list-after")
    D.press(K.B, 40)
    D.openHub(16)
    D.waitAnswer()
    D.shot("leaders")
    D.press(K.A, 40)
end)
