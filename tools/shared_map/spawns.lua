-- Shared wild spawns with two players (docs/shared_map.md):
--   python3 tools/world_shots/run.py tools/shared_map/spawns.lua <out> 60000 2
-- TEST1's game makes the wild Pokémon on Route 101 (one of them shiny);
-- TEST2 sees the same ones and hears about the shiny, battles it first, and
-- TEST1 sees it held with a VS bubble and watches the battle.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY
local P1 = INSTANCE == 1
local OBJ_SIZE, OFFSET = 0x24, 7
local SH = D.SYM.gSharedMapMailbox

local function living()
    D.write8(D.SYM.gAliveShowcase + 2, 1) -- visible wild Pokémon on a test boot too
end

local function objAt(localId)
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * OBJ_SIZE
        if (D.read8(o) & 1) == 1 and D.read8(o + 8) == localId then
            return D.read16(o + 0x10) - OFFSET, D.read16(o + 0x12) - OFFSET
        end
    end
end

local function playerAt() return objAt(0xFF) end

local function occupied(x, y)
    for i = 0, 63 do
        local o = D.SYM.gObjectEvents + i * OBJ_SIZE
        if (D.read8(o) & 1) == 1 and D.read8(o + 8) ~= 0xFF
         and D.read16(o + 0x10) - OFFSET == x and D.read16(o + 0x12) - OFFSET == y then
            return true
        end
    end
    return false
end

-- Walks next to the tile and faces it, never stepping into another object
-- (bumping into a wild Pokémon starts a battle).
local function walkNextTo(tx, ty)
    local moves = { { K.RIGHT, 1, 0 }, { K.LEFT, -1, 0 }, { K.DOWN, 0, 1 }, { K.UP, 0, -1 } }
    local last
    for _ = 1, 80 do
        local x, y = playerAt()
        if x == nil then return false end
        local dx, dy = tx - x, ty - y
        if math.abs(dx) + math.abs(dy) == 1 then
            local dir = dx == 1 and K.RIGHT or dx == -1 and K.LEFT or dy == 1 and K.DOWN or K.UP
            D.hold(dir, 2); D.wait(10)
            return true
        end
        table.sort(moves, function(a, b)
            local da = math.abs(tx - (x + a[2])) + math.abs(ty - (y + a[3])) + (a[1] == last and 0 or 0.1)
            local db = math.abs(tx - (x + b[2])) + math.abs(ty - (y + b[3])) + (b[1] == last and 0 or 0.1)
            return da < db
        end)
        local moved = false
        for _, m in ipairs(moves) do
            local nx, ny = x + m[2], y + m[3]
            if not (nx == tx and ny == ty) and not occupied(nx, ny) then
                D.hold(m[1], 16); D.wait(4)
                local ax, ay = playerAt()
                if ax ~= x or ay ~= y then
                    moved, last = true, m[1]
                    break
                end
            end
        end
        if not moved then D.wait(30) end -- wait for the others to wander off
    end
    return false
end

local function ownCount()
    local n = 0
    for i = 0, 3 do if D.read16(SH + 0x28 + i * 12) ~= 0 then n = n + 1 end end
    return n
end

local function remoteObjects()
    local list = {}
    for i = 0, 7 do
        local x, y = objAt(241 + i)
        if x then list[#list + 1] = { id = 241 + i, slot = i, x = x, y = y } end
    end
    return list
end

local function claimedOwn()
    for i = 0, 3 do
        if D.read16(SH + 0x28 + i * 12) ~= 0 and D.read8(SH + 0x28 + i * 12 + 11) == 1 then return i end
    end
end

D.run(function()
    D.boot()
    D.waitFor("the server", 600, D.online)
    living()
    if P1 then
        D.scene(12, MAPS.MAP_ROUTE101, 8, 12)
        D.waitFor("TEST2", 2400, function() return D.remoteCount() >= 1 end)
        D.write8(D.SYM.gSharedMapShowcase, 1) -- the next one is shiny
        D.waitFor("the shiny", 4000, function() return D.read8(D.SYM.gSharedMapShowcase) == 0 end)
        D.waitFor("wild Pokémon", 4000, function() return ownCount() >= 3 end)
        D.wait(60)
        D.shot("p1-own-spawns")
        local slot = D.waitFor("TEST2 to battle one", 9000, function() return claimedOwn() end) and claimedOwn()
        D.wait(40)
        D.shot("p1-vs-bubble")
        local x, y = objAt(252 - slot)
        if x and walkNextTo(x, y) then
            D.wait(10)
            D.shot("p1-vs-close")
            D.press(K.A, 60)
            D.shot("p1-ask-watch")
            D.press(K.A, 30)
            D.shot("p1-ask-watch-2")
            D.press(K.A, 120) -- YES
            D.shot("p1-live-list")
            D.press(K.A, 180)
            D.shot("p1-watching")
            D.wait(400)
            D.shot("p1-watching-2")
            -- Keep watching until TEST2 is done with it.
            D.waitFor("the Pokémon to be free", 6000, function() return claimedOwn() == nil end)
            D.wait(30)
            D.shot("p1-watch-over")
            D.press(K.B, 40)
            D.press(K.B, 30)
        else
            D.log("couldn't reach the held Pokémon")
            D.waitFor("the Pokémon to be free", 6000, function() return claimedOwn() == nil end)
        end
        D.wait(10)
        D.shot("p1-free-again")
    else
        D.write8(D.SYM.gSharedMapShowcase + 1, 1) -- only TEST1's game makes them
        local function shinyHere()
            for _, r in ipairs(remoteObjects()) do
                if (D.read16(SH + 0x58 + r.slot * 24 + 4) & 0x2000) ~= 0 then return r end
            end
        end
        -- Each rare notice (a toast) as it comes, until the shiny is here.
        local seq, toasts = D.read8(SH + 0x20), 0
        local function watchToasts()
            if D.read8(SH + 0x20) ~= seq then
                seq = D.read8(SH + 0x20)
                toasts = toasts + 1
                D.wait(45)
                D.shot("p2-rare-toast-" .. toasts .. "a")
                D.wait(120)
                D.shot("p2-rare-toast-" .. toasts .. "b")
            end
        end
        D.scene(12, MAPS.MAP_ROUTE101, 11, 12)
        D.waitFor("TEST1's Pokémon", 4000, function() return #remoteObjects() >= 3 end, watchToasts)
        D.wait(40)
        D.shot("p2-sees-them")
        D.waitFor("the shiny", 6000, function() return shinyHere() and toasts > 0 end, watchToasts)
        D.wait(30)
        D.shot("p2-shiny-here")
        D.wait(120)
        -- Battle the one nearest TEST1, so TEST1 can walk up to it.
        local net = D.SYM.gNetplayMailbox + 0x020
        local function s16(v) return v >= 0x8000 and v - 0x10000 or v end
        local p1x, p1y = s16(D.read16(net + 4)), s16(D.read16(net + 6))
        local target, best
        for _, r in ipairs(remoteObjects()) do
            local d = math.abs(r.x - p1x) + math.abs(r.y - p1y)
            if not best or d < best then target, best = r, d end
        end
        if not walkNextTo(target.x, target.y) then D.fail("couldn't reach it") end
        local nx, ny = objAt(target.id)
        if nx and (nx ~= target.x or ny ~= target.y) then walkNextTo(nx, ny) end
        D.shot("p2-next-to-it")
        D.press(K.A, 10)
        D.waitFor("the battle", 1200, D.inBattle, function(i) if i % 60 == 0 then D.press(K.A, 0) end end)
        D.wait(300)
        D.shot("p2-battle")
        D.wait(1500)
        D.runAway()
        D.wait(60)
        D.shot("p2-back")
    end
end)
