"""Mail and gifts: a short message with money and/or an item, sent to another
player, online or not. The game takes the gift out of the sender's bag or
wallet when the server accepts it, and adds it when the recipient opens it."""

from ext import Refused, text
import guilds

MAX_MONEY = 999999
MAX_INBOX = 20
PHRASES = 8  # message choices in the game


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS mail (
            id INTEGER PRIMARY KEY,
            to_name TEXT NOT NULL, to_tid INTEGER NOT NULL,
            from_name TEXT NOT NULL, from_tid INTEGER NOT NULL,
            phrase INTEGER NOT NULL, money INTEGER NOT NULL,
            item INTEGER NOT NULL, quantity INTEGER NOT NULL,
            opened INTEGER NOT NULL DEFAULT 0,
            sent_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
        """
    )


def sent_by(db, key):
    return db.execute("SELECT COUNT(*) FROM mail WHERE from_name = ? AND from_tid = ?", key).fetchone()[0]


def known_players(ctx):
    """Who can be mailed: everyone online, plus guildmates and anyone who sent
    us mail, online or not. name|tid|online"""
    seen = {}
    for p in ctx.online():
        if p.key != ctx.key:
            seen[p.key] = 1
    tag = guilds.guild_of(ctx.db, ctx.key)
    if tag:
        for m in guilds.members(ctx.db, tag):
            seen.setdefault(tuple(m), 0)
    for m in ctx.db.execute(
        "SELECT DISTINCT from_name, from_tid FROM mail WHERE to_name = ? AND to_tid = ?", ctx.key
    ).fetchall():
        seen.setdefault(tuple(m), 0)
    seen.pop(ctx.key, None)
    return sorted(seen.items(), key=lambda kv: (-kv[1], kv[0]))[:12]


def cmd_mail(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    if sub == "WHO":
        return text((k[0], k[1], online) for k, online in known_players(ctx))
    if sub == "SEND":
        to = (args[1], int(args[2]) & 0xFFFF)
        phrase, money, item, qty = (int(a) for a in args[3:7])
        if to == key:
            raise Refused("You can't mail yourself.")
        if not 0 <= phrase < PHRASES or not 0 <= money <= MAX_MONEY or item < 0 or not 0 <= qty <= 99:
            raise ValueError("bad gift")
        if (item == 0) != (qty == 0):
            raise ValueError("bad item")
        inbox = db.execute(
            "SELECT COUNT(*) FROM mail WHERE to_name = ? AND to_tid = ? AND opened = 0", to
        ).fetchone()[0]
        if inbox >= MAX_INBOX:
            raise Refused("Their mailbox is full.")
        db.execute(
            "INSERT INTO mail (to_name, to_tid, from_name, from_tid, phrase, money, item, quantity)"
            " VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            (*to, *key, phrase, money, item, qty),
        )
        for p in ctx.online():
            if p.key == to:
                p.send("XNOTE MAIL")
        return b""
    if sub == "INBOX":
        # id|from|phrase|money|item|quantity, unopened, oldest first
        rows = db.execute(
            "SELECT id, from_name, phrase, money, item, quantity FROM mail"
            " WHERE to_name = ? AND to_tid = ? AND opened = 0 ORDER BY id LIMIT 10", key
        ).fetchall()
        return text(rows)
    if sub == "OPEN":
        mid = int(args[1])
        row = db.execute(
            "SELECT from_name, phrase, money, item, quantity FROM mail"
            " WHERE id = ? AND to_name = ? AND to_tid = ? AND opened = 0", (mid, *key)
        ).fetchone()
        if row is None:
            raise Refused("That mail is gone.")
        db.execute("UPDATE mail SET opened = 1 WHERE id = ?", (mid,))
        return text([row])
    raise ValueError("bad MAIL request")


COMMANDS = {"MAIL": cmd_mail}
