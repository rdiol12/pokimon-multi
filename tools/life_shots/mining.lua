-- Underground mining: get the tools from the digger in Rusturf Tunnel,
-- dig the glittering rock next to him with the hammer and pick.
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local function moveTo(cx, cy, tx, ty)
    while cx < tx do D.press(K.RIGHT, 4); cx = cx + 1 end
    while cx > tx do D.press(K.LEFT, 4); cx = cx - 1 end
    while cy < ty do D.press(K.DOWN, 4); cy = cy + 1 end
    while cy > ty do D.press(K.UP, 4); cy = cy - 1 end
    return tx, ty
end

D.run(function()
    D.boot()
    D.wait(60)
    -- The first visit runs the tunnel's own map script; let it finish.
    D.scene(14, MAPS.MAP_RUSTURF_TUNNEL, 17, 4)
    D.wait(120)
    D.scene(14, MAPS.MAP_RUSTURF_TUNNEL, 17, 4)
    D.shot("tunnel")
    D.hold(K.LEFT, 6)
    D.wait(20)
    D.press(K.A, 60)
    D.shot("digger")
    for i = 1, 3 do D.press(K.A, 40) end
    D.wait(20)
    D.shot("digger-2")
    D.press(K.A, 200)
    D.shot("digger-got")
    for i = 1, 6 do D.press(K.A, 40) end
    D.wait(60)
    -- The rock is at (12, 4): stand under it.
    D.scene(14, MAPS.MAP_RUSTURF_TUNNEL, 12, 5)
    D.hold(K.UP, 6)
    D.wait(20)
    D.press(K.A, 60)
    D.shot("dig-ask")
    D.press(K.A, 200)
    D.shot("dig-wall")
    -- A few hammer blows across the wall, then the pick on each treasure
    -- (gLifeMine: buried[4] {item u16, x, y, size, ...} 8 bytes each,
    -- count at 32, crack at 33, depth[6][13] at 34).
    local m = D.read32(D.SYM.gLifeMine)
    local cx, cy = 6, 3
    for _, p in ipairs({{2, 1}, {10, 4}}) do
        cx, cy = moveTo(cx, cy, p[1], p[2])
        D.press(K.B, 14)
    end
    D.shot("dig-hammer")
    local count = D.read8(m + 32)
    for i = 0, count - 1 do
        local b = m + i * 8
        local bx, by, size = D.read8(b + 2), D.read8(b + 3), D.read8(b + 4)
        for y = by, by + size - 1 do
            for x = bx, bx + size - 1 do
                for _ = 1, 3 do
                    if D.read8(m + 34 + y * 13 + x) == 0 or D.read8(m + 33) >= 66 then break end
                    cx, cy = moveTo(cx, cy, x, y)
                    D.press(K.A, 14)
                end
            end
        end
        D.wait(20)
        D.shot("dig-found-" .. i)
    end
    D.wait(10)
    D.shot("dig-end")
    D.wait(90)
    D.shot("dig-results")
    D.press(K.A, 120)
    D.wait(60)
    D.shot("dig-back")
    D.press(K.B, 60)
    D.press(K.B, 60)
    D.hold(K.UP, 6)
    D.wait(20)
    D.press(K.A, 60)
    D.shot("dig-done-today")
end)
