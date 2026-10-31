-- 24 other players on one map (online_crowd.py starts the server and 23
-- stand-in players; with the screenshot player that's 24 visible others
-- counting none of ours). The GBA's 16 objects showed 8.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D = Driver

local function remotes()
    local mb, n = D.SYM.gNetplayMailbox, 0
    for slot = 0, 7 do if D.read16(mb + 0x020 + slot * 20) ~= 0 then n = n + 1 end end
    for slot = 0, 15 do if D.read16(mb + 0x700 + slot * 20) ~= 0 then n = n + 1 end end
    return n
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    D.scene(13, MAPS.MAP_PETALBURG_CITY, 15, 12, {})
    D.waitFor("the other players", 900, function() return remotes() >= 23 end)
    D.wait(900) -- let the news banner go
    D.log("players in the mailbox: " .. remotes())
    D.shot("petalburg-24-players")
    D.wait(120)
    D.shot("petalburg-24-players-2")
    D.result("ok")
end)
