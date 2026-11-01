-- Scars and lost parts outside battle (src/injury.c): the following Pokémon,
-- the party menu, the summary and the PC.
-- Run: python3 tools/injury/scars.py <out dir>
-- @common
local MON = 80

local function boxMon(box, pos)
    return D.read32(D.SYM.gPokemonStoragePtr) + 4 + (box * 30 + pos) * MON
end

local function backToField()
    D.waitFor("the overworld", 900, D.inOverworld, function(i)
        if i % 50 == 25 then D.press(K.B, 0) end
    end)
    D.wait(40)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 6, 10, { give = "MIGHTYENA", level = 30, lead = true })
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 6, 10, { give = "CHARIZARD", level = 36, lead = true })
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 6, 10, { give = "PIKACHU", level = 30, lead = true })
    -- Pikachu: scar and a lost tail. Charizard: scar, lost wing and eye. Mightyena: scar.
    setInjury(0, PART.NONE, 0, bit(PART.TAIL), FLAG.SCAR)
    setInjury(1, PART.NONE, 0, bit(PART.WING) | bit(PART.EYE), FLAG.SCAR)
    setInjury(2, PART.NONE, 0, 0, FLAG.SCAR)
    -- Reload the map so the follower is drawn again.
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 6, 10)
    D.hold(K.DOWN, 16); D.wait(40)
    D.shot("01-follower-pikachu")
    D.hold(K.RIGHT, 16); D.wait(40)
    D.shot("02-follower-pikachu-side")
    D.hold(K.UP, 16); D.wait(10)
    D.hold(K.UP, 16); D.wait(40)
    D.shot("03-follower-pikachu-down")

    -- Party menu.
    D.press(K.START, 60)
    D.press(K.A, 140) -- POKéMON
    D.wait(30)
    D.shot("04-party")
    D.press(K.A, 40)  -- Pikachu
    D.press(K.A, 160) -- SUMMARY
    D.shot("05-summary-pikachu")
    D.press(K.DOWN, 120)
    D.shot("06-summary-charizard")
    D.press(K.B, 150)
    backToField()

    -- Charizard leads.
    for b = 0, 99 do
        local a, c = D.SYM.gParties + b, D.SYM.gParties + 100 + b
        local x, y = D.read8(a), D.read8(c)
        D.write8(a, y); D.write8(c, x)
    end
    D.scene(13, MAPS.MAP_OLDALE_TOWN, 6, 10)
    D.wait(60)
    D.shot("07-follower-charizard")
    D.hold(K.LEFT, 16); D.wait(40)
    D.shot("08-follower-charizard-side")

    -- PC: the party copied into box 1.
    for i = 0, D.partyCount() - 1 do
        local from = D.SYM.gParties + i * 100
        for b = 0, MON - 1 do
            D.write8(boxMon(0, i) + b, D.read8(from + b))
        end
    end
    D.write8(D.read32(D.SYM.gPokemonStoragePtr), 0)
    D.wait(30)
    emu:write32(D.SYM.gMain + 4, D.SYM.CB2_ReturnToPokeStorage | 1)
    D.wait(240)
    D.shot("09-pc-box")
    D.press(K.RIGHT, 60)
    D.shot("10-pc-box-pikachu")
    D.press(K.LEFT, 60)
    D.shot("11-pc-box-charizard")
    D.result("ok")
end)
