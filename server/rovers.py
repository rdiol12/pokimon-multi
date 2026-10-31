"""Roaming trainers ("rovers") and the wild Pokémon on the quest boards.

150 trainers (server/data/rovers_cast.json, from tools/rovers/make_cast.py)
travel all over Hoenn, map to map along the game's own connections
(server/data/rovers_world.json.gz, from tools/rovers/make_rover_world.py).
The server keeps their lives going while nobody is watching: every ten
minutes each one may move on, train, catch something, take on a gym, use
the Day Care, fight another rover, or heal at a Pokémon Center.

When a player walks onto a map the game asks who is there (HERE) and plays
those rovers out in front of them: walking, catching and fighting. What
happens there comes back (SYNC, FIGHT, BEAT, LEAVE) and becomes the rover's
real progress. The game works out evolutions and moves itself (it has the
data), so a Pokémon the server levels up comes back evolved after the next
visit.

Quest boards (MAJOR): about a dozen "major" wild Pokémon, much stronger than
the others on their map, are posted at a time. A player who clears one
(knocks it out or catches it) collects the reward at any board. Rovers take
these jobs too.

Wire formats (little endian), the same as src/rovers.c:
  RoverMon (16 bytes): species u16, level u8, movesLevel u8, moves u16[4],
      injuryPart u8, injuryDamage u8 (2% units), lostParts u8, injuryFlags u8
  Rover (104 bytes): id u8, count u8, badges u8, hurt u8, wins u16, losses u16,
      RoverMon[6]
"""

import gzip
import json
import os
import random
import struct
import time

from ext import Refused, text

try:  # the Haven Reserve (server/community_reserve.py), if this server has it
    import community_reserve
except ImportError:
    community_reserve = None

HERE_DIR = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(HERE_DIR, "data", "rovers_cast.json")) as f:
    CAST = json.load(f)
with gzip.open(os.path.join(HERE_DIR, "data", "rovers_world.json.gz"), "rt") as f:
    _WORLD = json.load(f)
WORLD = _WORLD["maps"]
BASE = {int(k): v for k, v in _WORLD["base"].items()}
NAMES = {int(k): v for k, v in _WORLD["names"].items()}
BY_NAME = {v["name"]: k for k, v in WORLD.items()}

MON_SIZE = 16
ROVER_SIZE = 8 + 6 * MON_SIZE
MAX_TEAM = 6
MAX_HERE = 4          # the game shows at most this many on one map
STEP = 600            # seconds of rover life per simulation step
MAX_STEPS = 72        # catching up after a long quiet spell stops at 12 hours
TID_BASE = 70000      # rovers' trainer ids on the PvP ladder (above any real id)
MAX_MAJORS = 12
MAJOR_REWARD = 3000   # per 10 levels
MAX_NEWS = 40

# Gyms in badge order: (town, ace level). Sootopolis can't be walked to,
# so the eighth badge is won at Mossdeep's Space Center... in the sim.
GYMS = [("RustboroCity", 15), ("DewfordTown", 19), ("MauvilleCity", 24), ("LavaridgeTown", 29),
        ("PetalburgCity", 31), ("FortreeCity", 33), ("MossdeepCity", 42), ("SootopolisCity", 46)]
LEAGUE_LEVEL = 58
START_MAP = BY_NAME.get("LittlerootTown")

now = time.time  # tests replace this
rng = random.Random()


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS rovers (
            id INTEGER PRIMARY KEY,
            map TEXT NOT NULL,
            team BLOB NOT NULL,
            badges INTEGER NOT NULL DEFAULT 0,
            wins INTEGER NOT NULL DEFAULT 0,
            losses INTEGER NOT NULL DEFAULT 0,
            hurt INTEGER NOT NULL DEFAULT 0,
            egg INTEGER NOT NULL DEFAULT 0,
            egg_steps INTEGER NOT NULL DEFAULT 0,
            tick REAL NOT NULL
        )
        """
    )
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS rover_news (
            id INTEGER PRIMARY KEY, at REAL NOT NULL, line TEXT NOT NULL
        )
        """
    )
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS majors (
            id INTEGER PRIMARY KEY,
            map TEXT NOT NULL, species INTEGER NOT NULL, level INTEGER NOT NULL,
            posted REAL NOT NULL,
            cleared_by TEXT, cleared_tid INTEGER, cleared_at REAL, paid INTEGER NOT NULL DEFAULT 0
        )
        """
    )


# Teams ----------------------------------------------------------------------

def new_mon(species, level):
    return {"species": species, "level": level, "moves_level": 0, "moves": [0, 0, 0, 0],
            "part": 0, "damage": 0, "lost": 0, "flags": 0}


def pack_mon(m):
    return struct.pack("<HBB4HBBBB", m["species"], m["level"], m["moves_level"], *m["moves"],
                       m["part"], m["damage"], m["lost"], m["flags"])


def unpack_mon(b):
    species, level, ml, m1, m2, m3, m4, part, damage, lost, flags = struct.unpack("<HBB4HBBBB", b)
    return {"species": species, "level": level, "moves_level": ml, "moves": [m1, m2, m3, m4],
            "part": part, "damage": damage, "lost": lost, "flags": flags}


def pack_team(team):
    return b"".join(pack_mon(m) for m in team)


def unpack_team(blob):
    return [unpack_mon(blob[i:i + MON_SIZE]) for i in range(0, len(blob), MON_SIZE)]


def check_team(team):
    if not 1 <= len(team) <= MAX_TEAM:
        raise ValueError("bad team size")
    for m in team:
        if not 0 < m["species"] < 0xFFFF or not 1 <= m["level"] <= 100:
            raise ValueError("bad Pokémon")


class Rover:
    """One row of the rovers table, loaded."""

    FIELDS = ("id", "map", "team", "badges", "wins", "losses", "hurt", "egg", "egg_steps", "tick")

    def __init__(self, row):
        for k, v in zip(self.FIELDS, row):
            setattr(self, k, v)
        self.team = unpack_team(self.team)

    @property
    def cast(self):
        return CAST[self.id]

    @property
    def name(self):
        return self.cast["name"]

    @property
    def key(self):
        return (self.name, TID_BASE + self.id)

    def strength(self):
        levels = sorted((m["level"] for m in self.team), reverse=True)[:3]
        return sum(levels) * (1.0 - 0.1 * min(self.hurt, 5))

    def lead_level(self):
        return max(m["level"] for m in self.team)

    def cap(self):
        if self.badges >= len(GYMS):
            return min(100, LEAGUE_LEVEL + 10)
        return GYMS[self.badges][1] + 3

    def save(self, db):
        db.execute(
            "UPDATE rovers SET map = ?, team = ?, badges = ?, wins = ?, losses = ?, hurt = ?, egg = ?,"
            " egg_steps = ?, tick = ? WHERE id = ?",
            (self.map, pack_team(self.team), self.badges, self.wins, self.losses, self.hurt, self.egg,
             self.egg_steps, self.tick, self.id))

    def wire(self):
        head = struct.pack("<BBBBHH", self.id, len(self.team), self.badges, min(self.hurt, 255),
                           min(self.wins, 0xFFFF), min(self.losses, 0xFFFF))
        team = pack_team(self.team) + bytes(MON_SIZE * (MAX_TEAM - len(self.team)))
        return head + team


def news(db, line):
    line = line.replace("é", "e").replace("É", "E")
    db.execute("INSERT INTO rover_news (at, line) VALUES (?, ?)", (now(), line))
    db.execute("DELETE FROM rover_news WHERE id <= (SELECT MAX(id) FROM rover_news) - ?", (MAX_NEWS,))


def species_name(sid):
    return NAMES.get(sid, "Pokémon")


def ensure_rovers(db):
    count = db.execute("SELECT COUNT(*) FROM rovers").fetchone()[0]
    if count >= len(CAST):
        return
    places = [k for k, v in WORLD.items() if v["kind"] != "sea"]
    for i in range(count, len(CAST)):
        c = CAST[i]
        team = [new_mon(c["starter"], 5)]
        db.execute("INSERT INTO rovers (id, map, team, tick) VALUES (?, ?, ?, ?)",
                   (i, rng.choice(places), pack_team(team), now()))


def load(db, rid):
    row = db.execute("SELECT " + ", ".join(Rover.FIELDS) + " FROM rovers WHERE id = ?", (rid,)).fetchone()
    if row is None:
        raise Refused("Nobody by that name is around.")
    return Rover(row)


def load_all(db):
    return [Rover(r) for r in db.execute("SELECT " + ", ".join(Rover.FIELDS) + " FROM rovers ORDER BY id")]


# The world --------------------------------------------------------------------

def next_hop(start, goal_test):
    """The first map on a shortest path from start to a map where goal_test
    holds, or None."""
    if start not in WORLD:
        return None
    prev, todo = {start: None}, [start]
    while todo:
        cur = todo.pop(0)
        if goal_test(cur) and cur != start:
            while prev[cur] != start:
                cur = prev[cur]
            return cur
        for nxt in WORLD[cur]["links"]:
            if nxt not in prev:
                prev[nxt] = cur
                todo.append(nxt)
    return None


def gym_index(map_key):
    name = WORLD.get(map_key, {}).get("name")
    for i, (town, _) in enumerate(GYMS):
        if town == name:
            return i
    return None


def gym_target(r):
    """Where the next badge is won. Sootopolis isn't on foot, so badge 8 is
    won at Mossdeep instead."""
    if r.badges >= len(GYMS):
        return None
    town = GYMS[r.badges][0]
    if town not in BY_NAME:
        town = "MossdeepCity"
    return BY_NAME.get(town)


def add_injury(m, lo, hi):
    if m["part"] == 0:
        m["part"] = rng.randint(1, 5)
    m["damage"] = min(50, m["damage"] + rng.randint(lo, hi) // 2)
    if m["damage"] >= 50:
        m["lost"] |= 1 << (m["part"] - 1)
        m["part"], m["damage"] = 0, 0


def fight(db, a, b, winner_a):
    """Records a rover-vs-rover fight, on the rovers and the PvP ladder."""
    w, l = (a, b) if winner_a else (b, a)
    w.wins += 1
    l.losses += 1
    l.hurt += 1
    if rng.random() < 0.3:
        add_injury(rng.choice(l.team), 10, 30)
    db.execute("INSERT OR IGNORE INTO players (name, tid) VALUES (?, ?)", w.key)
    db.execute("INSERT OR IGNORE INTO players (name, tid) VALUES (?, ?)", l.key)
    return w, l


class Sim:
    def __init__(self, db, league, server=None):
        self.db, self.league, self.server = db, league, server

    def release(self, r, m):
        """A Pokémon a rover lets go goes to the Haven Reserve."""
        if community_reserve is not None and self.server is not None:
            try:
                community_reserve.drop_off(self.server, "ROVER " + r.name, m["species"], m["level"])
            except Exception:  # the reserve is optional; never break the sim
                pass

    def visit_reserve(self, r):
        """Near Verdanturf a rover may catch one of the reserve's Pokémon."""
        if community_reserve is None or self.server is None or len(r.team) >= MAX_TEAM:
            return
        try:
            got = community_reserve.pick_for_rover(self.server, "ROVER " + r.name, max_level=r.lead_level() + 5)
        except Exception:
            return
        if got:
            r.team.append(new_mon(got["species"], max(1, min(100, got["level"]))))
            news(self.db, f"{r.name} caught a {species_name(got['species'])} at the Haven Reserve.")

    def step(self, r, others):
        here = WORLD.get(r.map)
        if here is None:
            r.map = START_MAP
            return
        # Untreated injuries get worse; the clinic treats them.
        for m in r.team:
            if m["part"] and rng.random() < 0.25:
                add_injury(m, 2, 6)
        if here["center"] and (r.hurt or any(m["part"] for m in r.team)):
            r.hurt = 0
            for m in r.team:
                m["part"], m["damage"] = 0, 0
        # Eggs hatch.
        if r.egg:
            r.egg_steps -= 1
            if r.egg_steps <= 0:
                baby = new_mon(r.egg, 1)
                if len(r.team) < MAX_TEAM:
                    r.team.append(baby)
                elif r.cast["style"] == "BREEDER":
                    weakest = min(range(len(r.team)), key=lambda i: r.team[i]["level"])
                    self.release(r, r.team[weakest])
                    r.team[weakest] = baby
                news(self.db, f"{r.name}'s egg hatched into {species_name(r.egg)}!")
                r.egg = 0
        # Gyms.
        g = gym_index(r.map)
        target = gym_target(r)
        if target == r.map and (g is not None or WORLD[r.map]["name"] == "MossdeepCity"):
            ace = GYMS[r.badges][1]
            if r.lead_level() >= ace - 2 and r.hurt < 2:
                if rng.random() < 0.6:
                    r.badges += 1
                    news(self.db, f"{r.name} won badge {r.badges} in {WORLD[r.map]['name']}!")
                else:
                    r.hurt += 1
                return
        # Somebody else here? Maybe a fight.
        rivals = [o for o in others if o.map == r.map and o.id != r.id]
        if rivals and rng.random() < 0.12:
            o = rng.choice(rivals)
            sa, sb = r.strength(), o.strength()
            won = rng.random() < sa / max(sa + sb, 1)
            w, l = fight(self.db, r, o, won)
            self.league.record(w.key, "win", l.key, "loss")
            o.save(self.db)
            return
        # Day Care.
        if here["daycare"] and not r.egg and len(r.team) >= 2:
            chance = 0.5 if r.cast["style"] == "BREEDER" else 0.08
            if rng.random() < chance:
                parent = rng.choice(r.team)["species"]
                r.egg, r.egg_steps = BASE.get(parent, parent), 6
                news(self.db, f"{r.name} got an egg at the Day Care.")
                return
        # Moving on.
        dest = None
        if r.hurt >= 2:
            dest = next_hop(r.map, lambda k: WORLD[k]["center"])
        elif target and r.lead_level() >= GYMS[r.badges][1] - 2 and rng.random() < 0.7:
            dest = next_hop(r.map, lambda k: k == target)
        if dest is None and (not here["wild"] or rng.random() < 0.35):
            dest = rng.choice(here["links"]) if here["links"] else None
        if dest is not None:
            r.map = dest
            return
        if here["name"] == "VerdanturfTown" and rng.random() < 0.3:
            self.visit_reserve(r)
        if not here["wild"]:
            return
        # Training and catching.
        style = r.cast["style"]
        cap = r.cap()
        lead = max(r.team, key=lambda m: m["level"])
        for m in r.team:
            chance = (0.9 if style == "TRAINER" else 0.6) if m is lead else 0.3
            if m["level"] < cap and rng.random() < chance:
                m["level"] += 1
        catch = 0.3 if style == "COLLECTOR" else 0.15
        if rng.random() < catch:
            sid, lo, hi = rng.choice(here["wild"])
            level = rng.randint(lo, hi)
            if len(r.team) < MAX_TEAM:
                r.team.append(new_mon(sid, level))
                news(self.db, f"{r.name} caught a {species_name(sid)} on {here['name']}.")
            elif style == "COLLECTOR":
                weakest = min(range(len(r.team)), key=lambda i: r.team[i]["level"])
                if r.team[weakest]["level"] < level + 5:
                    self.release(r, r.team[weakest])
                    r.team[weakest] = new_mon(sid, level)
                    news(self.db, f"{r.name} caught a {species_name(sid)} and sent one home.")
        # Quest board jobs.
        major = self.db.execute("SELECT id, species, level FROM majors WHERE map = ? AND cleared_by IS NULL",
                                (r.map,)).fetchone()
        if major and r.lead_level() >= major[2] - 3 and rng.random() < 0.3:
            self.db.execute("UPDATE majors SET cleared_by = ?, cleared_tid = ?, cleared_at = ? WHERE id = ?",
                            (r.name, TID_BASE + r.id, now(), major[0]))
            news(self.db, f"{r.name} beat the rampaging {species_name(major[1])} on {here['name']}!")

    def run(self):
        """Brings every rover up to now."""
        ensure_rovers(self.db)
        rovers = load_all(self.db)
        t = now()
        for r in rovers:
            steps = int((t - r.tick) // STEP)
            if steps <= 0:
                continue
            for _ in range(min(steps, MAX_STEPS)):
                self.step(r, rovers)
            r.tick = t if steps > MAX_STEPS else r.tick + steps * STEP
            r.save(self.db)
        post_majors(self.db)
        return rovers


# Quest board -------------------------------------------------------------------

def post_majors(db):
    open_ = db.execute("SELECT COUNT(*) FROM majors WHERE cleared_by IS NULL").fetchone()[0]
    taken = {row[0] for row in db.execute("SELECT map FROM majors WHERE cleared_by IS NULL")}
    places = [k for k, v in WORLD.items() if v["wild"] and k not in taken]
    while open_ < MAX_MAJORS and places:
        k = places.pop(rng.randrange(len(places)))
        sid, lo, hi = max(WORLD[k]["wild"], key=lambda w: (w[2], rng.random()))
        level = min(100, hi + rng.randint(8, 12))
        db.execute("INSERT INTO majors (map, species, level, posted) VALUES (?, ?, ?, ?)", (k, sid, level, now()))
        open_ += 1


def hops(a, b, limit):
    if a == b:
        return 0
    seen, frontier = {a}, [a]
    for d in range(1, limit + 1):
        nxt = []
        for k in frontier:
            for n in WORLD.get(k, {}).get("links", []):
                if n == b:
                    return d
                if n not in seen:
                    seen.add(n)
                    nxt.append(n)
        frontier = nxt
    return None


# Commands -----------------------------------------------------------------------

def map_key(args, i):
    return "%d.%d" % (int(args[i]), int(args[i + 1]))


def cmd_here(ctx, args, data):
    """ROVER HERE <group> <num> <max>: the rovers on this map (pulling a few
    in from next door, so routes don't stay empty) and its major wild
    Pokémon. Answer: count|majorId|majorSpecies|majorLevel, NUL, the rovers."""
    key, most = map_key(args, 1), max(0, min(int(args[3]), MAX_HERE))
    rovers = Sim(ctx.db, ctx.league, ctx.server).run()
    here = [r for r in rovers if r.map == key]
    if key in WORLD and WORLD[key]["kind"] != "sea":
        nearby = [r for r in rovers if r.map in WORLD[key]["links"]]
        rng.shuffle(nearby)
        for r in nearby:
            if len(here) >= min(2, most) or rng.random() > 0.45:
                continue
            r.map = key
            r.save(ctx.db)
            here.append(r)
    here = here[:most]
    major = ctx.db.execute("SELECT id, species, level FROM majors WHERE map = ? AND cleared_by IS NULL",
                           (key,)).fetchone() or (0, 0, 0)
    head = text([(len(here), *major)])
    return head + b"\0" + b"".join(r.wire() for r in here)


def cmd_sync(ctx, args, data):
    """ROVER SYNC <id> <event> <arg> + one Rover: the team after what the
    player saw happen. Events: 0 nothing, 1 caught <species>, 2 evolved
    into <species>, 3 grew to level <arg>, 4 healed."""
    rid, event, arg = int(args[1]), int(args[2]), int(args[3])
    if len(data) != ROVER_SIZE:
        raise ValueError("bad rover")
    r = load(ctx.db, rid)
    count = data[1]
    team = unpack_team(data[8:8 + count * MON_SIZE])
    check_team(team)
    r.team = team
    r.hurt = min(data[3], 10)
    where = WORLD.get(r.map, {}).get("name", "the road")
    if event == 1:
        news(ctx.db, f"{r.name} caught a {species_name(arg)} on {where}.")
    elif event == 2:
        news(ctx.db, f"{r.name}'s Pokémon evolved into {species_name(arg)}!")
    r.save(ctx.db)
    return b""


def cmd_fight(ctx, args, data):
    """ROVER FIGHT <a> <b> <1 if a won>: two rovers fought on the player's
    map (watched or not)."""
    a, b = load(ctx.db, int(args[1])), load(ctx.db, int(args[2]))
    if a.id == b.id:
        raise ValueError("a rover can't fight itself")
    w, l = fight(ctx.db, a, b, int(args[3]) != 0)
    ctx.league.record(w.key, "win", l.key, "loss")
    news(ctx.db, f"{w.name} beat {l.name}!")
    a.save(ctx.db)
    b.save(ctx.db)
    return b""


def cmd_beat(ctx, args, data):
    """ROVER BEAT <id> <1 if the rover won> <who>: a rover fought the game's
    own trainer (who 0) or the player (who 1)."""
    r, won, who = load(ctx.db, int(args[1])), int(args[2]) != 0, int(args[3])
    if won:
        r.wins += 1
    else:
        r.losses += 1
        r.hurt += 1
    if who == 1:
        ctx.db.execute("INSERT OR IGNORE INTO players (name, tid) VALUES (?, ?)", r.key)
        if won:
            ctx.league.record(r.key, "win", ctx.key, "loss")
        else:
            ctx.league.record(ctx.key, "win", r.key, "loss")
        news(ctx.db, f"{ctx.key[0]} {'lost to' if won else 'beat'} {r.name}.")
    r.save(ctx.db)
    return b""


def cmd_leave(ctx, args, data):
    """ROVER LEAVE <id> <group> <num>: the rover walked off the player's map;
    it goes to a map next door."""
    r = load(ctx.db, int(args[1]))
    key = map_key(args, 2)
    if r.map == key and key in WORLD and WORLD[key]["links"]:
        r.map = rng.choice(WORLD[key]["links"])
        r.save(ctx.db)
    return b""


def cmd_board(ctx, args, data):
    """ROVER BOARD: the 10 best rovers (name|badges|wins|losses|lead species|
    lead level), then the latest news, one line each after a '-' row."""
    rovers = Sim(ctx.db, ctx.league, ctx.server).run()
    rovers.sort(key=lambda r: (-r.badges, -(r.wins - r.losses), -r.lead_level(), r.id))
    rows = []
    for r in rovers[:10]:
        lead = max(r.team, key=lambda m: m["level"])
        rows.append((r.name, r.badges, r.wins, r.losses, lead["species"], lead["level"]))
    rows.append(("-",))
    for (line,) in ctx.db.execute("SELECT line FROM rover_news ORDER BY id DESC LIMIT 6"):
        rows.append((line.encode("ascii", "replace").decode(),))
    return text(rows)


def cmd_ask(ctx, args, data):
    """ROVER INFO <id>: one rover, for the talk script (the Rover wire)."""
    return load(ctx.db, int(args[1])).wire()


def cmd_majors(ctx, args, data):
    """MAJOR LIST <group> <num>: the quest board in a town. First row: money
    the player has earned and not collected (now paid out). Then one row per
    open job within four maps: id|species|level|map name|maps away; then
    up to three cleared ones: 0|species|level|map name|cleared by."""
    key = map_key(args, 1)
    Sim(ctx.db, ctx.league, ctx.server).run()
    due = ctx.db.execute("SELECT COALESCE(SUM(level), 0) FROM majors WHERE cleared_by = ? AND cleared_tid = ?"
                         " AND paid = 0", ctx.key).fetchone()[0]
    reward = (due * MAJOR_REWARD) // 10
    ctx.db.execute("UPDATE majors SET paid = 1 WHERE cleared_by = ? AND cleared_tid = ?", ctx.key)
    rows = [(reward,)]
    jobs = []
    for mid, mk, sid, level in ctx.db.execute(
            "SELECT id, map, species, level FROM majors WHERE cleared_by IS NULL ORDER BY id"):
        d = hops(key, mk, 4)
        if d is not None:
            jobs.append((d, mid, sid, level, WORLD[mk]["name"]))
    jobs.sort()
    for d, mid, sid, level, name in jobs[:5]:
        rows.append((mid, sid, level, name, d))
    for sid, level, mk, who in ctx.db.execute(
            "SELECT species, level, map, cleared_by FROM majors WHERE cleared_by IS NOT NULL"
            " ORDER BY cleared_at DESC LIMIT 3"):
        rows.append((0, sid, level, WORLD.get(mk, {}).get("name", "?"), who))
    return text(rows)


def cmd_major_clear(ctx, args, data):
    """MAJOR CLEAR <id> <1 if caught>: the player beat or caught it."""
    mid = int(args[1])
    row = ctx.db.execute("SELECT species, map, cleared_by FROM majors WHERE id = ?", (mid,)).fetchone()
    if row is None:
        raise ValueError("no such job")
    if row[2] is not None:
        raise Refused(f"{row[2]} got to it first.")
    ctx.db.execute("UPDATE majors SET cleared_by = ?, cleared_tid = ?, cleared_at = ? WHERE id = ?",
                   (*ctx.key, now(), mid))
    verb = "caught" if int(args[2]) else "beat"
    news(ctx.db, f"{ctx.key[0]} {verb} the rampaging {species_name(row[0])}!")
    return b""


ROVER = {"HERE": cmd_here, "SYNC": cmd_sync, "FIGHT": cmd_fight, "BEAT": cmd_beat, "LEAVE": cmd_leave,
         "BOARD": cmd_board, "INFO": cmd_ask}
MAJOR = {"LIST": cmd_majors, "CLEAR": cmd_major_clear}


def cmd_rover(ctx, args, data):
    fn = ROVER.get(args[0])
    if fn is None:
        raise ValueError("bad ROVER request")
    return fn(ctx, args, data)


def cmd_major(ctx, args, data):
    fn = MAJOR.get(args[0])
    if fn is None:
        raise ValueError("bad MAJOR request")
    return fn(ctx, args, data)


COMMANDS = {"ROVER": cmd_rover, "MAJOR": cmd_major}
