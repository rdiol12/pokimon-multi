-- Runs the netplay bridge inside the phone app, in place of mGBA's
-- scripting: the same bridge files (tools/netplay_bridge/), with memory and
-- the server connection provided by the app (bridge.js) instead of mGBA's
-- emu and socket objects.
--
-- bridge.js sets SOURCES (the three bridge files) and the js_* functions
-- before running this.

local function run(name, source)
    local chunk, err = load(source, "@/bridge/" .. name)
    if not chunk then error(err) end
    return chunk()
end

run("netplay_world.lua", SOURCES.world)
local Bridge = run("netplay_bridge.lua", SOURCES.bridge)

local mem = {
    read8 = js_read8,
    read16 = js_read16,
    read32 = js_read32,
    write8 = js_write8,
    write16 = js_write16,
}

local net = {
    connect = js_connect,
    send = js_send,
    receive = js_receive,
    close = js_close,
}

-- The mailbox finders check every word of EWRAM through mem.read32, which
-- is slow when every call crosses into JavaScript. They get a stand-in that
-- reads only the words next to a magic number the app found for them.
local MAGIC_LO = 0x4E4D4B50 -- "PKMN", first word of every mailbox
local function scanMem(real)
    local near = {}
    for addr in string.gmatch(js_find_word(MAGIC_LO), "%d+") do
        near[tonumber(addr)] = true
    end
    return setmetatable({
        read32 = function(a)
            if near[a] or near[a - 4] then return real.read32(a) end
            return 0
        end,
    }, { __index = real })
end
local function fastFinder(class, name)
    local find = class[name]
    class[name] = function(self, m)
        if m then return find(self, scanMem(m)) end
        local real = self.mem
        self.mem = scanMem(real)
        local ok, found = pcall(find, self)
        self.mem = real
        if not ok then error(found) end
        return found
    end
end
fastFinder(Bridge, "findMailbox")
if NetplayWorld then fastFinder(NetplayWorld, "findMailbox") end

local bridge = Bridge.new(mem, net, js_screenshot)
bridge.logger = js_log

NetplayBridge = {
    addHook = function(hook) bridge:addHook(hook) end,
    send = function(line) if bridge.connected then net.send(line) end end,
    connected = function() return bridge.connected and bridge.localId ~= 0 end,
}
local Ext = run("netplay_ext.lua", SOURCES.ext)
if type(Ext) == "table" and Ext.find then fastFinder(Ext, "find") end

function PokimonFrame()
    bridge:onFrame()
end

function PokimonReset()
    bridge:onReset()
end

-- 0 offline (or not joined), 1 connecting, 2 online and welcomed.
function PokimonStatus()
    if not bridge.connected then return 0 end
    return bridge.localId ~= 0 and 2 or 1
end
