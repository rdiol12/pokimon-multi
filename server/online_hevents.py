"""Player-hosted events (HUB > PLAYER EVENTS, docs/online.md).

A player hosts a catch hunt or a tournament and puts up the prize: money,
items from their bag, or both. The game takes the prize away when the event
is posted and the server holds it (escrow) until the event ends:

- a CATCH HUNT is won by the first player who joined it and then catches
  the target species before time runs out. Nobody catches one: the host
  gets the prize back.
- a TOURNAMENT takes 4 or 8 players. It starts when it's full, or when
  sign-ups close with two or more players in it (fewer: the prize goes
  back). The bracket is seeded by ladder rating like the community cups
  (server/community_tournament.py); its matches are ordinary online battles
  between the two players. A round has ROUND_SECONDS: a match nobody played
  by then goes to the higher seed. The champion takes the prize.

Every open or running event is on the public events board, and everyone
online hears when one is posted and when it's won. Prizes and refunds wait
in online_hevent_payouts until the player opens PLAYER EVENTS (VIEW).

Hosting takes a few requests: NEW starts a draft (kept on the player, in
memory), TARGET / SIZE / TIME / MONEY / ITEM fill it in, and POST checks it
and puts it up. A host can call an event off while nobody has joined it.
"""

import logging
import os

from community_tournament import bracket_order
from online import Refused, answer, board
from online_ids import ITEMS, SPECIES, SPECIES_NAMES
from online_pquests import item_text

log = logging.getLogger("netplay.hevents")

HUNT, CUP = 0, 1
KIND_NAMES = {HUNT: "HUNT", CUP: "CUP"}
TARGETS = ["ZIGZAGOON", "WURMPLE", "LOTAD", "SEEDOT", "TAILLOW", "WINGULL", "RALTS", "SURSKIT", "SHROOMISH",
           "SLAKOTH", "WHISMUR", "MAKUHITA", "ARON", "ELECTRIKE", "ROSELIA", "SPINDA", "TRAPINCH", "SWABLU",
           "BARBOACH", "SHUPPET", "TROPIUS", "ABSOL", "FEEBAS", "BAGON"]
SIZES = [4, 8]
TIMES = [(30 * 60, "30 MINUTES"), (3600, "1 HOUR"), (3 * 3600, "3 HOURS"), (24 * 3600, "1 DAY")]
MONEY = [0, 1000, 5000, 10000, 50000, 100000, 500000]
ROUND_SECONDS = float(os.environ.get("NETPLAY_HEVENT_ROUND_SECONDS", str(24 * 3600)))
MAX_OPEN = 12        # events on the board at once
ITEM_NAMES = {v: k for k, v in ITEMS.items()}

LIVE = ("open", "signup", "running")


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_hevents (
            id INTEGER PRIMARY KEY, kind INTEGER NOT NULL,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            target INTEGER NOT NULL DEFAULT 0,     -- species of a hunt
            size INTEGER NOT NULL DEFAULT 0,       -- players in a cup
            money INTEGER NOT NULL DEFAULT 0, item INTEGER NOT NULL DEFAULT 0, count INTEGER NOT NULL DEFAULT 0,
            state TEXT NOT NULL,                   -- open (hunt), signup, running (cup), won, ended
            ends REAL NOT NULL,                    -- the hunt's end, the cup's sign-up close
            round_ends REAL,
            winner_name TEXT, winner_tid INTEGER
        );
        CREATE TABLE IF NOT EXISTS online_hevent_players (
            event INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, seed INTEGER,
            PRIMARY KEY (event, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_hevent_matches (
            event INTEGER NOT NULL, round INTEGER NOT NULL, slot INTEGER NOT NULL,
            a_name TEXT, a_tid INTEGER, b_name TEXT, b_tid INTEGER,
            winner_name TEXT, winner_tid INTEGER,
            PRIMARY KEY (event, round, slot)
        );
        CREATE TABLE IF NOT EXISTS online_hevent_payouts (
            id INTEGER PRIMARY KEY, name TEXT NOT NULL, tid INTEGER NOT NULL,
            money INTEGER NOT NULL, item INTEGER NOT NULL, count INTEGER NOT NULL, why TEXT NOT NULL
        );
        """
    )


# --- words -------------------------------------------------------------------

def species_name(species):
    return SPECIES_NAMES.get(species, "?").replace("_", " ")


def prize_text(money, item, count):
    parts = []
    if money:
        parts.append(f"${money}")
    if item and count:
        parts.append(item_text(item, count))
    return " and ".join(parts) or "nothing"


def prize_short(money, item, count):
    if money and item:
        return f"${money}+"
    if money:
        return f"${money}"
    return ITEM_NAMES.get(item, "?").replace("_", " ")[:10]


def time_left(seconds):
    seconds = max(0, int(seconds))
    if seconds >= 3600:
        return f"{seconds // 3600}h"
    return f"{max(1, seconds // 60)}m"


def event_name(ev):
    return f"{ev['name']}'s {'HUNT' if ev['kind'] == HUNT else 'CUP'}"


COLUMNS = "id, kind, name, tid, target, size, money, item, count, state, ends, round_ends, winner_name, winner_tid"


def as_event(row):
    return dict(zip([c.strip() for c in COLUMNS.split(",")], row)) if row else None


def get_event(db, eid):
    return as_event(db.execute(f"SELECT {COLUMNS} FROM online_hevents WHERE id = ?", (eid,)).fetchone())


def live_events(db):
    return [as_event(r) for r in db.execute(
        f"SELECT {COLUMNS} FROM online_hevents WHERE state IN {LIVE} ORDER BY id")]


def players(db, eid):
    return [(n, t) for n, t in db.execute(
        "SELECT name, tid FROM online_hevent_players WHERE event = ? ORDER BY COALESCE(seed, 99), rowid", (eid,))]


def joined(db, eid, key):
    return db.execute("SELECT 1 FROM online_hevent_players WHERE event = ? AND name = ? AND tid = ?",
                      (eid, *key)).fetchone() is not None


def is_host(ev, key):
    return (ev["name"], ev["tid"]) == tuple(key)


# --- the clock ---------------------------------------------------------------

def pay(db, key, money, item, count, why):
    db.execute("INSERT INTO online_hevent_payouts (name, tid, money, item, count, why) VALUES (?, ?, ?, ?, ?, ?)",
               (*key, money, item, count, why))


def refund(db, online, ev, why):
    db.execute("UPDATE online_hevents SET state = 'ended' WHERE id = ?", (ev["id"],))
    key = (ev["name"], ev["tid"])
    pay(db, key, ev["money"], ev["item"], ev["count"], f"Your {event_name(ev)} {why}. Your prize came back:")
    online.notify(key, f"Your {event_name(ev)} {why}. Pick up your prize in HUB > PLAYER EVENTS.")
    for p in players(db, ev["id"]):
        online.notify(p, f"{event_name(ev)} {why}.")


def award(db, online, ev, winner, how):
    db.execute("UPDATE online_hevents SET state = 'won', winner_name = ?, winner_tid = ? WHERE id = ?",
               (*winner, ev["id"]))
    prize = prize_text(ev["money"], ev["item"], ev["count"])
    pay(db, winner, ev["money"], ev["item"], ev["count"], f"You won {event_name(ev)}! Your prize:")
    online.notify(winner, f"You won {event_name(ev)}! Pick up {prize} in HUB > PLAYER EVENTS.")
    online.announce(f"{winner[0]} {how} and won {event_name(ev)}!", exclude=winner)
    log.info("%s won by %s", event_name(ev), winner[0])


def tick(db, online):
    """Ends hunts whose time is up, starts or calls off cups whose sign-ups
    closed, and settles cup rounds that ran out of time."""
    now = online.now()
    for ev in live_events(db):
        if ev["state"] == "open" and now >= ev["ends"]:
            refund(db, online, ev, "ended with no winner")
        elif ev["state"] == "signup" and now >= ev["ends"]:
            if len(players(db, ev["id"])) >= 2:
                start_cup(db, online, ev)
            else:
                refund(db, online, ev, "was called off: too few players signed up")
        elif ev["state"] == "running" and ev["round_ends"] is not None and now >= ev["round_ends"]:
            rnd = rounds(db, ev["id"])
            for _, slot, an, at, bn, bt, wn, _ in matches(db, ev["id"], rnd):
                if wn is None:  # nobody played it: the higher seed goes on
                    set_winner(db, online, ev, rnd, slot, (an, at), late=True)


# --- cups ---------------------------------------------------------------------

def rounds(db, eid):
    return db.execute("SELECT MAX(round) FROM online_hevent_matches WHERE event = ?", (eid,)).fetchone()[0] or 0


def matches(db, eid, rnd):
    return db.execute("SELECT round, slot, a_name, a_tid, b_name, b_tid, winner_name, winner_tid"
                      " FROM online_hevent_matches WHERE event = ? AND round = ? ORDER BY slot", (eid, rnd)).fetchall()


def start_cup(db, online, ev):
    eid = ev["id"]
    entrants = players(db, eid)
    entrants.sort(key=lambda k: -online.server.league.rating(k))
    for seed, key in enumerate(entrants, 1):
        db.execute("UPDATE online_hevent_players SET seed = ? WHERE event = ? AND name = ? AND tid = ?",
                   (seed, eid, *key))
    size = 2
    while size < len(entrants):
        size *= 2
    slots = entrants + [None] * (size - len(entrants))
    order = bracket_order(size)
    for slot in range(size // 2):
        a, b = slots[order[slot * 2] - 1], slots[order[slot * 2 + 1] - 1]
        db.execute("INSERT INTO online_hevent_matches (event, round, slot, a_name, a_tid, b_name, b_tid)"
                   " VALUES (?, 1, ?, ?, ?, ?, ?)", (eid, slot, *(a or (None, None)), *(b or (None, None))))
    db.execute("UPDATE online_hevents SET state = 'running' WHERE id = ?", (eid,))
    online.announce(f"{event_name(ev)} has begun with {len(entrants)} players!")
    new_round(db, online, ev, 1)


def round_label(n_matches, rnd):
    return "the FINAL" if n_matches == 1 else "the SEMIFINALS" if n_matches == 2 else f"round {rnd}"


def new_round(db, online, ev, rnd):
    db.execute("UPDATE online_hevents SET round_ends = ? WHERE id = ?", (online.now() + ROUND_SECONDS, ev["id"]))
    played = matches(db, ev["id"], rnd)
    for _, slot, an, at, bn, bt, wn, _ in played:
        if an is None or bn is None:
            continue
        label = round_label(len(played), rnd)
        online.notify((an, at), f"{event_name(ev)}: {label}! Battle {bn} online to go on.")
        online.notify((bn, bt), f"{event_name(ev)}: {label}! Battle {an} online to go on.")
    for _, slot, an, at, bn, bt, wn, _ in played:  # byes
        if wn is None and (an is None) != (bn is None):
            set_winner(db, online, ev, rnd, slot, (an, at) if an else (bn, bt))


def set_winner(db, online, ev, rnd, slot, key, late=False):
    eid = ev["id"]
    db.execute("UPDATE online_hevent_matches SET winner_name = ?, winner_tid = ? WHERE event = ? AND round = ?"
               " AND slot = ?", (*key, eid, rnd, slot))
    if late:
        online.notify(key, f"{event_name(ev)}: your match ran out of time. You go on as the higher seed!")
    played = matches(db, eid, rnd)
    if any(m[6] is None for m in played):
        return
    if len(played) == 1:
        award(db, online, ev, key, "beat everyone")
        return
    for i in range(0, len(played), 2):
        a, b = played[i], played[i + 1]
        db.execute("INSERT INTO online_hevent_matches (event, round, slot, a_name, a_tid, b_name, b_tid)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?)", (eid, rnd + 1, i // 2, a[6], a[7], b[6], b[7]))
    new_round(db, online, ev, rnd + 1)


def current_match(db, ev, key):
    rnd = rounds(db, ev["id"])
    for _, slot, an, at, bn, bt, wn, _ in matches(db, ev["id"], rnd):
        if wn is None and tuple(key) in ((an, at), (bn, bt)):
            return rnd, slot, (bn, bt) if (an, at) == tuple(key) else (an, at)
    return None


def on_battle(server, session):
    """An online battle ended: a cup match between the two moves the winner on."""
    if session.kind != "battle" or not hasattr(server, "online"):
        return
    a, b = session.host.key, session.guest.key
    ra, rb = session.reports.get(a), session.reports.get(b)
    if ra == "win" or rb == "loss":
        winner = a
    elif rb == "win" or ra == "loss":
        winner = b
    else:
        return
    db, online = server.league.db, server.online
    tick(db, online)
    for ev in live_events(db):
        if ev["state"] != "running":
            continue
        match = current_match(db, ev, a)
        if match and match[2] == tuple(b):
            rnd, slot, _ = match
            loser = b if winner == a else a
            online.notify(loser, f"{event_name(ev)}: {winner[0]} beat you. Thanks for playing!")
            set_winner(db, online, ev, rnd, slot, winner)
    db.commit()


def install(server):
    finish = server.finish_battle

    def finish_battle(session):
        finish(session)
        on_battle(server, session)

    server.finish_battle = finish_battle


def bracket_lines(db, ev):
    lines = []
    for rnd in range(1, rounds(db, ev["id"]) + 1):
        played = matches(db, ev["id"], rnd)
        label = "FINAL" if len(played) == 1 else "SEMIFINALS" if len(played) == 2 else f"ROUND {rnd}"
        lines.append([f"- {label} -", ""])
        for _, _, an, at, bn, bt, wn, wt in played:
            if an is None or bn is None:
                lines.append([an or bn, "bye", ""])
                continue
            mark = lambda n, t: (">" if (wn, wt) == (n, t) else "") + n  # noqa: E731
            lines.append([mark(an, at), "vs", mark(bn, bt)])
    return lines[-8:]


# --- the hunt -----------------------------------------------------------------

def on_catch(ctx, args):
    """CATCH species shiny mapGroup mapNum fished"""
    species = (args + [0])[0]
    db = ctx.db
    tick(db, ctx.online)
    for ev in live_events(db):
        if ev["kind"] == HUNT and ev["state"] == "open" and ev["target"] == species and joined(db, ev["id"], ctx.key):
            award(db, ctx.online, ev, tuple(ctx.key), f"caught a {species_name(species)}")


def status(ctx):
    tick(ctx.db, ctx.online)
    return []


# --- requests -------------------------------------------------------------------

def chosen(args, i=1):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def draft_of(ctx):
    draft = getattr(ctx.player, "hevent_draft", None)
    if draft is None:
        raise Refused("That didn't work. Please start again.")
    return draft


def describe(db, ev, now):
    prize = prize_text(ev["money"], ev["item"], ev["count"])
    n = len(players(db, ev["id"]))
    if ev["kind"] == HUNT:
        return (f"{ev['name']}'s CATCH HUNT: the first to catch a {species_name(ev['target'])} wins!"
                f"^Prize: {prize}.^{n} hunting, {time_left(ev['ends'] - now)} left.")
    what = f"{ev['name']}'s TOURNAMENT for {ev['size']} players. The champion wins!^Prize: {prize}.^"
    if ev["state"] == "signup":
        return what + f"{n}/{ev['size']} signed up. Sign-ups close in {time_left(ev['ends'] - now)}."
    return what + f"Under way with {n} players."


def summary(draft):
    prize = prize_text(draft.get("money", 0), draft.get("item", 0), draft.get("count", 0))
    if draft["kind"] == HUNT:
        what = f"A CATCH HUNT for {species_name(draft['target'])}, for {TIMES[draft['time']][1].lower()}."
    else:
        what = f"A TOURNAMENT for {draft['size']}, sign-ups open for {TIMES[draft['time']][1].lower()}."
    return f"{what}^Prize: {prize}."


def cmd_hevent(ctx, args, data):
    db, key, online = ctx.db, tuple(ctx.key), ctx.online
    word = args[0]
    tick(db, online)
    now = online.now()

    if word == "VIEW":
        # Prizes and refunds waiting for the player go first.
        owed = db.execute("SELECT id, money, item, count, why FROM online_hevent_payouts WHERE name = ? AND tid = ?"
                          " ORDER BY id", key).fetchall()
        if not owed:
            return answer()
        rows, said = [], []
        for pid, money, item, count, why in owed:
            db.execute("DELETE FROM online_hevent_payouts WHERE id = ?", (pid,))
            if money:
                rows.append(["+MONEY", money])
            if item and count:
                rows.append(["+ITEM", item, count])
            said.append(f"{why} {prize_text(money, item, count)}!")
        return answer("^".join(said[:3]), code=1, rows=rows)

    if word == "BOARD":
        lines = []
        for ev in live_events(db):
            if ev["kind"] == HUNT:
                what, left = f"HUNT {species_name(ev['target'])}", ev["ends"] - now
            elif ev["state"] == "signup":
                what, left = f"CUP {len(players(db, ev['id']))}/{ev['size']}", ev["ends"] - now
            else:
                what, left = "CUP PLAYING", (ev["round_ends"] or now) - now
            mine = is_host(ev, key) or joined(db, ev["id"], key)
            lines.append([("*" if mine else "") + ev["name"], what,
                          prize_short(ev["money"], ev["item"], ev["count"]), time_left(left)])
        return answer(rows=board("PLAYER EVENTS", [(6, "HOST"), (50, "EVENT"), (134, "PRIZE"), (182, "LEFT")],
                                 lines[:8]))

    if word == "LIST":
        rows = []
        for ev in live_events(db):
            if ev["kind"] == HUNT:
                label = f"{ev['name']}: {species_name(ev['target'])}"
            else:
                label = f"{ev['name']}: CUP {len(players(db, ev['id']))}/{ev['size']}"
            rows.append([label, ev["id"]])
        if not rows:
            raise Refused("No events are on right now. Why not host one?")
        return answer("Which event?", rows=rows[:20])

    if word == "SHOW":
        ev = get_event(db, chosen(args))
        if ev is None or ev["state"] not in LIVE:
            raise Refused("That event is over.")
        if ev["kind"] == CUP and ev["state"] == "running":
            return answer(describe(db, ev, now), code=3,
                          rows=board(f"{event_name(ev)}", [(6, ""), (78, ""), (100, "")], bracket_lines(db, ev)))
        # code: 0 can join, 1 joined, 2 the host's own
        code = 2 if is_host(ev, key) else 1 if joined(db, ev["id"], key) else 0
        return answer(describe(db, ev, now), code=code)

    if word == "JOIN":
        ev = get_event(db, chosen(args))
        if ev is None or ev["state"] not in ("open", "signup"):
            raise Refused("You can't join that one now.")
        if is_host(ev, key):
            raise Refused("That's your own event!")
        if not joined(db, ev["id"], key):
            if ev["kind"] == CUP and len(players(db, ev["id"])) >= ev["size"]:
                raise Refused("Sorry, it's full.")
            db.execute("INSERT INTO online_hevent_players (event, name, tid) VALUES (?, ?, ?)", (ev["id"], *key))
            online.notify((ev["name"], ev["tid"]),
                          f"{ctx.player.name} joined your {'HUNT' if ev['kind'] == HUNT else 'TOURNAMENT'}!")
        if ev["kind"] == HUNT:
            return answer(f"You joined the hunt! Catch a {species_name(ev['target'])} before anyone else.")
        if len(players(db, ev["id"])) >= ev["size"]:
            start_cup(db, online, ev)
            return answer("You're in! That fills it: the tournament starts now.")
        return answer("You're signed up! You'll hear when the tournament starts.")

    if word == "LEAVE":
        ev = get_event(db, chosen(args))
        if ev is None or ev["state"] not in ("open", "signup"):
            raise Refused("You can't leave that one now.")
        db.execute("DELETE FROM online_hevent_players WHERE event = ? AND name = ? AND tid = ?", (ev["id"], *key))
        return answer(f"You left {event_name(ev)}.")

    if word == "MINE":
        hosting = [ev for ev in live_events(db) if is_host(ev, key)]
        lines = []
        for ev in live_events(db):
            if joined(db, ev["id"], key):
                if ev["state"] == "running":
                    match = current_match(db, ev, key)
                    lines.append(f"{event_name(ev)}: " + (f"battle {match[2][0]} next!" if match else "you're out."))
                else:
                    lines.append(f"You're in {event_name(ev)}.")
        if hosting:
            ev = hosting[0]
            lines.insert(0, describe(db, ev, now))
            # code 1: the host can call it off (nobody has joined yet)
            return answer("^".join(lines), code=1 if not players(db, ev["id"]) and ev["state"] != "running" else 0)
        return answer("^".join(lines) or "You're not in any events.")

    if word == "CANCEL":
        ev = next((e for e in live_events(db) if is_host(e, key)), None)
        if ev is None or ev["state"] == "running" or players(db, ev["id"]):
            raise Refused("Players have joined, so it has to go on.")
        db.execute("UPDATE online_hevents SET state = 'ended' WHERE id = ?", (ev["id"],))
        rows = [["+MONEY", ev["money"]]] if ev["money"] else []
        if ev["item"] and ev["count"]:
            rows.append(["+ITEM", ev["item"], ev["count"]])
        return answer(f"You called it off and got back {prize_text(ev['money'], ev['item'], ev['count'])}.",
                      rows=rows)

    # --- hosting ---

    if word == "NEW":
        kind = chosen(args)
        if kind not in KIND_NAMES:
            raise ValueError("bad HEVENT kind")
        if any(is_host(ev, key) for ev in live_events(db)):
            raise Refused("You're already hosting an event. One at a time!")
        if len(live_events(db)) >= MAX_OPEN:
            raise Refused("The events board is full. Try again later.")
        ctx.player.hevent_draft = {"kind": kind}
        if kind == HUNT:
            return answer("Which POKéMON will they hunt?", rows=[[t, SPECIES[t]] for t in TARGETS])
        return answer("How many players?", rows=[[f"{n} PLAYERS", n] for n in SIZES])

    if word == "TARGET":
        draft = draft_of(ctx)
        species = chosen(args)
        if species not in [SPECIES[t] for t in TARGETS]:
            raise Refused("That can't be hunted.")
        draft["target"] = species
        return answer("How long will it run?", rows=[[label, i] for i, (_, label) in enumerate(TIMES)])

    if word == "SIZE":
        draft = draft_of(ctx)
        size = chosen(args)
        if size not in SIZES:
            raise Refused("That didn't work.")
        draft["size"] = size
        return answer("How long are sign-ups open?", rows=[[label, i] for i, (_, label) in enumerate(TIMES)])

    if word == "TIME":
        # TIME chosen money
        draft = draft_of(ctx)
        t, money = chosen(args), chosen(args, 2)
        if not 0 <= t < len(TIMES):
            raise Refused("That didn't work.")
        draft["time"] = t
        rows = [["NO MONEY" if m == 0 else f"${m}", m] for m in MONEY if m <= money]
        return answer("How much money is the prize?", rows=rows)

    if word == "MONEY":
        draft = draft_of(ctx)
        m = chosen(args)
        if m not in MONEY:
            raise Refused("That didn't work.")
        draft["money"] = m
        draft["item"] = draft["count"] = 0
        return answer()

    if word == "ITEM":
        # ITEM item count
        draft = draft_of(ctx)
        item, count = chosen(args), chosen(args, 2)
        if item not in ITEM_NAMES or not 1 <= count <= 99:
            raise Refused("That can't be a prize.")
        draft["item"], draft["count"] = item, count
        return answer()

    if word == "CHECK":
        draft = draft_of(ctx)
        if not draft.get("money") and not draft.get("item"):
            raise Refused("An event needs a prize: some money, an item or both.")
        return answer(summary(draft))

    if word == "POST":
        # POST money
        draft = draft_of(ctx)
        money = chosen(args)
        if "time" not in draft or (draft["kind"] == HUNT and "target" not in draft) or \
                (draft["kind"] == CUP and "size" not in draft):
            raise Refused("That didn't work. Please start again.")
        if not draft.get("money") and not draft.get("item"):
            raise Refused("An event needs a prize: some money, an item or both.")
        if draft.get("money", 0) > money:
            raise Refused("You don't have that much money.")
        if any(is_host(ev, key) for ev in live_events(db)):
            raise Refused("You're already hosting an event. One at a time!")
        kind = draft["kind"]
        cur = db.execute(
            "INSERT INTO online_hevents (kind, name, tid, target, size, money, item, count, state, ends)"
            " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (kind, *key, draft.get("target", 0), draft.get("size", 0), draft.get("money", 0), draft.get("item", 0),
             draft.get("count", 0), "open" if kind == HUNT else "signup", now + TIMES[draft["time"]][0]))
        ev = get_event(db, cur.lastrowid)
        ctx.player.hevent_draft = None
        rows = [["-MONEY", ev["money"]]] if ev["money"] else []
        if ev["item"]:
            rows.append(["-ITEM", ev["item"], ev["count"]])
        prize = prize_text(ev["money"], ev["item"], ev["count"])
        if kind == HUNT:
            online.announce(f"{ctx.player.name} is hosting a CATCH HUNT for {species_name(ev['target'])}! "
                            f"Prize: {prize}. HUB > PLAYER EVENTS", exclude=key)
        else:
            online.announce(f"{ctx.player.name} is hosting a {ev['size']}-player TOURNAMENT! Prize: {prize}."
                            " HUB > PLAYER EVENTS", exclude=key)
        return answer(f"Your event is on the board! The server holds {prize} until it ends.", rows=rows)

    raise ValueError("bad HEVENT")


COMMANDS = {"HEVENT": cmd_hevent}
EVENTS = {"CATCH": on_catch}
