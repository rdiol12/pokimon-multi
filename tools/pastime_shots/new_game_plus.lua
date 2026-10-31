-- New Game+: TEST1 is CHAMPION (FLAG_SYS_GAME_CLEAR), opens R > PASTIMES >
-- NEW GAME+ and starts over from the moving truck. Afterwards the party is
-- in the PC, the money and the pastime records are kept, and the bag has
-- the Shiny Charm; checks.txt has the numbers.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local NGP_COUNT, BOWL_BEST = 192, 160 -- struct PastimeSaveData offsets
local SB1_MONEY, SB1_FLAGS, FLAG_SYS_GAME_CLEAR = 1168, 4720, 2148

D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

local function sb1(off) return D.read32(D.SYM.gSaveBlock1Ptr) + off end
local function partyCount() return D.read8(D.SYM.gPartiesCount) end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_LILYCOVE_CITY, 30, 23)
    local flagByte = sb1(SB1_FLAGS + FLAG_SYS_GAME_CLEAR // 8)
    D.write8(flagByte, D.read8(flagByte) | (1 << (FLAG_SYS_GAME_CLEAR % 8)))
    D.pastime8(BOWL_BEST, 77)
    local f = io.open(INSTANCE_DIR .. "/checks.txt", "w")
    f:write(string.format("before: party=%d money(enc)=%d bowlBest=%d ngp=%d\n", partyCount(),
        D.read32(sb1(SB1_MONEY)), D.pastimeRead8(BOWL_BEST), D.pastimeRead8(NGP_COUNT)))
    D.shot("before")
    D.pastimeMenu(21)
    for i = 1, 5 do
        D.wait(60)
        D.shot("explain-" .. i)
        D.press(K.A, 0)
    end
    D.wait(60)
    D.press(K.A, 60) -- yes
    D.wait(60)
    D.shot("sure")
    D.press(K.A, 0) -- yes again
    for i = 1, 16 do
        D.wait(90)
        D.shot("start-" .. i)
    end
    local storage = D.read32(D.SYM.gPokemonStoragePtr)
    f:write(string.format("after: party=%d money(enc)=%d bowlBest=%d ngp=%d box1slot1personality=%d\n", partyCount(),
        D.read32(sb1(SB1_MONEY)), D.pastimeRead8(BOWL_BEST), D.pastimeRead8(NGP_COUNT), D.read32(storage + 4)))
    f:close()
end)
