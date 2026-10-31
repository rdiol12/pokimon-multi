-- Fusion breeding: two different Pokémon at the Route 117 Day Care make a
-- fusion egg; it hatches into a Pokémon with the mother's shape, the
-- father's colors (built by the server), a mixed name and types. Then a
-- few more fusions on the party, summary and battle screens.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local SB3_FUSION = 1096 -- offsetof(struct SaveBlock3, breedFusion)
local DAYCARE, DAYCARE_MON, OFFSPRING, STEPS = 12336, 140, 280, 284

local function sb1() return D.read32(D.SYM.gSaveBlock1Ptr) end

local function give(species, level)
    D.scene(12, MAPS.MAP_ROUTE117, 47, 5, { give = species, level = level or 20 })
    return D.partyCount() - 1
end

local function setPartyCount(n) D.write8(D.SYM.gPartiesCount, n) end

-- Leaves the last two party Pokémon at the Day Care (mother first).
local function leaveAtDaycare()
    local n = D.partyCount()
    local base = sb1() + DAYCARE
    for i = 0, 1 do
        local slot = base + i * DAYCARE_MON
        Fusion.clear(slot, DAYCARE_MON)
        Fusion.copy(Fusion.partyAddr(n - 2 + i), slot, 80)
    end
    Fusion.clear(Fusion.partyAddr(n - 2), 200)
    setPartyCount(n - 2)
end

local function summary(name, slot, pages)
    for _ = 1, 4 do
        D.step(K.START, nil, 40)
        D.press(K.A, 120)
        if D.callback2() == STATIC_CB2_UpdatePartyMenu then break end
        for _ = 1, 3 do D.press(K.B, 60) end
    end
    D.shot(name .. "-party")
    for _ = 1, slot do D.press(K.DOWN, 12) end
    D.press(K.A, 30)
    D.step(K.A, name .. "-summary", 150)
    for p = 2, pages or 1 do
        D.step(K.RIGHT, name .. "-summary-" .. p, 60)
    end
    for _ = 1, 3 do D.press(K.B, 120) end
    D.wait(60)
end

local function swapToLead(slot)
    local a, b = Fusion.partyAddr(0), Fusion.partyAddr(slot)
    local tmp = {}
    for i = 0, 99 do tmp[i] = D.read8(a + i) end
    Fusion.copy(b, a, 100)
    for i = 0, 99 do D.write8(b + i, tmp[i]) end
end

D.run(function()
    D.boot()
    D.wait(60)
    if INSTANCE ~= 1 then
        D.wait(60000) -- the second player only keeps the server company
        return
    end
    D.fastText()

    -- Two parents of different kinds: a female Wingull and a male Pikachu.
    local w = give("WINGULL", 20)
    Fusion.setGender(Fusion.partyAddr(w), true)
    local p = give("PIKACHU", 20)
    Fusion.setGender(Fusion.partyAddr(p), false)
    leaveAtDaycare()
    D.scene(12, MAPS.MAP_ROUTE117, 47, 5)
    D.talk(K.UP, "01-daycare-man", 4)
    D.wait(60)

    -- They had an egg.
    D.write8(sb1() + DAYCARE + OFFSPRING, 0x35)
    D.write8(sb1() + DAYCARE + OFFSPRING + 1, 0x12)
    D.mech({ var = { 0x86, 1 } }) -- FLAG_PENDING_DAYCARE_EGG
    local before = D.partyCount()
    D.hold(K.UP, 2)
    D.wait(10)
    local shots = 0
    D.waitFor("the egg", 2400, function() return D.partyCount() > before end, function(i)
        if i % 50 == 25 then
            shots = shots + 1
            D.shot(string.format("02-egg-%02d", shots))
            D.press(K.A, 0)
        end
    end)
    D.wait(60)
    D.shot("03-got-egg")
    for _ = 1, 6 do D.press(K.B, 40) end
    D.wait(120)

    -- The server builds the fusion while the egg is in the party.
    D.waitFor("the server's fusion", 1800, function() return Fusion.recipeCount(SB3_FUSION) >= 1 end)
    D.log("recipes: " .. Fusion.recipeCount(SB3_FUSION))

    -- Hatch it: no egg cycles left, and the next step checks.
    local egg = D.partyCount() - 1
    Fusion.setFriendship(Fusion.partyAddr(egg), 0)
    D.write8(sb1() + DAYCARE + STEPS, 0xF0)
    D.write8(sb1() + DAYCARE + STEPS + 1, 0x00)
    D.hold(K.DOWN, 18)
    D.waitFor("the hatching scene", 3000, function() return not D.inOverworld() end, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
    local n = 0
    D.waitFor("the overworld", 6000, D.inOverworld, function(i)
        if i % 60 == 30 and n < 14 then n = n + 1; D.shot(string.format("04-hatch-%02d", n)) end
        if i > 700 and i % 40 == 0 then D.press(K.B, 0) end
    end)
    D.wait(90)
    summary("06-wingchu", egg, 3)

    -- The fusion as the lead: follower and battle.
    swapToLead(egg)
    D.scene(12, MAPS.MAP_ROUTE117, 44, 8)
    D.hold(K.RIGHT, 34)
    D.wait(60)
    D.shot("07-follower")
    D.hold(K.DOWN, 18)
    D.wait(40)
    D.shot("07-follower-2")
    D.mech({ species = "ZIGZAGOON", level = 2, wild = true })
    D.waitFor("the battle", 1200, D.inBattle)
    D.waitFor("the action menu", 2000, function() return (D.read32(D.SYM.gBattlerControllerFuncs) & ~1) == STATIC_HandleInputChooseAction end, function(i)
        if i % 60 == 30 then D.press(K.B, 0) end
    end)
    D.wait(30)
    D.shot("08-battle")
    D.press(K.A, 60)
    D.shot("08-battle-moves")
    D.press(K.B, 60)
    D.runAway()

    -- More fusions: mother shape + father colors.
    local pairs = {
        { "TORCHIC", "MUDKIP", "Torckip" },
        { "RALTS", "GYARADOS", "Ralados" },
        { "ZIGZAGOON", "CHARIZARD", "Zigzzard" },
        { "SKITTY", "GENGAR", "Skingar" },
    }
    -- Keep the lead (the hatched fusion) and make room.
    Fusion.clear(Fusion.partyAddr(1), 500)
    setPartyCount(1)
    for i, pr in ipairs(pairs) do
        local slot = give(pr[1], 25)
        Fusion.setPartner(Fusion.partyAddr(slot), SPECIES_ID[pr[2]])
        Fusion.setNickname(Fusion.partyAddr(slot), pr[3])
        local m = Fusion.open(Fusion.partyAddr(slot))
        local w = m.sub[3][8] | (m.sub[3][9] << 8) | (m.sub[3][10] << 16) | (m.sub[3][11] << 24)
        local partner = ((w >> 20) & 0x7F) | (((m.sub[0][11] >> 6) & 3) << 7) | (((w >> 28) & 1) << 9)
            | (((D.read8(Fusion.partyAddr(slot) + 19) >> 7) & 1) << 10)
        D.log(pr[1] .. " partner " .. partner .. " want " .. SPECIES_ID[pr[2]])
    end
    D.scene(12, MAPS.MAP_ROUTE117, 47, 5)
    D.waitFor("the server's fusions", 3000, function() return Fusion.recipeCount(SB3_FUSION) >= 5 end, function(i)
        if i % 300 == 0 then
            local m = D.SYM.gCommunityMailbox
            D.log(string.format("mailbox req %d/%d type %d resp %d type %d status %d online %s recipes %d count %d",
                D.read8(m + 9), D.read8(m + 11), D.read8(m + 10), D.read8(m + 12), D.read8(m + 13), D.read8(m + 14),
                tostring(D.online()), Fusion.recipeCount(SB3_FUSION), D.partyCount()))
        end
    end)
    D.log("recipes: " .. Fusion.recipeCount(SB3_FUSION))
    for i, pr in ipairs(pairs) do
        summary(string.format("%02d-%s", 8 + i, pr[3]:lower()), i, 1)
    end
    D.result("ok")
end)
