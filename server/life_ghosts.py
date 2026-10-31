"""Ghost trainers: each online player's team is kept on the server, and other
players meet it on the routes as a ghost trainer they can battle.

The game uploads its team now and then (PUT), asks for a ghost when it
enters a route (GET), and reports how the battle went (RESULT). Each ghost
keeps a record, and its owner hears how it did the next time they upload.
"""

import random
import time

from ext import Refused, text
from wondertrade import MAX_SPECIES

MON_SIZE = 44      # struct LifeGhostMon in include/life.h
MAX_PARTY = 6
FRESH_DAYS = 30    # ghosts not uploaded for this long stop appearing
MAX_LEVEL = 100

now = time.time  # tests replace this


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS ghosts (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            gender INTEGER NOT NULL, party BLOB NOT NULL, count INTEGER NOT NULL,
            wins INTEGER NOT NULL DEFAULT 0, losses INTEGER NOT NULL DEFAULT 0,
            new_wins INTEGER NOT NULL DEFAULT 0, new_losses INTEGER NOT NULL DEFAULT 0,
            updated_at REAL NOT NULL,
            UNIQUE (name, tid)
        )
        """
    )


def check_party(count, data):
    if not 1 <= count <= MAX_PARTY or len(data) != count * MON_SIZE:
        raise ValueError("bad party")
    for i in range(count):
        mon = data[i * MON_SIZE:(i + 1) * MON_SIZE]
        species = int.from_bytes(mon[0:2], "little")
        level = mon[14]
        if not 0 < species < MAX_SPECIES or not 1 <= level <= MAX_LEVEL:
            raise ValueError("bad Pokémon")


def cmd_put(ctx, args, data):
    """GHOST PUT <gender> <count> + the team. Answers how the ghost did since
    the last upload: wins|losses|all wins|all losses."""
    gender, count = int(args[1]), int(args[2])
    check_party(count, data)
    row = ctx.db.execute(
        "SELECT id, wins, losses, new_wins, new_losses FROM ghosts WHERE name = ? AND tid = ?", ctx.key
    ).fetchone()
    if row is None:
        ctx.db.execute(
            "INSERT INTO ghosts (name, tid, gender, party, count, updated_at) VALUES (?, ?, ?, ?, ?, ?)",
            (*ctx.key, int(bool(gender)), data, count, now()),
        )
        return text([(0, 0, 0, 0)])
    gid, wins, losses, new_wins, new_losses = row
    ctx.db.execute(
        "UPDATE ghosts SET gender = ?, party = ?, count = ?, updated_at = ?, new_wins = 0, new_losses = 0"
        " WHERE id = ?", (int(bool(gender)), data, count, now(), gid),
    )
    return text([(new_wins, new_losses, wins, losses)])


def cmd_get(ctx, args, data):
    """GHOST GET: a random other player's ghost, id|name|gender|wins|losses|count
    then a NUL and the team. Empty if there's nobody."""
    rows = ctx.db.execute(
        "SELECT id, name, gender, wins, losses, count, party FROM ghosts"
        " WHERE NOT (name = ? AND tid = ?) AND updated_at > ?",
        (*ctx.key, now() - FRESH_DAYS * 86400),
    ).fetchall()
    if not rows:
        return b""
    gid, name, gender, wins, losses, count, party = random.choice(rows)
    return text([(gid, name, gender, wins, losses, count)]) + b"\0" + party


def cmd_result(ctx, args, data):
    """GHOST RESULT <id> <1 if the player won>."""
    gid, won = int(args[1]), int(args[2])
    row = ctx.db.execute("SELECT name, tid FROM ghosts WHERE id = ?", (gid,)).fetchone()
    if row is None:
        raise Refused("That ghost has moved on.")
    if tuple(row) == ctx.key:
        raise Refused("That's your own ghost.")
    if won:
        ctx.db.execute("UPDATE ghosts SET losses = losses + 1, new_losses = new_losses + 1 WHERE id = ?", (gid,))
    else:
        ctx.db.execute("UPDATE ghosts SET wins = wins + 1, new_wins = new_wins + 1 WHERE id = ?", (gid,))
    return b""


SUBCOMMANDS = {"PUT": cmd_put, "GET": cmd_get, "RESULT": cmd_result}


def cmd_ghost(ctx, args, data):
    fn = SUBCOMMANDS.get(args[0])
    if fn is None:
        raise ValueError("bad GHOST request")
    return fn(ctx, args, data)


COMMANDS = {"GHOST": cmd_ghost}
