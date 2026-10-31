dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1

local E = { read8 = Driver.read8, write8 = Driver.write8 }
local function r32(a) return E.read8(a) | (E.read8(a+1) << 8) | (E.read8(a+2) << 16) | (E.read8(a+3) << 24) end
local function w32(a, v) for i = 0, 3 do E.write8(a + i, (v >> (8 * i)) & 0xFF) end end
local ORDER1 = {1,1,2,3,2,3,0,0,0,0,0,0,2,3,1,1,3,2,2,3,1,1,3,2}
-- Teaches party slot `slot` a move it could never learn, keeping the checksum
-- right, the way a save editor would.
local function hackMove(slot, move)
    local mon = r32(D.SYM.gCommunityMailbox + 0x024) + slot * 100
    local pers, otid = r32(mon), r32(mon + 4)
    local key = pers ~ otid
    local w = {}
    for i = 0, 11 do w[i] = r32(mon + 32 + i * 4) ~ key end
    local word = ORDER1[pers % 24 + 1] * 3
    w[word] = (w[word] & 0xFFFF0000) | move
    local sum = 0
    for i = 0, 11 do sum = sum + (w[i] & 0xFFFF) + (w[i] >> 16) end
    E.write8(mon + 28, sum & 0xFF); E.write8(mon + 29, (sum >> 8) & 0xFF)
    for i = 0, 11 do w32(mon + 32 + i * 4, w[i] ~ key) end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.showcase({ level = 55, badges = 8, party = P1 and { "SCEPTILE", "SWELLOW", "MANECTRIC", "GARDEVOIR" } or { "BLAZIKEN", "ALTARIA", "AGGRON" } })
    if not P1 then D.wait(1) hackMove(0, 147) end -- Blaziken with Spore
    D.scene(13, MAPS.MAP_OLDALE_TOWN, P1 and 6 or 8, 12)
    local before = D.answers()
    D.community(6)
    D.waitAnswer(before)
    D.wait(40)
    D.shot("rows")
    D.press(K.A, 30)
    D.wait(40)
    D.shot("message")
    D.press(K.A, 30)
    D.wait(60)
end)
