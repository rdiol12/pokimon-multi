"""Wonder Trade: send a Pokémon blind and get one back from another player.

A sent Pokémon waits in the pool until someone else sends one. Whoever sends
second gets the waiting Pokémon at once; the first sender picks theirs up the
next time they check. Pokémon are stored as the game's 100-byte struct.
"""

from ext import Refused, text

MON_SIZE = 100
MAX_SPECIES = 2000


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS wonder_pool (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            species INTEGER NOT NULL, mon BLOB NOT NULL,
            sent_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS wonder_pickup (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            from_name TEXT NOT NULL, species INTEGER NOT NULL, mon BLOB NOT NULL
        );
        CREATE TABLE IF NOT EXISTS wonder_stats (
            name TEXT NOT NULL, tid INTEGER NOT NULL, trades INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (name, tid)
        );
        """
    )


def trades(db, key):
    row = db.execute("SELECT trades FROM wonder_stats WHERE name = ? AND tid = ?", key).fetchone()
    return row[0] if row else 0


def count(db, key):
    db.execute("INSERT OR IGNORE INTO wonder_stats (name, tid) VALUES (?, ?)", key)
    db.execute("UPDATE wonder_stats SET trades = trades + 1 WHERE name = ? AND tid = ?", key)


def got(from_name, species, mon):
    """Answer for a Pokémon the game should add: 'from|species' then a NUL and the mon."""
    return text([(from_name, species)]) + b"\0" + mon


def cmd_wonder(ctx, args, data):
    db, key = ctx.db, ctx.key
    if args[0] == "SEND":
        species = int(args[1])
        if len(data) != MON_SIZE or not 0 < species < MAX_SPECIES:
            raise ValueError("bad Pokémon")
        if db.execute("SELECT 1 FROM wonder_pool WHERE name = ? AND tid = ?", key).fetchone():
            raise Refused("You already have one waiting.")
        other = db.execute(
            "SELECT id, name, tid, species, mon FROM wonder_pool WHERE NOT (name = ? AND tid = ?)"
            " ORDER BY id LIMIT 1", key
        ).fetchone()
        if other is None:
            db.execute("INSERT INTO wonder_pool (name, tid, species, mon) VALUES (?, ?, ?, ?)", (*key, species, data))
            return b""  # waiting
        oid, oname, otid, ospecies, omon = other
        db.execute("DELETE FROM wonder_pool WHERE id = ?", (oid,))
        db.execute(
            "INSERT INTO wonder_pickup (name, tid, from_name, species, mon) VALUES (?, ?, ?, ?, ?)",
            (oname, otid, key[0], species, data),
        )
        count(db, key)
        count(db, (oname, otid))
        return got(oname, ospecies, omon)
    if args[0] == "CHECK":
        row = db.execute(
            "SELECT id, from_name, species, mon FROM wonder_pickup WHERE name = ? AND tid = ? ORDER BY id LIMIT 1", key
        ).fetchone()
        if row is not None:
            db.execute("DELETE FROM wonder_pickup WHERE id = ?", (row[0],))
            return got(row[1], row[2], row[3])
        waiting = db.execute("SELECT 1 FROM wonder_pool WHERE name = ? AND tid = ?", key).fetchone()
        return b"WAITING" if waiting else b""
    raise ValueError("bad WONDER request")


COMMANDS = {"WONDER": cmd_wonder}
