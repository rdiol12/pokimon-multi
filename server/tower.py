"""Online Battle Tower: climb floors against other players' saved teams.

A run starts with the player's current team, which is also saved as the team
others meet. Each floor's opponent is another player's saved team, picked
the same way for the same player and floor. A loss ends the run; the board
ranks players by the highest floor they cleared."""

import zlib

from ext import Refused, text

MAX_TEAM = 1024


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS tower_teams (
            name TEXT NOT NULL, tid INTEGER NOT NULL, team BLOB NOT NULL,
            saved_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS tower_runs (
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            floor INTEGER NOT NULL DEFAULT 0,  -- 0: no run going
            best INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (name, tid)
        );
        """
    )


def best(db, key):
    row = db.execute("SELECT best FROM tower_runs WHERE name = ? AND tid = ?", key).fetchone()
    return row[0] if row else 0


def state(db, key):
    db.execute("INSERT OR IGNORE INTO tower_runs (name, tid) VALUES (?, ?)", key)
    return db.execute("SELECT floor, best FROM tower_runs WHERE name = ? AND tid = ?", key).fetchone()


def opponent(db, key, floor):
    teams = db.execute(
        "SELECT name, tid, team FROM tower_teams WHERE NOT (name = ? AND tid = ?) ORDER BY name, tid", key
    ).fetchall()
    if not teams:
        return None
    seed = zlib.crc32(f"{key[0]}:{key[1]}".encode())
    return teams[(seed + floor - 1) % len(teams)]


def cmd_tower(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    if sub == "START":
        if not 0 < len(data) <= MAX_TEAM:
            raise ValueError("bad team")
        db.execute("INSERT OR REPLACE INTO tower_teams (name, tid, team) VALUES (?, ?, ?)", (*key, data))
        state(db, key)
        db.execute("UPDATE tower_runs SET floor = 1 WHERE name = ? AND tid = ?", key)
        return text([state(db, key)])
    if sub == "NEXT":
        # floor|opponent name, then a NUL and their team
        floor, _ = state(db, key)
        if floor == 0:
            raise Refused("Start a new run first.")
        opp = opponent(db, key, floor)
        if opp is None:
            raise Refused("No other teams yet. Come back when more players have entered.")
        return text([(floor, opp[0])]) + b"\0" + opp[2]
    if sub in ("WON", "LOST"):
        floor, top = state(db, key)
        if floor == 0:
            raise Refused("No run going.")
        if sub == "WON":
            db.execute(
                "UPDATE tower_runs SET floor = floor + 1, best = MAX(best, ?) WHERE name = ? AND tid = ?",
                (floor, *key),
            )
        else:
            db.execute("UPDATE tower_runs SET floor = 0 WHERE name = ? AND tid = ?", key)
        return text([state(db, key)])
    if sub == "STATE":
        return text([state(db, key)])
    if sub == "BOARD":
        # rank|name|best floor cleared, then me
        rows = db.execute(
            "SELECT name, tid, best FROM tower_runs WHERE best > 0 ORDER BY best DESC, name"
        ).fetchall()
        out = [(i, r[0], r[2]) for i, r in enumerate(rows[:10], 1)]
        me = next(((i, r[2]) for i, r in enumerate(rows, 1) if (r[0], r[1]) == key), (0, 0))
        out.append((me[0], key[0], me[1]))
        return text(out)
    raise ValueError("bad TOWER request")


COMMANDS = {"TOWER": cmd_tower}
