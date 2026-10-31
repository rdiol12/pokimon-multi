-- Two players online with their own looks: TEST1 opens the trainer card,
-- then the two battle. Each sees their own look on their back picture and
-- the other player's look on the opponent's picture.
dofile(LOOK_LIB)
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local BRENDAN, MAY = 0, 1

-- {skin, hat/hair, bandana, clothes, dark, hairstyle, headwear, face}
local LOOK1 = LOOK.pack({2, 4, 5, 8, 3, 0, 1, 1})   -- tan, red hat, blue, purple
local LOOK2 = LOOK.pack({4, 3, 7, 2, 4, 2, 3, 0})   -- dark, blonde, pink, orange

local function openStartMenuAction(action, desc)
    D.press(K.START, 30)
    D.waitFor(desc .. " in the start menu", 600, function()
        return D.read8(LOOK_SYMS.sCurrentStartMenuActions + D.read8(LOOK_SYMS.sStartMenuCursorPos)) == action
    end, function(i) if i % 12 == 0 then D.press(K.DOWN, 0) end end)
    D.wait(10)
    D.press(K.A, 0)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    LOOK.set(P1 and LOOK1 or LOOK2, P1 and BRENDAN or MAY)
    D.wait(200)
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 6 or 9, 16)
    D.hold(K.DOWN, 18)
    D.wait(20)
    D.hold(P1 and K.RIGHT or K.LEFT, 4)
    D.wait(200)

    if P1 then
        openStartMenuAction(4, "PLAYER") -- MENU_ACTION_PLAYER
        D.wait(150)
        D.shot("trainer-card")
        D.press(K.B, 0)
        D.waitFor("the overworld", 600, D.inOverworld)
        D.wait(60)
        D.waitFor("an invite", 3000, function() return D.mb8(0x0C9) == 1 end)
        D.wait(90)
        D.press(K.A, 40) -- YES
    else
        D.wait(400)
        -- One step west, next to TEST1.
        D.waitFor("a step west", 120, function() return D.pos() == 7 end, function() D.setKeys(K.LEFT) end)
        D.setKeys(0)
        D.wait(30)
        if select(3, D.pos()) ~= D.DIR.WEST then D.press(K.LEFT, 20) end
        D.press(K.A, 150)
        D.press(K.A, 30) -- BATTLE
    end
    D.waitFor("the battle", 3000, D.inBattle)
    for n = 1, 8 do
        D.wait(40)
        D.shot("battle-" .. n)
    end
end)
