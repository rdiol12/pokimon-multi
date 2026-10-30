-- Unit tests for netplay_world.lua (the world features part of the bridge).
-- Run: lua5.4 tools/netplay_bridge/test_world.lua

local dir = arg[0]:match("(.*/)") or "./"
package.path = dir .. "?.lua;" .. package.path
local Bridge = require("netplay_bridge")

local NET = 0x02012340
local WLD = 0x02020000

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
    local net = { sent = {}, inbox = {} }
    function net.connect() return true end
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

local function eq(actual, expected, what)
    if actual ~= expected then
        error(string.format("%s: expected %s, got %s", what or "value", tostring(expected), tostring(actual)), 2)
    end
end

local function has(lines, line)
    for _, l in ipairs(lines) do if l == line then return true end end
    return false
end

-- Both mailboxes; "Ash" on map 0.9, online with id 3.
local function online(shot)
    local mem, net = fakeMemory(), fakeNet()
    mem.write32(NET, 0x4E4D4B50)
    mem.write32(NET + 4, 0x3154454E)
    mem.write8(NET + 0x08, 2)
    mem.write8(NET + 0x0D2, 1)
    for i, c in ipairs({ 0xBB, 0xE7, 0xDC, 0xFF }) do mem.write8(NET + 0x0C + i - 1, c) end
    mem.write8(NET + 0x17, 9)
    mem.write32(WLD, 0x4E4D4B50)
    mem.write32(WLD + 4, 0x31444C57)
    mem.write8(WLD + 0x08, 1)
    local bridge = Bridge.new(mem, net, shot)
    for _ = 1, 3 do bridge:onFrame() end -- finds both mailboxes, connects
    net.push("WELCOME 3")
    bridge:onFrame()
    bridge:onFrame()
    return bridge, mem, net
end

local tests = {}

function tests.asks_for_the_world_and_sends_our_look()
    local bridge, mem, net = online()
    mem.write16(WLD + 0x018, 0x4000 + 258) -- following Mudkip
    mem.write8(WLD + 0x01C, 3)             -- a table in the house
    bridge:onFrame()
    local sent = net.take()
    assert(has(sent, "WORLD"), "WORLD sent")
    assert(has(sent, "LOOK 16642 0"), "look sent")
    assert(has(sent, "HOUSE 0 03000000000000000000000000000000"), "house sent")
    bridge:onFrame()
    eq(#net.take(), 0, "nothing new")
end

function tests.other_players_looks_go_in_their_slot()
    local bridge, mem, net = online()
    net.push("PLAYER 7 Misty 0 9 4 4 1 0 100")
    net.push("LOOK 7 16409 16462")
    net.push("HOUSE 7 1 0102000000000000000000000000000a")
    bridge:onFrame()
    eq(bridge.slots[0], 7, "netplay slot")
    eq(mem.read16(WLD + 0x034), 7, "id")
    eq(mem.read16(WLD + 0x036), 16409, "follower")
    eq(mem.read16(WLD + 0x038), 16462, "mount")
    eq(mem.read8(WLD + 0x03A), 1, "house open")
    eq(mem.read8(WLD + 0x03C), 1, "decor 0")
    eq(mem.read8(WLD + 0x04B), 10, "decor 15")
end

function tests.score_and_board_requests()
    local bridge, mem, net = online()
    net.take()
    mem.write8(WLD + 0x00A, 1) -- SCORE
    mem.write8(WLD + 0x00B, 0)
    mem.write32(WLD + 0x010, 1234)
    mem.write32(WLD + 0x014, 129)
    mem.write8(WLD + 0x009, mem.read8(WLD + 0x009) + 1)
    bridge:onFrame()
    assert(has(net.take(), "SCORE 0 0 1234 129"), "score sent")
    eq(mem.read8(WLD + 0x00E), mem.read8(WLD + 0x009), "acked")
    mem.write8(WLD + 0x00A, 2) -- BOARD
    mem.write8(WLD + 0x009, mem.read8(WLD + 0x009) + 1)
    bridge:onFrame()
    assert(has(net.take(), "WBOARD 0 0"), "board asked")
    net.push("WROW 0 1 Misty 1500 72")
    net.push("WROW 0 2 Ash 900 129")
    net.push("WBOARDEND 0 2")
    bridge:onFrame()
    eq(mem.read8(WLD + 0x02C), 1, "board seq")
    eq(mem.read8(WLD + 0x02E), 2, "rows")
    eq(mem.read8(WLD + 0x02F), 2, "my rank")
    eq(mem.read8(WLD + 0x0F4), 0xC7, "first row name starts with M")
    eq(mem.read32(WLD + 0x0F4 + 8), 1500, "value")
    eq(mem.read32(WLD + 0x104 + 12), 129, "second row extra")
end

function tests.hunt_and_photo()
    local saved = nil
    local path = os.tmpname()
    local bridge, mem, net = online(function(p)
        saved = p
        local f = io.open(p, "wb")
        f:write("\137PNG" .. string.rep("x", 200))
        f:close()
    end)
    bridge.world.photoDir = path:match("(.*/)")
    net.push("HUNT 42 17")
    bridge:onFrame()
    eq(mem.read16(WLD + 0x030), 42, "hunt id")
    eq(mem.read8(WLD + 0x032), 17, "minutes left")
    net.take()
    mem.write8(WLD + 0x00F, 1)
    bridge:onFrame()
    local sent = net.take()
    assert(saved, "screenshot taken")
    os.remove(saved)
    os.remove(path)
    eq(sent[1]:sub(1, 14), "PHOTO 89504e47", "photo data")
    eq(#sent, 3, "two chunks and the end")
    eq(sent[3], "PHOTOEND 0 9", "photo end")
end

local failed, count = 0, 0
local names = {}
for name in pairs(tests) do table.insert(names, name) end
table.sort(names)
for _, name in ipairs(names) do
    count = count + 1
    local ok, err = pcall(tests[name])
    if ok then
        print("ok   " .. name)
    else
        failed = failed + 1
        print("FAIL " .. name .. ": " .. tostring(err))
    end
end
print(string.format("%d tests, %d failed", count, failed))
os.exit(failed == 0 and 0 or 1)
