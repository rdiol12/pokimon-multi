"""Shared secret bases. A player uploads their secret base (layout,
decorations and team, the game's struct SecretBase). Others can list them,
copy one into their own game to visit it where it is in Hoenn, or battle its
owner's team right away. The owner doesn't have to be online."""

from ext import Refused, text

MAX_BASE = 512


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS bases (
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            location INTEGER NOT NULL, base BLOB NOT NULL,
            visits INTEGER NOT NULL DEFAULT 0,
            saved_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (name, tid)
        )
        """
    )


def cmd_base(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    if sub == "PUT":
        location = int(args[1])
        if not 0 < len(data) <= MAX_BASE or not 0 < location < 256:
            raise ValueError("bad base")
        db.execute(
            "INSERT INTO bases (name, tid, location, base) VALUES (?, ?, ?, ?)"
            " ON CONFLICT (name, tid) DO UPDATE SET location = excluded.location, base = excluded.base,"
            " saved_at = CURRENT_TIMESTAMP",
            (*key, location, data),
        )
        return b""
    if sub == "LIST":
        # rowid|owner|location|visits, other players' bases, most visited first
        rows = db.execute(
            "SELECT rowid, name, location, visits FROM bases WHERE NOT (name = ? AND tid = ?)"
            " ORDER BY visits DESC, saved_at DESC LIMIT 12", key
        ).fetchall()
        return text(rows)
    if sub == "GET":
        row = db.execute("SELECT name, location, base FROM bases WHERE rowid = ?", (int(args[1]),)).fetchone()
        if row is None:
            raise Refused("That base is gone.")
        db.execute("UPDATE bases SET visits = visits + 1 WHERE rowid = ?", (int(args[1]),))
        return text([(row[0], row[1])]) + b"\0" + row[2]
    raise ValueError("bad BASE request")


COMMANDS = {"BASE": cmd_base}
