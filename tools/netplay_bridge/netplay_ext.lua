-- Bridge extension for the Online Plaza (docs/netplay_ext.md). Loaded by
-- netplay_bridge.lua through extensions.txt.
--
-- The game's second mailbox (gNetplayExtMailbox) holds one request at a
-- time. Its data goes to the server as XD lines, the request as an X line,
-- and the XR answer is written back. XNOTE MAIL bumps the notes counter.

local Ext = {}
Ext.__index = Ext

local EWRAM_START, EWRAM_END = 0x02000000, 0x02040000
local MAGIC_LO = 0x4E4D4B50 -- "PKMN"
local MAGIC_HI = 0x31545845 -- "EXT1"
local REQ_SEQ = 0x009
local RESP_SEQ = 0x00A
local STATUS = 0x00B
local REQ_LEN = 0x00C
local RESP_LEN = 0x00E
local REQ_TEXT = 0x010
local TEXT_LENGTH = 64
local REQ_DATA = 0x050
local DATA_SIZE = 1024
local RESP = 0x450
local RESP_SIZE = 1536
local NOTES = 0xA50
local CHUNK = 100 -- bytes of data per XD line
local SCAN_INTERVAL = 60

function Ext.new(send)
    return setmetatable({ send = send, base = nil, lastSeq = nil, pending = nil, frame = 0 }, Ext)
end

function Ext:find(mem)
    for addr = EWRAM_START, EWRAM_END - 8, 4 do
        if mem.read32(addr) == MAGIC_LO and mem.read32(addr + 4) == MAGIC_HI then
            return addr
        end
    end
    return nil
end

function Ext:onFrame(mem)
    self.frame = self.frame + 1
    self.mem = mem
    -- A reset or a new ROM moves or clears the mailbox: look for it again.
    if self.base and (mem.read32(self.base) ~= MAGIC_LO or mem.read32(self.base + 4) ~= MAGIC_HI) then
        self.base = nil
    end
    if not self.base then
        if self.frame % SCAN_INTERVAL ~= 1 then return end
        self.base = self:find(mem)
        if not self.base then return end
        self.lastSeq = mem.read8(self.base + REQ_SEQ)
        self.pending = nil
    end
    local base = self.base
    local seq = mem.read8(base + REQ_SEQ)
    if seq == self.lastSeq then return end
    self.lastSeq = seq
    local text = {}
    for i = 0, TEXT_LENGTH - 1 do
        local b = mem.read8(base + REQ_TEXT + i)
        if b == 0 then break end
        -- Keep the line to printable ASCII so it can't split into two commands.
        text[#text + 1] = (b >= 32 and b < 127) and string.char(b) or " "
    end
    local len = math.min(mem.read16(base + REQ_LEN), DATA_SIZE)
    for start = 0, len - 1, CHUNK do
        local hex = {}
        for i = start, math.min(start + CHUNK, len) - 1 do
            hex[#hex + 1] = string.format("%02x", mem.read8(base + REQ_DATA + i))
        end
        self.send("XD " .. table.concat(hex))
    end
    self.pending = seq
    self.send("X " .. seq .. " " .. table.concat(text))
end

function Ext:onLine(args)
    local mem, base = self.mem, self.base
    if args[1] == "XR" then
        local seq, status, hex = tonumber(args[2]), tonumber(args[3]), args[4] or ""
        if base and seq == self.pending then
            self.pending = nil
            local n = math.min(#hex // 2, RESP_SIZE)
            for i = 0, n - 1 do
                mem.write8(base + RESP + i, tonumber(hex:sub(2 * i + 1, 2 * i + 2), 16))
            end
            mem.write16(base + RESP_LEN, n)
            mem.write8(base + STATUS, status)
            mem.write8(base + RESP_SEQ, seq) -- seq last: publishes the answer
        end
        return true
    elseif args[1] == "XNOTE" then
        if base then mem.write8(base + NOTES, (mem.read8(base + NOTES) + 1) & 0xFF) end
        return true
    end
    return false
end

if NetplayBridge ~= nil then
    local ext = Ext.new(NetplayBridge.send)
    NetplayBridge.addHook({
        onLine = function(args) return ext:onLine(args) end,
        onFrame = function(mem) ext:onFrame(mem) end,
    })
end

if emu == nil then
    return Ext
end
