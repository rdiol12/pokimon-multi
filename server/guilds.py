"""Guilds: a shared name and tag, and a guild ladder ranked by the average
PvP rating of the members who have rated battles."""

import re

from ext import Refused, text

TAG_RE = re.compile(r"^[A-Z0-9]{2,4}$")
NAME_RE = re.compile(r"^[A-Za-z0-9 .!?-]{1,12}$")
MAX_MEMBERS = 20


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS guilds (
            tag TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            leader_name TEXT NOT NULL,
            leader_tid INTEGER NOT NULL,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS guild_members (
            name TEXT NOT NULL,
            tid INTEGER NOT NULL,
            tag TEXT NOT NULL,
            PRIMARY KEY (name, tid)
        );
        """
    )


def guild_of(db, key):
    row = db.execute("SELECT tag FROM guild_members WHERE name = ? AND tid = ?", key).fetchone()
    return row[0] if row else None


def members(db, tag):
    return db.execute("SELECT name, tid FROM guild_members WHERE tag = ? ORDER BY name", (tag,)).fetchall()


def score(league, db, tag):
    """Average rating of ranked members, rounded; 0 if nobody is ranked."""
    ratings = [league.rank_of(tuple(m))[1] for m in members(db, tag) if league.rank_of(tuple(m))[0]]
    return round(sum(ratings) / len(ratings)) if ratings else 0


def ladder(league, db):
    rows = []
    for tag, name in db.execute("SELECT tag, name FROM guilds").fetchall():
        rows.append((score(league, db, tag), len(members(db, tag)), tag, name))
    rows.sort(key=lambda r: (-r[0], -r[1], r[2]))
    return rows


def cmd_guild(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    mine = guild_of(db, key)
    if sub == "INFO":
        # Line 1: tag|name|leader|members|score|rank, then one line per member: name|rating
        if mine is None:
            return b""
        tag, name, leader = db.execute("SELECT tag, name, leader_name FROM guilds WHERE tag = ?", (mine,)).fetchone()
        board = ladder(ctx.league, db)
        rank = next(i for i, r in enumerate(board, 1) if r[2] == tag)
        rows = [(tag, name, leader, len(members(db, tag)), score(ctx.league, db, tag), rank)]
        for m in members(db, tag):
            rows.append((m[0], round(ctx.league.rank_of(tuple(m))[1])))
        return text(rows)
    if sub == "LIST":
        # rank|tag|name|members|score, best first
        return text((i, tag, name, n, s) for i, (s, n, tag, name) in enumerate(ladder(ctx.league, db)[:10], 1))
    if sub == "CREATE":
        tag, name = args[1].upper(), " ".join(args[2:])
        if mine:
            raise Refused("Leave your guild first.")
        if not TAG_RE.match(tag) or not NAME_RE.match(name):
            raise Refused("Tags are 2-4 letters or digits.")
        if db.execute("SELECT 1 FROM guilds WHERE tag = ?", (tag,)).fetchone():
            raise Refused(f"The tag {tag} is taken.")
        db.execute("INSERT INTO guilds (tag, name, leader_name, leader_tid) VALUES (?, ?, ?, ?)", (tag, name, *key))
        db.execute("INSERT INTO guild_members (name, tid, tag) VALUES (?, ?, ?)", (*key, tag))
        return text([(tag, name)])
    if sub == "JOIN":
        tag = args[1].upper()
        if mine:
            raise Refused("Leave your guild first.")
        row = db.execute("SELECT name FROM guilds WHERE tag = ?", (tag,)).fetchone()
        if row is None:
            raise Refused("That guild is gone.")
        if len(members(db, tag)) >= MAX_MEMBERS:
            raise Refused("That guild is full.")
        db.execute("INSERT INTO guild_members (name, tid, tag) VALUES (?, ?, ?)", (*key, tag))
        return text([(tag, row[0])])
    if sub == "LEAVE":
        if mine is None:
            raise Refused("You aren't in a guild.")
        db.execute("DELETE FROM guild_members WHERE name = ? AND tid = ?", key)
        if not members(db, mine):
            db.execute("DELETE FROM guilds WHERE tag = ?", (mine,))
        else:
            leader = db.execute("SELECT leader_name, leader_tid FROM guilds WHERE tag = ?", (mine,)).fetchone()
            if tuple(leader) == key:
                heir = members(db, mine)[0]
                db.execute("UPDATE guilds SET leader_name = ?, leader_tid = ? WHERE tag = ?", (*heir, mine))
        return b""
    raise ValueError("bad GUILD request")


COMMANDS = {"GUILD": cmd_guild}
