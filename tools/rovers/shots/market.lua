-- Online: a rover's Pokémon on the market board in Oldale's Poké Mart.
-- Player 1 buys it; it arrives as a real Pokémon with the rover as its
-- trainer. Player 2 only keeps the server company.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    if INSTANCE == 2 then
        D.waitFor("player 1", 40000, function()
            local f = io.open(OUT_DIR .. "/done", "r")
            if f then f:close() return true end
            return false
        end)
        D.result("ok")
        return
    end
    os.execute(string.format('python3 "%s/seed_market.py" "%s/league.sqlite3"', ROVERS_SHOTS, OUT_DIR))
    D.scene(12, MAPS.MAP_OLDALE_TOWN_MART, 5, 2)
    D.press(K.UP, 20)
    local seq = D.mb8(0x518)
    D.press(K.A, 20)
    D.waitFor("the market board", 600, function() return D.mb8(0x518) ~= seq end)
    D.wait(30)
    D.shot("market-board")
    local count = D.partyCount()
    seq = D.mb8(0x518)
    D.press(K.A, 30)
    D.shot("market-confirm")
    D.press(K.A, 10)
    D.waitFor("the purchase", 600, function() return D.partyCount() ~= count end)
    D.wait(200)
    D.shot("market-bought")
    D.waitFor("the board to reload", 1200, function() return D.mb8(0x518) ~= seq end)
    D.wait(60)
    D.press(K.B, 120)
    D.waitFor("the overworld", 600, function() return not D.read8 or true end)
    D.wait(60)
    D.press(K.START, 60)
    D.press(K.A, 120) -- POKéMON
    D.shot("party")
    for _ = 1, count do D.press(K.DOWN, 12) end
    D.press(K.A, 30)
    D.press(K.A, 120) -- SUMMARY
    D.shot("summary")
    D.press(K.RIGHT, 60)
    D.shot("summary-moves")
    io.open(OUT_DIR .. "/done", "w"):close()
    D.result("ok")
end)
