"""The Stormwild: an extraction run (docs/stormwild.md).

The Storm Warden by Mauville's Pokémon Center sends players into the
Stormwild (src/stormwild.c). Up to MAX_PLAYERS share a run. Each one brings
at most three Pokémon and a pouch of up to ten kinds of items; the rest of
their bag waits at the gate. The run lasts RUN seconds:

- supply caches: a different CACHES_PER_RUN of the map's caches each run.
  The first player to open one gets its loot; it's gone for everyone else.
- extraction beacons: one of the three lights up at a time, for BEACON_OPEN
  seconds (the game has the player stand by it for 10 seconds). A player who
  gets out keeps everything in their bag and every Pokémon they caught.
- raids: two runners who battle each other (the plaza's battle) settle it
  here: the loser's game gives up its most valuable stack of items and up to
  two Pokémon caught in the run (XRUN ROB). The winner only gets them if
  they get out: the server holds them until XRUN OUT.
- when the clock runs out, or a runner's team all faints, or they leave any
  other way, the run fails: everything in the bag (what they brought and
  what they found) and every Pokémon caught in the run is lost (the game
  does that; XRUN FAIL just tells the server).

The server's clock runs the run: the game's status row says how long is
left, which beacon is lit, which caches are gone and if a raid was lost.
"""

import math
import os
import random

from online import Refused, answer, board
from online_ids import SPECIES_NAMES

MAX_PLAYERS = 12
LOBBY = float(os.environ.get("NETPLAY_XRUN_LOBBY", 30))
RUN = float(os.environ.get("NETPLAY_XRUN_LENGTH", 15 * 60))
BEACON_OPEN = 60
BEACON_GRACE = 5  # the game's clock may run a little behind
MAX_TEAM = 3
NUM_DROPS = 8
# As sBeacons and sCaches in src/stormwild.c (tools/areas/stormwild.py).
BEACONS = [(11, 10), (54, 28), (30, 53)]
CACHES = [
    (17, 9), (6, 14), (44, 7), (53, 12), (58, 20), (49, 24), (57, 38), (51, 46),
    (44, 52), (36, 58), (24, 56), (12, 51), (17, 44), (4, 40), (6, 24), (18, 24),
    (24, 29), (38, 25), (41, 37), (28, 42), (32, 31), (36, 16), (26, 13), (60, 52),
]
CACHES_PER_RUN = int(os.environ.get("NETPLAY_XRUN_CACHES", 14))
SEED = os.environ.get("NETPLAY_XRUN_SEED")  # fixed runs, for screenshots
REACH = 2  # positions lag a little
BOARD_SIZE = 8
OVER_KEEP = 600  # a finished run's spoils wait this long for stragglers

# Loot: (item constant name, how many, weight). The middle of the lake and
# the far corners have the rare table.
LOOT = [
    ("ITEM_HYPER_POTION", 2, 10), ("ITEM_ULTRA_BALL", 3, 10), ("ITEM_REVIVE", 2, 8),
    ("ITEM_FULL_HEAL", 3, 6), ("ITEM_MAX_REPEL", 2, 4), ("ITEM_ELIXIR", 1, 5),
    ("ITEM_STAR_PIECE", 1, 5), ("ITEM_NUGGET", 1, 5), ("ITEM_DUSK_BALL", 3, 6),
    ("ITEM_TIMER_BALL", 3, 5), ("ITEM_FIRE_STONE", 1, 3), ("ITEM_THUNDER_STONE", 1, 3),
    ("ITEM_WATER_STONE", 1, 3), ("ITEM_LEAF_STONE", 1, 3), ("ITEM_PP_UP", 1, 3),
]
RARE_LOOT = [
    ("ITEM_RARE_CANDY", 2, 10), ("ITEM_MAX_REVIVE", 2, 8), ("ITEM_FULL_RESTORE", 2, 8),
    ("ITEM_BIG_NUGGET", 1, 6), ("ITEM_PP_MAX", 1, 4), ("ITEM_ABILITY_CAPSULE", 1, 4),
    ("ITEM_DRAGON_SCALE", 1, 3), ("ITEM_RAZOR_CLAW", 1, 3), ("ITEM_MASTER_BALL", 1, 1),
]
RARE_CACHES = {9, 20, 23, 13}  # far south, the lake island, the far south-east, the west edge

# Phases.
LOBBY_PHASE, LIVE, OVER = "lobby", "live", "over"


def item_ids():
    from online_ids import ITEMS
    return ITEMS


class Run:
    count = 0

    def __init__(self, start, rng):
        Run.count += 1
        self.id = Run.count & 0x7FFF
        self.start = start  # when the run begins (the lobby ends)
        self.players = []  # keys, in drop order
        self.names = {}
        self.out = {}  # key -> "out" or "failed"
        self.taken = {}  # cache -> key
        self.caches = sorted(rng.sample(range(len(CACHES)), CACHES_PER_RUN))
        self.windows = beacon_windows(rng)
        self.raids = {}  # loser key -> [winner keys] still to settle
        self.spoils = {}  # winner key -> [("+ITEM", id, n) or ("+MON", species, level, shiny)]
        self.loot = {}  # key -> value found
        self.rng = rng

    def phase(self, now):
        if now < self.start:
            return LOBBY_PHASE
        if now < self.start + RUN and len(self.out) < len(self.players):
            return LIVE
        return OVER

    def left(self, now):
        return max(0, math.ceil(self.start + RUN - now))

    def beacon(self, now):
        """The beacon lit now and the seconds it stays lit, or (-1, 0)."""
        t = now - self.start
        for start, beacon in self.windows:
            if start <= t < start + BEACON_OPEN:
                return beacon, math.ceil(start + BEACON_OPEN - t)
        return -1, 0

    def lit_recently(self, beacon, now):
        t = now - self.start
        return any(b == beacon and start - BEACON_GRACE <= t < start + BEACON_OPEN + BEACON_GRACE
                   for start, b in self.windows)

    def active(self):
        return sum(1 << c for c in self.caches)

    def taken_mask(self):
        return sum(1 << c for c in self.taken)


def beacon_windows(rng):
    """When each beacon lights (seconds into the run): up to four times spread
    over the run, never the same beacon twice in a row, never two at once,
    the last one just before the end."""
    windows, last = [], -1
    slots = [RUN * f for f in (0.2, 0.45, 0.7)] + [RUN - BEACON_OPEN - 15]
    for i, at in enumerate(slots):
        jitter = 0 if i == len(slots) - 1 else rng.uniform(-RUN * 0.05, RUN * 0.05)
        beacon = rng.choice([b for b in range(len(BEACONS)) if b != last])
        at = max(0.0, at + jitter)
        if windows and at < windows[-1][0] + BEACON_OPEN + 5:
            continue
        windows.append((at, beacon))
        last = beacon
    return windows


def setup(db):
    db.execute(
        """CREATE TABLE IF NOT EXISTS online_xrun_hauls (
            week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            best INTEGER NOT NULL, runs INTEGER NOT NULL, escapes INTEGER NOT NULL,
            PRIMARY KEY (week, name, tid)
        )"""
    )


def runs(server):
    if not hasattr(server, "xruns"):
        server.xruns = []
    return server.xruns


def run_of(server, key):
    for run in runs(server):
        if tuple(key) in run.players and tuple(key) not in run.out:
            return run
    return None


def install(server):
    """Raids: a battle between two runners of the same run."""
    finish = server.finish_battle

    def finish_battle(session):
        finish(session)
        try:
            settle_raid(server, session)
        except Exception:  # never break the plaza's battles
            import logging
            logging.exception("stormwild raid")

    server.finish_battle = finish_battle


def settle_raid(server, session):
    if session.kind != "battle" or not hasattr(server, "online"):
        return
    h, g = session.host, session.guest
    run = run_of(server, h.key)
    if run is None or run is not run_of(server, g.key) or run.phase(server.online.now()) != LIVE:
        return
    rh, rg = session.reports.get(h.key), session.reports.get(g.key)
    if (rh, rg) in (("win", "loss"), ("win", None), (None, "loss")):
        winner, loser = h, g
    elif (rh, rg) in (("loss", "win"), ("loss", None), (None, "win")):
        winner, loser = g, h
    else:
        return
    run.raids.setdefault(tuple(loser.key), []).append(tuple(winner.key))
    server.online.notify(winner.key, f"You beat {loser.name}! Their best loot is yours if you get out.")


def num(args, i, default=0):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        return default


def near(player, spot):
    return max(abs(player.pos[0] - spot[0]), abs(player.pos[1] - spot[1])) <= REACH


def in_zone(ctx):
    from online_ids import MAPS
    return tuple(ctx.player.map) == tuple(MAPS["STORMWILD"])


def record(db, week, key, haul, escaped):
    db.execute("INSERT INTO online_xrun_hauls VALUES (?, ?, ?, ?, 1, ?) ON CONFLICT (week, name, tid) DO UPDATE"
               " SET best = MAX(best, excluded.best), runs = runs + 1, escapes = escapes + excluded.escapes",
               (week, *key, haul if escaped else 0, 1 if escaped else 0))


def pick_loot(rng, rare):
    table = RARE_LOOT if rare else LOOT
    name, count, _ = rng.choices(table, weights=[w for *_, w in table])[0]
    return name, count


def item_name(name):
    return name[5:].replace("_", " ")


def cmd_xrun(ctx, args, data):
    db, key = ctx.db, tuple(ctx.key)
    now = ctx.online.now()
    word = args[0] if args else ""
    server = ctx.server
    # Runs everyone has left, and long-over ones, are gone.
    for run in [r for r in runs(server) if r.phase(now) == OVER
                and (set(r.players) <= set(r.out) or now > r.start + RUN + OVER_KEEP)]:
        runs(server).remove(run)

    if word == "JOIN":
        # team size
        team = num(args, 1)
        if run_of(server, key):
            raise Refused("You're on a run already!")
        if team < 1:
            raise Refused("You'll need at least one Pokémon to go in!")
        if team > MAX_TEAM:
            raise Refused(f"Only {MAX_TEAM} Pokémon may go in. Leave the rest in a PC first.")
        run = next((r for r in runs(server) if r.phase(now) == LOBBY_PHASE and now < r.start - 3
                    and len(r.players) < MAX_PLAYERS), None)
        if run is None:
            run = Run(now + LOBBY, random.Random(int(SEED) if SEED else None))
            runs(server).append(run)
            ctx.online.announce(f"{ctx.player.name} is heading into the STORMWILD! Join them at the Storm"
                                " Warden in MAUVILLE.", exclude=key)
            run.note = ctx.online._note_id
        else:
            # They're coming: they needn't hear the run's announcement in there.
            ctx.player.online_seen = max(ctx.player.online_seen, getattr(run, "note", 0))
            for k in run.players:
                ctx.online.notify(k, f"{ctx.player.name} is coming on the run!")
        run.players.append(key)
        run.names[key] = ctx.player.name
        drop = (len(run.players) - 1) % NUM_DROPS
        seconds = max(1, round(run.start - now))
        others = len(run.players) - 1
        said = (f"The storm gate opens in {seconds} seconds." +
                (f" {others} other trainer{'s are' if others > 1 else ' is'} going in." if others else ""))
        return answer(said, code=seconds,
                      rows=[[run.id, drop, int(RUN), run.active(), CACHES_PER_RUN]])

    if word == "BOARD":
        week = ctx.online.week()
        rows = []
        for place, (name, tid, best, nruns, escapes) in enumerate(db.execute(
                "SELECT name, tid, best, runs, escapes FROM online_xrun_hauls WHERE week = ?"
                " ORDER BY best DESC, escapes DESC, name", (week,)), 1):
            rows.append([f"{'*' if (name, tid) == key else ''}{place}", name, best, f"{escapes}/{nruns}"])
        mine = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
        return answer("The best haul anyone got out with this week.",
                      rows=board("STORMWILD: BEST HAULS", [(6, "No."), (30, "TRAINER"), (110, "HAUL"),
                                                            (170, "OUT")], rows))

    run = run_of(server, key)
    if run is None:
        raise Refused("That run is over.")
    phase = run.phase(now)

    if word == "GO":
        # The lobby is over: VAR_0x800B is how many are on the run.
        n = len(run.players)
        return answer(f"Find what you can and get to a lit beacon before the storm closes in!"
                      f" {run.left(now) // 60} minutes.", code=n)

    if word == "QUIT":
        # Before the run starts only (in the zone, leaving is failing).
        run.out[key] = "failed"
        return answer()

    if word == "LOOT":
        cache = num(args, 1, -1)
        if phase != LIVE:
            raise Refused("The storm's too strong now!")
        if cache not in run.caches:
            raise Refused("It's empty.")
        if cache in run.taken:
            who = run.names.get(run.taken[cache], "Someone")
            raise Refused(f"{who} got here first. It's empty!")
        if not in_zone(ctx) or not near(ctx.player, CACHES[cache]):
            raise Refused("It's too far away.")
        run.taken[cache] = key
        name, count = pick_loot(run.rng, cache in RARE_CACHES)
        item = item_ids().get(name[5:], 0)
        run.loot[key] = run.loot.get(key, 0) + 1
        return answer(f"Found {count} {item_name(name)}!" if count > 1 else f"Found a {item_name(name)}!",
                      code=1 if cache in RARE_CACHES else 0, rows=[["+ITEM", item, count]])

    if word == "ROB":
        # The loser's game after a lost raid: item count species level shiny species level shiny
        winners = run.raids.get(key)
        if not winners:
            raise Refused("Nobody raided you.")
        winner = winners.pop(0)
        if not winners:
            run.raids.pop(key, None)
        item, count = num(args, 1), num(args, 2)
        took = []
        spoils = run.spoils.setdefault(winner, [])
        if item and count:
            spoils.append(("+ITEM", item, count))
            took.append("your best items")
        mons = []
        for i in range(2):
            species, level, shiny = num(args, 3 + i * 3), num(args, 4 + i * 3), num(args, 5 + i * 3)
            if species:
                spoils.append(("+MON", species, level, shiny))
                mons.append(SPECIES_NAMES.get(species, "a Pokémon").replace("_", " "))
        if mons:
            took.append(" and ".join(mons))
        wname = run.names.get(winner, "They")
        if took:
            ctx.online.notify(winner, f"{run.names[key]} gave up {' and '.join(took)}. Get out to keep them!")
            return answer(f"{wname} took {' and '.join(took)}!")
        return answer(f"{wname} won, but you had nothing worth taking!")

    if word == "OUT":
        # beacon haul-value catches
        beacon = num(args, 1, -1)
        if not 0 <= beacon < len(BEACONS) or not run.lit_recently(beacon, now):
            raise Refused("The beacon has gone dark!")
        if phase == LOBBY_PHASE:
            raise Refused("Not yet!")
        if now > run.start + RUN + BEACON_GRACE:
            raise Refused("The storm's closed in!")
        if not in_zone(ctx) or not near(ctx.player, BEACONS[beacon]):
            raise Refused("You need to be at the beacon.")
        if run.raids.get(key):
            raise Refused("Settle the raid first!")
        run.out[key] = "out"
        haul = max(0, num(args, 2))
        rows = []
        won = run.spoils.pop(key, [])
        for spoil in won:
            if spoil[0] == "+ITEM":
                rows.append(["+ITEM", spoil[1], spoil[2]])
                haul += 0  # the game counts items it has
            else:
                rows.append(["+MON", spoil[1], spoil[2], spoil[3]])
        record(db, ctx.online.week(), key, haul, True)
        others = [k for k in run.players if k != key]
        for k in others:
            if k not in run.out:
                ctx.online.notify(k, f"{ctx.player.name} got out of the STORMWILD!")
        said = f"You got out with a haul worth ${haul}!"
        if won:
            said += f" The raid spoils are yours too: {len(won)} prize{'s' if len(won) > 1 else ''}!"
        return answer(said, code=len(won), rows=rows)

    if word == "FAIL":
        run.out.setdefault(key, "failed")
        record(db, ctx.online.week(), key, 0, False)
        lost = run.spoils.pop(key, [])
        return answer("The storm took everything you carried." +
                      (" The raid spoils were lost too." if lost else ""))

    raise ValueError("bad XRUN")


def status(ctx):
    run = run_of(ctx.server, ctx.key)
    if run is None:
        return []
    now = ctx.online.now()
    beacon, secs = run.beacon(now)
    raids = len(run.raids.get(tuple(ctx.key), []))
    return [["XRUN", run.id, run.left(now) if run.phase(now) != LOBBY_PHASE else int(RUN),
             beacon * 100 + secs if beacon >= 0 else -1, run.taken_mask(), raids,
             len(run.players) - len(run.out)]]


COMMANDS = {"XRUN": cmd_xrun}
