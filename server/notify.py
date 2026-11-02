"""Phone notifications (docs/notifications.md): trade offers, friends coming
online, and tournaments starting.

The phone apps register for them over the game connection, after HELLO:

    NOTIFY ON [token]      -> NOTIFYKEY <name> <tid> <token> <last note id>

From then on that connection also gets each new note as it happens:

    NOTE <id> <kind> <text>

and the app shows it when the game is in the background. When the app isn't
running it asks now and then on a fresh connection, without HELLO:

    NOTIFYPOLL <name> <tid> <token> <since id>
                           -> NOTE lines newer than <since>, then NOTEEND <last id>

Kinds: trade (someone wants to trade or make a deal with you), friend (a
player on your friend list came online), tour (a cup you signed up for
started), fishing (a fishing tournament started; the apps leave it off unless
the player turns it on). Notes are only kept for players with a phone
registered, for NOTE_DAYS days and at most NOTE_KEEP each.
"""

import asyncio
import logging
import os
import secrets
import time

log = logging.getLogger("netplay.notify")

KINDS = ("trade", "friend", "tour", "fishing")
NOTE_KEEP = 50
NOTE_DAYS = 7
MAX_TEXT = 120
FRIEND_QUIET = float(os.environ.get("NETPLAY_NOTIFY_FRIEND_QUIET", str(30 * 60)))  # seconds between two "is online" notes
TICK = float(os.environ.get("NETPLAY_NOTIFY_TICK", "30"))
MAX_DEVICES = 4  # phones per player

SCHEMA = """
CREATE TABLE IF NOT EXISTS notify_devices (
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    token TEXT NOT NULL,
    created REAL NOT NULL,
    PRIMARY KEY (name, tid, token)
);
CREATE TABLE IF NOT EXISTS notify_notes (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    kind TEXT NOT NULL,
    text TEXT NOT NULL,
    at REAL NOT NULL
);
CREATE INDEX IF NOT EXISTS notify_notes_owner ON notify_notes (name, tid, id);
CREATE TABLE IF NOT EXISTS notify_state (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);
"""

TRADE_TEXT = {
    "trade": "{name} wants to trade with you!",
    "deal": "{name} offers you a trade deal!",
}


class Notes:
    """The notes and registered phones, in the league database."""

    def __init__(self, db, clock=time.time):
        self.db = db
        self.clock = clock
        self.db.executescript(SCHEMA)
        self.db.commit()

    # --- phones ---------------------------------------------------------------

    def register(self, key, token=None):
        """The token for this phone: the one it already has if it is still
        known, otherwise a new one."""
        if token and self.known(key, token):
            return token
        token = secrets.token_hex(12)
        self.db.execute("INSERT INTO notify_devices (name, tid, token, created) VALUES (?, ?, ?, ?)",
                        (*key, token, self.clock()))
        # The oldest phones go first once a player has too many
        self.db.execute(
            "DELETE FROM notify_devices WHERE name = ? AND tid = ? AND token NOT IN"
            " (SELECT token FROM notify_devices WHERE name = ? AND tid = ? ORDER BY created DESC LIMIT ?)",
            (*key, *key, MAX_DEVICES))
        self.db.commit()
        return token

    def known(self, key, token):
        return self.db.execute("SELECT 1 FROM notify_devices WHERE name = ? AND tid = ? AND token = ?",
                               (*key, token)).fetchone() is not None

    def registered(self, key):
        return self.db.execute("SELECT 1 FROM notify_devices WHERE name = ? AND tid = ? LIMIT 1",
                               key).fetchone() is not None

    def everyone(self):
        return self.db.execute("SELECT DISTINCT name, tid FROM notify_devices").fetchall()

    # --- notes ----------------------------------------------------------------

    def add(self, key, kind, text):
        """Keeps a note for a player with a phone; returns its id, or None."""
        if kind not in KINDS or not self.registered(key):
            return None
        text = " ".join(str(text).split())[:MAX_TEXT]
        now = self.clock()
        cur = self.db.execute("INSERT INTO notify_notes (name, tid, kind, text, at) VALUES (?, ?, ?, ?, ?)",
                              (*key, kind, text, now))
        self.db.execute(
            "DELETE FROM notify_notes WHERE name = ? AND tid = ? AND (at < ? OR id NOT IN"
            " (SELECT id FROM notify_notes WHERE name = ? AND tid = ? ORDER BY id DESC LIMIT ?))",
            (*key, now - NOTE_DAYS * 86400, *key, NOTE_KEEP))
        self.db.commit()
        return cur.lastrowid

    def since(self, key, since):
        return self.db.execute(
            "SELECT id, kind, text FROM notify_notes WHERE name = ? AND tid = ? AND id > ? ORDER BY id",
            (*key, since)).fetchall()

    def last_id(self, key):
        row = self.db.execute("SELECT MAX(id) FROM notify_notes WHERE name = ? AND tid = ?", key).fetchone()
        return row[0] or 0

    # --- small state ----------------------------------------------------------

    def get_state(self, key, default=None):
        row = self.db.execute("SELECT value FROM notify_state WHERE key = ?", (key,)).fetchone()
        return row[0] if row else default

    def set_state(self, key, value):
        self.db.execute("INSERT OR REPLACE INTO notify_state (key, value) VALUES (?, ?)", (key, str(value)))
        self.db.commit()


def notes(server):
    if getattr(server, "notes", None) is None:
        server.notes = Notes(server.league.db)
    return server.notes


def send(server, key, kind, text):
    """Keeps a note for `key` and passes it to their phone right away when
    it's connected."""
    note_id = notes(server).add(key, kind, text)
    if note_id is None:
        return None
    line = f"NOTE {note_id} {kind} {' '.join(str(text).split())[:MAX_TEXT]}"
    for p in server.players.values():
        if p.key == key and getattr(p, "notify", False):
            p.send(line)
    return note_id


# --- what makes a note ----------------------------------------------------------


def on_hello(server, player):
    """Tells everyone who has this player as a friend that they came online."""
    n = notes(server)
    now = n.clock()
    owners = server.league.db.execute(
        "SELECT owner_name, owner_tid FROM friends WHERE friend_name = ? AND friend_tid = ?", player.key).fetchall()
    for owner in owners:
        owner = tuple(owner)
        if owner == player.key:
            continue
        state = f"friend:{owner[0]}:{owner[1]}:{player.name}:{player.tid}"
        last = float(n.get_state(state, 0))
        if now - last < FRIEND_QUIET:
            continue
        n.set_state(state, now)
        send(server, owner, "friend", f"Your friend {player.name} is online!")


def on_request(server, player, kind, target_id):
    """A trade or deal invite: the other player hears of it on their phone."""
    if kind not in TRADE_TEXT:
        return
    try:
        target = server.players.get(int(target_id))
    except ValueError:
        return
    session = player.session
    if target is None or session is None or target is player or target not in session.members:
        return
    send(server, target.key, "trade", TRADE_TEXT[kind].format(name=player.name))


def on_cup_started(server, tours, cup):
    import community_tournament as ct

    name = ct.cup_name(cup)
    for key in tours.entrants(cup):
        key = tuple(key)
        match = tours.current_match(key)
        if match is not None:
            opponent = match[3][0]
            text = f"The {name} has started! Your first battle is against {opponent}."
        else:
            text = f"The {name} has started!"
        send(server, key, "tour", text)


def check_fishing(server):
    """A fishing tournament that just started (server/online_fishing.py)."""
    try:
        import online_fishing
    except ImportError:
        return
    n = notes(server)
    k, _, on = online_fishing.schedule(n.clock())
    if not on or n.get_state("fishing") == str(k):
        return
    n.set_state("fishing", k)
    for key in n.everyone():
        send(server, tuple(key), "fishing", "A fishing tournament has started! Rare catches score big.")


def tick(server):
    """Starts tournaments that are due (they'd otherwise wait for someone to
    look at the cup) and notices fishing tournaments."""
    tours = None
    community = getattr(server, "community", None)
    if community is not None:
        try:
            import community_tournament as ct

            tours = ct._tours(server)
        except ImportError:
            tours = None
    if tours is not None:
        tours.start_if_due()
    check_fishing(server)


async def ticker(server):
    while True:
        await asyncio.sleep(TICK)
        try:
            tick(server)
        except Exception:
            log.exception("notification tick failed")


def attach(server):
    """Starts the background check; call from inside the running event loop."""
    notes(server)
    server.notify_task = asyncio.get_running_loop().create_task(ticker(server))


# --- commands -----------------------------------------------------------------


def cmd_notify(self, player, action, token=None):
    """NOTIFY ON [token] | NOTIFY OFF"""
    if action == "ON":
        token = notes(self).register(player.key, token)
        player.notify = True
        player.send(f"NOTIFYKEY {player.name} {player.tid} {token} {notes(self).last_id(player.key)}")
    elif action == "OFF":
        player.notify = False
    else:
        raise ValueError("bad NOTIFY")


def poll(self, player, name, tid, token, since):
    """NOTIFYPOLL <name> <tid> <token> <since>: works without HELLO."""
    key = (name, int(tid) & 0xFFFF)
    n = notes(self)
    if not n.known(key, token):
        player.send("NOTEEND -1")
        return
    rows = n.since(key, int(since))
    for note_id, kind, text in rows:
        player.send(f"NOTE {note_id} {kind} {text}")
    player.send(f"NOTEEND {rows[-1][0] if rows else max(int(since), 0)}")


def install(server_cls):
    server_cls.cmd_notify = cmd_notify

    dispatch = server_cls.dispatch

    def dispatch_with_poll(self, player, args):
        if args and args[0] == "NOTIFYPOLL" and player.name is None:
            if len(args) != 5:
                raise ValueError("wrong arguments")
            return poll(self, player, *args[1:])
        return dispatch(self, player, args)

    server_cls.dispatch = dispatch_with_poll

    cmd_hello = server_cls.cmd_hello

    def hello_and_tell(self, player, *args):
        cmd_hello(self, player, *args)
        try:
            on_hello(self, player)
        except Exception:
            log.exception("friend notes failed")

    server_cls.cmd_hello = hello_and_tell

    cmd_req = server_cls.cmd_req

    def req_and_tell(self, player, kind, target_id):
        cmd_req(self, player, kind, target_id)
        try:
            on_request(self, player, kind, target_id)
        except Exception:
            log.exception("trade note failed")

    server_cls.cmd_req = req_and_tell

    try:
        import community_tournament as ct
    except ImportError:
        return
    start_if_due = ct.Tournaments.start_if_due

    def start_and_tell(self):
        cup = start_if_due(self)
        server = getattr(self, "notify_server", None)
        if cup and server is not None:
            try:
                on_cup_started(server, self, cup)
            except Exception:
                log.exception("tournament notes failed")
        return cup

    ct.Tournaments.start_if_due = start_and_tell

    tours = ct._tours

    def tours_with_server(server):
        t = tours(server)
        t.notify_server = server
        return t

    ct._tours = tours_with_server
