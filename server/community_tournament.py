"""Online tournaments (docs/community/tournaments.md).

There is always one cup open for sign-ups. It starts when it is full, or
once TOUR_SIGNUP_SECONDS have passed with at least two players in it; the
bracket is then seeded by ladder rating (byes go to the top seeds). Matches
are ordinary online battles: when two players who meet in the current
round battle each other, the result moves the winner on. The champion and
the runner-up get a prize the next time they check the cup, and a new cup
opens. The bracket is also a page on the server's website (/tournament).
"""

import html
import logging
import os
import time

import community
from community import Answer, listen, request

log = logging.getLogger("netplay.tournament")

REQ_STATUS, REQ_JOIN, REQ_LEAVE, REQ_BRACKET = 1, 2, 3, 4
# Answers to REQ_STATUS (data/scripts/community/tournament.inc).
NOT_IN, SIGNED_UP, PLAYING, PRIZE, OUT = 2, 3, 4, 5, 6
FULL = 7
SIZE = int(os.environ.get("NETPLAY_TOUR_SIZE", "8"))
SIGNUP_SECONDS = float(os.environ.get("NETPLAY_TOUR_SIGNUP_SECONDS", str(10 * 60)))
# include/constants/items.h
ITEM_MASTER_BALL, ITEM_RARE_CANDY, ITEM_GOLD_BOTTLE_CAP = 4, 102, 134
PRIZES = {1: (ITEM_MASTER_BALL, "a MASTER BALL"), 2: (ITEM_GOLD_BOTTLE_CAP, "a GOLD BOTTLE CAP")}
NAMES = ("PETALBURG", "SLATEPORT", "MAUVILLE", "LILYCOVE", "MOSSDEEP", "FORTREE", "RUSTBORO", "SOOTOPOLIS")

SCHEMA = """
CREATE TABLE IF NOT EXISTS tour_cups (
    id INTEGER PRIMARY KEY,
    state TEXT NOT NULL,          -- signup, running, done
    opened REAL NOT NULL,
    started REAL
);
CREATE TABLE IF NOT EXISTS tour_entrants (
    cup INTEGER NOT NULL,
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    seed INTEGER,
    place INTEGER,               -- 1 champion, 2 runner-up, after the cup
    prize_claimed INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (cup, name, tid)
);
CREATE TABLE IF NOT EXISTS tour_matches (
    cup INTEGER NOT NULL,
    round INTEGER NOT NULL,
    slot INTEGER NOT NULL,
    a_name TEXT, a_tid INTEGER,
    b_name TEXT, b_tid INTEGER,
    winner_name TEXT, winner_tid INTEGER,
    PRIMARY KEY (cup, round, slot)
);
"""


def cup_name(cup_id):
    return f"{NAMES[(cup_id - 1) % len(NAMES)]} CUP"


class Tournaments:
    def __init__(self, db, league, clock=time.time):
        self.db = db
        self.league = league
        self.clock = clock
        self.db.executescript(SCHEMA)
        self.db.commit()

    # --- cups ---------------------------------------------------------------

    def open_cup(self):
        row = self.db.execute("SELECT id FROM tour_cups WHERE state = 'signup' ORDER BY id DESC LIMIT 1").fetchone()
        if row:
            return row[0]
        cur = self.db.execute("INSERT INTO tour_cups (state, opened) VALUES ('signup', ?)", (self.clock(),))
        self.db.commit()
        return cur.lastrowid

    def running_cup(self):
        row = self.db.execute("SELECT id FROM tour_cups WHERE state = 'running' ORDER BY id LIMIT 1").fetchone()
        return row[0] if row else None

    def entrants(self, cup):
        return self.db.execute(
            "SELECT name, tid FROM tour_entrants WHERE cup = ? ORDER BY COALESCE(seed, 99), rowid", (cup,)
        ).fetchall()

    def join(self, key):
        """True when signed up (or already in), False when the cup is full."""
        cup = self.open_cup()
        if self.db.execute("SELECT 1 FROM tour_entrants WHERE cup = ? AND name = ? AND tid = ?", (cup, *key)).fetchone():
            return True
        if len(self.entrants(cup)) >= SIZE:
            return False
        self.db.execute("INSERT INTO tour_entrants (cup, name, tid) VALUES (?, ?, ?)", (cup, *key))
        self.db.commit()
        self.start_if_due()
        return True

    def leave(self, key):
        cup = self.open_cup()
        self.db.execute("DELETE FROM tour_entrants WHERE cup = ? AND name = ? AND tid = ?", (cup, *key))
        self.db.commit()

    def in_cup(self, cup, key):
        return self.db.execute(
            "SELECT 1 FROM tour_entrants WHERE cup = ? AND name = ? AND tid = ?", (cup, *key)
        ).fetchone() is not None

    def start_if_due(self):
        """Starts the open cup when it's full, or when sign-ups have run long
        enough with two or more players. Only one cup runs at a time."""
        if self.running_cup():
            return None
        cup = self.open_cup()
        opened = self.db.execute("SELECT opened FROM tour_cups WHERE id = ?", (cup,)).fetchone()[0]
        players = self.entrants(cup)
        if len(players) < SIZE and (len(players) < 2 or self.clock() - opened < SIGNUP_SECONDS):
            return None
        # Seed by rating: 1 v last, 2 v second last... byes go to the top.
        players.sort(key=lambda key: -self.league.rating(key))
        for seed, key in enumerate(players, 1):
            self.db.execute("UPDATE tour_entrants SET seed = ? WHERE cup = ? AND name = ? AND tid = ?", (seed, cup, *key))
        size = 2
        while size < len(players):
            size *= 2
        slots = players + [None] * (size - len(players))
        order = bracket_order(size)
        for slot in range(size // 2):
            a, b = slots[order[slot * 2] - 1], slots[order[slot * 2 + 1] - 1]
            self.db.execute(
                "INSERT INTO tour_matches (cup, round, slot, a_name, a_tid, b_name, b_tid) VALUES (?, 1, ?, ?, ?, ?, ?)",
                (cup, slot, *(a or (None, None)), *(b or (None, None))),
            )
        self.db.execute("UPDATE tour_cups SET state = 'running', started = ? WHERE id = ?", (self.clock(), cup))
        self.db.commit()
        log.info("%s started with %d players", cup_name(cup), len(players))
        self.settle_byes(cup)
        return cup

    def rounds(self, cup):
        return self.db.execute("SELECT MAX(round) FROM tour_matches WHERE cup = ?", (cup,)).fetchone()[0] or 0

    def matches(self, cup, rnd=None):
        sql = "SELECT round, slot, a_name, a_tid, b_name, b_tid, winner_name, winner_tid FROM tour_matches WHERE cup = ?"
        args = [cup]
        if rnd is not None:
            sql += " AND round = ?"
            args.append(rnd)
        return self.db.execute(sql + " ORDER BY round, slot", args).fetchall()

    def settle_byes(self, cup):
        for rnd, slot, an, at, bn, bt, wn, _ in self.matches(cup, self.rounds(cup)):
            if wn is None and (an is None) != (bn is None):
                self.set_winner(cup, rnd, slot, (an, at) if an else (bn, bt))

    def set_winner(self, cup, rnd, slot, key):
        self.db.execute(
            "UPDATE tour_matches SET winner_name = ?, winner_tid = ? WHERE cup = ? AND round = ? AND slot = ?",
            (*key, cup, rnd, slot),
        )
        self.db.commit()
        matches = self.matches(cup, rnd)
        if any(m[6] is None for m in matches):
            return
        if len(matches) == 1:
            self.finish(cup, matches[0])
            return
        for i in range(0, len(matches), 2):
            a, b = matches[i], matches[i + 1]
            self.db.execute(
                "INSERT INTO tour_matches (cup, round, slot, a_name, a_tid, b_name, b_tid) VALUES (?, ?, ?, ?, ?, ?, ?)",
                (cup, rnd + 1, i // 2, a[6], a[7], b[6], b[7]),
            )
        self.db.commit()
        self.settle_byes(cup)

    def finish(self, cup, final):
        champion = (final[6], final[7])
        runner_up = (final[4], final[5]) if (final[2], final[3]) == champion else (final[2], final[3])
        for place, key in ((1, champion), (2, runner_up)):
            if key[0] is not None:
                self.db.execute("UPDATE tour_entrants SET place = ? WHERE cup = ? AND name = ? AND tid = ?",
                                (place, cup, *key))
        self.db.execute("UPDATE tour_cups SET state = 'done' WHERE id = ?", (cup,))
        self.db.commit()
        log.info("%s won by %s", cup_name(cup), champion[0])
        self.start_if_due()

    def current_match(self, key):
        """(cup, round, slot, opponent key) of `key`'s unplayed match, or None."""
        cup = self.running_cup()
        if cup is None:
            return None
        for rnd, slot, an, at, bn, bt, wn, _ in self.matches(cup, self.rounds(cup)):
            if wn is None and key in ((an, at), (bn, bt)):
                return cup, rnd, slot, (bn, bt) if (an, at) == key else (an, at)
        return None

    def knocked_out(self, cup, key):
        for _, _, an, at, bn, bt, wn, wt in self.matches(cup):
            if wn is not None and key in ((an, at), (bn, bt)) and (wn, wt) != key:
                return True
        return False

    def record(self, a, b, winner):
        """A battle between a and b ended with `winner`. True if it was a cup match."""
        match = self.current_match(a)
        if match is None or match[3] != b:
            return False
        cup, rnd, slot, _ = match
        self.set_winner(cup, rnd, slot, winner)
        log.info("%s round %d: %s beat %s", cup_name(cup), rnd, winner[0], (b if winner == a else a)[0])
        return True

    def unclaimed_prize(self, key):
        return self.db.execute(
            "SELECT cup, place FROM tour_entrants WHERE name = ? AND tid = ? AND place IN (1, 2) AND prize_claimed = 0"
            " ORDER BY cup LIMIT 1", key
        ).fetchone()

    def claim(self, key, cup):
        self.db.execute("UPDATE tour_entrants SET prize_claimed = 1 WHERE cup = ? AND name = ? AND tid = ?", (cup, *key))
        self.db.commit()

    def last_champion(self):
        return self.db.execute(
            "SELECT e.cup, e.name FROM tour_entrants e JOIN tour_cups c ON c.id = e.cup"
            " WHERE c.state = 'done' AND e.place = 1 ORDER BY e.cup DESC LIMIT 1"
        ).fetchone()


def bracket_order(size):
    """Seeds in bracket order, so 1 and 2 can only meet in the final."""
    order = [1]
    while len(order) < size:
        n = len(order) * 2
        order = [s for seed in order for s in (seed, n + 1 - seed)]
    return order


def _tours(server):
    tours = server.community.get("tournament")
    if tours is None:
        tours = server.community["tournament"] = Tournaments(server.league.db, server.league)
    return tours


def _short(name):
    return name or "-"


def bracket_rows(tours, cup):
    rows = []
    rounds = tours.rounds(cup)
    for rnd in range(1, rounds + 1):
        matches = tours.matches(cup, rnd)
        label = "FINAL" if len(matches) == 1 else "SEMIFINALS" if len(matches) == 2 else f"ROUND {rnd}"
        rows.append(f"- {label} -")
        for _, _, an, at, bn, bt, wn, wt in matches:
            if an is None or bn is None:
                rows.append(f"{_short(an or bn):<7} bye")
                continue
            mark_a = "▶" if (wn, wt) == (an, at) else " "
            mark_b = "▶" if (wn, wt) == (bn, bt) else " "
            rows.append(f"{mark_a}{an:<7} vs {mark_b}{bn}")
    return rows


@request(REQ_STATUS)
def status(server, player, args, data):
    tours = _tours(server)
    tours.start_if_due()
    key = player.key
    prize = tours.unclaimed_prize(key)
    if prize:
        cup, place = prize
        item, text = PRIZES[place]
        tours.claim(key, cup)
        what = "CHAMPION" if place == 1 else "RUNNER-UP"
        return Answer(cup_name(cup), [f"You were the {what}!", f"Here's your prize:", f"{text}."],
                      status=PRIZE, args=(cup, place, item))
    match = tours.current_match(key)
    if match:
        cup, rnd, _, (opp_name, opp_tid) = match
        online = any(p.key == (opp_name, opp_tid) for p in server.online_players())
        size = len(tours.matches(cup, rnd))
        label = "the FINAL" if size == 1 else "the SEMIFINALS" if size == 2 else f"round {rnd}"
        where = "They're online now." if online else "They're offline right now."
        return Answer(cup_name(cup), [f"You're in {label}!", f"Your opponent: {opp_name}.", where],
                      status=PLAYING, args=(cup, rnd))
    running = tours.running_cup()
    if running and tours.in_cup(running, key) and tours.knocked_out(running, key):
        status_out = OUT
    else:
        status_out = None
    cup = tours.open_cup()
    count = len(tours.entrants(cup))
    if tours.in_cup(cup, key):
        return Answer(cup_name(cup), [f"You're signed up. {count}/{SIZE}", "It starts when it's full,",
                                      "or soon after two have joined."], status=SIGNED_UP, args=(cup, count))
    champ = tours.last_champion()
    third = f"Last champion: {champ[1]}." if champ else f"The winner gets a MASTER BALL."
    rows = [f"Sign-ups are open. {count}/{SIZE}", "Battles are online matches.", third]
    if status_out:
        rows[0] = f"You're out of {cup_name(running)}."
    return Answer(cup_name(cup), rows, status=NOT_IN, args=(cup, count))


@request(REQ_JOIN)
def join(server, player, args, data):
    tours = _tours(server)
    ok = tours.join(player.key)
    cup = tours.open_cup()
    if not ok:
        return Answer(cup_name(cup), ["Sorry, this cup is full."], status=FULL)
    started = tours.running_cup()
    if started and tours.current_match(player.key):
        notify_matches(server, tours)
    return Answer(cup_name(started or cup), ["You're signed up!"], status=SIGNED_UP)


@request(REQ_LEAVE)
def leave(server, player, args, data):
    tours = _tours(server)
    tours.leave(player.key)
    return Answer(cup_name(tours.open_cup()), ["You've withdrawn."], status=NOT_IN)


@request(REQ_BRACKET)
def bracket(server, player, args, data):
    tours = _tours(server)
    cup = tours.running_cup()
    if cup is None:
        done = tours.last_champion()
        cup = done[0] if done else None
    if cup is None:
        cup = tours.open_cup()
        rows = [f"{name}" for name, _ in tours.entrants(cup)] or ["Nobody has signed up yet."]
        return Answer(f"{cup_name(cup)}: SIGN-UPS", rows)
    return Answer(f"{cup_name(cup)} BRACKET", bracket_rows(tours, cup)[:community.ROWS - 1])


def notify_matches(server, tours):
    """Tells everyone with a match to play who they face (the ROM shows it
    once, when the opponent changes)."""
    for p in server.online_players():
        match = tours.current_match(p.key)
        if match is None or not community.is_community(p):
            continue
        cup, rnd, _, (opp, _) = match
        data = bytes([cup & 0xFF, rnd]) + community.encode_name(opp, 8) + community.encode_name(cup_name(cup), 16)
        community.send_event(p, 2, data)  # COMMUNITY_EVENT_TOUR


@listen("battle_finished")
def on_battle(server, session):
    if session.kind != "battle":
        return
    a, b = session.host.key, session.guest.key
    ra, rb = session.reports.get(a), session.reports.get(b)
    if ra == "win" or rb == "loss":
        winner = a
    elif rb == "win" or ra == "loss":
        winner = b
    else:
        return  # a draw or no reports: play it again
    tours = _tours(server)
    if tours.record(a, b, winner):
        notify_matches(server, tours)


@listen("hello")
def on_hello(server, player):
    tours = _tours(server)
    if tours.current_match(player.key):
        notify_matches(server, tours)


def page(server):
    tours = _tours(server)
    cup = tours.running_cup()
    title = "No cup running"
    body = ""
    if cup is None:
        done = tours.last_champion()
        cup = done[0] if done else None
    if cup is not None:
        title = cup_name(cup)
        cols = []
        size = len(tours.matches(cup, 1))  # matches in round 1; each later round has half
        rnd = 1
        while size >= 1:
            cells = []
            played = tours.matches(cup, rnd)
            for _, _, an, at, bn, bt, wn, wt in played:
                def side(n, t):
                    if n is None:
                        return '<div class="p bye">bye</div>'
                    won = ' win' if (wn, wt) == (n, t) else ''
                    return f'<div class="p{won}">{html.escape(n)}</div>'
                cells.append(f'<div class="m">{side(an, at)}{side(bn, bt)}</div>')
            cells += ['<div class="m"><div class="p tbd">?</div><div class="p tbd">?</div></div>'] * (size - len(played))
            label = "Final" if size == 1 else "Semifinals" if size == 2 else "Quarterfinals" if size == 4 else f"Round {rnd}"
            cols.append(f'<div class="r"><h3>{label}</h3>{"".join(cells)}</div>')
            size //= 2
            rnd += 1
        body = f'<div class="b">{"".join(cols)}</div>'
        champ = tours.db.execute("SELECT name FROM tour_entrants WHERE cup = ? AND place = 1", (cup,)).fetchone()
        if champ:
            body += f"<p>Champion: <b>{html.escape(champ[0])}</b></p>"
    open_cup = tours.open_cup()
    names = ", ".join(html.escape(n) for n, _ in tours.entrants(open_cup)) or "nobody yet"
    body += f"<h2>Sign-ups for {cup_name(open_cup)}</h2><p>{len(tours.entrants(open_cup))}/{SIZE}: {names}</p>"
    return 200, "text/html; charset=utf-8", PAGE.format(title=html.escape(title), body=body)


PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="30">
<title>{title} - Pokimon Multi</title>
<style>
body {{ font-family: system-ui, sans-serif; margin: 0 auto; max-width: 60rem; padding: 1rem; }}
.b {{ display: flex; gap: 1.5rem; overflow-x: auto; }}
.r {{ display: flex; flex-direction: column; justify-content: space-around; min-width: 9rem; padding-top: 1.4rem; position: relative; }}
.r h3 {{ font-size: .9rem; color: #555; margin: 0; position: absolute; top: 0; }}
.m {{ border: 1px solid #bbb; border-radius: 4px; margin: .4rem 0; }}
.p {{ padding: .25rem .5rem; }}
.p + .p {{ border-top: 1px solid #ddd; }}
.win {{ font-weight: bold; background: #e6f4e6; }}
.bye, .tbd {{ color: #999; font-style: italic; }}
.r h3 + .m {{ margin-top: 0; }}
</style></head><body>
<p><a href="/">League</a></p>
<h1>{title}</h1>
{body}
</body></html>
"""

community.WEB_ROUTES["/tournament"] = ("Tournament", page)
