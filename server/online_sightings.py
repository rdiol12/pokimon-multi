"""Legend sightings: when a player meets a roaming Pokémon or a legend, the
server keeps where and when, tells everyone online, and HUB > LEGEND
SIGHTINGS shows where each one was last seen."""

from online import answer, board
from online_ids import MAP_NAMES, MAP_SECTIONS, SECTION_GRID, SPECIES_NAMES

ANNOUNCE_AGAIN = 10 * 60  # seconds before the same player's sighting is announced again
BOARD_SIZE = 8


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_sightings (
            id INTEGER PRIMARY KEY,
            species INTEGER NOT NULL,
            map_group INTEGER NOT NULL,
            map_num INTEGER NOT NULL,
            roaming INTEGER NOT NULL,
            name TEXT NOT NULL,
            tid INTEGER NOT NULL,
            at REAL NOT NULL
        );
        """
    )


def species_name(species):
    return SPECIES_NAMES.get(species, f"No.{species}").replace("_", " ")


def place_name(group, num):
    """The map's name as the game's region map shows it."""
    map_name = MAP_NAMES.get((group, num))
    if map_name is None:
        return "somewhere"
    section = MAP_SECTIONS.get(map_name)
    if section in SECTION_GRID and SECTION_GRID[section][4]:
        return SECTION_GRID[section][4]
    return map_name.replace("_", " ")


def ago(seconds):
    """Short, to fit the board: now, 12m, 3h, 2d."""
    minutes = int(seconds // 60)
    if minutes < 1:
        return "now"
    if minutes < 60:
        return f"{minutes}m"
    if minutes < 48 * 60:
        return f"{minutes // 60}h"
    return f"{minutes // 1440}d"


def latest(db, limit=BOARD_SIZE):
    """The last sighting of each species, newest first."""
    return db.execute(
        "SELECT species, map_group, map_num, name, MAX(at) FROM online_sightings"
        " GROUP BY species ORDER BY MAX(at) DESC LIMIT ?", (limit,)).fetchall()


def on_sight(ctx, args):
    """SIGHT species mapGroup mapNum roaming"""
    species, group, num, roaming = (args + [0] * 4)[:4]
    if species <= 0:
        return
    db, now = ctx.db, ctx.online.now()
    last = db.execute("SELECT MAX(at) FROM online_sightings WHERE species = ? AND name = ? AND tid = ?",
                      (species, *ctx.key)).fetchone()[0]
    db.execute("INSERT INTO online_sightings (species, map_group, map_num, roaming, name, tid, at)"
               " VALUES (?, ?, ?, ?, ?, ?, ?)", (species, group, num, 1 if roaming else 0, *ctx.key, now))
    if last is None or now - last >= ANNOUNCE_AGAIN:
        ctx.online.announce(f"{ctx.player.name} spotted {species_name(species)} at {place_name(group, num)}!",
                            exclude=ctx.key)


def cmd_sight(ctx, args, data):
    if args[0] != "BOARD":
        raise ValueError("bad SIGHT")
    now = ctx.online.now()
    rows = [[species_name(sp), place_name(g, n), name, ago(now - at)]
            for sp, g, n, name, at in latest(ctx.db)]
    return answer(rows=board("LEGEND SIGHTINGS", [(6, "POK#MON"), (64, "LAST SEEN"), (148, "BY"), (186, "WHEN")], rows))


COMMANDS = {"SIGHT": cmd_sight}
EVENTS = {"SIGHT": on_sight}
