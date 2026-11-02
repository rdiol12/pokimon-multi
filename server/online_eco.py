"""Living ecosystem: the wild Pokémon of every route are shared by everyone
on the server, and they react to what players (and the living NPC trainers)
catch.

- Thinning: every catch on a map lowers that species' population there
  (100 is a full population). Thinned species turn up less in the grass
  (the game passes over their slot, see src/online_eco.c), and the other
  species there take their place. Populations grow back over about a day.
- Fleeing: a species thinned below FLEE_AT on a map flees: for FLEE_HOURS
  some of it turns up on a linked map, even if it never lives there.
- Migrations: every day the server moves MIGRATIONS_PER_DAY herds from
  their home map to a linked one for the day. The home map is thinned of
  them, the other one has them in MIGRANT_RATE percent of its encounters.
  Players get a notice the first time they come to a map with newcomers.

The game learns about the map the player is on from OSTAT (status key
"ECO", see `status`), and HUB > WILDLIFE REPORT shows the area's
populations and today's migrations.
"""

import gzip
import json
import os
import random

from online import answer, board

HERE_DIR = os.path.dirname(os.path.abspath(__file__))
with gzip.open(os.path.join(HERE_DIR, "data", "rovers_world.json.gz"), "rt") as f:
    _WORLD = json.load(f)
WORLD = _WORLD["maps"]               # "group.num" -> map (title, kind, links, wild [[species, lo, hi]])
NAMES = {int(k): v for k, v in _WORLD["names"].items()}

FULL = 100
CATCH_COST = 8          # one player's catch
ROVER_COST = 3          # one living NPC trainer's catch
RECOVER_PER_HOUR = 4.0  # from 0 back to full in a day
SCARCE = 70             # below this, the report calls it thinned (the game passes over any below 100)
FLEE_AT = 40
FLEE_HOURS = 12
FLEE_RATE = int(os.environ.get("NETPLAY_ECO_FLEE_RATE", "15"))  # percent of the encounters where they fled to
MIGRATIONS_PER_DAY = int(os.environ.get("NETPLAY_ECO_MIGRATIONS", "3"))
MIGRANT_RATE = int(os.environ.get("NETPLAY_ECO_MIGRANT_RATE", "25"))
HOME_AFTER_MIGRATION = 45  # what's left at home when a herd leaves
STATUS_SCARCE = 4       # at most this many thinned species go to the game
STATUS_MIGRANTS = 2

HERDS = ["A herd", "A flock", "A swarm", "A big group", "A family"]


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS eco_pop (
            map TEXT NOT NULL, species INTEGER NOT NULL, pop REAL NOT NULL, at REAL NOT NULL,
            PRIMARY KEY (map, species)
        );
        CREATE TABLE IF NOT EXISTS eco_moves (
            id INTEGER PRIMARY KEY, kind TEXT NOT NULL, species INTEGER NOT NULL,
            src TEXT NOT NULL, dst TEXT NOT NULL, rate INTEGER NOT NULL,
            start REAL NOT NULL, until REAL NOT NULL
        );
        CREATE TABLE IF NOT EXISTS eco_days (day INTEGER PRIMARY KEY);
        CREATE TABLE IF NOT EXISTS eco_catches (
            day INTEGER NOT NULL, map TEXT NOT NULL, count INTEGER NOT NULL,
            PRIMARY KEY (day, map)
        );
        """
    )


def species_name(species):
    return NAMES.get(species, f"No.{species}").upper()


def title(key):
    m = WORLD.get(key)
    return m["title"].upper() if m else "SOMEWHERE"


def map_key(group, num):
    return f"{group}.{num}"


def home_species(key):
    m = WORLD.get(key)
    return [w[0] for w in m["wild"]] if m else []


def linked_wild(key, depth=3):
    """The nearest maps (through towns, up to `depth` steps) with wild
    Pokémon of the same kind (grass or cave)."""
    m = WORLD.get(key)
    if not m:
        return []
    seen, frontier = {key}, [key]
    for _ in range(depth):
        nxt = []
        for k in frontier:
            for link in WORLD[k]["links"] if k in WORLD else []:
                if link not in seen and link in WORLD:
                    seen.add(link)
                    nxt.append(link)
        found = sorted(k for k in nxt if WORLD[k]["wild"] and WORLD[k]["kind"] == m["kind"])
        if found:
            return found
        frontier = nxt
    return []


# --- populations ---------------------------------------------------------

def population(db, key, species, now):
    row = db.execute("SELECT pop, at FROM eco_pop WHERE map = ? AND species = ?", (key, species)).fetchone()
    if row is None:
        return float(FULL)
    pop, at = row
    return min(float(FULL), pop + max(0.0, now - at) / 3600.0 * RECOVER_PER_HOUR)


def set_population(db, key, species, pop, now):
    pop = max(0.0, min(float(FULL), pop))
    if pop >= FULL:
        db.execute("DELETE FROM eco_pop WHERE map = ? AND species = ?", (key, species))
        return
    db.execute("INSERT INTO eco_pop (map, species, pop, at) VALUES (?, ?, ?, ?)"
               " ON CONFLICT (map, species) DO UPDATE SET pop = excluded.pop, at = excluded.at",
               (key, species, pop, now))


def thinned(db, key, now):
    """[(species, population)] of this map's species below full, lowest first."""
    rows = db.execute("SELECT species FROM eco_pop WHERE map = ?", (key,)).fetchall()
    out = [(sp, population(db, key, sp, now)) for (sp,) in rows]
    return sorted([(sp, p) for sp, p in out if p < FULL - 0.5], key=lambda x: x[1])


def active_moves(db, now, dst=None, src=None):
    q = "SELECT kind, species, src, dst, rate, until FROM eco_moves WHERE start <= ? AND until > ?"
    args = [now, now]
    if dst is not None:
        q += " AND dst = ?"
        args.append(dst)
    if src is not None:
        q += " AND src = ?"
        args.append(src)
    return db.execute(q + " ORDER BY rate DESC, id", args).fetchall()


# --- migrations ----------------------------------------------------------

def plan_migrations(day, count=None):
    """Today's migrations: [(species, src, dst)], the same for any server."""
    count = MIGRATIONS_PER_DAY if count is None else count
    rng = random.Random(day * 7919 + 17)
    sources = sorted(k for k in WORLD if WORLD[k]["wild"] and WORLD[k]["kind"] in ("route", "cave") and linked_wild(k))
    out, used = [], set()
    for _ in range(count * 6):
        if len(out) >= count or not sources:
            break
        src = rng.choice(sources)
        dst = rng.choice(linked_wild(src))
        candidates = [sp for sp in home_species(src) if sp not in home_species(dst)]
        if not candidates or src in used or dst in used:
            continue
        out.append((rng.choice(candidates), src, dst))
        used.update((src, dst))
    return out


def ensure_day(ctx):
    """Starts today's migrations the first time anyone asks today."""
    db, online = ctx.db, ctx.online
    day, now = online.day(), online.now()
    if db.execute("SELECT 1 FROM eco_days WHERE day = ?", (day,)).fetchone():
        return
    db.execute("INSERT INTO eco_days (day) VALUES (?)", (day,))
    end = (day + 1) * 86400
    for species, src, dst in plan_migrations(day):
        db.execute("INSERT INTO eco_moves (kind, species, src, dst, rate, start, until) VALUES (?, ?, ?, ?, ?, ?, ?)",
                   ("migrate", species, src, dst, MIGRANT_RATE, now, end))
        set_population(db, src, species, min(population(db, src, species, now), HOME_AFTER_MIGRATION), now)


def thin(ctx, key, species, cost, who):
    """A catch on map `key`: lowers the population, and maybe they flee."""
    db, now = ctx.db, ctx.online.now()
    before = population(db, key, species, now)
    after = before - cost
    set_population(db, key, species, after, now)
    db.execute("INSERT INTO eco_catches (day, map, count) VALUES (?, ?, 1)"
               " ON CONFLICT (day, map) DO UPDATE SET count = count + 1", (ctx.online.day(), key))
    if not (after < FLEE_AT <= before) or species not in home_species(key):
        return
    if any(m[0] == "flee" and m[1] == species for m in active_moves(db, now, src=key)):
        return
    links = linked_wild(key)
    if not links:
        return
    dst = links[(species + int(now // 3600)) % len(links)]
    db.execute("INSERT INTO eco_moves (kind, species, src, dst, rate, start, until) VALUES (?, ?, ?, ?, ?, ?, ?)",
               ("flee", species, key, dst, FLEE_RATE, now, now + FLEE_HOURS * 3600))
    if hasattr(ctx, "player"):  # rovers' catches thin them quietly
        ctx.online.notify(ctx.key, f"So many {species_name(species)} were caught on {title(key)} "
                                   f"that the rest are fleeing to {title(dst)}!")


def on_catch(ctx, args):
    """CATCH species shiny mapGroup mapNum fished"""
    species, _, group, num, _ = (args + [0] * 5)[:5]
    if species <= 0:
        return
    ensure_day(ctx)
    thin(ctx, map_key(group, num), species, CATCH_COST, ctx.player.name)


class _RoverCtx:
    def __init__(self, db, online):
        self.db, self.online = db, online


def rover_catch(online, db, key, species, name):
    """Called by server/rovers.py when a living NPC trainer catches one."""
    try:
        thin(_RoverCtx(db, online), key, species, ROVER_COST, name)
    except Exception:  # never stop the trainers over this
        pass


def rover_weights(online, db, key, wild):
    """How likely each of `wild` ([species, lo, hi]) is for a rover to meet:
    thinned ones less, plus any migrants here (added to the list)."""
    now = online.now()
    out = [(w, population(db, key, w[0], now) / FULL) for w in wild]
    lo, hi = (min(w[1] for w in wild), max(w[2] for w in wild)) if wild else (2, 5)
    for kind, species, src, dst, rate, until in active_moves(db, now, dst=key):
        out.append(([species, lo, hi], rate / 100.0 * len(wild)))
    return out


# --- what the game hears --------------------------------------------------

def status(ctx):
    """S|ECO|group.num|sp:pop,sp:pop|sp:pop,sp:pop|sp:rate|sp:rate for the
    map the player is on; nothing if nothing there has changed."""
    ensure_day(ctx)
    player = ctx.player
    if getattr(player, "map", None) is None:
        return []
    key = map_key(*player.map)
    now = ctx.online.now()
    scarce = thinned(ctx.db, key, now)[:STATUS_SCARCE]
    seen, migrants = set(), []
    told = player.__dict__.setdefault("eco_told", set())
    for move_id, kind, species, src, rate in ctx.db.execute(
            "SELECT id, kind, species, src, rate FROM eco_moves WHERE dst = ? AND start <= ? AND until > ?"
            " ORDER BY rate DESC, id", (key, now, now)).fetchall():
        if species not in seen:
            seen.add(species)
            migrants.append((species, rate))
        if move_id not in told:  # the first time the player is here while they are
            told.add(move_id)
            if kind == "migrate":
                herd = HERDS[(species + move_id) % len(HERDS)]
                ctx.online.notify(ctx.key, f"{herd} of {species_name(species)} has migrated here from "
                                           f"{title(src)} for the day!")
            else:
                ctx.online.notify(ctx.key, f"{species_name(species)} that fled from {title(src)} "
                                           f"are hiding around here!")
    migrants = migrants[:STATUS_MIGRANTS]
    if not scarce and not migrants:
        return []
    pairs = [f"{sp}:{int(p)}" for sp, p in scarce]
    fields = [",".join(pairs[:2]), ",".join(pairs[2:4])]
    fields += [f"{sp}:{rate}" for sp, rate in migrants]
    fields += [""] * (4 - len(fields))
    return [["ECO", key, *fields]]


def cmd_eco(ctx, args, data):
    db, now = ctx.db, ctx.online.now()
    ensure_day(ctx)
    if args[0] == "AREA":
        player = ctx.player
        if getattr(player, "map", None) is None or map_key(*player.map) not in WORLD \
                or not WORLD[map_key(*player.map)]["wild"]:
            return answer("There are no wild Pokémon around here to report on.", code=1)
        key = map_key(*player.map)
        rows, listed = [], set()
        for species in home_species(key):
            if species in listed:
                continue
            listed.add(species)
            p = int(population(db, key, species, now))
            note = "fled" if p < FLEE_AT else ("thinned" if p < SCARCE else "")
            for kind, sp, src, dst, rate, until in active_moves(db, now, src=key):
                if sp == species and kind == "migrate":
                    note = "away"
            rows.append([species_name(species), f"{p}%", note])
        for kind, species, src, dst, rate, until in active_moves(db, now, dst=key):
            if species in listed:
                continue
            listed.add(species)
            hours = max(1, int((until - now) // 3600))
            note = "migrant" if kind == "migrate" else "fled here"
            rows.append([f"*{species_name(species)}", "new", f"{note}, {hours}h left"])
        caught = db.execute("SELECT count FROM eco_catches WHERE day = ? AND map = ?",
                            (ctx.online.day(), key)).fetchone()
        caught = caught[0] if caught else 0
        lines = board(f"WILDLIFE: {title(key)}",
                      [(6, "POK#MON"), (84, "LEFT"), (120, f"CAUGHT TODAY: {caught}")], rows[:8])
        return answer(rows=lines)
    if args[0] == "MOVES":
        rows = []
        for kind, species, src, dst, rate, until in active_moves(db, now):
            hours = max(1, int((until - now) // 3600))
            rows.append([species_name(species), title(src)[:10], title(dst)[:10], f"{hours}h"])
        lines = board("MIGRATIONS NOW",
                      [(6, "POK#MON"), (74, "FROM"), (136, "TO"), (190, "")], rows[:8])
        return answer(rows=lines)
    raise ValueError("bad ECO")


COMMANDS = {"ECO": cmd_eco}
EVENTS = {"CATCH": on_catch}
