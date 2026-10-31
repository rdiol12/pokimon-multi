-- Voltspire Power Plant: the guide in Mauville (badge check), the lobby,
-- the breakers and barriers, walking through an opened barrier, the
-- generator restart, the Rotom in the appliances and the chief's reward.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P = MAPS.MAP_VOLTSPIRE_PLANT

local function say(dir, name, pages)
    D.talk(dir, name, pages)
    D.wait(30)
end

local function close(n)
    for _ = 1, n do D.press(K.A, 30) end
    D.press(K.B, 30)
end

local function flip(x, y, name)
    D.scene(12, P, x, y)
    D.wait(60)
    D.talk(K.UP, name .. "-ask", 1)
    D.press(K.A, 120)        -- yes
    D.wait(60)
    D.shot(name .. "-flipped")
    close(1)
end

D.run(function()
    D.boot()
    D.wait(60)
    D.season(1)
    D.scene(12, MAPS.MAP_MAUVILLE_CITY, 33, 9, { give = "GYARADOS", level = 60, lead = true })
    D.wait(240)
    say(K.UP, "01-guide-needs-badge", 2)
    close(1)
    D.flag("FLAG_BADGE03_GET", true)
    say(K.UP, "02-guide", 2)
    D.press(K.A, 40)
    D.press(K.A, 40)
    D.wait(200)
    D.waitFor("the overworld", 1200, D.inOverworld)
    D.wait(150)
    D.shot("03-lobby")
    D.scene(12, P, 23, 27); say(K.UP, "04-chief", 4); close(2)
    D.scene(12, P, 21, 22); D.wait(60); D.shot("05-lobby-doors")
    flip(37, 4, "06-red-breaker")
    flip(19, 20, "07-yellow-breaker")
    D.scene(12, P, 17, 24)
    D.wait(60)
    D.hold(K.LEFT, 64)
    D.wait(60)
    D.shot("08-through-the-barrier")
    D.scene(12, P, 10, 8); D.wait(60); D.shot("09-coil-hall")
    D.scene(12, P, 13, 7)
    D.hold(K.RIGHT, 48)
    D.wait(60)
    D.shot("10-into-generator-room")
    D.scene(12, P, 19, 7); say(K.UP, "11-generator", 2)
    D.press(K.A, 120)
    D.wait(120)
    D.shot("12-generator-on")
    close(2)
    D.scene(12, P, 35, 22); D.wait(60); D.shot("13-break-room")
    local xs = {31, 33, 35, 37, 39}
    local found = false
    for i = 0, 24 do
        local x = xs[(i % 5) + 1]
        D.scene(12, P, x, 27)
        D.wait(40)
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 60)
        if i == 0 then D.shot("14-appliance") end
        for _ = 1, 6 do
            D.press(K.A, 40)
            if D.inBattle() then break end
        end
        for _ = 1, 20 do
            if D.inBattle() then found = true break end
            D.wait(15)
        end
        if found then break end
        D.press(K.B, 30)
    end
    D.wait(200)
    D.shot("15-rotom-battle")
    D.result(found and "ok" or "no rotom")
end)
