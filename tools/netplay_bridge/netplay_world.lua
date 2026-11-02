-- World features for the netplay bridge (docs/world.md): copies the ROM's
-- world mailbox (gWorldMailbox, include/world_mailbox.h) to and from the
-- server's world commands (server/world.py), and saves photos.
--
-- netplay_bridge.lua loads this file from its own folder; the server's
-- bridge download has it prepended. It defines the global NetplayWorld,
-- which the bridge uses as bridge.world.

local EWRAM_START = 0x02000000
local EWRAM_END = 0x02040000
local MAGIC_LO = 0x4E4D4B50 -- "PKMN"
local MAGIC_HI = 0x31444C57 -- "WLD1"
local VERSION = 2

local OFF_VERSION = 0x008
local OFF_REQ_SEQ = 0x009
local OFF_REQ_TYPE = 0x00A
local OFF_REQ_BOARD = 0x00B
local OFF_REQ_KEY = 0x00C
local OFF_REQ_ACK = 0x00E
local OFF_PHOTO_SEQ = 0x00F
local OFF_REQ_VALUE = 0x010
local OFF_REQ_EXTRA = 0x014
local OFF_FOLLOWER = 0x018
local OFF_MOUNT = 0x01A
local OFF_DECOR = 0x01C
local OFF_BOARD_SEQ = 0x02C
local OFF_BOARD_ID = 0x02D
local OFF_BOARD_COUNT = 0x02E
local OFF_BOARD_RANK = 0x02F
local OFF_HUNT_ID = 0x030
local OFF_HUNT_LEFT = 0x032
local OFF_HOUSE_OPEN = 0x033
local OFF_REMOTES = 0x034
local REMOTE_SIZE = 0x18
local OFF_ROWS = 0x0F4
local OFF_ACCESSORY = 0x194
local OFF_CLOCK_SET = 0x195
local OFF_CLOCK_MINUTE = 0x196
local OFF_TRAINER_LOOK = 0x198  -- u32: colours and accessories (include/trainer_look.h)
local OFF_REMOTE_TRAINER_LOOKS = 0x19C -- u32 per slot
local ROW_SIZE = 0x10
local BOARD_ROWS = 10
local DECOR_SIZE = 16
local MAX_REMOTES = 24 -- slots 0-7 at OFF_REMOTES, 8-23 at OFF_MORE_REMOTES
local MAILBOX_REMOTES = 8
local OFF_MORE_REMOTES = 0x1BC
local OFF_MORE_TRAINER_LOOKS = 0x33C
local OFF_FOLLOWER_INJURY = 0x37C -- u8: the follower's scar and lost parts (INJURY_LOOKS_*)
local OFF_REMOTE_INJURIES = 0x37D -- u8 per slot 0-23
local NAME_LENGTH = 8

local REQ_SCORE, REQ_BOARD = 1, 2
local SCAN_INTERVAL = 60
local HUNT_INTERVAL = 60 * 60 -- frames between hunt checks (about a minute)
local GBA_FPS = 59.7275       -- frames per second, for ticking the server clock
local PHOTO_CHUNK = 120       -- bytes per PHOTO line (240 hex digits)

NetplayWorld = {}
NetplayWorld.__index = NetplayWorld

-- bridge: the netplay Bridge (its mem, net, players and slots).
-- shot(path): saves a screenshot as PNG (mGBA's emu:screenshot), or nil.
function NetplayWorld.new(bridge, shot)
    local self = setmetatable({}, NetplayWorld)
    self.bridge = bridge
    self.mem = bridge.mem
    self.shot = shot
    -- Photos are saved next to the script.
    local source = debug and debug.getinfo(1, "S").source or ""
    self.photoDir = source:match("^@(.*[/\\])") or ""
    self.base = nil
    self.looks = {}  -- player id -> {follower, mount}
    self.houses = {} -- player id -> {open, decor (16 bytes)}
    self:reset()
    -- Shared wild spawns and map channels (NetplayShared, below).
    if NetplayShared then self.shared = NetplayShared.new(bridge) end
    return self
end

function NetplayWorld:reset()
    self.online = false
    self.lastLook = nil
    self.lastHouse = nil
    self.rows = nil
    self.nextHunt = 0
    self.clock = nil -- {seconds = server seconds since midnight, frame = when we heard it}
    self.looks = {}
    self.houses = {}
end

function NetplayWorld:r8(off) return self.mem.read8(self.base + off) end
function NetplayWorld:r16(off) return self.mem.read16(self.base + off) end
function NetplayWorld:r32(off) return self.mem.read32(self.base + off) end
function NetplayWorld:w8(off, v) self.mem.write8(self.base + off, v & 0xFF) end
function NetplayWorld:w16(off, v) self.mem.write16(self.base + off, v & 0xFFFF) end
function NetplayWorld:w32(off, v)
    self:w16(off, v & 0xFFFF)
    self:w16(off + 2, (v >> 16) & 0xFFFF)
end

function NetplayWorld:findMailbox()
    for addr = EWRAM_START, EWRAM_END - 8, 4 do
        if self.mem.read32(addr) == MAGIC_LO and self.mem.read32(addr + 4) == MAGIC_HI
            and self.mem.read8(addr + OFF_VERSION) == VERSION then
            return addr
        end
    end
    return nil
end

function NetplayWorld:attach(addr)
    self.base = addr
    self.lastPhoto = self:r8(OFF_PHOTO_SEQ)
    self:w8(OFF_REQ_ACK, self:r8(OFF_REQ_SEQ))
end

-- Keeps the server's clock ticking in the mailbox between CLOCK lines, so
-- the ROM's day and night follow the server (src/world_daynight.c).
function NetplayWorld:writeClock(frame)
    if not self.clock then
        self:w8(OFF_CLOCK_SET, 0)
        return
    end
    local seconds = self.clock.seconds + (frame - self.clock.frame) / GBA_FPS
    self:w16(OFF_CLOCK_MINUTE, math.floor(seconds / 60) % 1440)
    self:w8(OFF_CLOCK_SET, 1)
end

function NetplayWorld:decorHex(off)
    local hex = {}
    for i = 0, DECOR_SIZE - 1 do hex[#hex + 1] = string.format("%02x", self:r8(off + i)) end
    return table.concat(hex)
end

-- Server lines this module handles. Returns true if it took the line.
function NetplayWorld:handleLine(cmd, args)
    if self.shared and self.shared:handleLine(cmd, args) then
        return true
    end
    if cmd == "LOOK" then
        self.looks[tonumber(args[2])] = { follower = tonumber(args[3]), mount = tonumber(args[4]),
                                          accessory = tonumber(args[5]) or 0, trainer = tonumber(args[6]) or 0,
                                          injury = tonumber(args[7]) or 0 }
    elseif cmd == "HOUSE" then
        self.houses[tonumber(args[2])] = { open = tonumber(args[3]), decor = args[4] }
    elseif cmd == "CLOCK" then
        -- CLOCK seconds: the server's time of day, in seconds since midnight.
        self.clock = { seconds = tonumber(args[2]) or 0, frame = self.frame or 0 }
    elseif cmd == "HUNT" and self.base then
        self:w8(OFF_HUNT_LEFT, math.min(tonumber(args[3]), 255))
        self:w16(OFF_HUNT_ID, tonumber(args[2]))
    elseif cmd == "WROW" then
        self.rows = self.rows or {}
        table.insert(self.rows, { name = args[4], value = tonumber(args[5]), extra = tonumber(args[6]) })
    elseif cmd == "WBOARDEND" and self.base then
        local rows = self.rows or {}
        self.rows = nil
        local count = math.min(#rows, BOARD_ROWS)
        for i = 1, count do
            local off = OFF_ROWS + (i - 1) * ROW_SIZE
            for c = 0, NAME_LENGTH - 1 do
                local ch = rows[i].name:sub(c + 1, c + 1)
                self:w8(off + c, ch == "" and 0xFF or self.bridge.asciiToChar(ch))
            end
            self:w32(off + 8, rows[i].value)
            self:w32(off + 12, rows[i].extra)
        end
        self:w8(OFF_BOARD_ID, tonumber(args[2]))
        self:w8(OFF_BOARD_COUNT, count)
        self:w8(OFF_BOARD_RANK, tonumber(args[3]))
        self:w8(OFF_BOARD_SEQ, self:r8(OFF_BOARD_SEQ) + 1) -- seq last: publishes the rows
    elseif cmd == "GONE" then
        return false -- the bridge's line too; looks stay until the player disconnects
    else
        return false
    end
    return true
end

function NetplayWorld:sendLook(net)
    local look = string.format("LOOK %d %d %d %d %d", self:r16(OFF_FOLLOWER), self:r16(OFF_MOUNT), self:r8(OFF_ACCESSORY),
                               self:r32(OFF_TRAINER_LOOK), self:r8(OFF_FOLLOWER_INJURY))
    if look ~= self.lastLook then
        self.lastLook = look
        net.send(look)
    end
    local house = string.format("HOUSE %d %s", self:r8(OFF_HOUSE_OPEN), self:decorHex(OFF_DECOR))
    if house ~= self.lastHouse then
        self.lastHouse = house
        net.send(house)
    end
end

function NetplayWorld:handleRequest(net)
    local seq = self:r8(OFF_REQ_SEQ)
    if self:r8(OFF_REQ_ACK) == seq then return end
    local type, board, key = self:r8(OFF_REQ_TYPE), self:r8(OFF_REQ_BOARD), self:r16(OFF_REQ_KEY)
    if type == REQ_SCORE then
        net.send(string.format("SCORE %d %d %d %d", board, key, self:r32(OFF_REQ_VALUE), self:r32(OFF_REQ_EXTRA)))
    elseif type == REQ_BOARD then
        self.rows = {}
        net.send(string.format("WBOARD %d %d", board, key))
    end
    self:w8(OFF_REQ_ACK, seq)
end

-- Other players' looks and houses, in the netplay slots the bridge gave them.
function NetplayWorld:writeRemotes()
    for slot = 0, MAX_REMOTES - 1 do
        local more = slot >= MAILBOX_REMOTES
        local off = more and OFF_MORE_REMOTES + (slot - MAILBOX_REMOTES) * REMOTE_SIZE or OFF_REMOTES + slot * REMOTE_SIZE
        local lookOff = more and OFF_MORE_TRAINER_LOOKS + (slot - MAILBOX_REMOTES) * 4 or OFF_REMOTE_TRAINER_LOOKS + slot * 4
        local id = self.online and self.bridge.slots[slot]
        local look = id and self.looks[id]
        local house = id and self.houses[id]
        if id and (look or house) then
            self:w16(off + 2, look and look.follower or 0)
            self:w16(off + 4, look and look.mount or 0)
            self:w8(off + 6, house and house.open or 0)
            self:w8(off + 7, look and look.accessory or 0)
            self:w32(lookOff, look and look.trainer or 0)
            self:w8(OFF_REMOTE_INJURIES + slot, look and look.injury or 0)
            for i = 0, DECOR_SIZE - 1 do
                local byte = house and tonumber(house.decor:sub(i * 2 + 1, i * 2 + 2), 16) or 0
                self:w8(off + 8 + i, byte)
            end
            self:w16(off, id)
        else
            self:w16(off, 0)
            self:w32(lookOff, 0)
            self:w8(OFF_REMOTE_INJURIES + slot, 0)
        end
    end
end

-- The ROM bumped photoSeq: save a screenshot, and upload it when online.
function NetplayWorld:takePhoto(net)
    local seq = self:r8(OFF_PHOTO_SEQ)
    if seq == self.lastPhoto then return end
    self.lastPhoto = seq
    if not self.shot then return end
    local dir = self.photoDir or ""
    local path = string.format("%sphoto-%s-%d.png", dir, os.date("%Y%m%d-%H%M%S"), seq)
    local ok = pcall(self.shot, path)
    if not ok then return end
    self.bridge:log("photo saved: " .. path)
    if not self.online then return end
    local f = io.open(path, "rb")
    if not f then return end
    local data = f:read("a")
    f:close()
    for i = 1, #data, PHOTO_CHUNK do
        local chunk = data:sub(i, i + PHOTO_CHUNK - 1)
        net.send("PHOTO " .. chunk:gsub(".", function(c) return string.format("%02x", c:byte()) end))
    end
    local b = self.bridge
    net.send(string.format("PHOTOEND %d %d", b.base and b:r8(0x016) or 0, b.base and b:r8(0x017) or 0))
end

-- Called by the bridge every frame. online: connected and welcomed.
function NetplayWorld:onFrame(online, frame)
    self.frame = frame
    if self.base and not (self.mem.read32(self.base) == MAGIC_LO and self.mem.read32(self.base + 4) == MAGIC_HI) then
        self.base = nil
    end
    if not self.base then
        if frame % SCAN_INTERVAL ~= 2 then return end
        local addr = self:findMailbox()
        if not addr then return end
        self:attach(addr)
    end
    local net = self.bridge.net
    if online and not self.online then
        net.send("WORLD") -- everyone's looks and houses, and the hunt
        self.nextHunt = frame + HUNT_INTERVAL
    elseif not online and self.online then
        self:reset()
        self:w16(OFF_HUNT_ID, 0)
        self:w8(OFF_CLOCK_SET, 0)
    end
    self.online = online
    if self.shared then self.shared:onFrame(self, online) end
    self:takePhoto(net)
    if not online then
        self:w8(OFF_REQ_ACK, self:r8(OFF_REQ_SEQ))
        self:writeRemotes()
        return
    end
    if frame >= self.nextHunt then
        self.nextHunt = frame + HUNT_INTERVAL
        net.send("HUNT")
        net.send("CLOCK")
    end
    self:writeClock(frame)
    self:sendLook(net)
    self:handleRequest(net)
    self:writeRemotes()
end


-- Shared wild spawns and map channels (docs/shared_map.md) ----------------
-- The ROM's gSharedMapMailbox (include/shared_map.h), found through the
-- world mailbox's `shared` pointer, to and from server/shared_map.py.

local SH_PTR = 0x398 -- in the world mailbox
local SH_MAGIC_HI = 0x31524853 -- "SHR1"
local SH_VERSION = 1
local SH = {
    REQ_SEQ = 0x009, REQ_TYPE = 0x00A, REQ_ARG = 0x00B, REQ_OWNER = 0x00C, REQ_UID = 0x00E, REQ_ACK = 0x010,
    CLAIM_SEQ = 0x011, CLAIM_RESULT = 0x012, CHANNEL = 0x013, CHANNEL_COUNT = 0x014, CHANNEL_SEQ = 0x015,
    LIST_SEQ = 0x016, LIST_COUNT = 0x017, CLAIMER = 0x018, RARE_SEQ = 0x020, JOIN_RESULT = 0x021,
    LOCAL_ID = 0x022, RARE_OWNER = 0x024, RARE_UID = 0x026, OWN = 0x028, REMOTE = 0x058, ROWS = 0x118,
    OWN_CLAIMER = 0x178,
}
local SH_OWN, SH_OWN_SIZE = 4, 0x0C
local SH_REMOTE, SH_REMOTE_SIZE = 8, 0x18
local SH_ROWS, SH_ROW_SIZE = 8, 0x0C
local SH_REQ_CLAIM, SH_REQ_END, SH_REQ_CHANNELS, SH_REQ_JOIN = 1, 2, 3, 4
local SH_CLAIMED = { ok = 1, taken = 2, gone = 3 }

NetplayShared = {}
NetplayShared.__index = NetplayShared

function NetplayShared.new(bridge)
    local self = setmetatable({}, NetplayShared)
    self.bridge = bridge
    self.mem = bridge.mem
    self.base = nil
    self:reset()
    return self
end

function NetplayShared:reset()
    self.spawns = {}     -- "owner:uid" -> spawn of another player
    self.slots = {}      -- remote slot (0-7) -> key
    self.written = {}    -- remote slot -> what was last written there
    self.sent = {}       -- own slot -> the last WSPAWN line
    self.sentUid = {}    -- own slot -> its uid
    self.ownClaims = {}  -- own uid -> name of who is battling it, or true when taken
    self.mine = {}       -- own uids this player claimed (battling it now)
    self.deferred = {}   -- own uids gone from the map during this player's own battle
    self.rows = nil
    self.joining = false
    self.map = nil
end

function NetplayShared:r8(off) return self.mem.read8(self.base + off) end
function NetplayShared:r16(off) return self.mem.read16(self.base + off) end
function NetplayShared:w8(off, v) self.mem.write8(self.base + off, v & 0xFF) end
function NetplayShared:w16(off, v) self.mem.write16(self.base + off, v & 0xFFFF) end
function NetplayShared:bump(off) self:w8(off, self:r8(off) + 1) end

local function sh_s16(v)
    if v >= 0x8000 then return v - 0x10000 end
    return v
end

function NetplayShared:writeName(off, name)
    for c = 0, 7 do
        local ch = (name or ""):sub(c + 1, c + 1)
        self:w8(off + c, ch == "" and 0xFF or self.bridge.asciiToChar(ch))
    end
end

-- world: the NetplayWorld whose mailbox holds the pointer.
function NetplayShared:find(world)
    if not world.base then return nil end
    local addr = self.mem.read32(world.base + SH_PTR)
    if addr < 0x02000000 or addr >= 0x02040000 or addr % 4 ~= 0 then return nil end
    if self.mem.read32(addr) ~= MAGIC_LO or self.mem.read32(addr + 4) ~= SH_MAGIC_HI then return nil end
    if self.mem.read8(addr + 8) ~= SH_VERSION then return nil end
    return addr
end

function NetplayShared:localMap()
    local b = self.bridge
    if not b.base then return nil end
    return b:r8(0x016) * 256 + b:r8(0x017)
end

-- Server lines. Returns true if it took the line.
function NetplayShared:handleLine(cmd, args)
    local me = self.bridge.localId
    if cmd == "WILD" then
        local owner, uid = tonumber(args[2]), tonumber(args[3])
        local claimed, claimer = tonumber(args[12]) == 1, args[13]
        if owner == me then
            self.ownClaims[uid] = claimed and claimer or nil
        else
            self.spawns[owner .. ":" .. uid] = {
                owner = owner, uid = uid, gfx = tonumber(args[4]), level = tonumber(args[5]), flags = tonumber(args[6]),
                x = tonumber(args[7]), y = tonumber(args[8]), facing = tonumber(args[9]),
                map = tonumber(args[10]) * 256 + tonumber(args[11]), claimed = claimed, claimer = claimer,
            }
        end
    elseif cmd == "WILDGONE" then
        local owner, uid = tonumber(args[2]), tonumber(args[3])
        if owner == me then
            if not self.mine[uid] then self.ownClaims[uid] = true end -- caught or beaten by someone else
        else
            self.spawns[owner .. ":" .. uid] = nil
        end
    elseif cmd == "WCLAIMED" and self.base then
        local owner, uid = tonumber(args[2]), tonumber(args[3])
        local result = SH_CLAIMED[args[4]] or 3
        if result == 1 and owner == me then self.mine[uid] = true end
        self:writeName(SH.CLAIMER, args[5] ~= "-" and args[5] or "")
        self:w8(SH.CLAIM_RESULT, result)
        self:bump(SH.CLAIM_SEQ)
    elseif cmd == "WRARE" and self.base then
        self:w16(SH.RARE_OWNER, tonumber(args[2]))
        self:w16(SH.RARE_UID, tonumber(args[3]))
        self:bump(SH.RARE_SEQ)
    elseif cmd == "CHANNEL" and self.base then
        self:w8(SH.CHANNEL, tonumber(args[2]))
        self:w8(SH.CHANNEL_COUNT, tonumber(args[3]))
        self:bump(SH.CHANNEL_SEQ)
        if self.joining then
            self.joining = false
            self:w8(SH.JOIN_RESULT, 1)
            self:bump(SH.LIST_SEQ)
        end
    elseif cmd == "CHANFULL" and self.base then
        self.joining = false
        self:w8(SH.JOIN_RESULT, 2)
        self:bump(SH.LIST_SEQ)
    elseif cmd == "CHANROW" then
        self.rows = self.rows or {}
        table.insert(self.rows, { channel = tonumber(args[2]), players = tonumber(args[3]),
                                  friends = tonumber(args[4]), friend = args[5] ~= "-" and args[5] or "" })
    elseif cmd == "CHANEND" and self.base then
        local rows = self.rows or {}
        self.rows = nil
        local count = math.min(#rows, SH_ROWS)
        for i = 1, count do
            local off = SH.ROWS + (i - 1) * SH_ROW_SIZE
            self:w8(off, rows[i].channel)
            self:w8(off + 1, math.min(rows[i].players, 255))
            self:w8(off + 2, math.min(rows[i].friends, 255))
            self:writeName(off + 4, rows[i].friend)
        end
        self:w8(SH.LIST_COUNT, count)
        self:w8(SH.CHANNEL, tonumber(args[2]))
        self:w8(SH.CHANNEL_COUNT, tonumber(args[3]))
        self:w8(SH.JOIN_RESULT, 0)
        self:bump(SH.LIST_SEQ)
    else
        return false
    end
    return true
end

function NetplayShared:handleRequest(net)
    local seq = self:r8(SH.REQ_SEQ)
    if self:r8(SH.REQ_ACK) == seq then return end
    local type, arg = self:r8(SH.REQ_TYPE), self:r8(SH.REQ_ARG)
    local owner, uid = self:r16(SH.REQ_OWNER), self:r16(SH.REQ_UID)
    if type == SH_REQ_CLAIM then
        net.send(string.format("WCLAIM %d %d", owner, uid))
    elseif type == SH_REQ_END then
        net.send(string.format("WEND %d %d %d", owner, uid, arg))
        if owner == self.bridge.localId then
            self.mine[uid] = nil
            if self.deferred[uid] then
                self.deferred[uid] = nil
                net.send("WGONE " .. uid)
            end
        elseif arg == 1 or arg == 2 then
            self.spawns[owner .. ":" .. uid] = nil -- the server tells everyone else
        end
    elseif type == SH_REQ_CHANNELS then
        self.rows = {}
        net.send("CHAN")
    elseif type == SH_REQ_JOIN then
        self.joining = true
        net.send("CHAN JOIN " .. arg)
    end
    self:w8(SH.REQ_ACK, seq)
end

-- This game's spawns go to the server; claims on them come back.
function NetplayShared:syncOwn(net)
    for i = 0, SH_OWN - 1 do
        local off = SH.OWN + i * SH_OWN_SIZE
        local uid = self:r16(off)
        local last = self.sentUid[i]
        if last and last ~= uid then
            -- Its battle isn't over: it stays for the others to watch.
            if self.mine[last] then self.deferred[last] = true else net.send("WGONE " .. last) end
            self.ownClaims[last] = nil
            self.sentUid[i], self.sent[i] = nil, nil
        end
        if uid ~= 0 then
            local line = string.format("WSPAWN %d %d %d %d %d %d %d", uid, self:r16(off + 2), self:r8(off + 8),
                                       self:r8(off + 9), sh_s16(self:r16(off + 4)), sh_s16(self:r16(off + 6)), self:r8(off + 10))
            if line ~= self.sent[i] then
                self.sent[i], self.sentUid[i] = line, uid
                net.send(line)
            end
            local claim = self.ownClaims[uid]
            self:w8(off + 11, claim == true and 2 or claim and 1 or 0)
            self:writeName(SH.OWN_CLAIMER + i * 8, type(claim) == "string" and claim or "")
        else
            self:w8(off + 11, 0)
        end
    end
end

-- Other players' spawns on this map, nearest first, in the 8 slots.
function NetplayShared:writeRemotes()
    local map = self:localMap()
    local b = self.bridge
    local px, py = 0, 0
    if b.base then px, py = sh_s16(b:r16(0x018)), sh_s16(b:r16(0x01A)) end
    local used = {}
    for slot = 0, SH_REMOTE - 1 do
        local key = self.slots[slot]
        local s = key and self.spawns[key]
        if not s or s.map ~= map then
            self.slots[slot] = nil
        else
            used[key] = true
        end
    end
    local waiting = {}
    for key, s in pairs(self.spawns) do
        if not used[key] and s.map == map then
            waiting[#waiting + 1] = { key = key, d = math.abs(s.x - px) + math.abs(s.y - py) }
        end
    end
    table.sort(waiting, function(a, c) return a.d < c.d end)
    local n = 1
    for slot = 0, SH_REMOTE - 1 do
        if not self.slots[slot] and waiting[n] then
            self.slots[slot] = waiting[n].key
            n = n + 1
        end
    end
    for slot = 0, SH_REMOTE - 1 do
        local off = SH.REMOTE + slot * SH_REMOTE_SIZE
        local s = self.slots[slot] and self.spawns[self.slots[slot]]
        local text = s and table.concat({ s.owner, s.uid, s.gfx, s.level, s.flags, s.x, s.y, s.facing, s.map,
                                          s.claimed and s.claimer or "-" }, " ") or ""
        if text ~= self.written[slot] then
            self.written[slot] = text
            if s then
                self:w16(off + 2, s.uid)
                self:w16(off + 4, s.gfx)
                self:w16(off + 6, s.x)
                self:w16(off + 8, s.y)
                self:w8(off + 10, s.level)
                self:w8(off + 11, s.flags)
                self:w8(off + 12, s.facing)
                self:w8(off + 13, s.claimed and 1 or 0)
                self:w8(off + 14, s.map >> 8)
                self:w8(off + 15, s.map & 0xFF)
                self:writeName(off + 16, s.claimed and s.claimer or "")
                self:w16(off, s.owner) -- owner last: publishes the slot
            else
                self:w16(off, 0)
            end
        end
    end
end

function NetplayShared:onFrame(world, online)
    if self.base and not (self.mem.read32(self.base) == MAGIC_LO and self.mem.read32(self.base + 4) == SH_MAGIC_HI) then
        self.base = nil
    end
    if not self.base then
        self.base = self:find(world)
        if not self.base then return end
        self:w8(SH.REQ_ACK, self:r8(SH.REQ_SEQ))
        self.written = {}
    end
    if not online then
        if self.online then self:reset() end
        self.online = false
        self:w8(SH.REQ_ACK, self:r8(SH.REQ_SEQ))
        self:w8(SH.CHANNEL, 0)
        self:w8(SH.CHANNEL_COUNT, 0)
        self:writeRemotes()
        return
    end
    self.online = true
    self:w16(SH.LOCAL_ID, self.bridge.localId)
    local map = self:localMap()
    if map ~= self.map then
        -- A new map: channel 1 of 1 until the server says otherwise.
        self.map = map
        self:w8(SH.CHANNEL, 1)
        self:w8(SH.CHANNEL_COUNT, 1)
        for key, s in pairs(self.spawns) do
            if s.map ~= map then self.spawns[key] = nil end
        end
    end
    local net = self.bridge.net
    self:handleRequest(net)
    self:syncOwn(net)
    self:writeRemotes()
end
