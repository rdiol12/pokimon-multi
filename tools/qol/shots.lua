-- Quality-of-life pack and emote wheel, with two players (run with
-- tools/world_shots/run.py tools/qol/shots.lua <out> 200000 2):
-- TEST1 saves, goes through the new OPTION rows (INSTANT text, AUTO-RUN,
-- AUTOSAVE, COLORS, CONTROLS), runs without B, sends emotes and a phrase
-- from the wheel, shows each colour mode, and walks into the Poké Center to
-- autosave. TEST2 watches the emotes and sends one back.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local OLDALE = MAPS.MAP_OLDALE_TOWN
local CENTER = MAPS.MAP_OLDALE_TOWN_POKEMON_CENTER_1F

-- SaveBlock2 options byte 0x15: bit 4 AUTO-RUN, bit 5 AUTOSAVE, bits 6-7 COLORS.
local function setColors(mode)
    local sb2 = D.read32(D.SYM.gSaveBlock2Ptr)
    D.write8(sb2 + 0x15, (D.read8(sb2 + 0x15) & 0x3F) | (mode << 6))
end

local function sawEmote(id)
    return function() return D.mb8(0x0C9) == 6 and D.mb8(0x0CE) == id end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if P1 then
        D.scene(12, OLDALE, 9, 13)
    else
        D.scene(12, OLDALE, 13, 13)
    end
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(60)

    if P1 then
        -- Save once by hand (autosave never writes over another save file).
        D.press(K.START, 30)
        for _ = 1, 5 do D.press(K.DOWN, 8) end -- SAVE
        D.press(K.A, 90)
        D.press(K.A, 240)
        D.shot("saved-by-hand")
        D.press(K.A, 60)
        D.waitFor("the save to close", 600, function() return D.read8(D.SYM.sLockFieldControls) == 0 end, function(i)
            if i % 40 == 0 then D.press(K.B, 0) end
        end)
        D.wait(30)

        -- OPTION
        D.press(K.START, 30)
        D.press(K.DOWN, 8) -- OPTION (the cursor stays on SAVE)
        D.press(K.A, 90)
        D.press(K.RIGHT, 20) -- TEXT SPEED: FAST -> INSTANT
        D.shot("options-instant-text")
        for _ = 1, 6 do D.press(K.DOWN, 8) end
        D.press(K.LEFT, 20) -- AUTO-RUN: ON
        D.shot("options-auto-run")
        D.press(K.DOWN, 8)
        D.press(K.LEFT, 20) -- AUTOSAVE: ON
        D.shot("options-autosave")
        D.press(K.DOWN, 8)
        D.press(K.RIGHT, 30) -- COLORS: RED-WEAK, shown at once
        D.shot("options-colors-red-weak")
        D.press(K.LEFT, 30) -- back to NORMAL
        D.press(K.DOWN, 8)
        D.shot("options-controls")
        D.press(K.A, 30)
        D.shot("controls")
        D.press(K.RIGHT, 20) -- the A button's job moves to B (and B's to A)
        D.shot("controls-swapped")
        for _ = 1, 6 do D.press(K.DOWN, 8) end
        D.press(K.A, 20) -- USE DEFAULTS
        D.shot("controls-defaults")
        D.press(K.B, 30)
        D.press(K.B, 90)
        D.press(K.B, 30) -- the START menu
        D.waitFor("the field", 600, function() return D.read8(D.SYM.sLockFieldControls) == 0 end)

        -- AUTO-RUN: hold DOWN for 32 frames with no B and TEST1 runs 4 tiles;
        -- with B held TEST1 walks 2.
        local sb1 = D.read32(D.SYM.gSaveBlock1Ptr)
        local shoes = sb1 + 0x1270 + (0x8C0 >> 3) -- SaveBlock1.flags, FLAG_SYS_B_DASH (Running Shoes)
        D.write8(shoes, D.read8(shoes) | 1)
        local _, y0 = D.pos()
        D.hold(K.DOWN, 32)
        D.wait(20)
        local _, y1 = D.pos()
        D.scene(12, OLDALE, 9, 13)
        D.hold(K.DOWN | K.B, 32)
        D.wait(20)
        local _, y2 = D.pos()
        D.log(string.format("auto-run: %d tiles running, %d tiles walking with B", y1 - y0, y2 - y0))
        AUTO_RUN = { y1 - y0, y2 - y0 }
        D.scene(12, OLDALE, 9, 13)

        -- Emote wheel.
        D.press(K.L, 20)
        D.shot("wheel-faces")
        D.hold(K.UP | K.RIGHT, 6)
        D.wait(10)
        D.shot("wheel-pointing-love")
        D.press(K.R, 20)
        D.shot("wheel-more")
        D.press(K.UP, 10) -- THUMBS UP
        D.press(K.A, 18)
        D.shot("emote-thumbs-up")
        D.wait(150)
        D.press(K.L, 20)
        D.press(K.R, 20) -- PHRASES
        D.press(K.LEFT, 10) -- FOLLOW ME!
        D.shot("wheel-phrases")
        D.press(K.A, 20)
        D.shot("phrase-follow-me")
        D.waitFor("TEST2's emote", 1200, sawEmote(15))
        D.wait(16)
        D.shot("sees-lol")
        D.wait(200)

        -- COLORS on the overworld.
        local names = { [0] = "normal", [1] = "red-weak", [2] = "green-weak", [3] = "blue-weak" }
        for mode = 0, 3 do
            setColors(mode)
            D.wait(10)
            D.shot("colors-" .. names[mode])
        end
        setColors(0)

        -- AUTOSAVE: a minute of play after the last save, walk into the Poké Center.
        D.wait(60 * 60)
        D.scene(12, OLDALE, 6, 17)
        D.hold(K.UP, 16)
        D.waitFor("the Poké Center", 600, function()
            return D.mb8(0x016) == CENTER[1] and D.mb8(0x017) == CENTER[2] and D.inOverworld()
                and D.read8(D.SYM.sLockFieldControls) == 0
        end)
        D.wait(12)
        D.shot("autosaving")
        D.wait(190) -- the save takes about two seconds, then "Saved!" shows for 1.5
        D.shot("autosaved")
    else
        D.waitFor("an emote", 2400, sawEmote(14))
        D.wait(16)
        D.shot("sees-thumbs-up")
        D.waitFor("a phrase", 1200, sawEmote(22))
        D.wait(20)
        D.shot("sees-follow-me")
        D.wait(200)
        D.press(K.L, 20)
        D.press(K.R, 20) -- MORE
        D.hold(K.UP | K.RIGHT, 6) -- LOL
        D.wait(10)
        D.shot("wheel-lol")
        D.press(K.A, 18)
        D.shot("sends-lol")
        D.wait(600)
    end
    D.result(AUTO_RUN and string.format("ok (auto-run %d tiles, with B %d)", AUTO_RUN[1], AUTO_RUN[2]) or "ok")
end)
