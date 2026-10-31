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
local VERSION = 1

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
local OFF_TRAINER_LOOK = 0x198  -- u32: colours and accessories (include/trainer_look.h)
local OFF_REMOTE_TRAINER_LOOKS = 0x19C -- u32 per slot
local ROW_SIZE = 0x10
local BOARD_ROWS = 10
local DECOR_SIZE = 16
local MAX_REMOTES = 8
local NAME_LENGTH = 8

local REQ_SCORE, REQ_BOARD = 1, 2
local SCAN_INTERVAL = 60
local HUNT_INTERVAL = 60 * 60 -- frames between hunt checks (about a minute)
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
    return self
end

function NetplayWorld:reset()
    self.online = false
    self.lastLook = nil
    self.lastHouse = nil
    self.rows = nil
    self.nextHunt = 0
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

function NetplayWorld:decorHex(off)
    local hex = {}
    for i = 0, DECOR_SIZE - 1 do hex[#hex + 1] = string.format("%02x", self:r8(off + i)) end
    return table.concat(hex)
end

-- Server lines this module handles. Returns true if it took the line.
function NetplayWorld:handleLine(cmd, args)
    if cmd == "LOOK" then
        self.looks[tonumber(args[2])] = { follower = tonumber(args[3]), mount = tonumber(args[4]),
                                          accessory = tonumber(args[5]) or 0, trainer = tonumber(args[6]) or 0 }
    elseif cmd == "HOUSE" then
        self.houses[tonumber(args[2])] = { open = tonumber(args[3]), decor = args[4] }
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
    local look = string.format("LOOK %d %d %d %d", self:r16(OFF_FOLLOWER), self:r16(OFF_MOUNT), self:r8(OFF_ACCESSORY),
                               self:r32(OFF_TRAINER_LOOK))
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
        local off = OFF_REMOTES + slot * REMOTE_SIZE
        local id = self.online and self.bridge.slots[slot]
        local look = id and self.looks[id]
        local house = id and self.houses[id]
        if id and (look or house) then
            self:w16(off + 2, look and look.follower or 0)
            self:w16(off + 4, look and look.mount or 0)
            self:w8(off + 6, house and house.open or 0)
            self:w8(off + 7, look and look.accessory or 0)
            self:w32(OFF_REMOTE_TRAINER_LOOKS + slot * 4, look and look.trainer or 0)
            for i = 0, DECOR_SIZE - 1 do
                local byte = house and tonumber(house.decor:sub(i * 2 + 1, i * 2 + 2), 16) or 0
                self:w8(off + 8 + i, byte)
            end
            self:w16(off, id)
        else
            self:w16(off, 0)
            self:w32(OFF_REMOTE_TRAINER_LOOKS + slot * 4, 0)
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
    end
    self.online = online
    self:takePhoto(net)
    if not online then
        self:w8(OFF_REQ_ACK, self:r8(OFF_REQ_SEQ))
        self:writeRemotes()
        return
    end
    if frame >= self.nextHunt then
        self.nextHunt = frame + HUNT_INTERVAL
        net.send("HUNT")
    end
    self:sendLook(net)
    self:handleRequest(net)
    self:writeRemotes()
end

