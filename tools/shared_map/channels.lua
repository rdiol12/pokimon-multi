-- Map channels with two players and a stand-in (docs/shared_map.md), run by
-- tools/shared_map/shots.py with channels of 2 players. TEST2 adds TEST1 as
-- a friend; Route 101 holds TEST1 and a stand-in, so TEST2 lands in channel
-- 2, sees the channel list with its friend in channel 1 (full), and moves
-- there once the stand-in has left.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local SH = D.SYM.gSharedMapMailbox

local function flag(name)
    local f = io.open(OUT_DIR .. "/" .. name, "w")
    f:write("1\n")
    f:close()
end

local function flagged(name)
    local f = io.open(OUT_DIR .. "/" .. name, "r")
    if f then f:close() return true end
    return false
end

local function waitFlag(name, timeout)
    D.waitFor(name, timeout or 6000, function() return flagged(name) end)
end

-- START > ONLINE > CHANNEL (ONLINE: LADDER, SEASON, FRIENDS, WATCH, PLAZA,
-- HUB, COMMUNITY, AUCTION, CHANNEL).
local function openChannels()
    D.press(K.START, 40)
    for _ = 1, 4 do D.press(K.DOWN, 10) end
    D.press(K.A, 30) -- ONLINE
    D.shot("p2-online-menu")
    for _ = 1, 8 do D.press(K.DOWN, 8) end
    D.press(K.A, 10) -- CHANNEL
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.write8(D.SYM.gSharedMapShowcase + 1, 1) -- no wild Pokémon in the way
    if P1 then
        D.scene(12, MAPS.MAP_ROUTE102, 20, 10)
        waitFlag("friend-added")
        D.scene(12, MAPS.MAP_ROUTE101, 8, 12)
        D.wait(60)
        D.shot("p1-route101-with-standin")
        flag("p1-there")
        waitFlag("p2-moved", 12000)
        D.waitFor("TEST2", 600, function() return D.remoteCount() >= 1 end)
        D.wait(60)
        D.shot("p1-sees-p2")
        waitFlag("p2-done", 3000)
    else
        D.scene(12, MAPS.MAP_ROUTE102, 21, 10)
        D.waitFor("TEST1", 1200, function() return D.remoteCount() >= 1 end)
        D.wait(60)
        D.hold(K.LEFT, 2)
        D.wait(10)
        D.press(K.A, 60)
        D.shot("p2-talk-menu")
        for _ = 1, 3 do D.press(K.DOWN, 8) end
        D.press(K.A, 90) -- ADD FRIEND
        D.shot("p2-friend-added")
        D.press(K.A, 30)
        D.press(K.B, 30)
        flag("friend-added")
        waitFlag("p1-there")
        D.wait(30)
        -- Route 101: channel 1 has TEST1 and the stand-in, so channel 2.
        local seq = D.read8(SH + 0x15)
        D.scene(12, MAPS.MAP_ROUTE101, 11, 12)
        D.waitFor("the channel", 600, function() return D.read8(SH + 0x15) ~= seq end)
        for i = 1, 8 do
            D.wait(60)
            D.shot("p2-channel-toast-" .. i)
        end
        D.wait(60)
        D.shot("p2-alone-in-channel-2")
        openChannels()
        D.wait(90)
        D.shot("p2-channel-list")
        D.press(K.UP, 10)
        D.press(K.A, 90)
        D.shot("p2-channel-full")
        flag("standin-leave")
        D.wait(240)
        D.press(K.A, 120) -- refresh
        D.shot("p2-channel-list-2")
        D.press(K.UP, 10)
        D.press(K.A, 120)
        D.shot("p2-moved-list")
        D.press(K.B, 60)
        D.waitFor("TEST1", 600, function() return D.remoteCount() >= 1 end)
        D.wait(60)
        D.shot("p2-with-p1")
        flag("p2-moved")
        D.wait(120)
        flag("p2-done")
    end
end)
