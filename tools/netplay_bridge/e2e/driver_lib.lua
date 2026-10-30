-- Helpers for e2e driver scripts. A driver calls Driver.run(function() ... end);
-- the function runs as a coroutine, one resume per frame, and can wait() for
-- frames or conditions without blocking the emulator.

Driver = {}

-- mGBA only allows its API on the main Lua thread, so the coroutine asks the
-- frame callback to make each emu call for it.
local function call(method, ...)
    return coroutine.yield({ method = method, args = { ... } })
end

local E = {}
for _, m in ipairs({ "read8", "read16", "read32", "write8", "setKeys", "screenshot" }) do
    E[m] = function(...) return call(m, ...) end
end

local SYM = dofile(INSTANCE_DIR .. "/../symbols.lua")
Driver.SYM = SYM

local KEY = { A = 1, B = 2, SELECT = 4, START = 8, RIGHT = 16, LEFT = 32, UP = 64, DOWN = 128, R = 256, L = 512 }
Driver.KEY = KEY
Driver.DIR = { SOUTH = 1, NORTH = 2, WEST = 3, EAST = 4 }

local MB = SYM.gNetplayMailbox
local frame = 0
local shotCount = 0

function Driver.log(msg)
    print(string.format("[P%d %6d] %s", INSTANCE, frame, msg))
end

function Driver.result(text)
    local f = io.open(INSTANCE_DIR .. "/result.txt", "w")
    f:write(text .. "\n")
    f:close()
end

function Driver.fail(msg)
    Driver.shot("fail")
    error({ fail = msg })
end

function Driver.shot(name)
    shotCount = shotCount + 1
    E.screenshot(string.format("%s/%02d-%s.png", INSTANCE_DIR, shotCount, name))
end

function Driver.wait(frames)
    for _ = 1, frames or 1 do coroutine.yield() end
end

function Driver.waitFor(desc, timeout, pred, every)
    for i = 1, timeout do
        if pred() then return i end
        if every then every(i) end
        coroutine.yield()
    end
    Driver.fail("timed out waiting for " .. desc)
end

function Driver.setKeys(keys) E.setKeys(keys) end

function Driver.hold(keys, frames)
    E.setKeys(keys)
    Driver.wait(frames)
    E.setKeys(0)
end

function Driver.press(keys, after)
    Driver.hold(keys, 3)
    Driver.wait(after or 10)
end

-- Memory helpers
function Driver.read8(addr) return E.read8(addr) end
function Driver.write8(addr, v) E.write8(addr, v) end
function Driver.mb8(off) return E.read8(MB + off) end
function Driver.mb16(off) return E.read16(MB + off) end
function Driver.callback2() return E.read32(SYM.gMain + 4) & ~1 end
function Driver.inOverworld() return Driver.callback2() == SYM.CB2_Overworld end
function Driver.inBattle() return Driver.callback2() == SYM.BattleMainCB2 end
function Driver.online() return Driver.mb8(0x009) == 2 end
function Driver.remoteCount()
    local n = 0
    for slot = 0, 7 do
        if Driver.mb16(0x020 + slot * 20) ~= 0 then n = n + 1 end
    end
    return n
end
function Driver.pos() return Driver.mb16(0x018), Driver.mb16(0x01A), Driver.mb8(0x01C) end

local function decodeName(addr, len)
    local s = ""
    for i = 0, len - 1 do
        local c = E.read8(addr + i)
        if c == 0xFF then break end
        if c >= 0xA1 and c <= 0xAA then s = s .. string.char(c - 0xA1 + 48)
        elseif c >= 0xBB and c <= 0xD4 then s = s .. string.char(c - 0xBB + 65)
        elseif c >= 0xD5 and c <= 0xEE then s = s .. string.char(c - 0xD5 + 97)
        else s = s .. "?" end
    end
    return s
end

-- struct Pokemon is 100 bytes: nickname at +8, OT name at +20.
function Driver.partyMon(slot)
    local base = SYM.gParties + slot * 100
    return { nickname = decodeName(base + 8, 10), ot = decodeName(base + 20, 7) }
end

function Driver.partyCount() return E.read8(SYM.gPartiesCount) end

-- Money is stored xor'd with the save's encryption key.
local OFF_SB1_MONEY = 0x490
local OFF_SB2_ENCRYPTION_KEY = 0xAC
function Driver.money()
    local sb1, sb2 = E.read32(SYM.gSaveBlock1Ptr), E.read32(SYM.gSaveBlock2Ptr)
    return (E.read32(sb1 + OFF_SB1_MONEY) ~ E.read32(sb2 + OFF_SB2_ENCRYPTION_KEY)) & 0xFFFFFFFF
end

-- Boots straight into the overworld through the netplay test boot.
function Driver.boot()
    Driver.waitFor("overworld", 4000, Driver.inOverworld, function(i)
        E.write8(MB + 0x0D1, 0xB0 | INSTANCE)
        if i % 40 == 0 then Driver.press(i % 80 == 0 and KEY.START or KEY.A, 0) end
    end)
    Driver.log("in the overworld")
end

-- Logs link state changes, to see where a failing run went wrong.
local lastTrace
local lastRxTail
local function traceLink()
    local mb = SYM.gNetplayMailbox
    local state = string.format("link state=%d status=%08X remote=%d linkcb=%08X ready=%d%d tx=%d/%d rx=%d/%d cb2=%08X",
        emu:read8(SYM.gLink + 1), emu:read32(SYM.gLinkStatus) & 0x7F07F, emu:read8(SYM.gReceivedRemoteLinkPlayers),
        emu:read32(SYM.gLinkCallback), emu:read8(SYM.gReadyToCloseLink), emu:read8(SYM.gReadyToCloseLink + 1),
        emu:read8(mb + 0x0D4), emu:read8(mb + 0x0D5), emu:read8(mb + 0x218), emu:read8(mb + 0x219),
        emu:read32(SYM.gMain + 4))
    if os.getenv("TRACE_PACKETS") then
        local t = emu:read8(mb + 0x219)
        if t ~= lastRxTail then
            local i = lastRxTail or t
            while i ~= t do
                local p = mb + 0x21C + i * 20
                local hex = {}
                for j = 0, 19 do hex[#hex + 1] = string.format("%02x", emu:read8(p + j)) end
                Driver.log("rx " .. table.concat(hex))
                i = (i + 1) % 16
            end
            lastRxTail = t
        end
    end
    local key = state:gsub(" tx=.*cb2", " cb2")
    if key ~= lastTrace then
        lastTrace = key
        Driver.log(state)
    end
end

function Driver.run(fn)
    local co = coroutine.create(function()
        fn()
        Driver.result("ok")
        Driver.log("done")
        HARNESS_DONE = true
    end)
    callbacks:add("frame", function()
        frame = frame + 1
        if TRACE_LINK then traceLink() end
        if coroutine.status(co) == "dead" then return end
        local result
        while true do
            local ok, req = coroutine.resume(co, result)
            if not ok then
                local msg = type(req) == "table" and ("fail: " .. req.fail) or ("error: " .. tostring(req))
                Driver.log(msg)
                Driver.result(msg)
                HARNESS_DONE = true
                return
            end
            if type(req) ~= "table" then return end -- waiting for the next frame
            result = emu[req.method](emu, table.unpack(req.args))
        end
    end)
end
