-- Haven Reserve with all twenty spots filled (eight on the GBA): 11 shared
-- Pokémon from the server (crowd.py seeds them), six the player left while
-- offline and the day's three strays.
--   python3 tools/reserve_shots/crowd.py OUT
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- SaveBlock3.reserve (include/reserve_save.h): six offline Pokémon.
local RESERVE = 1388 -- offsetof(struct SaveBlock3, reserve)
local LOCAL = { "PIKACHU", "EEVEE", "SKITTY", "BULBASAUR", "PSYDUCK", "TOGEPI" }

local function leaveOffline()
    local r = emu:read32(D.SYM.gSaveBlock3Ptr) + RESERVE
    for i, name in ipairs(LOCAL) do
        local m = r + (i - 1) * 12
        emu:write32(m, 0x1000 * i + 7)
        emu:write32(m + 4, 0x3FFFFFFF)
        emu:write16(m + 8, SPECIES_ID[name])
        emu:write8(m + 10, 12 + i)
    end
    emu:write8(r + 6 * 12 + 3, #LOCAL)
end

D.run(function()
    D.boot()
    D.summer()
    D.waitFor("the server", 600, D.online)
    leaveOffline()
    D.scene(12, MAPS.MAP_VERDANTURF_TOWN, 14, 10)
    D.wait(400) -- the season banner
    D.talk(K.UP, "gate-guard", 1)
    -- A through the guard's talk (the first YES/NO's cursor is on YES)
    -- until the warp into the reserve.
    local sb1 = emu:read32(D.SYM.gSaveBlock1Ptr)
    local R = MAPS.MAP_HAVEN_RESERVE
    D.waitFor("the reserve", 3000, function()
        return emu:read8(sb1 + 4) == R[1] and emu:read8(sb1 + 5) == R[2] and D.inOverworld()
    end, function(i)
        if i % 30 == 0 then D.press(K.A, 0) end
    end)
    D.wait(300)
    D.scene(12, MAPS.MAP_HAVEN_RESERVE, 11, 10)
    D.wait(240)
    D.shot("meadow-west")
    D.scene(12, MAPS.MAP_HAVEN_RESERVE, 24, 10)
    D.wait(240)
    D.shot("meadow-east")
    D.scene(12, MAPS.MAP_HAVEN_RESERVE, 17, 14)
    D.wait(240)
    D.shot("meadow-middle")
    local n = 0
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * 0x24
        local id = emu:read8(o + 8)
        if emu:read8(o) & 1 == 1 and emu:read8(o + 2) & 1 == 0 and ((id >= 1 and id <= 8) or (id >= 14 and id <= 25)) then n = n + 1 end
    end
    D.log("roaming Pokémon out near the camera: " .. n .. ", spots filled: " .. emu:read8(D.SYM.sShownCount))
    D.result("ok")
end)
