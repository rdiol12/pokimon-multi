"""Hall of Fame wall: every team that enters the Hall of Fame goes up on a
wall on the server, for everyone to see in game and on the web page.

The game sends a team when the player enters the Hall of Fame (or later,
once online), and asks for the wall when it is opened:

    HOF ADD     data: 6 x (species u16 LE, level u8, shiny u8), empty slots 0
                -> entry number|total entries
    HOF LIST    -> newest entries first, one per row:
                   number|trainer|date|species.level.shiny|... (up to 6)
                   the first row is total|mine (entries on the wall, and
                   how many of them are this player's)
"""

import os
import re
import struct

from ext import Refused, text

TEAM_SIZE = 6
MON_FORMAT = "<HBB"
MON_SIZE = struct.calcsize(MON_FORMAT)
MAX_SPECIES = 2047
LIST_ROWS = 16
SPECIES_H = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "include", "constants", "species.h")
SPECIES_RE = re.compile(r"^\s*SPECIES_(\w+)\s*=\s*(\d+)\s*,", re.M)


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS hall_of_fame (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            team TEXT NOT NULL, -- species.level.shiny, comma separated
            entered_at TEXT NOT NULL DEFAULT (date('now'))
        );
        """
    )


def parse_team(data):
    if len(data) != TEAM_SIZE * MON_SIZE:
        raise ValueError("bad team")
    team = []
    for i in range(TEAM_SIZE):
        species, level, shiny = struct.unpack_from(MON_FORMAT, data, i * MON_SIZE)
        if species == 0:
            continue
        if species > MAX_SPECIES or not 1 <= level <= 100:
            raise ValueError("bad mon")
        team.append(f"{species}.{level}.{1 if shiny else 0}")
    if not team:
        raise ValueError("empty team")
    return ",".join(team)


def add(db, key, team):
    """Puts a team on the wall. The same player entering again with the
    same team (a reload, a second walk through the League) adds nothing."""
    last = db.execute(
        "SELECT id, team FROM hall_of_fame WHERE name = ? AND tid = ? ORDER BY id DESC LIMIT 1", key
    ).fetchone()
    if last and last[1] == team:
        return last[0]
    return db.execute("INSERT INTO hall_of_fame (name, tid, team) VALUES (?, ?, ?)", (*key, team)).lastrowid


def entries(db, limit=None):
    sql = "SELECT id, name, entered_at, team FROM hall_of_fame ORDER BY id DESC"
    if limit:
        sql += f" LIMIT {int(limit)}"
    return db.execute(sql).fetchall()


def cmd_hof(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    total = db.execute("SELECT COUNT(*) FROM hall_of_fame").fetchone()[0]
    if sub == "ADD":
        number = add(db, key, parse_team(data))
        total = db.execute("SELECT COUNT(*) FROM hall_of_fame").fetchone()[0]
        return text([(number, total)])
    if sub == "LIST":
        mine = db.execute("SELECT COUNT(*) FROM hall_of_fame WHERE name = ? AND tid = ?", key).fetchone()[0]
        rows = [(total, mine)]
        for number, name, date, team in entries(db, LIST_ROWS):
            rows.append((number, name, date, *team.split(",")))
        return text(rows)
    raise Refused("The wall doesn't know that.")


COMMANDS = {"HOF": cmd_hof}


_names = None


def species_names():
    """Species names from the game's own list, for the web page."""
    global _names
    if _names is None:
        _names = {}
        if os.path.exists(SPECIES_H):
            with open(SPECIES_H, encoding="utf-8") as f:
                for name, num in SPECIES_RE.findall(f.read()):
                    _names.setdefault(int(num), name.replace("_", " ").title())
    return _names
