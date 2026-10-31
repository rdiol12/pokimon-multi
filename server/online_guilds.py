"""Guild halls, clan wars and guild base raids, for the plaza's guilds
(server/guilds.py: a tag, a name and members).

- Guild hall: HUB > GUILD HALL takes a guild member to their hall
  (MAP_ONLINE_GUILD_HALL). Every guild has its own: on that map the server
  only shows players of the same guild to each other (room_of). The clerk
  shows the members and keeps the guild chest, items any member can put in
  or take out.
- Clan war: every week, guilds score war points. A member who beats a member
  of another guild in a PvP battle scores 3 (2 each in a 2v2), a won raid 5,
  and a base that holds off a raid 2. The three best guilds of the week win
  prizes for every member, collected from the war officer the week after.
- Guild base raids: members register their team as the guild's defenders.
  The guild's base team is the best Pokémon of each defender (up to six).
  Raiders battle it like a secret base team, three raids a day.
"""

import struct
import zlib

from online import Refused, answer, board
from online_ids import ITEMS, MAPS

HALL = tuple(MAPS["ONLINE_GUILD_HALL"])
CHEST_MAX = 99
CHEST_SLOTS = 20
RAIDS_PER_DAY = 3
WAR_WIN, WAR_TEAM_WIN, WAR_RAID, WAR_DEFENDED = 3, 2, 5, 2
# Place -> (item, count, money) for every member.
WAR_PRIZES = {1: ("RARE_CANDY", 3, 10000), 2: ("RARE_CANDY", 2, 5000), 3: ("RARE_CANDY", 1, 2500)}
PLACE_NAMES = {1: "1st", 2: "2nd", 3: "3rd"}
BOARD_SIZE = 8

# A team record is the game's struct SecretBase (160 bytes); its party starts
# at 0x34: personality u32[6], moves u16[24], species u16[6], items u16[6],
# levels u8[6], EVs u8[6].
RECORD_SIZE = 160
PARTY = 0x34
PERSONALITY, MOVES, SPECIES, HELD, LEVELS, EVS = PARTY, PARTY + 24, PARTY + 72, PARTY + 84, PARTY + 96, PARTY + 102

# VAR_0x800B of GHALL VIEW.
NO_GUILD, MEMBER, LEADER = 0, 1, 2

ITEM_NAMES = {v: k for k, v in ITEMS.items()}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_guild_chest (
            tag TEXT NOT NULL, item INTEGER NOT NULL, count INTEGER NOT NULL, PRIMARY KEY (tag, item)
        );
        CREATE TABLE IF NOT EXISTS online_guild_war (
            week INTEGER NOT NULL, tag TEXT NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            points INTEGER NOT NULL, PRIMARY KEY (week, tag, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_guild_war_claims (
            week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, PRIMARY KEY (week, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_guild_defenders (
            tag TEXT NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, team BLOB NOT NULL,
            PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_guild_raids (
            day INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, target TEXT NOT NULL,
            won INTEGER
        );
        """
    )


def guild_of(db, key):
    """(tag, name, leader key) of the player's guild, or None."""
    row = db.execute("SELECT g.tag, g.name, g.leader_name, g.leader_tid FROM guild_members m"
                     " JOIN guilds g ON g.tag = m.tag WHERE m.name = ? AND m.tid = ?", (*key,)).fetchone()
    return (row[0], row[1], (row[2], row[3])) if row else None


def members(db, tag):
    return [tuple(m) for m in db.execute("SELECT name, tid FROM guild_members WHERE tag = ? ORDER BY name", (tag,))]


def item_text(item, count):
    name = ITEM_NAMES.get(item, "?").replace("_", " ").title().replace("Poke", "Poké")
    if count == 1:
        return name
    if name.endswith("y"):
        return f"{count} {name[:-1]}ies"
    return f"{count} {name}{'' if name.endswith('s') else 's'}"


# --- clan war ---------------------------------------------------------------

def add_war_points(server, key, points, why=None):
    """Gives the player's guild war points this week; returns its tag."""
    db = server.league.db
    guild = guild_of(db, key)
    if guild is None or not hasattr(server, "online"):
        return None
    week = server.online.week()
    db.execute("INSERT INTO online_guild_war VALUES (?, ?, ?, ?, ?) ON CONFLICT (week, tag, name, tid)"
               " DO UPDATE SET points = points + excluded.points", (week, guild[0], *key, points))
    db.commit()
    if why:
        server.online.notify(key, f"{why} +{points} war points for ({guild[0]})!")
    return guild[0]


def standings(db, week):
    """[(tag, name, points)], best first."""
    return db.execute("SELECT w.tag, g.name, SUM(w.points) AS p FROM online_guild_war w JOIN guilds g ON g.tag = w.tag"
                      " WHERE w.week = ? GROUP BY w.tag ORDER BY p DESC, w.tag", (week,)).fetchall()


def unclaimed_prizes(db, key, week):
    """[(week, place)] of past weeks the player's guild placed in and they haven't claimed."""
    guild = guild_of(db, key)
    if guild is None:
        return []
    out = []
    for (w,) in db.execute("SELECT DISTINCT week FROM online_guild_war WHERE tag = ? AND week < ?", (guild[0], week)):
        if db.execute("SELECT 1 FROM online_guild_war_claims WHERE week = ? AND name = ? AND tid = ?",
                      (w, *key)).fetchone():
            continue
        for place, (tag, name, points) in enumerate(standings(db, w)[:3], 1):
            if tag == guild[0]:
                out.append((w, place))
    return out


def on_battle(server, session):
    """A PvP battle ended: members who beat another guild's members score."""
    if session.kind not in ("battle", "double", "multi"):
        return
    keys = [p.key for p in session.members]
    db = server.league.db
    if session.kind == "multi":
        sides = ((keys[0], keys[2]), (keys[1], keys[3]))
        said = [{session.reports.get(k) for k in side} - {None} for side in sides]
    else:
        sides = ((keys[0],), (keys[1],))
        said = [{session.reports.get(keys[0])} - {None}, {session.reports.get(keys[1])} - {None}]
    if said[0] == {"win"} and said[1] in ({"loss"}, set()):
        winners, losers = sides
    elif said[1] == {"win"} and said[0] in ({"loss"}, set()):
        winners, losers = sides[1], sides[0]
    else:
        return
    loser_guilds = {g[0] for g in (guild_of(db, k) for k in losers) if g}
    points = WAR_TEAM_WIN if session.kind == "multi" else WAR_WIN
    for k in winners:
        mine = guild_of(db, k)
        if mine and loser_guilds - {mine[0]}:
            add_war_points(server, k, points, "Clan war win!")


# --- raids ------------------------------------------------------------------

# Defenders who are still in the guild.
DEFENDERS = ("SELECT {} FROM online_guild_defenders d JOIN guild_members m"
             " ON m.name = d.name AND m.tid = d.tid AND m.tag = d.tag WHERE d.tag = ?")


def best_mon(team):
    """The highest-level Pokémon of a team record: its slot, or None."""
    best = None
    for i in range(6):
        species = struct.unpack_from("<H", team, SPECIES + 2 * i)[0]
        if species and (best is None or team[LEVELS + i] > team[LEVELS + best]):
            best = i
    return best


def base_team(db, tag):
    """The guild base's team record: each defender's best Pokémon, strongest
    first, up to six; None without defenders."""
    picks = []
    for name, tid, team in db.execute(DEFENDERS.format("d.name, d.tid, d.team") + " ORDER BY d.name", (tag,)):
        slot = best_mon(team) if len(team) == RECORD_SIZE else None
        if slot is not None:
            picks.append((team[LEVELS + slot], team, slot))
    if not picks:
        return None
    picks.sort(key=lambda p: -p[0])
    out = bytearray(RECORD_SIZE)
    # The trainer is the guild: its tag as the name, a trainer id from it.
    out[2:2 + 7] = b"\xff" * 7
    for i, ch in enumerate(tag.encode("ascii")):
        out[2 + i] = game_char(ch)
    out[9:13] = struct.pack("<I", zlib.crc32(tag.encode()))
    out[13] = 2  # LANGUAGE_ENGLISH
    for n, (level, team, slot) in enumerate(picks[:6]):
        out[PERSONALITY + 4 * n:PERSONALITY + 4 * n + 4] = team[PERSONALITY + 4 * slot:PERSONALITY + 4 * slot + 4]
        out[MOVES + 8 * n:MOVES + 8 * n + 8] = team[MOVES + 8 * slot:MOVES + 8 * slot + 8]
        for field in (SPECIES, HELD):
            out[field + 2 * n:field + 2 * n + 2] = team[field + 2 * slot:field + 2 * slot + 2]
        out[LEVELS + n] = team[LEVELS + slot]
        out[EVS + n] = team[EVS + slot]
    return bytes(out)


def game_char(c):
    """An ASCII letter or digit in the game's charset."""
    if ord("A") <= c <= ord("Z"):
        return 0xBB + c - ord("A")
    if ord("0") <= c <= ord("9"):
        return 0xA1 + c - ord("0")
    return 0x00  # space


def raids_today(db, key, day):
    return db.execute("SELECT COUNT(*) FROM online_guild_raids WHERE day = ? AND name = ? AND tid = ?",
                      (day, *key)).fetchone()[0]


def install(server):
    def room_of(player, map_key):
        guild = guild_of(server.league.db, player.key) if map_key == HALL else None
        return guild[0] if guild else None

    server.room_of = room_of
    finish = server.finish_battle

    def finish_battle(session):
        finish(session)
        on_battle(server, session)

    server.finish_battle = finish_battle


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def need_guild(db, key):
    guild = guild_of(db, key)
    if guild is None:
        raise Refused("Join a guild first, in the Online Plaza's GUILDS.")
    return guild


def cmd_ghall(ctx, args, data):
    db, key = ctx.db, ctx.key
    word = args[0]

    if word == "VIEW":
        guild = guild_of(db, key)
        if guild is None:
            return answer("Join a guild in the Online Plaza's GUILDS~to get into a guild hall!", code=NO_GUILD)
        tag, name, leader = guild
        online = sum(1 for p in ctx.server.players.values() if p.name and guild_of(db, p.key) == guild)
        return answer(f"Off to the hall of {name} ({tag})!~{online} of {len(members(db, tag))} members are online.",
                      code=LEADER if tuple(leader) == tuple(key) else MEMBER, vars=[tag, name])

    tag, name, leader = need_guild(db, key)
    week = ctx.online.week()

    if word == "MEMBERS":
        here = {p.key for p in ctx.server.players.values() if p.name}
        points = dict(((n, t), p) for n, t, p in db.execute(
            "SELECT name, tid, points FROM online_guild_war WHERE week = ? AND tag = ?", (week, tag)))
        rows = []
        for m in members(db, tag):
            mark = "*" if m == tuple(key) else ""
            role = "leader" if m == tuple(leader) else ""
            rows.append([f"{mark}{m[0]}", "online" if m in here else "", role, points.get(m, 0)])
        return answer(f"{name} ({tag}) has {len(rows)} members.",
                      rows=board(f"{name.upper()} ({tag})", [(6, "TRAINER"), (70, "ONLINE"), (120, "ROLE"),
                                                             (176, "WAR")], rows[:12]))

    if word == "CHEST":
        rows = [[item_text(item, count), item, item, count]
                for item, count in db.execute("SELECT item, count FROM online_guild_chest WHERE tag = ? AND count > 0"
                                              " ORDER BY item", (tag,))]
        if not rows:
            raise Refused("The guild chest is empty. Put something in for your guildmates!")
        return answer("What will you take out?", rows=rows)

    if word == "PUT":
        # item count
        item, count = number(args, 1), number(args, 2)
        if item not in ITEM_NAMES or not 1 <= count <= CHEST_MAX:
            raise Refused("That can't go in the chest.")
        have = db.execute("SELECT count FROM online_guild_chest WHERE tag = ? AND item = ?", (tag, item)).fetchone()
        slots = db.execute("SELECT COUNT(*) FROM online_guild_chest WHERE tag = ? AND count > 0", (tag,)).fetchone()[0]
        if have is None and slots >= CHEST_SLOTS:
            raise Refused("The guild chest is full.")
        count = min(count, CHEST_MAX - (have[0] if have else 0))
        if count <= 0:
            raise Refused("The chest has no room for more of those.")
        db.execute("INSERT INTO online_guild_chest VALUES (?, ?, ?) ON CONFLICT (tag, item)"
                   " DO UPDATE SET count = count + excluded.count", (tag, item, count))
        return answer(f"{ctx.player.name} put {item_text(item, count)} in the guild chest.",
                      rows=[["-ITEM", item, count]])

    if word == "TAKE":
        item = number(args, 1)
        have = db.execute("SELECT count FROM online_guild_chest WHERE tag = ? AND item = ?", (tag, item)).fetchone()
        if not have or have[0] <= 0:
            raise Refused("Someone took that already!")
        db.execute("DELETE FROM online_guild_chest WHERE tag = ? AND item = ?", (tag, item))
        return answer(f"{ctx.player.name} took {item_text(item, have[0])} from the guild chest.",
                      rows=[["+ITEM", item, have[0]]])

    raise ValueError("bad GHALL")


def cmd_war(ctx, args, data):
    db, key = ctx.db, ctx.key
    tag, name, leader = need_guild(db, key)
    week = ctx.online.week()
    word = args[0]

    if word == "BOARD":
        rows = []
        for place, (t, n, points) in enumerate(standings(db, week), 1):
            rows.append([f"{'*' if t == tag else ''}{place}", f"({t}) {n}", points])
        mine = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
        message = (f"({tag}) is number {mine[0][0][1:]} this week, with {mine[0][2]} points." if mine
                   else "Win battles against other guilds and raid their bases to score!")
        return answer(message, code=len(unclaimed_prizes(db, key, week)),
                      rows=board("CLAN WAR: THIS WEEK", [(6, "No."), (24, "GUILD"), (160, "POINTS")], rows))

    if word == "CLAIM":
        prizes = unclaimed_prizes(db, key, week)
        # Weeks without a prize are settled too.
        for (w,) in db.execute("SELECT DISTINCT week FROM online_guild_war WHERE tag = ? AND week < ?",
                               (tag, week)).fetchall():
            db.execute("INSERT OR IGNORE INTO online_guild_war_claims VALUES (?, ?, ?)", (w, *key))
        if not prizes:
            raise Refused("No clan war prizes are waiting.")
        rows, said = [], []
        for w, place in prizes:
            item, count, money = WAR_PRIZES[place]
            rows += [["+ITEM", ITEMS[item], count], ["+MONEY", money]]
            said.append(f"{PLACE_NAMES[place]} place: {item_text(ITEMS[item], count)} and ${money}")
        return answer(f"{', '.join(said)}!", rows=rows)

    raise ValueError("bad WAR")


def cmd_raid(ctx, args, data):
    db, key = ctx.db, ctx.key
    tag, name, leader = need_guild(db, key)
    day = ctx.online.day()
    word = args[0]

    if word == "DEFEND":
        if len(data) != RECORD_SIZE or best_mon(data) is None:
            raise Refused("Your team can't defend the base.")
        db.execute("INSERT OR REPLACE INTO online_guild_defenders VALUES (?, ?, ?, ?)", (tag, *key, data))
        slot = best_mon(data)
        n = db.execute(DEFENDERS.format("COUNT(*)"), (tag,)).fetchone()[0]
        return answer(f"Your Lv. {data[LEVELS + slot]} POKéMON guards the ({tag}) base now!~"
                      f"Defenders: {n}.")

    if word == "LIST":
        left = RAIDS_PER_DAY - raids_today(db, key, day)
        rows = []
        for t, n in db.execute("SELECT d.tag, g.name FROM online_guild_defenders d JOIN guilds g ON g.tag = d.tag"
                               " JOIN guild_members m ON m.name = d.name AND m.tid = d.tid AND m.tag = d.tag"
                               " WHERE d.tag != ? GROUP BY d.tag ORDER BY COUNT(*) DESC, d.tag", (tag,)):
            rows.append([f"({t}) {n}", t])
        if not rows:
            raise Refused("No other guild has defenders yet.")
        if left <= 0:
            raise Refused(f"You've raided {RAIDS_PER_DAY} times today. Come back tomorrow!")
        return answer(f"Which base will you raid?~Raids left today: {left}.", rows=rows[:20])

    if word == "GET":
        target = args[1] if len(args) > 1 else ""
        if target == tag:
            raise Refused("That's your own base!")
        if raids_today(db, key, day) >= RAIDS_PER_DAY:
            raise Refused(f"You've raided {RAIDS_PER_DAY} times today. Come back tomorrow!")
        team = base_team(db, target)
        if team is None:
            raise Refused("That base has no defenders left.")
        db.execute("INSERT INTO online_guild_raids (day, name, tid, target) VALUES (?, ?, ?, ?)", (day, *key, target))
        ctx.player.guild_raid = (db.execute("SELECT last_insert_rowid()").fetchone()[0], target)
        n = sum(1 for i in range(6) if team[SPECIES + 2 * i])
        top = team[LEVELS]
        return answer(f"The ({target}) base: {n} defender{'' if n == 1 else 's'}, the best at Lv. {top}.~Go!",
                      vars=[target]) + b"\0" + team

    if word == "RESULT":
        won = number(args, 1)
        raid = getattr(ctx.player, "guild_raid", None)
        if raid is None:
            raise Refused("That raid is over.")
        ctx.player.guild_raid = None
        rowid, target = raid
        db.execute("UPDATE online_guild_raids SET won = ? WHERE rowid = ?", (1 if won else 0, rowid))
        defenders = [tuple(d) for d in db.execute(DEFENDERS.format("d.name, d.tid"), (target,))]
        if won:
            add_war_points(ctx.server, key, WAR_RAID)
            for d in defenders:
                ctx.online.notify(d, f"{ctx.player.name} of ({tag}) raided the ({target}) base!")
            return answer(f"The ({target}) base fell! +{WAR_RAID} war points for ({tag})!")
        # The base held: its defenders' guild scores.
        if defenders:
            add_war_points(ctx.server, defenders[0], WAR_DEFENDED)
            for d in defenders:
                ctx.online.notify(d, f"The ({target}) base held off {ctx.player.name} of ({tag})!")
        return answer(f"The ({target}) base held... Train up and try again!")

    raise ValueError("bad RAID")


COMMANDS = {"GHALL": cmd_ghall, "WAR": cmd_war, "RAID": cmd_raid}
