-- 32 PC boxes (was 14): boxes 15-32 are kept in the extended save.
-- Opens the PC on box 32 with a few Pokémon in it.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local MON = 80

local function boxMon(box, pos)
    return D.read32(D.SYM.gPokemonStoragePtr) + 4 + (box * 30 + pos) * MON
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(13, MAPS.MAP_PETALBURG_CITY, 15, 12, { give = "RALTS", level = 25 })
    -- The party, copied into box 32.
    for i = 0, D.partyCount() - 1 do
        local from = D.SYM.gParties + i * 100
        D.log("party " .. i .. ": " .. D.partyMon(i).nickname)
        for b = 0, MON - 1 do
            D.write8(boxMon(31, i * 2) + b, D.read8(from + b))
        end
    end
    D.write8(D.read32(D.SYM.gPokemonStoragePtr), 31) -- current box: 32
    D.wait(30)
    emu:write32(D.SYM.gMain + 4, D.SYM.CB2_ReturnToPokeStorage | 1)
    D.wait(240)
    D.shot("pc-box-32")
    D.press(K.UP, 30) -- to the box name: left and right change box
    D.press(K.RIGHT, 90)
    D.shot("pc-box-1-after-32")
    D.press(K.LEFT, 90)
    D.press(K.LEFT, 90)
    D.shot("pc-box-31")
    D.result("ok")
end)
