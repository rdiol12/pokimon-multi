"""The weekly Rover League (docs/rovers.md): every week the eight strongest
roaming trainers (rovers.py) play a knockout bracket, quarterfinals to the
final, on the shared server clock. Players watch it on the quest boards, can
watch any rover-vs-rover match still to be played (what they see is the
result), and can enter: a player takes the seat of one of the lowest seeds
before the quarterfinals end, and plays their matches in their own game.

The week starts on Monday 00:00 UTC. Each round has a deadline: the
quarterfinals end on Wednesday, the semifinals on Friday, the final on
Sunday. At a deadline every match still open is decided: two rovers by
strength (as their fights on the map are), a player who didn't play loses.

ROVER LEAGUE INFO: the bracket. First row: week|round (0-2, 3 when it's
over)|hours left in the round|my state|my match id|my opponent|open to enter.
My state: 0 not in, 1 my match is waiting for me, 2 in and waiting for the
next round, 3 knocked out, 4 champion. Then one row per match: round|slot|
seat A's name|seat B's name|winner (0, 1 A, 2 B)|match id|watchable (1: two
rovers, not played yet).
ROVER LEAGUE ENTER: takes a seat. Answer: match id|opponent.
ROVER LEAGUE FIGHT: the player's match: match id|round|opponent, NUL, the
opponent's Rover.
ROVER LEAGUE RESULT <match> <won>: the player's match was played.
ROVER LEAGUE WATCH <match>: match id|round, NUL, both Rovers.
ROVER LEAGUE SEEN <match> <a won>: a watched match ended like that.
ROVER LEAGUE HALL: the champions, newest first (Laurel City's Hall of
Champions). First row: how many; then weeks ago (0 this week)|name.

The league is played at the Rover League Stadium in Laurel City: the
week's rovers travel there (bound(), used by rovers.Sim) and stay until
they're knocked out; the champion stays until the week is over.
"""

WEEK = 7 * 86400
MONDAY = 4 * 86400          # 1970-01-01 was a Thursday
ROUND_ENDS = (2 * 86400, 4 * 86400, 6 * 86400)
ROUND_NAMES = ("quarterfinal", "semifinal", "final")
SEATS = 8
MAX_PLAYERS = 4             # they take seats 7, 6, 5, 4 (the lowest seeds)
QUARTERS = ((0, 7), (3, 4), (1, 6), (2, 5))
CHAMPION_MONEY = 5000

NOT_IN, MY_MATCH, WAITING, OUT, CHAMPION = range(5)


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS rover_league_seats (
            week INTEGER NOT NULL, seat INTEGER NOT NULL,
            rover INTEGER, name TEXT, tid INTEGER,
            PRIMARY KEY (week, seat)
        );
        CREATE TABLE IF NOT EXISTS rover_league_matches (
            id INTEGER PRIMARY KEY,
            week INTEGER NOT NULL, round INTEGER NOT NULL, slot INTEGER NOT NULL,
            a INTEGER NOT NULL, b INTEGER NOT NULL,
            winner INTEGER, how TEXT
        );
        """
    )


def week_of(t):
    return int((t - MONDAY) // WEEK)


def week_start(week):
    return week * WEEK + MONDAY


class League:
    def __init__(self, db, R):
        self.db, self.R = db, R  # R: the rovers module

    # Seats ---------------------------------------------------------------------

    def seats(self, week):
        """seat -> (rover id or None, player key or None)"""
        return {s: (r, (n, t) if r is None else None) for s, r, n, t in self.db.execute(
            "SELECT seat, rover, name, tid FROM rover_league_seats WHERE week = ?", (week,))}

    def seat_name(self, week, seat, seats=None):
        seats = seats or self.seats(week)
        rover, player = seats[seat]
        return self.R.CAST[rover]["name"] if rover is not None else player[0]

    def ensure(self, week):
        if self.db.execute("SELECT 1 FROM rover_league_seats WHERE week = ?", (week,)).fetchone():
            return
        self.R.ensure_rovers(self.db)
        rovers = self.R.load_all(self.db)
        rovers.sort(key=lambda r: (-r.badges, -r.strength(), r.id))
        for seat, r in enumerate(rovers[:SEATS]):
            self.db.execute("INSERT INTO rover_league_seats (week, seat, rover) VALUES (?, ?, ?)",
                            (week, seat, r.id))
        for slot, (a, b) in enumerate(QUARTERS):
            self.db.execute("INSERT INTO rover_league_matches (week, round, slot, a, b) VALUES (?, 0, ?, ?, ?)",
                            (week, slot, a, b))
        names = ", ".join(r.name for r in rovers[:SEATS])
        self.R.news(self.db, f"This week's Rover League: {names}.")

    def matches(self, week):
        return self.db.execute("SELECT id, round, slot, a, b, winner, how FROM rover_league_matches"
                               " WHERE week = ? ORDER BY round, slot", (week,)).fetchall()

    def round_now(self, week):
        """The round being played (3 once the final is decided)."""
        rows = self.matches(week)
        for r in range(3):
            mine = [m for m in rows if m[1] == r]
            if not mine or any(m[5] is None for m in mine):
                return r
        return 3

    # Deciding matches ------------------------------------------------------------

    def decide(self, week, match, winner_seat, how):
        mid, rnd, slot, a, b, winner, _ = match
        if winner is not None:
            return
        self.db.execute("UPDATE rover_league_matches SET winner = ?, how = ? WHERE id = ?", (winner_seat, how, mid))
        seats = self.seats(week)
        loser_seat = b if winner_seat == a else a
        wr, wp = seats[winner_seat]
        lr, lp = seats[loser_seat]
        wname, lname = self.seat_name(week, winner_seat, seats), self.seat_name(week, loser_seat, seats)
        if wr is not None and lr is not None:
            w, l = self.R.load(self.db, wr), self.R.load(self.db, lr)
            self.R.fight(self.db, w, l, True)
            w.save(self.db)
            l.save(self.db)
        elif wr is not None or lr is not None:
            r = self.R.load(self.db, wr if wr is not None else lr)
            if wr is not None:
                r.wins += 1
            else:
                r.losses += 1
                r.hurt += 1
            r.save(self.db)
        if how != "forfeit":
            self.R.news(self.db, f"Rover League {ROUND_NAMES[rnd]}: {wname} beat {lname}.")
        else:
            self.R.news(self.db, f"Rover League {ROUND_NAMES[rnd]}: {wname} goes through ({lname} didn't play).")
        if rnd == 2:
            self.R.news(self.db, f"{wname} won this week's Rover League!")
            if wr is not None:
                w = self.R.load(self.db, wr)
                w.life["money"] += CHAMPION_MONEY
                w.save(self.db)
        self.next_round(week)

    def next_round(self, week):
        rows = self.matches(week)
        for rnd in (0, 1):
            done = [m for m in rows if m[1] == rnd]
            if len(done) != 4 // (2 ** rnd) or any(m[5] is None for m in done):
                return
            if any(m[1] == rnd + 1 for m in rows):
                continue
            winners = [m[5] for m in sorted(done, key=lambda m: m[2])]
            for slot in range(len(winners) // 2):
                self.db.execute("INSERT INTO rover_league_matches (week, round, slot, a, b) VALUES (?, ?, ?, ?, ?)",
                                (week, rnd + 1, slot, winners[2 * slot], winners[2 * slot + 1]))
            rows = self.matches(week)

    def advance(self, t):
        """Plays out every round whose deadline has passed. Returns the week."""
        week = week_of(t)
        self.ensure(week)
        start = week_start(week)
        for rnd in range(3):
            if t < start + ROUND_ENDS[rnd]:
                break
            seats = self.seats(week)
            for m in self.matches(week):
                if m[1] != rnd or m[5] is not None:
                    continue
                a, b = m[3], m[4]
                ra, rb = seats[a][0], seats[b][0]
                if ra is not None and rb is not None:
                    sa = self.R.load(self.db, ra).strength()
                    sb = self.R.load(self.db, rb).strength()
                    won_a = self.R.rng.random() < sa / max(sa + sb, 1)
                    self.decide(week, m, a if won_a else b, "sim")
                elif ra is None and rb is None:
                    self.decide(week, m, a if self.R.rng.random() < 0.5 else b, "forfeit")
                else:
                    self.decide(week, m, a if ra is not None else b, "forfeit")
        return week

    # A player's place in it ------------------------------------------------------

    def my_seat(self, week, key):
        row = self.db.execute("SELECT seat FROM rover_league_seats WHERE week = ? AND rover IS NULL"
                              " AND name = ? AND tid = ?", (week, *key)).fetchone()
        return row[0] if row else None

    def my_match(self, week, seat):
        for m in self.matches(week):
            if m[5] is None and seat in (m[3], m[4]):
                return m
        return None

    def state(self, week, key):
        seat = self.my_seat(week, key)
        if seat is None:
            return NOT_IN, None
        m = self.my_match(week, seat)
        if m is not None:
            return MY_MATCH, m
        rows = self.matches(week)
        if any(r[5] is not None and seat in (r[3], r[4]) and r[5] != seat for r in rows):
            return OUT, None
        final = [r for r in rows if r[1] == 2]
        if final and final[0][5] == seat:
            return CHAMPION, None
        return WAITING, None

    def can_enter(self, week, t, key):
        if t >= week_start(week) + ROUND_ENDS[0] or self.my_seat(week, key) is not None:
            return None
        players = self.db.execute("SELECT COUNT(*) FROM rover_league_seats WHERE week = ? AND rover IS NULL",
                                  (week,)).fetchone()[0]
        if players >= MAX_PLAYERS:
            return None
        seat = SEATS - 1 - players
        m = self.my_match(week, seat)
        return seat if m is not None and m[1] == 0 else None

    def enter(self, week, t, key):
        seat = self.can_enter(week, t, key)
        if seat is None:
            raise self.R.Refused("The league can't take\nanyone else this week.")
        old = self.seat_name(week, seat)
        self.db.execute("UPDATE rover_league_seats SET rover = NULL, name = ?, tid = ? WHERE week = ? AND seat = ?",
                        (*key, week, seat))
        self.R.news(self.db, f"{key[0]} entered the Rover League in place of {old}.")
        return seat


def bound(db, t):
    """The rovers that should be in Laurel City now: this week's seated
    rovers not knocked out yet."""
    week = week_of(t)
    rows = db.execute("SELECT seat, rover FROM rover_league_seats WHERE week = ? AND rover IS NOT NULL",
                      (week,)).fetchall()
    if not rows:
        return set()
    lost = {b if w == a else a for a, b, w in db.execute(
        "SELECT a, b, winner FROM rover_league_matches WHERE week = ? AND winner IS NOT NULL", (week,))}
    return {rover for seat, rover in rows if seat not in lost}


def champions(league, t, most=6):
    """(weeks ago, name) of the latest champions, newest first."""
    week = week_of(t)
    out = []
    for w, winner in league.db.execute(
            "SELECT week, winner FROM rover_league_matches WHERE round = 2 AND winner IS NOT NULL"
            " AND week <= ? ORDER BY week DESC LIMIT ?", (week, most)):
        out.append((week - w, league.seat_name(w, winner)))
    return out


def opponent(league, week, match, seat):
    other = match[4] if match[3] == seat else match[3]
    return other, league.seat_name(week, other)


def cmd_league(ctx, args, data, R):
    league = League(ctx.db, R)
    t = R.now()
    week = league.advance(t)
    sub = args[1] if len(args) > 1 else "INFO"
    if sub == "HALL":
        rows = champions(league, t)
        return R.text([(len(rows),)] + rows)
    if sub == "INFO":
        rnd = league.round_now(week)
        left = 0 if rnd == 3 else max(0, int((week_start(week) + ROUND_ENDS[rnd] - t) // 3600))
        state, m = league.state(week, ctx.key)
        seat = league.my_seat(week, ctx.key)
        foe = opponent(league, week, m, seat)[1] if m else "-"
        rows = [(week, rnd, left, state, m[0] if m else 0, foe,
                 int(league.can_enter(week, t, ctx.key) is not None))]
        seats = league.seats(week)
        for mid, r, slot, a, b, winner, _ in league.matches(week):
            watchable = int(winner is None and seats[a][0] is not None and seats[b][0] is not None)
            rows.append((r, slot, league.seat_name(week, a, seats), league.seat_name(week, b, seats),
                         0 if winner is None else (1 if winner == a else 2), mid, watchable))
        return R.text(rows)
    if sub == "ENTER":
        seat = league.enter(week, t, ctx.key)
        m = league.my_match(week, seat)
        return R.text([(m[0], opponent(league, week, m, seat)[1])])
    if sub == "FIGHT":
        seat = league.my_seat(week, ctx.key)
        m = league.my_match(week, seat) if seat is not None else None
        if m is None:
            raise R.Refused("You have no match to play\nright now.")
        other, name = opponent(league, week, m, seat)
        rover = league.seats(week)[other][0]
        if rover is None:
            raise R.Refused("Your opponent is another\nplayer. Battle them online!")
        return R.text([(m[0], m[1], name)]) + b"\0" + R.load(ctx.db, rover).wire()
    if sub == "RESULT":
        mid, won = int(args[2]), int(args[3])
        seat = league.my_seat(week, ctx.key)
        m = league.my_match(week, seat) if seat is not None else None
        if m is None or m[0] != mid:
            raise R.Refused("That match is over.")
        other, _ = opponent(league, week, m, seat)
        league.decide(week, m, seat if won else other, "played")
        return b""
    if sub == "WATCH":
        mid = int(args[2])
        m = next((m for m in league.matches(week) if m[0] == mid), None)
        seats = league.seats(week)
        if m is None or m[5] is not None or seats[m[3]][0] is None or seats[m[4]][0] is None:
            raise R.Refused("That match has been played.")
        a, b = R.load(ctx.db, seats[m[3]][0]), R.load(ctx.db, seats[m[4]][0])
        return R.text([(m[0], m[1])]) + b"\0" + a.wire() + b.wire()
    if sub == "SEEN":
        mid, a_won = int(args[2]), int(args[3])
        m = next((m for m in league.matches(week) if m[0] == mid), None)
        seats = league.seats(week)
        if m is not None and m[5] is None and seats[m[3]][0] is not None and seats[m[4]][0] is not None:
            league.decide(week, m, m[3] if a_won else m[4], "watched")
        return b""
    raise ValueError("bad LEAGUE request")
