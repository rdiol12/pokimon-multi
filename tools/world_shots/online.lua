-- The online world features with two players (run.py ... 2):
-- each sees the other's following Pokémon (TEST1's is shiny), TEST1 rides
-- off on it, both pick berries and TEST2 opens the berry leaderboard,
-- TEST1 checks the Safari hunt, TEST2 takes a photo for the gallery, and
-- TEST2 visits TEST1's furnished house.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local HOUSE = MAPS.MAP_STARFALL_ISLE_PLAYER_HOUSE

-- R menu: RIDE, ACHIEVEMENTS, RECORDS, QUESTS, LEADERBOARDS, SAFARI HUNT, PHOTO, CLOSE.
local function rMenu(index)
    D.press(K.R, 40)
    for _ = 1, index do D.press(K.DOWN, 8) end
    D.press(K.A, 40)
end

-- Another player with an open house is in the house map with us.
local function hostHere()
    local net, wld = Driver.SYM.gNetplayMailbox, Driver.SYM.gWorldMailbox
    for slot = 0, 7 do
        local remote = net + 0x020 + slot * 20
        local id = D.read16(remote)
        if id ~= 0 and D.read8(remote + 2) == HOUSE[1] and D.read8(remote + 3) == HOUSE[2] then
            for w = 0, 7 do
                local look = wld + 0x034 + w * 0x18
                if D.read16(look) == id and D.read8(look + 6) ~= 0 then return true end
            end
        end
    end
    return false
end

local function placeFurniture(item, x, y)
    D.scene(12, HOUSE, 1, 2)
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60) -- "It's your house."
    D.press(K.A, 40) -- DECORATE
    D.press(K.A, 60) -- "It's a furniture catalog."
    D.press(K.A, 40)
    for _ = 2, item do D.press(K.DOWN, 8) end
    D.press(K.A, 60)
    D.press(K.A, 40)
    D.scene(12, HOUSE, x, y)
    D.hold(K.UP, 2)
    D.wait(20)
    D.press(K.A, 40)
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)

    -- Followers: TEST1 leads with a shiny Rapidash, TEST2 with Pikachu.
    if P1 then
        D.scene(12, MAPS.MAP_OLDALE_TOWN, 6, 10, { give = "RAPIDASH", level = 40, lead = true, shiny = true })
    else
        D.scene(12, MAPS.MAP_OLDALE_TOWN, 12, 10, { give = "PIKACHU", level = 30, lead = true })
    end
    D.waitFor("the other player", 900, function() return D.remoteCount() > 0 end)
    D.wait(240)
    D.shot("followers")
    if P1 then
        D.hold(K.RIGHT, 32) -- two steps toward TEST2
        D.wait(60)
    else
        D.wait(92)
        D.shot("remote-follower-walks")
    end

    -- TEST1 rides its Rapidash.
    if P1 then
        rMenu(0)
        D.wait(120)
        D.shot("riding")
    else
        D.wait(200)
        D.shot("remote-mount")
    end
    D.wait(60)
    if P1 then rMenu(0); D.wait(60) else D.wait(120) end -- get off

    -- Berries for the leaderboard.
    D.scene(13, MAPS.MAP_ROUTE102, 24, 3, { ripen = true })
    D.wait(60) -- the map name popup
    D.hold(K.UP, 2)
    D.wait(10)
    D.press(K.A, 60)
    D.press(K.A, 60)
    D.press(K.A, 60) -- YES, pick
    D.wait(30)
    D.playMeter(P1) -- TEST1 misses on purpose
    for _ = 1, 14 do D.press(K.B, 40) end
    D.wait(200)
    if not P1 then
        rMenu(4) -- LEADERBOARDS
        D.press(K.DOWN, 8)
        D.press(K.A, 120) -- BERRY PICKERS
        D.shot("leaderboard-berries")
        D.press(K.B, 40)
    else
        rMenu(5) -- SAFARI HUNT
        D.wait(30)
        D.shot("safari-hunt")
        D.press(K.A, 90) -- "See the standings?"
        D.press(K.A, 120) -- YES
        D.shot("safari-hunt-standings")
        D.press(K.B, 40)
    end
    D.wait(200)

    -- A photo for the gallery.
    if not P1 then
        D.scene(18, MAPS.MAP_ROUTE102, 20, 6)
        D.wait(60)
        rMenu(6)
        D.wait(80)
        D.shot("photo-taken")
        D.press(K.B, 40)
    else
        D.wait(400)
    end

    -- TEST1 furnishes the house and opens it; TEST2 comes to visit.
    if P1 then
        placeFurniture(8, 7, 4)  -- aquarium
        placeFurniture(5, 11, 4) -- bed
        placeFurniture(3, 5, 8)  -- table
        placeFurniture(1, 0, 3)  -- plant
        D.scene(12, HOUSE, 1, 2)
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 60) -- "It's your house."
        D.press(K.DOWN, 8)
        D.press(K.A, 60) -- INVITE VISITORS
        D.shot("house-opened")
        D.press(K.A, 40)
        D.scene(12, HOUSE, 9, 5)
        D.wait(3000) -- TEST2 drops by
    else
        D.wait(1200)
        D.scene(12, HOUSE, 6, 6)
        D.waitFor("TEST1's open house", 3000, hostHere)
        D.wait(120)
        D.shot("visiting")
        D.scene(12, HOUSE, 1, 2)
        D.hold(K.UP, 2)
        D.wait(10)
        D.press(K.A, 60)
        D.shot("visiting-catalog")
        D.press(K.A, 40)
    end
    D.wait(120)
end)
