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
local MAILBOX_VERSION = 3

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
local OFF_FRIENDS = 0x43C
local MAX_FRIENDS = 10
local FRIEND_SIZE = 20
local FRIEND_ADD_RESULTS = { ok = 1, full = 2 }
local OFF_SEASON = 0x508
local OFF_MARKET = 0x518
local OFF_REPORT = 0x5E0
local OFF_WATCH = 0x63C
local OFF_WATCH_VIEW = OFF_WATCH + 0x68
local MAX_LIVE = 5
local LIVE_SIZE = 20
local SIDE_SIZE = 12
local OFF_TEXT = 0x1C -- in a battle report
local BATTLE_TEXT_LENGTH = 64
local MARKET_PAGE_SIZE = 5
local MARKET_LISTING_SIZE = 16
local MON_SIZE = 100
local MARKET_ANSWERS = { ok = 1, gone = 2, full = 3, self = 4 }
local NO_TIER = 0xFF
local RING_SIZE = 16
local PACKET_SIZE = 20
local NAME_LENGTH = 8

local STATE_OFFLINE, STATE_CONNECTING, STATE_ONLINE = 0, 1, 2

local REQ_CHALLENGE, REQ_TRADE, REQ_ACCEPT, REQ_DECLINE, REQ_RESULT, REQ_LEAVE, REQ_DEAL, REQ_LADDER, REQ_EMOTE = 1, 2, 3, 4, 5, 6, 7, 8, 9
local REQ_FRIENDS, REQ_FRIEND_ADD, REQ_FRIEND_DEL, REQ_SEASON, REQ_CLAIM = 10, 11, 12, 13, 14
local REQ_MARKET, REQ_MARKET_SELL, REQ_MARKET_BUY, REQ_MARKET_CANCEL, REQ_MARKET_COLLECT = 15, 16, 17, 18, 19
local REQ_LIVE, REQ_WATCH, REQ_UNWATCH, REQ_COOP = 20, 21, 22, 23
local EV_INVITE, EV_START, EV_DECLINED, EV_ENDED, EV_PENDING, EV_EMOTE = 1, 2, 3, 4, 5, 6
local KIND_IDS = { battle = 1, trade = 2, deal = 3, coop = 4 }
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

-- Battle text both ways: punctuation too, and control codes dropped.
local TEXT_TO_ASCII = {
    [0x00] = " ", [0x1B] = "e", [0x2D] = "&", [0x2E] = "+", [0x35] = "=", [0x5B] = "%", [0x5C] = "(", [0x5D] = ")",
    [0xAB] = "!", [0xAC] = "?", [0xAD] = ".", [0xAE] = "-", [0xB0] = "...", [0xB1] = '"', [0xB2] = '"',
    [0xB3] = "'", [0xB4] = "'", [0xB8] = ",", [0xBA] = "/", [0xF0] = ":",
    [0x39] = " ", [0xFA] = " ", [0xFB] = " ", [0xFE] = " ",
}
local ASCII_TO_TEXT = {
    [" "] = 0x00, ["&"] = 0x2D, ["+"] = 0x2E, ["="] = 0x35, ["%"] = 0x5B, ["("] = 0x5C, [")"] = 0x5D,
    ["!"] = 0xAB, ["?"] = 0xAC, ["."] = 0xAD, ["-"] = 0xAE, ['"'] = 0xB1, ["'"] = 0xB4, [","] = 0xB8,
    ["/"] = 0xBA, [":"] = 0xF0,
}

local function textToAscii(bytes)
    local out, i = {}, 1
    while i <= #bytes do
        local c = bytes[i]
        if c == 0xFF then break end
        if c == 0xFC or c == 0xFD then
            i = i + 1 -- a control code and its argument
        else
            out[#out + 1] = charToAscii(c) or TEXT_TO_ASCII[c] or ""
        end
        i = i + 1
    end
    return (table.concat(out):gsub("%s+", " "):gsub("^ ", ""):gsub(" $", ""))
end

local function asciiToText(str)
    local out = {}
    for ch in str:gmatch(".") do
        out[#out + 1] = ASCII_TO_TEXT[ch] or asciiToChar(ch)
    end
    return out
end

local function s16(v)
    if v >= 0x8000 then return v - 0x10000 end
    return v
end

local Bridge = {}
Bridge.__index = Bridge
Bridge.asciiToChar = asciiToChar

-- mem: read8/read16/read32/write8/write16 (address based, like mGBA's emu)
-- net: connect() -> bool, send(line), receive() -> string or nil, close()
function Bridge.new(mem, net, shot)
    local self = setmetatable({}, Bridge)
    self.mem = mem
    self.net = net
    self.frame = 0
    self.base = nil
    self.hooks = {}
    self:resetConnection()
    -- World features (followers, houses, leaderboards, photos): netplay_world.lua.
    if NetplayWorld then self.world = NetplayWorld.new(self, shot) end
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
    self.friendRows = nil -- rows of the friend list being received
    self.friendList = {}  -- the last friend list given to the ROM
    self.marketRows = nil -- rows of the market page being received
    self.marketIds = {}   -- listing ids of the last market page given to the ROM
    self.liveRows = nil   -- live battles being received
    self.liveIds = {}     -- session ids of the last live list given to the ROM
    self.reportSeq = nil  -- the ROM's battle report we last sent
    self.reportText = nil
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

-- Extensions (other netplay features with their own mailbox) add hooks:
--   onLine(args, line) -> true when it handled a server line
--   onFrame(mem)       -> each frame while connected and welcomed
function Bridge:addHook(hook)
    table.insert(self.hooks, hook)
end

function Bridge:handleLine(line)
    if os.getenv("NETPLAY_TRACE") then self:log("<< " .. line:sub(1, 60)) end
    local args = {}
    for word in line:gmatch("%S+") do table.insert(args, word) end
    for _, hook in ipairs(self.hooks) do
        if hook.onLine and hook.onLine(args, line) then return end
    end
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
    elseif cmd == "FRIEND" and self.friendRows then
        table.insert(self.friendRows, {
            name = args[2], tid = args[3], id = tonumber(args[4]),
            group = tonumber(args[5]), num = tonumber(args[6]),
            x = tonumber(args[7]), y = tonumber(args[8]), gfx = tonumber(args[9]), busy = tonumber(args[10]),
        })
    elseif cmd == "FRIENDSEND" and self.friendRows then
        self:writeFriends(self.friendRows)
        self.friendRows = nil
    elseif cmd == "FRIENDADD" then
        self:w8(OFF_FRIENDS + 3, FRIEND_ADD_RESULTS[args[2]] or 0)
        self:w8(OFF_FRIENDS + 2, self:r8(OFF_FRIENDS + 2) + 1)
    elseif cmd == "SEASON" then
        -- SEASON <n> <days left> <rated> <rating> <reward tier or -1> <reward season>
        local tier = tonumber(args[6])
        self:w8(OFF_SEASON + 1, tier >= 0 and tier or NO_TIER)
        self:w16(OFF_SEASON + 2, tonumber(args[2]))
        self:w16(OFF_SEASON + 4, tonumber(args[3]))
        self:w16(OFF_SEASON + 6, math.min(tonumber(args[5]), 0xFFFF))
        self:w8(OFF_SEASON + 8, tonumber(args[4]))
        self:w16(OFF_SEASON + 0x0C, tonumber(args[7]))
        self:w8(OFF_SEASON, self:r8(OFF_SEASON) + 1)
    elseif cmd == "CLAIMED" then
        local tier = tonumber(args[2])
        self:w8(OFF_SEASON + 0x0A, tier >= 0 and tier or NO_TIER)
        if tier >= 0 then self:w8(OFF_SEASON + 1, NO_TIER) end
        self:w8(OFF_SEASON + 9, self:r8(OFF_SEASON + 9) + 1)
    elseif cmd == "MITEM" and self.marketRows then
        table.insert(self.marketRows, {
            id = tonumber(args[2]), seller = args[3], species = tonumber(args[4]),
            level = tonumber(args[5]), price = tonumber(args[6]), mine = tonumber(args[7]),
        })
    elseif cmd == "MARKETEND" and self.marketRows then
        self:writeMarket(self.marketRows, tonumber(args[2]), tonumber(args[3]))
        self.marketRows = nil
    elseif cmd == "MLISTED" then
        self:marketAnswer(args[2])
    elseif cmd == "MBOUGHT" then
        self:marketAnswer(args[2], tonumber(args[3]), args[4])
    elseif cmd == "MCANCELED" then
        self:marketAnswer(args[2], 0, args[3])
    elseif cmd == "MCOLLECT" then
        self:w16(OFF_MARKET + 6, tonumber(args[3]))
        self:marketAnswer("ok", tonumber(args[2]))
    elseif cmd == "LIVE" and self.liveRows then
        table.insert(self.liveRows, { sid = tonumber(args[2]), host = args[3], guest = args[4] })
    elseif cmd == "LIVEEND" and self.liveRows then
        self:writeLive(self.liveRows)
        self.liveRows = nil
    elseif cmd == "WATCHING" then
        self:w8(OFF_WATCH + 2, 0)
    elseif cmd == "WSTATE" then
        for side = 0, 1 do
            local off = OFF_WATCH_VIEW + 4 + side * SIDE_SIZE
            local f = function(i) return tonumber(args[3 + side * 6 + i]) end
            self:w16(off, f(0))
            self:w8(off + 2, f(1))
            self:w16(off + 4, f(2))
            self:w16(off + 6, f(3))
            self:w8(off + 3, f(4))
            self:w8(off + 8, f(5))
        end
        self:w8(OFF_WATCH_VIEW, self:r8(OFF_WATCH_VIEW) + 1)
    elseif cmd == "WTEXT" then
        local text = asciiToText(line:match("^WTEXT %d+ (.*)$") or "")
        for i = 0, BATTLE_TEXT_LENGTH - 1 do
            self:w8(OFF_WATCH_VIEW + OFF_TEXT + i, text[i + 1] or 0xFF)
        end
        self:w8(OFF_WATCH_VIEW + OFF_TEXT + BATTLE_TEXT_LENGTH - 1, 0xFF)
        self:w8(OFF_WATCH_VIEW, self:r8(OFF_WATCH_VIEW) + 1)
    elseif cmd == "WEND" then
        self:w8(OFF_WATCH + 2, 1)
        self:w8(OFF_WATCH_VIEW, self:r8(OFF_WATCH_VIEW) + 1)
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
    elseif self.world then
        self.world:handleLine(cmd, args)
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
    elseif type == REQ_COOP then
        self.net.send("REQ coop " .. target)
    elseif type == REQ_ACCEPT then
        self.net.send("ACCEPT " .. sid)
    elseif type == REQ_DECLINE then
        self.net.send("DECLINE " .. sid)
    elseif type == REQ_RESULT and RESULTS[arg] then
        self.net.send("RESULT " .. sid .. " " .. RESULTS[arg])
    elseif type == REQ_FRIENDS then
        self.friendRows = {}
        self.net.send("FRIENDS")
    elseif type == REQ_FRIEND_ADD then
        self.net.send("FRIEND ADD " .. target)
    elseif type == REQ_FRIEND_DEL then
        local f = self.friendList[arg + 1]
        if f then self.net.send("FRIEND DEL " .. f.name .. " " .. f.tid) end
    elseif type == REQ_SEASON then
        self.net.send("SEASON")
    elseif type == REQ_CLAIM then
        self.net.send("CLAIM")
    elseif type == REQ_MARKET then
        self.marketRows = {}
        self.net.send("MARKET LIST " .. arg)
    elseif type == REQ_MARKET_SELL then
        local hex = {}
        for i = 0, MON_SIZE - 1 do hex[#hex + 1] = string.format("%02x", self:r8(OFF_MARKET + 0x64 + i)) end
        local price = self:r16(OFF_MARKET + 0x5C) | (self:r16(OFF_MARKET + 0x5E) << 16)
        self.net.send(string.format("MARKET SELL %d %d %d %s",
            self:r16(OFF_MARKET + 0x60), self:r8(OFF_MARKET + 0x62), price, table.concat(hex)))
    elseif type == REQ_MARKET_BUY or type == REQ_MARKET_CANCEL then
        local id = self.marketIds[arg + 1]
        if id then
            self.net.send((type == REQ_MARKET_BUY and "MARKET BUY " or "MARKET CANCEL ") .. id)
        else
            self:marketAnswer("gone")
        end
    elseif type == REQ_MARKET_COLLECT then
        self.net.send("MARKET COLLECT")
    elseif type == REQ_LIVE then
        self.liveRows = {}
        self.net.send("LIVE")
    elseif type == REQ_WATCH then
        -- Start from a blank view: the server catches us up.
        for i = 1, OFF_TEXT - 1 do self:w8(OFF_WATCH_VIEW + i, 0) end
        self:w8(OFF_WATCH_VIEW + OFF_TEXT, 0xFF)
        self:w8(OFF_WATCH + 2, 0)
        self:w8(OFF_WATCH_VIEW, self:r8(OFF_WATCH_VIEW) + 1)
        local sid = self.liveIds[arg + 1]
        if sid then
            self.net.send("WATCH " .. sid)
        else
            self:w8(OFF_WATCH + 2, 1)
        end
    elseif type == REQ_UNWATCH then
        self.net.send("UNWATCH")
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

function Bridge:writeFriends(rows)
    local count = math.min(#rows, MAX_FRIENDS)
    self.friendList = {}
    for i = 1, count do
        local f = rows[i]
        local off = OFF_FRIENDS + 4 + (i - 1) * FRIEND_SIZE
        self.friendList[i] = f
        for c = 0, NAME_LENGTH - 1 do
            local ch = f.name:sub(c + 1, c + 1)
            self:w8(off + c, ch == "" and 0xFF or asciiToChar(ch))
        end
        self:w16(off + 8, f.id)
        self:w8(off + 10, f.group)
        self:w8(off + 11, f.num)
        self:w16(off + 12, f.x)
        self:w16(off + 14, f.y)
        self:w16(off + 16, f.gfx)
        self:w8(off + 18, f.busy)
    end
    self:w8(OFF_FRIENDS + 1, count)
    self:w8(OFF_FRIENDS, self:r8(OFF_FRIENDS) + 1) -- publish
end

function Bridge:writeMarket(rows, page, pages)
    local count = math.min(#rows, MARKET_PAGE_SIZE)
    self.marketIds = {}
    for i = 1, count do
        local r = rows[i]
        local off = OFF_MARKET + 0x0C + (i - 1) * MARKET_LISTING_SIZE
        self.marketIds[i] = r.id
        self:w16(off, r.species)
        self:w8(off + 2, r.level)
        self:w8(off + 3, r.mine)
        self:w16(off + 4, r.price & 0xFFFF)
        self:w16(off + 6, r.price >> 16)
        for c = 0, NAME_LENGTH - 1 do
            local ch = r.seller:sub(c + 1, c + 1)
            self:w8(off + 8 + c, ch == "" and 0xFF or asciiToChar(ch))
        end
    end
    self:w8(OFF_MARKET + 1, count)
    self:w8(OFF_MARKET + 2, page)
    self:w8(OFF_MARKET + 3, pages)
    self:w8(OFF_MARKET, self:r8(OFF_MARKET) + 1) -- publish
end

-- The answer to a sell, buy, cancel or collect, with the Pokémon handed back.
function Bridge:marketAnswer(answer, money, monHex)
    if monHex and #monHex == MON_SIZE * 2 then
        for i = 0, MON_SIZE - 1 do
            self:w8(OFF_MARKET + 0x64 + i, tonumber(monHex:sub(i * 2 + 1, i * 2 + 2), 16))
        end
    end
    money = money or 0
    self:w16(OFF_MARKET + 8, money & 0xFFFF)
    self:w16(OFF_MARKET + 10, money >> 16)
    self:w8(OFF_MARKET + 5, MARKET_ANSWERS[answer] or MARKET_ANSWERS.gone)
    self:w8(OFF_MARKET + 4, self:r8(OFF_MARKET + 4) + 1) -- publish
end

function Bridge:writeLive(rows)
    local count = math.min(#rows, MAX_LIVE)
    self.liveIds = {}
    for i = 1, count do
        local r = rows[i]
        local off = OFF_WATCH + 4 + (i - 1) * LIVE_SIZE
        self.liveIds[i] = r.sid
        self:w16(off, r.sid)
        for c = 0, NAME_LENGTH - 1 do
            local h, g = r.host:sub(c + 1, c + 1), r.guest:sub(c + 1, c + 1)
            self:w8(off + 2 + c, h == "" and 0xFF or asciiToChar(h))
            self:w8(off + 10 + c, g == "" and 0xFF or asciiToChar(g))
        end
    end
    self:w8(OFF_WATCH + 1, count)
    self:w8(OFF_WATCH, self:r8(OFF_WATCH) + 1) -- publish
end

-- The ROM reports the battle it hosts; pass changes on for spectators.
function Bridge:sendBattleReport()
    local seq = self:r8(OFF_REPORT)
    if seq == self.reportSeq then return end
    self.reportSeq = seq
    if not self.session then return end
    local fields = {}
    for side = 0, 1 do
        local off = OFF_REPORT + 4 + side * SIDE_SIZE
        fields[#fields + 1] = string.format("%d %d %d %d %d %d", self:r16(off), self:r8(off + 2),
            self:r16(off + 4), self:r16(off + 6), self:r8(off + 3), self:r8(off + 8))
    end
    self.net.send("BSTATE " .. self.session .. " " .. table.concat(fields, " "))
    local bytes = {}
    for i = 0, BATTLE_TEXT_LENGTH - 1 do bytes[#bytes + 1] = self:r8(OFF_REPORT + OFF_TEXT + i) end
    local text = textToAscii(bytes)
    if os.getenv("NETPLAY_TRACE") then
        local hex = {}
        for i = 1, #bytes do hex[i] = string.format("%02x", bytes[i]) end
        self:log("report " .. table.concat(hex))
    end
    if text ~= self.reportText and text ~= "" then
        self.reportText = text
        self.net.send("BTEXT " .. self.session .. " " .. text)
    end
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
    self:step()
    if self.world then self.world:onFrame(self.connected and self.localId ~= 0, self.frame) end
end

-- One frame of the connection: connect, then sync with the server.
function Bridge:step()
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
        self:sendBattleReport()
    end
    self:deliverEvent()
    self:writeRemotes()
    if self.localId ~= 0 then
        for _, hook in ipairs(self.hooks) do
            if hook.onFrame then hook.onFrame(self.mem) end
        end
    end
end

-- World features live in netplay_world.lua next to this file (the server's
-- download has it prepended instead).
if NetplayWorld == nil then
    local source = debug and debug.getinfo(1, "S").source or ""
    local dir = source:match("^@(.*[/\\])") or ""
    pcall(dofile, dir .. "netplay_world.lua")
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
    local bridge = Bridge.new(mem, net, function(path) emu:screenshot(path) end)
    bridge.logger = function(msg) console:log(msg) end

    -- The API extensions see. Each file named in extensions.txt (one per
    -- line, next to this script) is loaded after the bridge starts.
    NetplayBridge = {
        addHook = function(hook) bridge:addHook(hook) end,
        send = function(line) if bridge.connected then net.send(line) end end,
        connected = function() return bridge.connected and bridge.localId ~= 0 end,
    }
    local dir = (script and script.dir) or "."
    local list = io.open(dir .. "/extensions.txt", "r")
    if list then
        for name in list:lines() do
            name = name:match("^%s*(.-)%s*$")
            if name ~= "" and name:sub(1, 1) ~= "#" then
                local ok, err = pcall(dofile, dir .. "/" .. name)
                console:log(ok and ("netplay: loaded " .. name) or ("netplay: cannot load " .. name .. ": " .. tostring(err)))
            end
        end
        list:close()
    end
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
