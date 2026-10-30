-- Unit tests for netplay_bridge.lua with fake memory and a fake server.
-- Run: lua5.4 tools/netplay_bridge/test_bridge.lua

local dir = arg[0]:match("(.*/)") or "./"
package.path = dir .. "?.lua;" .. package.path
local Bridge = require("netplay_bridge")

local BASE = 0x02012340

local function fakeMemory()
    local bytes = {}
    local mem = {}
    function mem.read8(a) return bytes[a] or 0 end
    function mem.write8(a, v) bytes[a] = v & 0xFF end
    function mem.read16(a) return mem.read8(a) | (mem.read8(a + 1) << 8) end
    function mem.write16(a, v) mem.write8(a, v); mem.write8(a + 1, v >> 8) end
    function mem.read32(a) return mem.read16(a) | (mem.read16(a + 2) << 16) end
    function mem.write32(a, v) mem.write16(a, v); mem.write16(a + 2, v >> 16) end
    return mem
end

local function fakeNet()
    local net = { sent = {}, inbox = {}, up = true }
    function net.connect() return net.up end
    function net.send(line) table.insert(net.sent, line) end
    function net.receive() return table.remove(net.inbox, 1) end
    function net.close() end
    function net.push(line) table.insert(net.inbox, line .. "\n") end
    function net.take()
        local s = net.sent
        net.sent = {}
        return s
    end
    return net
end

-- A ROM-side mailbox with the player "Ash" (tid 4660) at (5, 7) on map 0.9.
local function setup()
    local mem, net = fakeMemory(), fakeNet()
    mem.write32(BASE, 0x4E4D4B50)
    mem.write32(BASE + 4, 0x3154454E)
    mem.write8(BASE + 0x08, 1)
    local name = { 0xBB, 0xE7, 0xDC, 0xFF } -- "Ash"
    for i, c in ipairs(name) do mem.write8(BASE + 0x0C + i - 1, c) end
    mem.write16(BASE + 0x14, 4660)
    mem.write8(BASE + 0x16, 0)
    mem.write8(BASE + 0x17, 9)
    mem.write16(BASE + 0x18, 5)
    mem.write16(BASE + 0x1A, 7)
    mem.write8(BASE + 0x1C, 1)
    mem.write8(BASE + 0x1D, 3)
    mem.write16(BASE + 0x1E, 300)
    local bridge = Bridge.new(mem, net)
    return bridge, mem, net
end

local function online()
    local bridge, mem, net = setup()
    bridge:onFrame() -- finds the mailbox, connects
    net.push("WELCOME 3")
    bridge:onFrame()
    net.take()
    return bridge, mem, net
end

local function eq(actual, expected, what)
    if actual ~= expected then
        error(string.format("%s: expected %s, got %s", what or "value", tostring(expected), tostring(actual)), 2)
    end
end

local function list(t) return table.concat(t, " | ") end

local tests = {}

function tests.connects_and_reports_position()
    local bridge, mem, net = setup()
    bridge:onFrame()
    eq(bridge.base, BASE, "mailbox address")
    eq(list(net.take()), "HELLO Ash 4660", "hello")
    net.push("WELCOME 3")
    bridge:onFrame()
    eq(mem.read16(BASE + 0x0A), 3, "local id")
    eq(mem.read8(BASE + 0x09), 2, "online state")
    eq(list(net.take()), "POS 0 9 5 7 1 3 300", "first position")
    bridge:onFrame()
    eq(list(net.take()), "", "unchanged position is not resent")
    mem.write16(BASE + 0x18, 0xFFFF) -- x = -1
    bridge:onFrame()
    eq(list(net.take()), "POS 0 9 -1 7 1 3 300", "moved")
end

function tests.waits_for_a_save_before_connecting()
    local bridge, mem, net = setup()
    mem.write8(BASE + 0x0C, 0xFF)
    bridge:onFrame()
    eq(list(net.take()), "", "no hello without a name")
end

function tests.shows_only_players_on_the_same_map()
    local bridge, mem, net = online()
    net.push("PLAYER 8 Misty 0 9 6 7 2 3 400")
    net.push("PLAYER 9 Brock 1 1 0 0 1 0 100")
    bridge:onFrame()
    eq(mem.read16(BASE + 0x20), 8, "slot 0 id")
    eq(mem.read16(BASE + 0x24), 6, "slot 0 x")
    eq(mem.read8(BASE + 0x28), 2, "slot 0 facing")
    eq(mem.read8(BASE + 0x29), 3, "slot 0 elevation")
    eq(mem.read16(BASE + 0x2A), 400, "slot 0 gfx")
    eq(mem.read8(BASE + 0x2C), 0xC7, "slot 0 name[0] = 'M'")
    eq(mem.read8(BASE + 0x31), 0xFF, "slot 0 name terminator")
    eq(mem.read16(BASE + 0x20 + 20), 0, "slot 1 empty (Brock is elsewhere)")
    net.push("GONE 8")
    bridge:onFrame()
    eq(mem.read16(BASE + 0x20), 0, "slot 0 cleared")
end

function tests.players_keep_their_slot()
    local bridge, mem, net = online()
    net.push("PLAYER 8 Misty 0 9 6 7 2 0 1")
    net.push("PLAYER 9 Brock 0 9 1 1 1 0 1")
    bridge:onFrame()
    local first = mem.read16(BASE + 0x20)
    net.push("PLAYER 8 Misty 0 9 6 8 2 0 1")
    net.push("PLAYER 9 Brock 0 9 1 2 1 0 1")
    bridge:onFrame()
    eq(mem.read16(BASE + 0x20), first, "slot 0 unchanged")
end

function tests.forwards_requests_once_per_seq()
    local bridge, mem, net = online()
    mem.write8(BASE + 0xC1, 1)      -- challenge
    mem.write16(BASE + 0xC2, 8)     -- target
    mem.write8(BASE + 0xC0, 1)      -- seq
    bridge:onFrame()
    eq(list(net.take()), "REQ battle 8", "challenge")
    bridge:onFrame()
    eq(list(net.take()), "", "same seq not resent")
    mem.write8(BASE + 0xC1, 5)      -- result
    mem.write16(BASE + 0xC4, 12)    -- session
    mem.write16(BASE + 0xC6, 2)     -- lost
    mem.write8(BASE + 0xC0, 2)
    bridge:onFrame()
    eq(list(net.take()), "RESULT 12 loss", "result")
end

function tests.delivers_events_one_at_a_time()
    local bridge, mem, net = online()
    net.push("INVITE 12 trade 8")
    net.push("PENDING 13")
    bridge:onFrame()
    eq(mem.read8(BASE + 0xC8), 1, "event seq")
    eq(mem.read8(BASE + 0xC9), 1, "invite")
    eq(mem.read16(BASE + 0xCA), 8, "peer")
    eq(mem.read16(BASE + 0xCC), 12, "session")
    eq(mem.read8(BASE + 0xCE), 2, "kind trade")
    bridge:onFrame()
    eq(mem.read8(BASE + 0xC8), 1, "second event waits for ack")
    mem.write8(BASE + 0xD0, 1)
    bridge:onFrame()
    eq(mem.read8(BASE + 0xC8), 2, "second event seq")
    eq(mem.read8(BASE + 0xC9), 5, "pending")
    eq(mem.read16(BASE + 0xCC), 13, "pending session")
end

local TX, RX = BASE + 0x0D4, BASE + 0x218

function tests.relays_link_packets_both_ways()
    local bridge, mem, net = online()
    -- Leftovers from an earlier session are dropped on START.
    mem.write8(TX, 3)
    mem.write8(RX + 1, 5)
    net.push("START 12 battle 8 1")
    bridge:onFrame()
    eq(mem.read8(BASE + 0xCF), 1, "guest role")
    eq(mem.read8(TX + 1), 3, "tx emptied")
    eq(mem.read8(RX), 5, "rx emptied")
    eq(list(net.take()), "", "nothing sent")
    -- ROM queues one 20-byte packet.
    for i = 0, 19 do mem.write8(TX + 4 + 3 * 20 + i, i) end
    mem.write8(TX, 4)
    bridge:onFrame()
    eq(list(net.take()), "LINK 12 000102030405060708090a0b0c0d0e0f10111213", "tx")
    eq(mem.read8(TX + 1), 4, "tx tail advanced")
    -- Server sends 20 packets; the ring takes 15 (one slot stays empty).
    for i = 1, 20 do net.push(string.format("LINK 12 %02x", i)) end
    net.push("LINK 99 ee") -- other session: ignored
    bridge:onFrame()
    eq(mem.read8(RX), (5 + 15) % 16, "rx head")
    eq(mem.read8(RX + 4 + 5 * 20), 1, "first rx packet")
    mem.write8(RX + 1, 4) -- ROM consumed everything
    bridge:onFrame()
    eq(mem.read8(RX), 9, "rx head after the remaining 5")
    eq(mem.read8(RX + 4 + 8 * 20), 20, "last packet")
end

function tests.reset_drops_the_connection()
    local bridge, mem, net = online()
    local closed = false
    net.close = function() closed = true end
    bridge:onReset()
    eq(closed, true, "closed")
    eq(bridge.base, nil, "rescans")
    for _ = 1, 61 do bridge:onFrame() end
    eq(list(net.take()), "HELLO Ash 4660", "reconnected")
end

function tests.reconnects_after_disconnect()
    local bridge, mem, net = online()
    net.push("PLAYER 8 Misty 0 9 6 7 2 0 1")
    bridge:onFrame()
    table.insert(net.inbox, false)
    bridge:onFrame()
    eq(mem.read8(BASE + 0x09), 0, "offline")
    eq(mem.read16(BASE + 0x20), 0, "remotes cleared")
    for _ = 1, 180 do bridge:onFrame() end
    eq(list(net.take()), "HELLO Ash 4660", "reconnected")
end

local failed = 0
local names = {}
for name in pairs(tests) do table.insert(names, name) end
table.sort(names)
for _, name in ipairs(names) do
    local ok, err = pcall(tests[name])
    print((ok and "ok   " or "FAIL ") .. name .. (ok and "" or ("\n     " .. tostring(err))))
    if not ok then failed = failed + 1 end
end
print(string.format("%d tests, %d failed", #names, failed))
os.exit(failed == 0 and 0 or 1)
