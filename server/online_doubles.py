"""Ranked double battles: a PvP double battle (the "Ranked double battle"
choice when talking to another player, link kind "double") counts for its
own ladder, with Elo ratings like the singles ladder. HUB > DOUBLES LADDER
shows it.

install() gives the plaza server its record_double, which finish_battle
calls for doubles instead of the singles league's record.
"""

from online import answer, board

START_RATING = 1000
ELO_K = 32
BOARD_SIZE = 8
SCORES = {"win": 1.0, "loss": 0.0, "draw": 0.5}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_doubles (
            name TEXT NOT NULL, tid INTEGER NOT NULL, rating REAL NOT NULL,
            wins INTEGER NOT NULL DEFAULT 0, losses INTEGER NOT NULL DEFAULT 0, draws INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (name, tid)
        );
        """
    )


def rating(db, key):
    db.execute("INSERT OR IGNORE INTO online_doubles (name, tid, rating) VALUES (?, ?, ?)", (*key, START_RATING))
    return db.execute("SELECT rating FROM online_doubles WHERE name = ? AND tid = ?", (*key,)).fetchone()[0]


def record(server, a, a_report, b, b_report):
    """Like League.record, for the doubles ladder. Returns the match status."""
    db = server.league.db
    if a_report is None or b_report is None:
        reported = a_report if a_report is not None else b_report
        score_a = SCORES[reported] if a_report is not None else 1.0 - SCORES[reported]
        status = "forfeit"
    elif (a_report, b_report) in (("win", "loss"), ("loss", "win"), ("draw", "draw")):
        score_a = SCORES[a_report]
        status = "ok"
    else:
        return "disputed"
    ra, rb = rating(db, a), rating(db, b)
    delta = ELO_K * (score_a - 1.0 / (1.0 + 10 ** ((rb - ra) / 400.0)))
    for key, d, score in ((a, delta, score_a), (b, -delta, 1.0 - score_a)):
        column = {1.0: "wins", 0.0: "losses", 0.5: "draws"}[score]
        db.execute(f"UPDATE online_doubles SET rating = rating + ?, {column} = {column} + 1 WHERE name = ? AND tid = ?",
                   (d, *key))
    db.commit()
    if hasattr(server, "online"):
        winner = a if score_a == 1.0 else b if score_a == 0.0 else None
        if winner:
            loser = b if winner == a else a
            server.online.announce(f"{winner[0]} beat {loser[0]} in a ranked double battle!", exclude=None)
    return status


def install(server):
    server.record_double = lambda a, ar, b, br: record(server, a, ar, b, br)


def cmd_doubles(ctx, args, data):
    db, key = ctx.db, ctx.key
    if args[0] != "BOARD":
        raise ValueError("bad DOUBLES")
    ranked = db.execute("SELECT name, tid, rating, wins, losses, draws FROM online_doubles"
                        " WHERE wins + losses + draws > 0 ORDER BY rating DESC, name").fetchall()
    rows = []
    for place, (n, t, r, w, l, d) in enumerate(ranked, 1):
        rows.append([f"{'*' if (n, t) == tuple(key) else ''}{place}", n, round(r), f"{w}-{l}-{d}"])
    mine = [r for r in rows if r[0].startswith("*")]
    rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
    message = (f"You're number {mine[0][0][1:]} with {mine[0][2]} points." if mine
               else "Talk to another player and pick RANKED DOUBLE BATTLE to get on the ladder!")
    return answer(message, rows=board("RANKED DOUBLES", [(6, "No."), (24, "TRAINER"), (110, "RATING"), (156, "W-L-D")],
                                      rows))


COMMANDS = {"DOUBLES": cmd_doubles}
