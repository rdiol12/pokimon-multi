"""Global Pokédex race: who has caught the most, and who completed it first."""

from ext import text

COMPLETE = 1025  # NATIONAL_DEX_COUNT in the game


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS dex (
            name TEXT NOT NULL,
            tid INTEGER NOT NULL,
            seen INTEGER NOT NULL,
            caught INTEGER NOT NULL,
            reached_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            completed_at TEXT,
            PRIMARY KEY (name, tid)
        )
        """
    )


def caught(db, key):
    row = db.execute("SELECT caught FROM dex WHERE name = ? AND tid = ?", key).fetchone()
    return row[0] if row else 0


def cmd_dex(ctx, args, data):
    db, key = ctx.db, ctx.key
    if args[0] == "SUBMIT":
        seen, got, total = int(args[1]), int(args[2]), int(args[3]) if len(args) > 3 else COMPLETE
        if not 0 <= got <= seen <= total <= 2000:
            raise ValueError("bad counts")
        old = db.execute("SELECT caught FROM dex WHERE name = ? AND tid = ?", key).fetchone()
        if old is None:
            db.execute("INSERT INTO dex (name, tid, seen, caught) VALUES (?, ?, ?, ?)", (*key, seen, got))
        elif got != old[0]:
            # reached_at breaks ties: whoever got there first stays ahead.
            db.execute(
                "UPDATE dex SET seen = ?, caught = ?, reached_at = CURRENT_TIMESTAMP WHERE name = ? AND tid = ?",
                (seen, got, *key),
            )
        else:
            db.execute("UPDATE dex SET seen = ? WHERE name = ? AND tid = ?", (seen, *key))
        if got >= total:
            db.execute(
                "UPDATE dex SET completed_at = COALESCE(completed_at, CURRENT_TIMESTAMP) WHERE name = ? AND tid = ?",
                key,
            )
        return b""
    if args[0] == "BOARD":
        # rank|name|caught|seen|done(1/0) for the top 10, then a last line for me.
        rows = db.execute(
            "SELECT name, tid, caught, seen, completed_at FROM dex"
            " ORDER BY completed_at IS NULL, completed_at, caught DESC, reached_at, name"
        ).fetchall()
        out = [(i, r[0], r[2], r[3], int(r[4] is not None)) for i, r in enumerate(rows[:10], 1)]
        me = next(((i, r) for i, r in enumerate(rows, 1) if (r[0], r[1]) == key), None)
        out.append((me[0], me[1][0], me[1][2], me[1][3], int(me[1][4] is not None)) if me else (0, key[0], 0, 0, 0))
        return text(out)
    raise ValueError("bad DEX request")


COMMANDS = {"DEX": cmd_dex}
