-- Detective cases: the DETECTIVE in Mauville opens THE VANISHED PIE; TEST1
-- asks the three witnesses in Petalburg, then names ZIGZAGOON and solves
-- it. Then R > PASTIMES > CASE FILES.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local CASE, CLUES, SOLVED = 180, 181, 182 -- struct PastimeSaveData offsets

D.PASTIMES_ROW = 11 -- this branch has the BOND, JOBS and ALBUM rows

-- Talks to whoever is above, pressing A through the messages until done().
local function ask(name, done)
    D.talk(K.UP, name, 1)
    local n = 0
    if done() then return end
    D.waitFor(name, 1500, done, function(i)
        if i % 50 == 0 then
            n = n + 1
            if n <= 4 then D.shot(name .. "-" .. n) end
            D.press(K.A, 0)
        end
    end)
    for _ = 1, 3 do D.press(K.B, 30) end
end

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, 17, 11)
    D.wait(30)
    D.shot("agency")
    ask("new-case", function() return D.pastimeRead8(CASE) == 1 end)
    -- The story and "go and ask around", two pages each. (More A presses
    -- would talk to the DETECTIVE again.)
    for i = 1, 4 do
        D.wait(60)
        D.shot("story-" .. i)
        D.press(K.A, 0)
    end
    D.wait(60)
    local witnesses = {{5, 8}, {16, 14}, {13, 21}}
    for n, w in ipairs(witnesses) do
        D.scene(12, MAPS.MAP_PETALBURG_CITY, w[1], w[2])
        D.wait(20)
        if n == 1 then D.shot("witness") end
        ask("clue-" .. n, function() return D.pastimeRead8(CLUES) == (1 << n) - 1 end)
        D.wait(20)
        D.shot("toast-" .. n)
    end
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, 17, 11)
    D.talk(K.UP, "back", 1) -- and on to the suspects
    D.wait(30)
    D.shot("suspects")
    D.press(K.A, 60) -- ZIGZAGOON
    for i = 1, 8 do
        D.wait(60)
        D.shot("solved-" .. i)
        D.press(K.A, 0)
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(19)
    D.wait(30)
    D.shot("case-files")
    D.press(K.B, 60)
end)
