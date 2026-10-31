"""Player titles and badges, earned from PvP ladder rank and achievements in
the other online features. A player shows one title and its badge above their
name; other players see it on the overworld. Ladder data is only read."""

from ext import Refused, text
import bounties, dexrace, guilds, mail, tower, wondertrade

# id, name, test(ctx-free stats) -> bool. The id is also the badge's index
# in the game's badge sheet.
TITLES = [
    (0, "ROOKIE", lambda s: True),
    (1, "CHAMPION", lambda s: s["rank"] == 1),
    (2, "ELITE", lambda s: 0 < s["rank"] <= 4),
    (3, "ACE", lambda s: 0 < s["rank"] <= 10),
    (4, "VETERAN", lambda s: s["wins"] >= 10),
    (5, "CLIMBER", lambda s: s["tower"] >= 3),
    (6, "TYCOON", lambda s: s["tower"] >= 10),
    (7, "COLLECTOR", lambda s: s["caught"] >= 50),
    (8, "DEX MASTER", lambda s: s["caught"] >= 200),
    (9, "HUNTER", lambda s: s["bounties"] >= 3),
    (10, "FOUNDER", lambda s: s["founder"]),
    (11, "GENEROUS", lambda s: s["gifts"] >= 5),
    (12, "WONDERER", lambda s: s["wonder"] >= 5),
]


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS titles (
            name TEXT NOT NULL, tid INTEGER NOT NULL, title INTEGER NOT NULL,
            PRIMARY KEY (name, tid)
        )
        """
    )


def stats(league, db, key):
    rank, _, wins, _, _ = league.rank_of(key)
    founder = db.execute(
        "SELECT 1 FROM guilds WHERE leader_name = ? AND leader_tid = ?", key
    ).fetchone() is not None
    return {
        "rank": rank,
        "wins": wins,
        "tower": tower.best(db, key),
        "caught": dexrace.caught(db, key),
        "bounties": bounties.filled_by(db, key),
        "founder": founder,
        "gifts": mail.sent_by(db, key),
        "wonder": wondertrade.trades(db, key),
    }


def earned(league, db, key):
    s = stats(league, db, key)
    return [t for t in TITLES if t[2](s)]


def shown(league, db, key):
    """The title a player shows: their pick if they still have it, else ROOKIE."""
    row = db.execute("SELECT title FROM titles WHERE name = ? AND tid = ?", key).fetchone()
    have = {t[0] for t in earned(league, db, key)}
    return row[0] if row and row[0] in have else 0


def cmd_title(ctx, args, data):
    db, key = ctx.db, ctx.key
    sub = args[0]
    if sub == "LIST":
        # id|name|shown(1/0) for each earned title
        current = shown(ctx.league, db, key)
        return text((t[0], t[1], int(t[0] == current)) for t in earned(ctx.league, db, key))
    if sub == "SET":
        tid = int(args[1])
        if tid not in {t[0] for t in earned(ctx.league, db, key)}:
            raise Refused("You haven't earned that title.")
        db.execute("INSERT OR REPLACE INTO titles (name, tid, title) VALUES (?, ?, ?)", (*key, tid))
        return b""
    if sub == "OF":
        # player id|title id|title|guild tag, for players online now
        rows = []
        for pid in args[1:9]:
            p = ctx.server.players.get(int(pid))
            if p is None or p.name is None:
                continue
            t = shown(ctx.league, db, p.key)
            rows.append((p.id, t, TITLES[t][1], guilds.guild_of(db, p.key) or ""))
        return text(rows)
    raise ValueError("bad TITLE request")


COMMANDS = {"TITLE": cmd_title}
