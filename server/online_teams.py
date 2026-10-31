"""2v2 team battles: two players form a team (HUB > TEAM LADDER), and a team
challenges another by talking to one of its members and picking
"2v2 team battle". All four games link up (link kind "multi") for a multi
battle, each player with the first three Pokémon of their party. Teams have
their own Elo ladder.

install() gives the plaza server its team_partners (who comes along to a
team battle) and record_multi (the result).
"""

from online import Refused, answer, board

START_RATING = 1000
ELO_K = 32
BOARD_SIZE = 8
INVITE_SECONDS = 15 * 60
SCORES = {"win": 1.0, "loss": 0.0, "draw": 0.5}

# What VAR_0x800B is after TEAM VIEW.
NONE, INVITED, ASKED, TEAMED = 0, 1, 2, 3


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_teams (
            id INTEGER PRIMARY KEY,
            a_name TEXT NOT NULL, a_tid INTEGER NOT NULL, b_name TEXT NOT NULL, b_tid INTEGER NOT NULL,
            rating REAL NOT NULL DEFAULT 1000,
            wins INTEGER NOT NULL DEFAULT 0, losses INTEGER NOT NULL DEFAULT 0, draws INTEGER NOT NULL DEFAULT 0,
            ended INTEGER NOT NULL DEFAULT 0
        );
        CREATE TABLE IF NOT EXISTS online_team_invites (
            from_name TEXT NOT NULL, from_tid INTEGER NOT NULL, to_name TEXT NOT NULL, to_tid INTEGER NOT NULL,
            at REAL NOT NULL, PRIMARY KEY (from_name, from_tid)
        );
        """
    )


def team_of(db, key):
    """(team id, partner key, rating, wins, losses, draws) or None."""
    row = db.execute("SELECT id, a_name, a_tid, b_name, b_tid, rating, wins, losses, draws FROM online_teams"
                     " WHERE ended = 0 AND ((a_name = ? AND a_tid = ?) OR (b_name = ? AND b_tid = ?))",
                     (*key, *key)).fetchone()
    if row is None:
        return None
    tid, an, at, bn, bt, rating, w, l, d = row
    partner = (bn, bt) if (an, at) == tuple(key) else (an, at)
    return tid, partner, rating, w, l, d


def team_name(a, b):
    return f"{a} & {b}"


def invite_to(db, key, now):
    return db.execute("SELECT from_name, from_tid FROM online_team_invites WHERE to_name = ? AND to_tid = ? AND at > ?"
                      " ORDER BY at DESC", (*key, now - INVITE_SECONDS)).fetchone()


def invite_from(db, key, now):
    return db.execute("SELECT to_name, to_tid FROM online_team_invites WHERE from_name = ? AND from_tid = ? AND at > ?",
                      (*key, now - INVITE_SECONDS)).fetchone()


def online_player(server, key):
    for p in server.players.values():
        if p.name is not None and p.key == tuple(key):
            return p
    return None


def team_partners(server, host, guest):
    """The partners of both leaders, if both teams can play now; tells the
    host why not otherwise."""
    db = server.league.db

    def why(text):
        if hasattr(server, "online"):
            server.online.notify(host.key, text)
        return None

    mine, theirs = team_of(db, host.key), team_of(db, guest.key)
    if mine is None:
        return why("Form a team in HUB > TEAM LADDER first!")
    if theirs is None:
        return why(f"{guest.name} isn't on a team.")
    if mine[0] == theirs[0]:
        return why("You can't battle your own team!")
    partners = []
    for team in (mine, theirs):
        p = online_player(server, team[1])
        if p is None:
            return why(f"{team[1][0]} isn't online.")
        if p.session:
            return why(f"{team[1][0]} is busy.")
        partners.append(p)
    return tuple(partners)


def team_report(reports, keys):
    """What a team says happened: its members' reports, if they agree."""
    said = {reports[k] for k in keys if k in reports}
    if len(said) > 1:
        return "disputed"
    return said.pop() if said else None


def record(server, session):
    """Rates the team battle `session` on the team ladder. Returns the match status."""
    db = server.league.db
    keys = [p.key for p in session.members]
    a_keys, b_keys = (keys[0], keys[2]), (keys[1], keys[3])
    ta, tb = team_of(db, keys[0]), team_of(db, keys[1])
    if ta is None or tb is None:
        return "no team"
    a_report, b_report = team_report(session.reports, a_keys), team_report(session.reports, b_keys)
    if "disputed" in (a_report, b_report) or (a_report, b_report) == (None, None):
        return "disputed"
    if a_report is None or b_report is None:
        reported = a_report if a_report is not None else b_report
        score_a = SCORES[reported] if a_report is not None else 1.0 - SCORES[reported]
        status = "forfeit"
    elif (a_report, b_report) in (("win", "loss"), ("loss", "win"), ("draw", "draw")):
        score_a = SCORES[a_report]
        status = "ok"
    else:
        return "disputed"
    ra, rb = ta[2], tb[2]
    delta = ELO_K * (score_a - 1.0 / (1.0 + 10 ** ((rb - ra) / 400.0)))
    for team, d, score in ((ta, delta, score_a), (tb, -delta, 1.0 - score_a)):
        column = {1.0: "wins", 0.0: "losses", 0.5: "draws"}[score]
        db.execute(f"UPDATE online_teams SET rating = rating + ?, {column} = {column} + 1 WHERE id = ?", (d, team[0]))
    db.commit()
    if hasattr(server, "online") and score_a != 0.5:
        names = [p.name for p in session.members]
        winners, losers = (names[0], names[2]), (names[1], names[3])
        if score_a == 0.0:
            winners, losers = losers, winners
        server.online.announce(f"{team_name(*winners)} beat {team_name(*losers)} in a 2v2 team battle!",
                               exclude=None)
    return status


def install(server):
    server.team_partners = lambda host, guest: team_partners(server, host, guest)
    server.record_multi = lambda session: record(server, session)


def cmd_team(ctx, args, data):
    db, key = ctx.db, ctx.key
    now = ctx.online.now()
    word = args[0]

    if word == "VIEW":
        team = team_of(db, key)
        if team:
            tid, partner, rating, w, l, d = team
            return answer(f"Team {team_name(ctx.player.name, partner[0])}: {round(rating)} points, {w}-{l}-{d}.~"
                          "Talk to a rival to battle!", code=TEAMED, vars=[partner[0]])
        inv = invite_to(db, key, now)
        if inv:
            return answer(f"{inv[0]} wants to form a team~with you. Will you?", code=INVITED, vars=[inv[0]])
        asked = invite_from(db, key, now)
        if asked:
            return answer(f"Waiting for {asked[0]} to answer.", code=ASKED, vars=[asked[0]])
        return answer("Team up with another trainer for 2v2 battles against other teams!", code=NONE)

    if word == "WHO":
        rows = [[p.name, p.id] for p in ctx.server.players.values()
                if p.name is not None and p is not ctx.player and team_of(db, p.key) is None]
        if not rows:
            raise Refused("Nobody online is free to team up right now.")
        return answer("Who will you team up with?", rows=rows[:20])

    if word == "ASK":
        target = ctx.server.players.get(int(args[1]) if len(args) > 1 and args[1].isdigit() else -1)
        if target is None or target.name is None:
            raise Refused("That trainer has gone offline.")
        if team_of(db, key):
            raise Refused("You're on a team already.")
        if team_of(db, target.key):
            raise Refused(f"{target.name} is on a team already.")
        db.execute("INSERT OR REPLACE INTO online_team_invites VALUES (?, ?, ?, ?, ?)", (*key, *target.key, now))
        ctx.online.notify(target.key, f"{ctx.player.name} wants to form a team! Answer in HUB > TEAM LADDER.")
        return answer(f"You asked {target.name} to form a team.")

    if word in ("YES", "NO"):
        inv = invite_to(db, key, now)
        if inv is None:
            raise Refused("That invitation has run out.")
        db.execute("DELETE FROM online_team_invites WHERE (from_name = ? AND from_tid = ?)"
                   " OR (from_name = ? AND from_tid = ?)", (*inv, *key))
        if word == "NO":
            ctx.online.notify(inv, f"{ctx.player.name} can't team up right now.")
            return answer()
        if team_of(db, inv) or team_of(db, key):
            raise Refused(f"{inv[0]} is on a team already.")
        db.execute("INSERT INTO online_teams (a_name, a_tid, b_name, b_tid, rating) VALUES (?, ?, ?, ?, ?)",
                   (*inv, *key, START_RATING))
        ctx.online.notify(inv, f"{ctx.player.name} joined your team! You can take on other teams now.")
        return answer(f"You formed a team with {inv[0]}!")

    if word == "LEAVE":
        team = team_of(db, key)
        if team is None:
            raise Refused("You're not on a team.")
        db.execute("UPDATE online_teams SET ended = 1 WHERE id = ?", (team[0],))
        ctx.online.notify(team[1], f"{ctx.player.name} split up your team.")
        return answer(f"You split up with {team[1][0]}.")

    if word == "BOARD":
        teams = db.execute("SELECT id, a_name, a_tid, b_name, b_tid, rating, wins, losses, draws FROM online_teams"
                           " WHERE wins + losses + draws > 0 ORDER BY rating DESC, id").fetchall()
        mine = team_of(db, key)
        rows = []
        for place, (tid, an, at, bn, bt, r, w, l, d) in enumerate(teams, 1):
            mark = "*" if mine and mine[0] == tid else ""
            rows.append([f"{mark}{place}", team_name(an, bn), round(r), f"{w}-{l}-{d}"])
        marked = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in marked if r not in rows[:BOARD_SIZE]]
        message = (f"Your team is number {marked[0][0][1:]} with {marked[0][2]} points." if marked
                   else "Win a 2v2 team battle to get on the ladder!")
        return answer(message, rows=board("2V2 TEAM LADDER", [(6, "No."), (24, "TEAM"), (126, "RATING"),
                                                              (164, "W-L-D")], rows))

    raise ValueError("bad TEAM")


COMMANDS = {"TEAM": cmd_team}
