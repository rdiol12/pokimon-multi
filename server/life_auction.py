"""Auction house: put a Pokémon up for a set time, other players bid on it,
and the top bid wins when the time runs out.

Money is held by the server while it's in play. The game takes a bid out of
the bidder's money once the server accepts it; a bidder who is outbid gets it
back from COLLECT. When an auction ends the winner collects the Pokémon and
the seller collects the price less the house's cut. A Pokémon nobody bid on
goes back to its seller. Bids in the last few minutes push the end back, so
nobody wins by sniping.

The market (market.py) is the fixed-price board; this is the bidding one.
"""

import os
import time

from ext import Refused, text
from wondertrade import MAX_SPECIES, MON_SIZE

AUCTION_HOURS = (1, 8, 24)
MAX_LISTINGS = 3          # open auctions per seller
MIN_START = 100
MAX_MONEY = 999999
HOUSE_CUT = 5             # percent of the final price
SNIPE_WINDOW = 5 * 60     # seconds: a late bid moves the end to now + this
LIST_ROWS = 20

# Flags in the last LIST field.
MINE, TOP, SHINY = 1, 2, 4


def now():
    """The clock. Tests replace this; the screenshot runs move it forward
    by writing seconds into the file named by LIFE_AUCTION_SKEW_FILE."""
    skew = 0.0
    path = os.environ.get("LIFE_AUCTION_SKEW_FILE")
    if path:
        try:
            with open(path) as f:
                skew = float(f.read().strip() or 0)
        except (OSError, ValueError):
            pass
    return time.time() + skew


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS auctions (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            species INTEGER NOT NULL, level INTEGER NOT NULL, shiny INTEGER NOT NULL,
            mon BLOB NOT NULL,
            start INTEGER NOT NULL, bid INTEGER NOT NULL DEFAULT 0, bids INTEGER NOT NULL DEFAULT 0,
            bidder_name TEXT, bidder_tid INTEGER,
            ends_at REAL NOT NULL,
            status TEXT NOT NULL DEFAULT 'open'  -- open, ended, canceled
        )
        """
    )
    # Things waiting for a player: money (refunds, sales) and Pokémon (wins,
    # unsold or canceled listings).
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS auction_inbox (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            kind TEXT NOT NULL,  -- WON, SOLD, OUTBID, UNSOLD, CANCELED
            money INTEGER NOT NULL DEFAULT 0,
            species INTEGER NOT NULL, other TEXT NOT NULL DEFAULT '',
            mon BLOB
        )
        """
    )


def min_bid(start, bid):
    """The lowest bid the auction takes now: the start price, or the top bid
    plus 5% (at least 100), rounded up to 100. The game works it out the same
    way (LifeAuction_MinBid)."""
    if bid == 0:
        return start
    step = max(100, bid // 20)
    return min(MAX_MONEY, (bid + step + 99) // 100 * 100)


def _inbox(db, key, kind, species, money=0, other="", mon=None):
    db.execute(
        "INSERT INTO auction_inbox (name, tid, kind, money, species, other, mon) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (*key, kind, money, species, other, mon),
    )


def settle(db):
    """Closes every auction whose time is up."""
    rows = db.execute(
        "SELECT id, name, tid, species, mon, bid, bidder_name, bidder_tid FROM auctions"
        " WHERE status = 'open' AND ends_at <= ?", (now(),)
    ).fetchall()
    for aid, name, tid, species, mon, bid, bname, btid in rows:
        if bname is None:
            _inbox(db, (name, tid), "UNSOLD", species, mon=mon)
        else:
            _inbox(db, (bname, btid), "WON", species, money=bid, other=name, mon=mon)
            _inbox(db, (name, tid), "SOLD", species, money=bid - bid * HOUSE_CUT // 100, other=bname)
        db.execute("UPDATE auctions SET status = 'ended' WHERE id = ?", (aid,))


def cmd_list(ctx, args, data):
    # id|seller|species|level|price|bids|seconds left|flags
    t = now()
    rows = ctx.db.execute(
        "SELECT id, name, tid, species, level, shiny, start, bid, bids, bidder_name, bidder_tid, ends_at"
        " FROM auctions WHERE status = 'open' ORDER BY ends_at LIMIT ?", (LIST_ROWS,)
    ).fetchall()
    out = []
    for aid, name, tid, species, level, shiny, start, bid, bids, bname, btid, ends in rows:
        flags = (MINE if (name, tid) == ctx.key else 0) | (TOP if (bname, btid) == ctx.key else 0)
        flags |= SHINY if shiny else 0
        out.append((aid, name, species, level, bid or start, bids, max(0, int(ends - t)), flags))
    return text(out)


def cmd_post(ctx, args, data):
    species, level, shiny, start, hours = (int(a) for a in args[1:6])
    if len(data) != MON_SIZE or not 0 < species < MAX_SPECIES or not 1 <= level <= 100:
        raise ValueError("bad Pokémon")
    if hours not in AUCTION_HOURS:
        raise ValueError("bad length")
    if not MIN_START <= start <= MAX_MONEY:
        raise Refused("Starting bids are 100 to 999,999.")
    n = ctx.db.execute(
        "SELECT COUNT(*) FROM auctions WHERE name = ? AND tid = ? AND status = 'open'", ctx.key
    ).fetchone()[0]
    if n >= MAX_LISTINGS:
        raise Refused("You already have 3 auctions\nrunning.")
    cur = ctx.db.execute(
        "INSERT INTO auctions (name, tid, species, level, shiny, mon, start, ends_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        (*ctx.key, species, level, int(bool(shiny)), data, start, now() + hours * 3600),
    )
    return text([(cur.lastrowid,)])


def cmd_bid(ctx, args, data):
    aid, amount = int(args[1]), int(args[2])
    row = ctx.db.execute(
        "SELECT name, tid, species, start, bid, bidder_name, bidder_tid, ends_at, status FROM auctions WHERE id = ?",
        (aid,),
    ).fetchone()
    if row is None or row[8] != "open" or row[7] <= now():
        raise Refused("That auction is over.")
    name, tid, species, start, bid, bname, btid, ends, _ = row
    if (name, tid) == ctx.key:
        raise Refused("You can't bid on your own auction.")
    if (bname, btid) == ctx.key:
        raise Refused("You're already the top bidder.")
    if amount > MAX_MONEY:
        raise ValueError("bad bid")
    if amount < min_bid(start, bid):
        raise Refused(f"Someone bid first. The lowest\nbid now is {min_bid(start, bid)}.")
    if bname is not None:
        _inbox(ctx.db, (bname, btid), "OUTBID", species, money=bid, other=ctx.player.name)
    ends = max(ends, now() + SNIPE_WINDOW)
    ctx.db.execute(
        "UPDATE auctions SET bid = ?, bids = bids + 1, bidder_name = ?, bidder_tid = ?, ends_at = ? WHERE id = ?",
        (amount, *ctx.key, ends, aid),
    )
    return text([(amount, int(ends - now()))])


def cmd_cancel(ctx, args, data):
    aid = int(args[1])
    row = ctx.db.execute("SELECT name, tid, species, mon, bids, status FROM auctions WHERE id = ?", (aid,)).fetchone()
    if row is None or (row[0], row[1]) != ctx.key or row[5] != "open":
        raise Refused("You can't take that one back.")
    if row[4] != 0:
        raise Refused("Someone has bid on it, so it\nhas to run until the end.")
    ctx.db.execute("UPDATE auctions SET status = 'canceled' WHERE id = ?", (aid,))
    _inbox(ctx.db, ctx.key, "CANCELED", row[2], mon=row[3])
    return b""


def cmd_collect(ctx, args, data):
    """One thing waiting for the player: kind|money|species|other, then a
    NUL and the Pokémon if there is one. Empty when there's nothing. The game
    asks again until it's empty."""
    row = ctx.db.execute(
        "SELECT id, kind, money, species, other, mon FROM auction_inbox WHERE name = ? AND tid = ?"
        " ORDER BY id LIMIT 1", ctx.key
    ).fetchone()
    if row is None:
        return b""
    rid, kind, money, species, other, mon = row
    ctx.db.execute("DELETE FROM auction_inbox WHERE id = ?", (rid,))
    # A win was paid for when the bid went in: its money is only shown.
    out = text([(kind, 0 if kind == "WON" else money, species, other or "-", money)])
    if mon is not None:
        out += b"\0" + mon
    return out


SUBCOMMANDS = {"LIST": cmd_list, "POST": cmd_post, "BID": cmd_bid, "CANCEL": cmd_cancel, "COLLECT": cmd_collect}


def cmd_auction(ctx, args, data):
    settle(ctx.db)
    fn = SUBCOMMANDS.get(args[0])
    if fn is None:
        raise ValueError("bad AUCTION request")
    return fn(ctx, args, data)


COMMANDS = {"AUCTION": cmd_auction}
