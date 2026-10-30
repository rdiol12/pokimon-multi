-- Netplay bridge for mGBA 0.10+.
-- Copies the ROM's netplay mailbox (gNetplayMailbox) to and from the netplay
-- server once per frame. Layout and protocol: docs/netplay.md.
--
-- Load in mGBA with Tools > Scripting > File > Load script. It connects
-- once the player picks JOIN ONLINE, to the server built into the game
-- (include/config/netplay.h). The globals NETPLAY_HOST / NETPLAY_PORT, set
-- before loading, override that address.

local EWRAM_START = 0x02000000
local EWRAM_END = 0x02040000
local MAGIC_LO = 0x4E4D4B50 -- "PKMN"
local MAGIC_HI = 0x3154454E -- "NET1"
local MAILBOX_VERSION = 2

local OFF_VERSION = 0x008
local OFF_STATE = 0x009
local OFF_LOCAL_ID = 0x00A
local OFF_NAME = 0x00C
local OFF_TID = 0x014
local OFF_MAP_GROUP = 0x016
local OFF_MAP_NUM = 0x017
local OFF_X = 0x018
local OFF_Y = 0x01A
local OFF_FACING = 0x01C
local OFF_ELEVATION = 0x01D
local OFF_GFX = 0x01E
local OFF_REMOTES = 0x020
local REMOTE_SIZE = 20
local MAX_REMOTES = 8
local OFF_REQUEST = 0x0C0
local OFF_EVENT = 0x0C8
local OFF_EVENT_ACK = 0x0D0
local OFF_TX = 0x0D4
local OFF_RX = 0x218
local OFF_JOIN_ONLINE = 0x0D2
local OFF_SERVER_HOST = 0x35C
local HOST_LENGTH = 40
local OFF_SERVER_PORT = 0x384
local OFF_LADDER = 0x388
local LADDER_SIZE = 10
local LADDER_ENTRY_SIZE = 16
local RING_SIZE = 16
local PACKET_SIZE = 20
local NAME_LENGTH = 8

local STATE_OFFLINE, STATE_CONNECTING, STATE_ONLINE = 0, 1, 2

local REQ_CHALLENGE, REQ_TRADE, REQ_ACCEPT, REQ_DECLINE, REQ_RESULT, REQ_LEAVE, REQ_DEAL, REQ_LADDER, REQ_EMOTE = 1, 2, 3, 4, 5, 6, 7, 8, 9
local EV_INVITE, EV_START, EV_DECLINED, EV_ENDED, EV_PENDING, EV_EMOTE = 1, 2, 3, 4, 5, 6
local KIND_IDS = { battle = 1, trade = 2, deal = 3 }
local RESULTS = { "win", "loss", "draw" }

local SCAN_INTERVAL = 60
local RECONNECT_INTERVAL = 180

-- Game charset <-> ASCII, letters and digits only.
local function charToAscii(c)
    if c >= 0xA1 and c <= 0xAA then return string.char(c - 0xA1 + 48) end
    if c >= 0xBB and c <= 0xD4 then return string.char(c - 0xBB + 65) end
    if c >= 0xD5 and c <= 0xEE then return string.char(c - 0xD5 + 97) end
    return nil
end

local function asciiToChar(ch)
    local b = ch:byte()
    if b >= 48 and b <= 57 then return b - 48 + 0xA1 end
    if b >= 65 and b <= 90 then return b - 65 + 0xBB end
    if b >= 97 and b <= 122 then return b - 97 + 0xD5 end
    return 0x00 -- space
end

local function s16(v)
    if v >= 0x8000 then return v - 0x10000 end
    return v
end

local Bridge = {}
Bridge.__index = Bridge

-- mem: read8/read16/read32/write8/write16 (address based, like mGBA's emu)
-- net: connect() -> bool, send(line), receive() -> string or nil, close()
function Bridge.new(mem, net)
    local self = setmetatable({}, Bridge)
    self.mem = mem
    self.net = net
    self.frame = 0
    self.base = nil
    self:resetConnection()
    return self
end

function Bridge:resetConnection()
    self.connected = false
    self.localId = 0
    self.buffer = ""
    self.players = {}     -- id -> {name, group, num, x, y, facing, gfx}
    self.slots = {}       -- slot index -> player id
    self.events = {}      -- queued events for the ROM
    self.rxQueue = {}     -- queued link packets (byte strings) for the ROM
    self.session = nil
    self.lastPos = nil
    self.ladder = nil     -- rows of the ladder being received
    self.nextConnect = self.frame
end

-- The game was reset or a different ROM loaded: drop the connection so the
-- server ends our sessions, and reconnect once the game is back.
function Bridge:onReset()
    if self.connected then
        self.net.close()
        self:resetConnection()
    end
    self.base = nil
end

function Bridge:log(msg)
    if self.logger then self.logger(msg) end
end

-- Mailbox access ---------------------------------------------------------

function Bridge:r8(off) return self.mem.read8(self.base + off) end
function Bridge:r16(off) return self.mem.read16(self.base + off) end
function Bridge:w8(off, v) self.mem.write8(self.base + off, v & 0xFF) end
function Bridge:w16(off, v) self.mem.write16(self.base + off, v & 0xFFFF) end

function Bridge:findMailbox()
    for addr = EWRAM_START, EWRAM_END - 8, 4 do
        if self.mem.read32(addr) == MAGIC_LO and self.mem.read32(addr + 4) == MAGIC_HI
            and self.mem.read8(addr + OFF_VERSION) == MAILBOX_VERSION then
            return addr
        end
    end
    return nil
end

function Bridge:attach(addr)
    self.base = addr
    self.lastReqSeq = self:r8(OFF_REQUEST)
    self:w8(OFF_STATE, self.connected and STATE_ONLINE or STATE_OFFLINE)
    self:log(string.format("netplay mailbox found at 0x%08X", addr))
end

function Bridge:serverAddress()
    local host = {}
    for i = 0, HOST_LENGTH - 1 do
        local b = self:r8(OFF_SERVER_HOST + i)
        if b == 0 then break end
        host[#host + 1] = string.char(b)
    end
    host = table.concat(host)
    local port = self:r16(OFF_SERVER_PORT)
    return NETPLAY_HOST or (host ~= "" and host or "127.0.0.1"), NETPLAY_PORT or (port ~= 0 and port or 7777)
end

function Bridge:mailboxValid()
    return self.mem.read32(self.base) == MAGIC_LO and self.mem.read32(self.base + 4) == MAGIC_HI
end

function Bridge:readName()
    local name = ""
    for i = 0, NAME_LENGTH - 1 do
        local c = self:r8(OFF_NAME + i)
        if c == 0xFF then break end
        name = name .. (charToAscii(c) or "")
    end
    return name:sub(1, 7)
end

function Bridge:readLocal()
    return {
        group = self:r8(OFF_MAP_GROUP),
        num = self:r8(OFF_MAP_NUM),
        x = s16(self:r16(OFF_X)),
        y = s16(self:r16(OFF_Y)),
        facing = self:r8(OFF_FACING),
        elevation = self:r8(OFF_ELEVATION),
        gfx = self:r16(OFF_GFX),
    }
end

-- Server messages --------------------------------------------------------

function Bridge:queueEvent(type, peer, sid, kind, role)
    table.insert(self.events, { type = type, peer = peer, sid = sid, kind = kind or 0, role = role or 0 })
end

function Bridge:handleLine(line)
    local args = {}
    for word in line:gmatch("%S+") do table.insert(args, word) end
    local cmd = args[1]
    if cmd == "WELCOME" then
        self.localId = tonumber(args[2])
        self:w16(OFF_LOCAL_ID, self.localId)
        self:w8(OFF_STATE, STATE_ONLINE)
    elseif cmd == "PLAYER" then
        self.players[tonumber(args[2])] = {
            name = args[3],
            group = tonumber(args[4]), num = tonumber(args[5]),
            x = tonumber(args[6]), y = tonumber(args[7]),
            facing = tonumber(args[8]), elevation = tonumber(args[9]),
            gfx = tonumber(args[10]),
        }
    elseif cmd == "GONE" then
        self.players[tonumber(args[2])] = nil
    elseif cmd == "INVITE" then
        self:queueEvent(EV_INVITE, tonumber(args[4]), tonumber(args[2]), KIND_IDS[args[3]])
    elseif cmd == "PENDING" then
        self:queueEvent(EV_PENDING, 0, tonumber(args[2]))
    elseif cmd == "START" then
        self.session = tonumber(args[2])
        self.rxQueue = {}
        -- Drop anything left in the rings from an earlier session.
        self:w8(OFF_RX, self:r8(OFF_RX + 1))
        self:w8(OFF_TX + 1, self:r8(OFF_TX))
        self:queueEvent(EV_START, tonumber(args[4]), self.session, KIND_IDS[args[3]], tonumber(args[5]))
    elseif cmd == "DECLINED" then
        self:queueEvent(EV_DECLINED, 0, tonumber(args[2]))
    elseif cmd == "ENDED" then
        local sid = tonumber(args[2])
        if sid == self.session then
            -- Keep rxQueue: the peer's last packets arrived before ENDED and
            -- the ROM still needs them to finish its link.
            self.session = nil
        end
        self:queueEvent(EV_ENDED, 0, sid)
    elseif cmd == "LINK" then
        if tonumber(args[2]) == self.session then
            local hex = args[3]
            local bytes = hex:gsub("%x%x", function(h) return string.char(tonumber(h, 16)) end)
            table.insert(self.rxQueue, bytes)
        end
    elseif cmd == "EMOTE" then
        self:queueEvent(EV_EMOTE, tonumber(args[2]), 0, tonumber(args[3]))
    elseif cmd == "RANK" and self.ladder then
        table.insert(self.ladder, {
            name = args[3], rating = tonumber(args[4]),
            wins = tonumber(args[5]), losses = tonumber(args[6]), draws = tonumber(args[7]),
        })
    elseif cmd == "BOARDEND" and self.ladder then
        self:writeLadderTop(self.ladder)
    elseif cmd == "MYRANK" and self.ladder then
        self.ladder = nil
        self:writeLadderMe(tonumber(args[2]), {
            name = self:readName(), rating = tonumber(args[3]),
            wins = tonumber(args[4]), losses = tonumber(args[5]), draws = tonumber(args[6]),
        })
    elseif cmd == "ERR" then
        self:log("server: " .. line)
    end
end

function Bridge:pump()
    while true do
        local data = self.net.receive()
        if data == nil then break end
        if data == false then -- connection lost
            self:log("netplay: disconnected")
            self.net.close()
            self:resetConnection()
            self.nextConnect = self.frame + RECONNECT_INTERVAL
            self:w8(OFF_STATE, STATE_OFFLINE)
            self:writeRemotes()
            return
        end
        self.buffer = self.buffer .. data
    end
    while true do
        local nl = self.buffer:find("\n", 1, true)
        if not nl then break end
        local line = self.buffer:sub(1, nl - 1)
        self.buffer = self.buffer:sub(nl + 1)
        self:handleLine(line)
    end
end

-- Mailbox sync -----------------------------------------------------------

function Bridge:writeRemotes()
    local here = self.connected and self:readLocal() or nil
    local function visible(id)
        local p = self.players[id]
        return here and p and p.group == here.group and p.num == here.num
    end
    -- Keep players in the slot they already have so the ROM's sprites stay put.
    local placed = {}
    for slot = 0, MAX_REMOTES - 1 do
        local id = self.slots[slot]
        if id and visible(id) then placed[id] = true else self.slots[slot] = nil end
    end
    for id in pairs(self.players) do
        if not placed[id] and visible(id) then
            for slot = 0, MAX_REMOTES - 1 do
                if not self.slots[slot] then
                    self.slots[slot] = id
                    break
                end
            end
        end
    end
    for slot = 0, MAX_REMOTES - 1 do
        local off = OFF_REMOTES + slot * REMOTE_SIZE
        local id = self.slots[slot]
        local p = id and self.players[id]
        if p then
            self:w8(off + 2, p.group)
            self:w8(off + 3, p.num)
            self:w16(off + 4, p.x)
            self:w16(off + 6, p.y)
            self:w8(off + 8, p.facing)
            self:w8(off + 9, p.elevation)
            self:w16(off + 10, p.gfx)
            for i = 0, NAME_LENGTH - 1 do
                local ch = p.name:sub(i + 1, i + 1)
                self:w8(off + 12 + i, ch == "" and 0xFF or asciiToChar(ch))
            end
            self:w16(off, id) -- id last: a non-zero id marks the slot as filled
        else
            self:w16(off, 0)
        end
    end
end

function Bridge:sendPosition()
    local pos = self:readLocal()
    local last = self.lastPos
    if last and last.group == pos.group and last.num == pos.num and last.x == pos.x and last.y == pos.y
        and last.facing == pos.facing and last.elevation == pos.elevation and last.gfx == pos.gfx then
        return
    end
    self.lastPos = pos
    self.net.send(string.format("POS %d %d %d %d %d %d %d",
        pos.group, pos.num, pos.x, pos.y, pos.facing, pos.elevation, pos.gfx))
end

function Bridge:handleRequest()
    local seq = self:r8(OFF_REQUEST)
    if seq == self.lastReqSeq then return end
    self.lastReqSeq = seq
    local type = self:r8(OFF_REQUEST + 1)
    local target = self:r16(OFF_REQUEST + 2)
    local sid = self:r16(OFF_REQUEST + 4)
    local arg = self:r16(OFF_REQUEST + 6)
    if type == REQ_CHALLENGE then
        self.net.send("REQ battle " .. target)
    elseif type == REQ_TRADE then
        self.net.send("REQ trade " .. target)
    elseif type == REQ_DEAL then
        self.net.send("REQ deal " .. target)
    elseif type == REQ_ACCEPT then
        self.net.send("ACCEPT " .. sid)
    elseif type == REQ_DECLINE then
        self.net.send("DECLINE " .. sid)
    elseif type == REQ_RESULT and RESULTS[arg] then
        self.net.send("RESULT " .. sid .. " " .. RESULTS[arg])
    elseif type == REQ_EMOTE then
        self.net.send("EMOTE " .. arg)
    elseif type == REQ_LADDER then
        self.ladder = {}
        self.net.send("BOARD")
        self.net.send("MYRANK")
    elseif type == REQ_LEAVE then
        self.net.send("QUIT " .. sid)
        if sid == self.session then self.session = nil end
    end
end

function Bridge:writeLadderEntry(off, e)
    for i = 0, NAME_LENGTH - 1 do
        local ch = e.name:sub(i + 1, i + 1)
        self:w8(off + i, ch == "" and 0xFF or asciiToChar(ch))
    end
    self:w16(off + 8, math.min(e.rating, 0xFFFF))
    self:w16(off + 10, math.min(e.wins, 0xFFFF))
    self:w16(off + 12, math.min(e.losses, 0xFFFF))
    self:w16(off + 14, math.min(e.draws, 0xFFFF))
end

function Bridge:writeLadderTop(rows)
    local count = math.min(#rows, LADDER_SIZE)
    for i = 1, count do
        self:writeLadderEntry(OFF_LADDER + 0x14 + (i - 1) * LADDER_ENTRY_SIZE, rows[i])
    end
    self:w8(OFF_LADDER + 1, count)
end

-- MYRANK comes last, so the whole ladder is in place: publish it.
function Bridge:writeLadderMe(rank, me)
    self:w16(OFF_LADDER + 2, rank)
    if rank ~= 0 then self:writeLadderEntry(OFF_LADDER + 4, me) end
    self:w8(OFF_LADDER, self:r8(OFF_LADDER) + 1)
end

function Bridge:deliverEvent()
    local ev = self.events[1]
    if not ev then return end
    local seq = self:r8(OFF_EVENT)
    if self:r8(OFF_EVENT_ACK) ~= seq then return end -- ROM still busy with the last one
    table.remove(self.events, 1)
    self:w8(OFF_EVENT + 1, ev.type)
    self:w16(OFF_EVENT + 2, ev.peer)
    self:w16(OFF_EVENT + 4, ev.sid)
    self:w8(OFF_EVENT + 6, ev.kind)
    self:w8(OFF_EVENT + 7, ev.role)
    self:w8(OFF_EVENT, seq + 1) -- seq last: publishes the event
end

function Bridge:syncLink()
    -- ROM -> server
    local head, tail = self:r8(OFF_TX), self:r8(OFF_TX + 1)
    while tail ~= head do
        if self.session then
            local hex = {}
            local p = OFF_TX + 4 + tail * PACKET_SIZE
            for i = 0, PACKET_SIZE - 1 do hex[#hex + 1] = string.format("%02x", self:r8(p + i)) end
            self.net.send("LINK " .. self.session .. " " .. table.concat(hex))
        end
        tail = (tail + 1) % RING_SIZE
    end
    self:w8(OFF_TX + 1, tail)
    -- server -> ROM
    head, tail = self:r8(OFF_RX), self:r8(OFF_RX + 1)
    while #self.rxQueue > 0 and (head + 1) % RING_SIZE ~= tail do
        local bytes = table.remove(self.rxQueue, 1)
        local p = OFF_RX + 4 + head * PACKET_SIZE
        for i = 0, PACKET_SIZE - 1 do self:w8(p + i, bytes:byte(i + 1) or 0) end
        head = (head + 1) % RING_SIZE
    end
    self:w8(OFF_RX, head)
end

function Bridge:onFrame()
    self.frame = self.frame + 1
    if self.base and not self:mailboxValid() then
        self.base = nil
    end
    if not self.base then
        if self.frame % SCAN_INTERVAL == 1 then
            local addr = self:findMailbox()
            if addr then self:attach(addr) end
        end
        if not self.base then return end
    end

    if not self.connected then
        if self.frame < self.nextConnect then return end
        if self:r8(OFF_JOIN_ONLINE) == 0 then return end -- playing offline
        local name = self:readName()
        if name == "" then return end -- no save loaded yet
        self:w8(OFF_STATE, STATE_CONNECTING)
        local host, port = self:serverAddress()
        if self.net.connect(host, port) then
            self.connected = true
            self.net.send(string.format("HELLO %s %d", name, self:r16(OFF_TID)))
        else
            self.nextConnect = self.frame + RECONNECT_INTERVAL
            self:w8(OFF_STATE, STATE_OFFLINE)
            return
        end
    end

    self:pump()
    if not self.connected then return end
    -- Link packets go out before requests, so the last packets of a link
    -- reach the peer before our QUIT ends the session.
    self:syncLink()
    if self.localId ~= 0 then
        self:sendPosition()
        self:handleRequest()
    end
    self:deliverEvent()
    self:writeRemotes()
end

-- mGBA glue --------------------------------------------------------------

if emu ~= nil and callbacks ~= nil and socket ~= nil then
    local sock = nil
    local net = {
        connect = function(host, port)
            sock = socket.tcp()
            local ok, err = sock:connect(host, port)
            if not ok then
                console:log("netplay: cannot reach " .. host .. ":" .. port .. " (" .. tostring(err) .. ")")
                sock = nil
                return false
            end
            console:log("netplay: connected to " .. host .. ":" .. port)
            return true
        end,
        send = function(line)
            if sock then sock:send(line .. "\n") end
        end,
        receive = function()
            if not sock then return false end
            sock:poll()
            if not sock:hasdata() then return nil end
            local data, err = sock:receive(4096)
            if data == nil or data == "" then
                if err == socket.ERRORS.AGAIN then return nil end
                return false
            end
            return data
        end,
        close = function()
            if sock then pcall(function() sock:close() end) end
            sock = nil
            collectgarbage()
        end,
    }
    local mem = {
        read8 = function(a) return emu:read8(a) end,
        read16 = function(a) return emu:read16(a) end,
        read32 = function(a) return emu:read32(a) end,
        write8 = function(a, v) emu:write8(a, v) end,
        write16 = function(a, v) emu:write16(a, v) end,
    }
    local bridge = Bridge.new(mem, net)
    bridge.logger = function(msg) console:log(msg) end
    local failed = false
    callbacks:add("frame", function()
        if failed then return end
        local ok, err = pcall(bridge.onFrame, bridge)
        if not ok then
            failed = true
            console:log("netplay bridge stopped: " .. tostring(err))
        end
    end)
    callbacks:add("reset", function() bridge:onReset() end)
    console:log("netplay bridge loaded; it connects after JOIN ONLINE")
end

-- Only return the module when loaded by the unit tests: mGBA leaves a script's
-- return value on the Lua stack, which breaks its later callback calls.
if emu == nil then
    return Bridge
end
