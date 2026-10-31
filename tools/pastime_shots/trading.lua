-- Trading post: the trader at the end of the Slateport market. TEST1 brings
-- what today's first barter needs and a Pokémon of today's swap type,
-- makes the barter, tries it again (done for today), swaps the Pokémon and
-- opens R > PASTIMES > TRADING POST.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

-- Mirrors sBarters and sSwaps in src/pastime_trading.c: what each needs.
local BARTER_ITEMS = { "ORAN_BERRY", "PECHA_BERRY", "TINY_MUSHROOM", "PEARL", "STARDUST", "NUGGET",
    "POKE_BALL", "GREAT_BALL", "POTION", "SUPER_POTION", "REPEL", "SUPER_REPEL", "FRESH_WATER",
    "BIG_PEARL", "STAR_PIECE", "BIG_MUSHROOM", "HEART_SCALE", "SHOAL_SALT" }
local SWAP_MONS = { "TORKOAL", "WINGULL", "ODDISH", "SURSKIT", "ZIGZAGOON", "ELECTRIKE", "GULPIN",
    "TAILLOW", "NOSEPASS" }
-- FIRST_BERRY_INDEX hides the berries from run.py; the Oran Berry is Cheri + 6.
ITEM_ID.PECHA_BERRY = ITEM_ID.ORAN_BERRY - 4

D.run(function()
    D.boot()
    D.wait(60)
    D.scene(12, MAPS.MAP_SLATEPORT_CITY, 32, 28)
    local day = D.read16(D.SYM.gLocalTime)
    local barter = (day * 5) % #BARTER_ITEMS + 1
    local swap = (day * 4) % #SWAP_MONS + 1
    D.log("day " .. day .. " barter " .. BARTER_ITEMS[barter] .. " swap " .. SWAP_MONS[swap])
    D.scene(12, MAPS.MAP_SLATEPORT_CITY, 32, 28, { item = BARTER_ITEMS[barter], give = SWAP_MONS[swap], level = 18, lead = true })
    D.wait(30)
    D.shot("trader")
    D.talk(K.UP, "welcome", 1)
    D.wait(60)
    D.shot("offers")
    D.press(K.A, 90) -- the first barter
    D.shot("barter")
    D.press(K.A, 90) -- yes
    D.wait(120)
    D.shot("got-item")
    D.press(K.A, 90)
    D.press(K.A, 90)
    D.shot("deal")
    D.press(K.A, 90)
    D.wait(30)
    D.shot("offers-after")
    D.press(K.A, 90) -- the same barter again
    D.shot("done-today")
    D.press(K.A, 90)
    D.wait(30)
    for _ = 1, 3 do D.press(K.DOWN, 10) end
    D.shot("swap-row")
    D.press(K.A, 90)
    D.shot("swap-offer")
    D.press(K.A, 90)
    D.shot("swap-offer-2")
    D.press(K.A, 150) -- yes
    D.wait(60)
    D.shot("party")
    -- The new Pokémon is the lead; pick MUDKIP (slot 2) to show the type check.
    D.press(K.DOWN, 20)
    D.press(K.A, 120)
    D.shot("wrong-type")
    D.press(K.A, 90)
    D.wait(60)
    for _ = 1, 3 do D.press(K.DOWN, 10) end
    D.press(K.A, 90)
    D.press(K.A, 90)
    D.press(K.A, 150) -- yes
    D.wait(60)
    D.press(K.A, 120) -- the lead
    for i = 1, 10 do
        D.wait(40)
        D.shot("swapped-" .. i)
        if i >= 6 and i % 2 == 0 then D.press(K.B, 0) end
    end
    for _ = 1, 4 do D.press(K.B, 40) end
    D.wait(60)
    D.pastimeMenu(9)
    D.wait(30)
    D.shot("trading-list")
    D.press(K.B, 60)
    D.press(K.START, 60)
    D.press(K.A, 90)
    D.shot("party-after")
end)
