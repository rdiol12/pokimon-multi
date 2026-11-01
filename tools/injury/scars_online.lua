-- Two players online (scars.py <out> online): each sees the scar and lost
-- parts of the other's following Pokémon. TEST1 leads with Pikachu (scar,
-- lost tail), TEST2 with Charizard (scar, lost wing and eye).
-- @common
local P1 = INSTANCE == 1

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if P1 then
        D.scene(14, MAPS.MAP_OLDALE_TOWN, 6, 10, { give = "PIKACHU", level = 30, lead = true })
        setInjury(0, PART.NONE, 0, bit(PART.TAIL), FLAG.SCAR)
        D.scene(14, MAPS.MAP_OLDALE_TOWN, 6, 10)
    else
        D.scene(14, MAPS.MAP_OLDALE_TOWN, 10, 10, { give = "CHARIZARD", level = 36, lead = true })
        setInjury(0, PART.NONE, 0, bit(PART.WING) | bit(PART.EYE), FLAG.SCAR)
        D.scene(14, MAPS.MAP_OLDALE_TOWN, 10, 10)
    end
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    -- Step apart, so both followers are seen side on.
    D.wait(120)
    D.hold(P1 and K.LEFT or K.RIGHT, 16)
    D.wait(20)
    D.hold(P1 and K.LEFT or K.RIGHT, 16)
    D.wait(300)
    local mine = D.read8(D.SYM.gWorldMailbox + 0x37C)
    local theirs = {}
    for slot = 0, 23 do
        local v = D.read8(D.SYM.gWorldMailbox + 0x37D + slot)
        if v ~= 0 then theirs[#theirs + 1] = slot .. "=" .. v end
    end
    D.log("my follower's marks: " .. mine .. ", other players': " .. table.concat(theirs, " "))
    D.shot("online-followers")
    D.wait(120)
    D.shot("online-followers-2")
    D.result("ok")
end)
