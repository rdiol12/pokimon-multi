-- Helpers for the trainer look scenarios.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver
local K = D.KEY

LOOK = {}

function LOOK.cb2() return D.read32(D.SYM.gMain + 4) & ~1 end
function LOOK.onScreen() return LOOK.cb2() == LOOK_SYMS.CB2_Look end

-- struct SaveBlock2: playerStyle at 0x90, trainerLook at 0x91.
function LOOK.set(look, style)
    local sb2 = D.read32(D.SYM.gSaveBlock2Ptr)
    for i = 0, 3 do D.write8(sb2 + 0x91 + i, (look >> (i * 8)) & 0xFF) end
    if style then D.write8(sb2 + 0x90, style + 1) end
end

-- Packs fields {skin, hat, band, clothes, dark, hairstyle, headwear, face}.
function LOOK.pack(f)
    local v = 0
    for i, x in ipairs(f) do v = v | ((x or 0) << ((i - 1) * 4)) end
    return v
end

-- From the title screen through Professor Birch to the Look screen.
function LOOK.newGameToLookScreen(female)
    D.waitFor("the main menu", 3000, function() return LOOK.cb2() == LOOK_SYMS.CB2_MainMenu end, function(i)
        if i % 60 == 0 then D.press(K.START, 0) end
    end)
    D.wait(60)
    D.press(K.A, 30) -- NEW GAME
    -- A through the speech; on the naming screen, START jumps to OK and
    -- A keeps the suggested name.
    D.waitFor("the Look screen", 20000, LOOK.onScreen, function(i)
        if i % 30 ~= 0 then return end
        if LOOK.cb2() == LOOK_SYMS.CB2_NamingScreen then
            D.press(K.START, 20)
        end
        D.press(K.A, 0)
    end)
    D.wait(60)
end

-- struct LookScreen: the cursor row is at +14.
function LOOK.cursor() return D.read8(D.read32(LOOK_SYMS.sLook) + 14) end
function LOOK.go(row)
    D.waitFor("row " .. row, 400, function() return LOOK.cursor() == row end, function(i)
        if i % 12 == 0 then D.press(LOOK.cursor() > row and K.UP or K.DOWN, 0) end
    end)
    D.wait(8)
end
function LOOK.change(row, times, key)
    LOOK.go(row)
    for _ = 1, times do D.press(key or K.RIGHT, 10) end
end
ROW = {BODY = 0, SKIN = 1, HAT = 2, BAND = 3, CLOTHES = 4, DARK = 5, HAIRSTYLE = 6, HEADWEAR = 7, FACE = 8, RANDOM = 9, DONE = 10}

-- Opens START > STYLE from the field.
function LOOK.openFromStartMenu()
    local STYLE = 15 -- MENU_ACTION_STYLE
    D.press(K.START, 30)
    D.waitFor("STYLE in the start menu", 600, function()
        return D.read8(LOOK_SYMS.sCurrentStartMenuActions + D.read8(LOOK_SYMS.sStartMenuCursorPos)) == STYLE
    end, function(i) if i % 12 == 0 then D.press(K.DOWN, 0) end end)
    D.wait(10)
    D.press(K.A, 0)
    D.waitFor("the Look screen", 600, LOOK.onScreen)
    D.wait(60)
end
