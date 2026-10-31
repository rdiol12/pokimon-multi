-- Community features for the netplay bridge (docs/community.md): copies the
-- ROM's community mailbox (gCommunityMailbox, include/community.h) to and
-- from the server's community commands (server/community.py).
--
-- The bridge knows nothing about the features themselves. A request goes
-- out as CDATA lines (its bytes) and one C line; the answer comes back as
-- CROW lines (text rows, already in the game's charset) and one CRESP.
-- CEV rewrites an event section, and CPARTY asks for the party's raw bytes
-- (for the server's legality checks).
--
-- netplay_bridge.lua loads this file from its own folder; the server's
-- bridge download has it prepended. It defines the global NetplayCommunity,
-- which the bridge uses as bridge.community.

local EWRAM_START = 0x02000000
local EWRAM_END = 0x02040000
local MAGIC_LO = 0x4E4D4B50 -- "PKMN"
local MAGIC_HI = 0x314D4F43 -- "COM1"
local VERSION = 1

local OFF_VERSION = 0x008
local OFF_REQ_SEQ = 0x009
local OFF_REQ_TYPE = 0x00A
local OFF_REQ_ACK = 0x00B
local OFF_RESP_SEQ = 0x00C
local OFF_RESP_TYPE = 0x00D
local OFF_RESP_STATUS = 0x00E
local OFF_ROW_COUNT = 0x00F
local OFF_REQ_ARGS = 0x010
local OFF_REQ_LEN = 0x020
local OFF_PARTY_COUNT = 0x022
local OFF_MON_SIZE = 0x023
local OFF_PARTY_ADDR = 0x024
local OFF_RESP_ARGS = 0x028
local OFF_RESP_LEN = 0x038
local OFF_REQ_DATA = 0x03C
local OFF_RESP_DATA = 0x13C
local OFF_ROWS = 0x23C
local OFF_EVENTS = 0x3BC
local DATA_SIZE = 0x100
local ROWS = 12
local ROW_LENGTH = 32
local EVENT_SIZE = 0xC0
local EVENT_COUNT = 4
local CHUNK = 96 -- request bytes per CDATA line

local SCAN_INTERVAL = 60

NetplayCommunity = {}
NetplayCommunity.__index = NetplayCommunity

function NetplayCommunity.new(bridge)
    local self = setmetatable({}, NetplayCommunity)
    self.bridge = bridge
    self.mem = bridge.mem
    self.base = nil
    self.rows = {}
    self.online = false
    return self
end

function NetplayCommunity:r8(off) return self.mem.read8(self.base + off) end
function NetplayCommunity:r16(off) return self.mem.read16(self.base + off) end
function NetplayCommunity:r32(off) return self.mem.read32(self.base + off) end
function NetplayCommunity:w8(off, v) self.mem.write8(self.base + off, v & 0xFF) end
function NetplayCommunity:w16(off, v) self.mem.write16(self.base + off, v & 0xFFFF) end
function NetplayCommunity:w32(off, v)
    self:w16(off, v & 0xFFFF)
    self:w16(off + 2, (v >> 16) & 0xFFFF)
end

local function toHex(read, addr, len)
    local hex = {}
    for i = 0, len - 1 do hex[#hex + 1] = string.format("%02x", read(addr + i)) end
    return table.concat(hex)
end

function NetplayCommunity:findMailbox()
    for addr = EWRAM_START, EWRAM_END - 8, 4 do
        if self.mem.read32(addr) == MAGIC_LO and self.mem.read32(addr + 4) == MAGIC_HI
            and self.mem.read8(addr + OFF_VERSION) == VERSION then
            return addr
        end
    end
    return nil
end

-- Writes hex bytes at off, at most max of them. Returns how many.
function NetplayCommunity:writeHex(off, hex, max)
    if hex == nil or hex == "-" then return 0 end
    local n = math.min(#hex // 2, max)
    for i = 0, n - 1 do
        self:w8(off + i, tonumber(hex:sub(i * 2 + 1, i * 2 + 2), 16) or 0)
    end
    return n
end

-- Server lines this module handles. Returns true if it took the line.
function NetplayCommunity:handleLine(cmd, args)
    if cmd == "CROW" then
        if self.base then table.insert(self.rows, args[3] or "") end
    elseif cmd == "CRESP" then
        if not self.base then return true end
        local count = math.min(#self.rows, ROWS)
        for i = 1, count do
            local off = OFF_ROWS + (i - 1) * ROW_LENGTH
            local n = self:writeHex(off, self.rows[i], ROW_LENGTH - 1)
            for c = n, ROW_LENGTH - 1 do self:w8(off + c, 0xFF) end
        end
        self.rows = {}
        for i = 0, 3 do self:w32(OFF_RESP_ARGS + i * 4, tonumber(args[4 + i]) or 0) end
        self:w16(OFF_RESP_LEN, self:writeHex(OFF_RESP_DATA, args[8], DATA_SIZE))
        self:w8(OFF_ROW_COUNT, count)
        self:w8(OFF_RESP_TYPE, tonumber(args[2]) or 0)
        self:w8(OFF_RESP_STATUS, tonumber(args[3]) or 0)
        self:w8(OFF_RESP_SEQ, self:r8(OFF_RESP_SEQ) + 1) -- last: publishes the answer
    elseif cmd == "CEV" then
        local section = tonumber(args[2])
        if self.base and section and section >= 0 and section < EVENT_COUNT then
            local off = OFF_EVENTS + section * EVENT_SIZE
            self:writeHex(off + 1, args[3], EVENT_SIZE - 1)
            self:w8(off, self:r8(off) + 1)
        end
    elseif cmd == "CPARTY" then
        self:sendParty(args[2])
    else
        return false
    end
    return true
end

-- The party's raw bytes, one mon per line.
function NetplayCommunity:sendParty(nonce)
    local net = self.bridge.net
    if not self.base then
        net.send(string.format("CPARTYEND %s 0", nonce))
        return
    end
    local addr, count, size = self:r32(OFF_PARTY_ADDR), math.min(self:r8(OFF_PARTY_COUNT), 6), self:r8(OFF_MON_SIZE)
    for i = 0, count - 1 do
        net.send(string.format("CPARTYMON %s %d %s", nonce, i, toHex(self.mem.read8, addr + i * size, size)))
    end
    net.send(string.format("CPARTYEND %s %d", nonce, count))
end

function NetplayCommunity:handleRequest(net)
    local seq = self:r8(OFF_REQ_SEQ)
    if self:r8(OFF_REQ_ACK) == seq then return end
    local len = math.min(self:r16(OFF_REQ_LEN), DATA_SIZE)
    for start = 0, len - 1, CHUNK do
        local n = math.min(CHUNK, len - start)
        net.send("CDATA " .. toHex(self.mem.read8, self.base + OFF_REQ_DATA + start, n))
    end
    local a = {}
    for i = 0, 3 do a[#a + 1] = tostring(self:r32(OFF_REQ_ARGS + i * 4)) end
    net.send(string.format("C %d %s", self:r8(OFF_REQ_TYPE), table.concat(a, " ")))
    self:w8(OFF_REQ_ACK, seq)
end

-- Called by the bridge every frame. online: connected and welcomed.
function NetplayCommunity:onFrame(online, frame)
    if self.base and not (self.mem.read32(self.base) == MAGIC_LO and self.mem.read32(self.base + 4) == MAGIC_HI) then
        self.base = nil
    end
    if not self.base then
        if frame % SCAN_INTERVAL ~= 3 then return end
        self.base = self:findMailbox()
        if not self.base then return end
        self:w8(OFF_REQ_ACK, self:r8(OFF_REQ_SEQ))
    end
    if online and not self.online then
        self.bridge.net.send("CHELLO")
    end
    self.online = online
    if not online then
        self.rows = {}
        self:w8(OFF_REQ_ACK, self:r8(OFF_REQ_SEQ))
        return
    end
    self:handleRequest(self.bridge.net)
end
