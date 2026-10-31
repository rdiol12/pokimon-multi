
-- Community screenshot helpers (tools/community_shots/run.py appends these
-- to the driver library).

function Driver.cmb8(off) return E.read8(Driver.SYM.gCommunityMailbox + off) end

-- Answers received so far (gCommunityMailbox.respSeq).
function Driver.answers() return Driver.cmb8(0x00C) end

function Driver.waitAnswer(before, desc)
    Driver.waitFor(desc or "the server's answer", 900, function() return Driver.answers() ~= before end)
end

-- Event section counter (COMMUNITY_EVENT_*).
function Driver.eventSeq(section) return Driver.cmb8(0x3BC + section * 0xC0) end

-- Back to the top of a no-wrap menu, then down `n` rows and A.
local function pick(n, after)
    for _ = 1, 8 do Driver.press(Driver.KEY.UP, 4) end
    for _ = 1, n do Driver.press(Driver.KEY.DOWN, 6) end
    Driver.press(Driver.KEY.A, after or 30)
end

-- START > ONLINE > COMMUNITY, then the row-th entry (from 0).
-- START: POKéMON, BAG, PLAYER, STYLE, ONLINE, SAVE, OPTION, EXIT. That menu
-- wraps around and remembers its cursor, so count from where it was left.
Driver.startCursor = 0
Driver.START_ONLINE = 4
function Driver.community(row, shotName)
    Driver.press(Driver.KEY.START, 40)
    for _ = 1, (Driver.START_ONLINE - Driver.startCursor) % 8 do Driver.press(Driver.KEY.DOWN, 6) end
    Driver.startCursor = Driver.START_ONLINE
    Driver.press(Driver.KEY.A, 30)
    pick(4)
    if shotName then Driver.shot(shotName) end
    pick(row, 10)
end

-- Applies gCommunityShowcase (src/community.c) on the next overworld frame.
-- opts: {gameClear = true, badges = 8, level = 60, party = {"SCEPTILE", ...},
--        flags = {0x1DE}, vars = {{0x40A8, 2}}}
function Driver.showcase(opts)
    local s = Driver.SYM.gCommunityShowcase
    for i = 0, 31 do E.write8(s + i, 0) end
    E.write8(s + 1, opts.gameClear and 1 or 0)
    E.write8(s + 2, opts.badges or 0)
    E.write8(s + 3, opts.level or 50)
    for i, name in ipairs(opts.party or {}) do
        local sp = SPECIES_ID[name]
        E.write8(s + 4 + (i - 1) * 2, sp & 0xFF); E.write8(s + 5 + (i - 1) * 2, sp >> 8)
    end
    for i, flag in ipairs(opts.flags or {}) do
        E.write8(s + 16 + (i - 1) * 2, flag & 0xFF); E.write8(s + 17 + (i - 1) * 2, flag >> 8)
    end
    for i, pair in ipairs(opts.vars or {}) do
        E.write8(s + 24 + (i - 1) * 2, pair[1] & 0xFF); E.write8(s + 25 + (i - 1) * 2, pair[1] >> 8)
        E.write8(s + 28 + (i - 1) * 2, pair[2] & 0xFF); E.write8(s + 29 + (i - 1) * 2, pair[2] >> 8)
    end
    E.write8(s, 1)
    Driver.waitFor("the showcase", 600, function() return E.read8(s) == 0 end, function(i)
        if i % 30 == 0 then Driver.press(Driver.KEY.B, 0) end
    end)
    Driver.wait(10)
end

-- Presses A through message boxes, taking a screenshot of each page.
function Driver.pages(name, count, gap)
    for p = 1, count do
        Driver.wait(gap or 50)
        Driver.shot(name .. (count > 1 and ("-" .. p) or ""))
        Driver.press(Driver.KEY.A, 10)
    end
end

-- Wins the trainer battle that's starting: presses A through the intro,
-- then keeps the opponents at 1 HP and the player's side at full HP and
-- presses A (FIGHT, first move, first target) until it's over.
-- shotName: a screenshot once both sides are out.
function Driver.winBattle(shotName)
    local size = Driver.cmb8(0x023)
    local party = Driver.SYM.gParties + 6 * size -- gParties[B_TRAINER_OPPONENT_A]
    local mons, bsize = Driver.SYM.gBattleMons, Driver.SYM.BATTLEMON_SIZE
    local hpAt -- offset of hp in struct BattlePokemon: found by matching battler 1 to its party slot
    local started, shot = nil, not shotName
    -- Battle style SET (no "switch?" question after a knockout), no animations.
    local options = E.read32(Driver.SYM.gSaveBlock2Ptr) + 0x15
    E.write8(options, E.read8(options) | 0x06)
    Driver.waitFor("the battle to end", 30000, function() return started and Driver.inOverworld() end, function(i)
        if not started and Driver.inBattle() then started = i end
        if started and not hpAt and i - started > 200 then
            local hp, level, maxHP = E.read16(party + 86), E.read8(party + 84), E.read16(party + 88)
            for o = 0, bsize - 6, 2 do
                local b = mons + bsize
                if E.read16(b + o) == hp and E.read8(b + o + 2) == level and E.read16(b + o + 4) == maxHP then hpAt = o; break end
            end
        end
        if hpAt and Driver.inBattle() then
            for battler = 0, 3 do
                local b = mons + battler * bsize
                local v = battler % 2 == 1 and math.min(E.read16(b + hpAt), 1) or E.read16(b + hpAt + 4)
                E.write8(b + hpAt, v & 0xFF); E.write8(b + hpAt + 1, v >> 8)
            end
        end
        if hpAt and i % 600 == 0 then
            Driver.log(string.format("battle hp@%d: %d %d %d %d", hpAt, E.read16(mons + hpAt), E.read16(mons + bsize + hpAt),
                E.read16(mons + 2 * bsize + hpAt), E.read16(mons + 3 * bsize + hpAt)))
        end
        if started and not shot and i - started == 560 then Driver.shot(shotName); shot = true end
        if (not started or i - started > 560 or i - started < 200) and i % 20 == 0 then Driver.press(Driver.KEY.A, 0) end
    end)
    Driver.wait(30)
end

-- Sets party slot `slot`'s first move, keeping the checksum right (the
-- legality check's screenshots use it to make an illegal Pokémon too).
local ATTACKS_AT = {1,1,2,3,2,3,0,0,0,0,0,0,2,3,1,1,3,2,2,3,1,1,3,2} -- src/pokemon.c sSubstructOffsets[1]
function Driver.setMove(slot, move)
    local function r32(a) return E.read16(a) | (E.read16(a + 2) << 16) end
    local mon = Driver.SYM.gParties + slot * Driver.cmb8(0x023)
    local pers, otid = r32(mon), r32(mon + 4)
    local w = {}
    for i = 0, 11 do w[i] = r32(mon + 32 + i * 4) ~ pers ~ otid end
    local word = ATTACKS_AT[pers % 24 + 1] * 3
    w[word] = (w[word] & 0xFFFFF800) | move
    w[word + 2] = (w[word + 2] & 0xFFFFFF80) | 99 -- its PP
    local sum = 0
    for i = 0, 11 do sum = sum + (w[i] & 0xFFFF) + (w[i] >> 16) end
    E.write8(mon + 28, sum & 0xFF); E.write8(mon + 29, (sum >> 8) & 0xFF)
    for i = 0, 11 do
        local v = w[i] ~ pers ~ otid
        for b = 0, 3 do E.write8(mon + 32 + i * 4 + b, (v >> (8 * b)) & 0xFF) end
    end
end

-- Sets party slot `slot`'s contest condition (0 COOL .. 4 TOUGH) and sheen.
local CONDITION_AT = {2,3,1,1,3,2,2,3,1,1,3,2,0,0,0,0,0,0,3,2,3,2,1,1} -- sSubstructOffsets[2]
function Driver.setCondition(slot, category, value, sheen)
    local function r32(a) return E.read16(a) | (E.read16(a + 2) << 16) end
    local mon = Driver.SYM.gParties + slot * Driver.cmb8(0x023)
    local pers, otid = r32(mon), r32(mon + 4)
    local bytes = {}
    for i = 0, 47 do bytes[i] = (r32(mon + 32 + (i & ~3)) ~ pers ~ otid) >> (8 * (i & 3)) & 0xFF end
    local base = CONDITION_AT[pers % 24 + 1] * 12
    bytes[base + 6 + category] = value
    bytes[base + 11] = sheen
    local sum = 0
    for i = 0, 47, 2 do sum = sum + bytes[i] + (bytes[i + 1] << 8) end
    E.write8(mon + 28, sum & 0xFF); E.write8(mon + 29, (sum >> 8) & 0xFF)
    for i = 0, 47, 4 do
        local v = (bytes[i] | (bytes[i + 1] << 8) | (bytes[i + 2] << 16) | (bytes[i + 3] << 24)) ~ pers ~ otid
        for b = 0, 3 do E.write8(mon + 32 + i + b, (v >> (8 * b)) & 0xFF) end
    end
end
