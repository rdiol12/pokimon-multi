"""Bounties: post a reward for a Pokémon species. Whoever hands one in gets the
reward; the poster picks the Pokémon up from the board. The game takes the
reward out of the poster's money when the bounty goes up, so the server holds
it until the bounty is filled or canceled."""

from ext import Refused, text
from wondertrade import MON_SIZE, MAX_SPECIES, got

MAX_OPEN = 3
MAX_REWARD = 999999


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS bounties (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            species INTEGER NOT NULL, reward INTEGER NOT NULL,
            filler_name TEXT, filler_tid INTEGER, mon BLOB,
            status TEXT NOT NULL DEFAULT 'open',  -- open, filled, collected, canceled
            posted_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
        """
    )


def filled_by(db, key):
    return db.execute(
        "SELECT COUNT(*) FROM bounties WHERE filler_name = ? AND filler_tid = ?", key
    ).fetchone()[0]


def cmd_bounty(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    if sub == "LIST":
        # id|poster|species|reward|mine(1/0), open bounties, newest first
        rows = db.execute(
            "SELECT id, name, tid, species, reward FROM bounties WHERE status = 'open' ORDER BY id DESC LIMIT 20"
        ).fetchall()
        return text((r[0], r[1], r[3], r[4], int((r[1], r[2]) == key)) for r in rows)
    if sub == "POST":
        species, reward = int(args[1]), int(args[2])
        if not 0 < species < MAX_SPECIES or not 100 <= reward <= MAX_REWARD:
            raise Refused("Rewards are 100 to 999,999.")
        n = db.execute(
            "SELECT COUNT(*) FROM bounties WHERE name = ? AND tid = ? AND status IN ('open', 'filled')", key
        ).fetchone()[0]
        if n >= MAX_OPEN:
            raise Refused("You have 3 bounties up already.")
        cur = db.execute("INSERT INTO bounties (name, tid, species, reward) VALUES (?, ?, ?, ?)", (*key, species, reward))
        return text([(cur.lastrowid,)])
    if sub == "FILL":
        bid, species = int(args[1]), int(args[2])
        if len(data) != MON_SIZE:
            raise ValueError("bad Pokémon")
        row = db.execute("SELECT name, tid, species, reward, status FROM bounties WHERE id = ?", (bid,)).fetchone()
        if row is None or row[4] != "open":
            raise Refused("Someone beat you to it.")
        if (row[0], row[1]) == key:
            raise Refused("That's your own bounty.")
        if row[2] != species:
            raise Refused("That's not the POKéMON they want.")
        db.execute(
            "UPDATE bounties SET status = 'filled', filler_name = ?, filler_tid = ?, mon = ? WHERE id = ?",
            (*key, data, bid),
        )
        return text([(row[3],)])
    if sub == "COLLECT":
        # A filled bounty of mine: hand over its Pokémon.
        row = db.execute(
            "SELECT id, filler_name, species, mon FROM bounties WHERE name = ? AND tid = ? AND status = 'filled'"
            " ORDER BY id LIMIT 1", key
        ).fetchone()
        if row is None:
            return b""
        db.execute("UPDATE bounties SET status = 'collected' WHERE id = ?", (row[0],))
        return got(row[1], row[2], row[3])
    if sub == "CANCEL":
        bid = int(args[1])
        row = db.execute("SELECT name, tid, reward, status FROM bounties WHERE id = ?", (bid,)).fetchone()
        if row is None or (row[0], row[1]) != key or row[3] != "open":
            raise Refused("You can't cancel that one.")
        db.execute("UPDATE bounties SET status = 'canceled' WHERE id = ?", (bid,))
        return text([(row[2],)])  # refund
    raise ValueError("bad BOUNTY request")


COMMANDS = {"BOUNTY": cmd_bounty}
