#!/usr/bin/env python3
"""Netplay server: overworld presence, battle/trade matchmaking, link relay and
league leaderboard. Protocol and rules are described in docs/netplay.md."""

import argparse
import asyncio
import itertools
import logging
import os
import re
import sqlite3
import time

from friends import Friends
from market import Market
from seasons import Seasons

log = logging.getLogger("netplay")

START_RATING = 1000
ELO_K = 32
MAX_EMOTE = 32
MAX_LIVE = 5
MAX_BATTLE_TEXT = 80  # emote ids the ROM may send (see Emotes in src/netplay.c)
EMOTE_BURST = 4  # emotes a player can send at once; one more each EMOTE_REFILL seconds
EMOTE_REFILL = 1.0
NAME_RE = re.compile(r"^[A-Za-z0-9]{1,7}$")
HEX_RE = re.compile(r"^[0-9a-fA-F]{2,64}$")
KINDS = ("battle", "trade", "deal", "coop", "double", "multi")
OUTCOMES = ("win", "loss", "draw")
BATTLE_KINDS = ("battle", "double")  # two-player PvP battles with a result
RESULT_KINDS = BATTLE_KINDS + ("multi",)  # and 2v2 team battles (server/online_teams.py)
MAX_LINE = 256


class League:
    """Leaderboard storage. Players are keyed by name + trainer id."""

    def __init__(self, path):
        self.db = sqlite3.connect(path)
        self.db.executescript(
            """
            CREATE TABLE IF NOT EXISTS players (
                name TEXT NOT NULL,
                tid INTEGER NOT NULL,
                rating REAL NOT NULL DEFAULT 1000,
                wins INTEGER NOT NULL DEFAULT 0,
                losses INTEGER NOT NULL DEFAULT 0,
                draws INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (name, tid)
            );
            CREATE TABLE IF NOT EXISTS matches (
                id INTEGER PRIMARY KEY,
                a_name TEXT, a_tid INTEGER, a_report TEXT,
                b_name TEXT, b_tid INTEGER, b_report TEXT,
                status TEXT NOT NULL,
                played_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
            );
            """
        )
        self.db.commit()

    def _ensure(self, key):
        self.db.execute(
            "INSERT OR IGNORE INTO players (name, tid, rating) VALUES (?, ?, ?)",
            (*key, START_RATING),
        )

    def rating(self, key):
        self._ensure(key)
        return self.db.execute(
            "SELECT rating FROM players WHERE name = ? AND tid = ?", key
        ).fetchone()[0]

    def record(self, a, a_report, b, b_report):
        """Record a finished battle. a/b are (name, tid); reports are
        'win'/'loss'/'draw' or None when that side never reported.
        Returns the match status."""
        if a_report is None or b_report is None:
            # Only one side reported because the other disconnected.
            reported = a_report if a_report is not None else b_report
            score_a = {"win": 1.0, "loss": 0.0, "draw": 0.5}[reported]
            if a_report is None:
                score_a = 1.0 - score_a
            status = "forfeit"
        elif (a_report, b_report) in (("win", "loss"), ("loss", "win"), ("draw", "draw")):
            score_a = {"win": 1.0, "loss": 0.0, "draw": 0.5}[a_report]
            status = "ok"
        else:
            score_a = None
            status = "disputed"

        self.db.execute(
            "INSERT INTO matches (a_name, a_tid, a_report, b_name, b_tid, b_report, status)"
            " VALUES (?, ?, ?, ?, ?, ?, ?)",
            (*a, a_report, *b, b_report, status),
        )
        if score_a is not None:
            ra, rb = self.rating(a), self.rating(b)
            expected_a = 1.0 / (1.0 + 10 ** ((rb - ra) / 400.0))
            delta = ELO_K * (score_a - expected_a)
            for key, d, score in ((a, delta, score_a), (b, -delta, 1.0 - score_a)):
                column = {1.0: "wins", 0.0: "losses", 0.5: "draws"}[score]
                self.db.execute(
                    f"UPDATE players SET rating = rating + ?, {column} = {column} + 1"
                    " WHERE name = ? AND tid = ?",
                    (d, *key),
                )
        self.db.commit()
        return status

    def rank_of(self, key):
        """(rank, rating, wins, losses, draws) for a player; rank 0 until
        they have a rated battle. Ties share the better rank."""
        row = self.db.execute(
            "SELECT rating, wins, losses, draws FROM players WHERE name = ? AND tid = ?", key
        ).fetchone()
        if row is None or sum(row[1:]) == 0:
            return (0, START_RATING, 0, 0, 0) if row is None else (0, *row)
        above = self.db.execute(
            "SELECT COUNT(*) FROM players WHERE wins + losses + draws > 0 AND rating > ?", (row[0],)
        ).fetchone()[0]
        return (above + 1, *row)

    def top(self, limit=10):
        return self.db.execute(
            "SELECT name, rating, wins, losses, draws FROM players"
            " WHERE wins + losses + draws > 0"
            " ORDER BY rating DESC, wins DESC, name LIMIT ?",
            (limit,),
        ).fetchall()


class Player:
    def __init__(self, pid, writer):
        self.id = pid
        self.writer = writer
        self.name = None
        self.tid = 0
        self.map = None
        self.pos = (0, 0, 0, 0, 0)  # x, y, facing, elevation, gfx
        self.session = None
        self.watching = None  # the battle session this player spectates
        self.emote_tokens = EMOTE_BURST
        self.emote_time = time.monotonic()

    @property
    def key(self):
        return (self.name, self.tid)

    def send(self, line):
        self.writer.write((line + "\n").encode())

    def describe(self):
        x, y, facing, elevation, gfx = self.pos
        return f"PLAYER {self.id} {self.name} {self.map[0]} {self.map[1]} {x} {y} {facing} {elevation} {gfx}"


class Session:
    def __init__(self, sid, kind, host, guest, partners=()):
        self.id = sid
        self.kind = kind
        self.host = host
        self.guest = guest
        # Everyone linked, by link role: a 2v2 team battle adds the host's
        # partner (role 2) and the guest's (role 3); roles 0 and 2 are a team.
        self.members = [host, guest, *partners]
        self.accepted = {host}
        self.started = False
        self.reports = {}
        self.recorded = False
        # Spectating: who watches, and the host's latest report to replay
        # to anyone who starts watching.
        self.watchers = set()
        self.state = None
        self.text = None

    def peer(self, player):
        return self.guest if player is self.host else self.host


class Server:
    def __init__(self, league, season_days=28, clock=time.time):
        self.league = league
        self.friends = Friends(league.db)
        self.seasons = Seasons(league.db, START_RATING, season_days, clock)
        self.market = Market(league.db)
        self.players = {}
        self.sessions = {}
        # Battles that ended before both players reported a result.
        self.pending = {}
        self._ids = itertools.count(1)
        self._sids = itertools.count(1)
        # Commands added by other modules (server/ext.py): name -> fn(server, player, *args).
        self.extra_commands = {}

    # --- connection lifecycle -------------------------------------------

    async def handle(self, reader, writer):
        player = Player(next(self._ids), writer)
        self.players[player.id] = player
        log.info("player %d connected", player.id)
        try:
            while True:
                line = await reader.readline()
                if not line:
                    break
                if len(line) > MAX_LINE:
                    player.send("ERR line too long")
                    break
                try:
                    self.dispatch(player, line.decode("ascii").split())
                except (ValueError, UnicodeDecodeError) as e:
                    player.send(f"ERR {e}")
                await writer.drain()
        except (ConnectionError, asyncio.IncompleteReadError):
            pass
        finally:
            self.disconnect(player)
            writer.close()

    def disconnect(self, player):
        if self.players.pop(player.id, None) is None:
            return
        log.info("player %d (%s) disconnected", player.id, player.name)
        self.stop_watching(player)
        for session in list(self.sessions.values()):
            if player in session.members:
                self.end_session(session, player)
        self.settle_pending(player)
        if player.map is not None:
            self.broadcast_map(player.map, f"GONE {player.id}", exclude=player)

    # --- helpers --------------------------------------------------------

    def on_map(self, map_key, exclude=None):
        return [
            p for p in self.players.values()
            if p.map == map_key and p is not exclude and p.name is not None
        ]

    def broadcast_map(self, map_key, line, exclude=None):
        for p in self.on_map(map_key, exclude):
            p.send(line)

    def end_session(self, session, leaver):
        """Close a session because `leaver` quit or disconnected."""
        self.sessions.pop(session.id, None)
        for p in session.members:
            if p is not leaver and p.id in self.players:
                p.send(f"ENDED {session.id}")
        for p in session.members:
            if p.session is session:
                p.session = None
        for watcher in session.watchers:
            watcher.watching = None
            if watcher.id in self.players:
                watcher.send(f"WEND {session.id}")
        session.watchers.clear()
        if session.kind in RESULT_KINDS and session.started and not session.recorded:
            self.pending[session.id] = session
            self.settle_pending(leaver)

    def settle_pending(self, player):
        """Close ended battles where `player` can no longer report: they
        disconnected, or moved on to another session. Whatever the other side
        reported then stands on its own."""
        for session in list(self.pending.values()):
            if player not in session.members or player.key in session.reports:
                continue
            if player.id in self.players and player.session is None:
                continue
            if session.reports:
                self.finish_battle(session)
            else:
                self.pending.pop(session.id, None)

    def finish_battle(self, session):
        session.recorded = True
        self.pending.pop(session.id, None)
        h, g = session.host, session.guest
        if session.kind == "multi":
            log.info("team battle %d: %s", session.id, self.record_multi(session))
            return
        # Ranked doubles have their own ladder (server/online_doubles.py).
        record = self.league.record if session.kind == "battle" else getattr(self, "record_" + session.kind)
        status = record(
            h.key, session.reports.get(h.key), g.key, session.reports.get(g.key)
        )
        log.info("battle %d %s vs %s: %s", session.id, h.name, g.name, status)

    def team_partners(self, host, guest):
        """The partners `host` and `guest` bring to a 2v2 team battle, or None
        if they can't have one; server/online_teams.py replaces this."""
        return None

    def require_session(self, player, sid, started=None):
        session = self.sessions.get(int(sid))
        if session is None and started:
            session = self.pending.get(int(sid))
        if session is None or player not in session.members:
            raise ValueError("no such session")
        if started is not None and session.started != started:
            raise ValueError("session not ready")
        return session

    # --- commands -------------------------------------------------------

    def dispatch(self, player, args):
        if not args:
            return
        cmd, args = args[0], args[1:]
        if cmd == "HELLO":
            return self.cmd_hello(player, *args)
        if player.name is None:
            raise ValueError("say HELLO first")
        self.seasons.roll_if_due()
        handler = getattr(self, "cmd_" + cmd.lower(), None)
        if handler is None and cmd in self.extra_commands:
            extra = self.extra_commands[cmd]
            handler = lambda player, *args: extra(self, player, *args)  # noqa: E731
        if handler is None or cmd == "HELLO":
            raise ValueError("unknown command")
        try:
            handler(player, *args)
        except TypeError:
            raise ValueError("wrong arguments") from None

    def cmd_hello(self, player, name, tid):
        if player.name is not None:
            raise ValueError("already said HELLO")
        if not NAME_RE.match(name):
            raise ValueError("bad name")
        player.name = name
        player.tid = int(tid) & 0xFFFF
        player.send(f"WELCOME {player.id}")

    def cmd_pos(self, player, group, num, x, y, facing, elevation, gfx):
        new_map = (int(group), int(num))
        player.pos = (int(x), int(y), int(facing), int(elevation), int(gfx))
        old_map = player.map
        player.map = new_map
        if old_map != new_map:
            if old_map is not None:
                self.broadcast_map(old_map, f"GONE {player.id}", exclude=player)
                for other in self.on_map(old_map, exclude=player):
                    player.send(f"GONE {other.id}")
            for other in self.on_map(new_map, exclude=player):
                player.send(other.describe())
        self.broadcast_map(new_map, player.describe(), exclude=player)

    def cmd_req(self, player, kind, target_id):
        if kind not in KINDS:
            raise ValueError("bad kind")
        if player.session:
            raise ValueError("busy")
        sid = next(self._sids)
        player.send(f"PENDING {sid}")
        target = self.players.get(int(target_id))
        if target is None or target is player or target.name is None or target.session:
            player.send(f"DECLINED {sid}")
            return
        partners = ()
        if kind == "multi":
            # Both leaders bring their team partner (server/online_teams.py).
            partners = self.team_partners(player, target)
            if partners is None:
                player.send(f"DECLINED {sid}")
                return
        session = Session(sid, kind, player, target, partners)
        self.sessions[session.id] = session
        for p in session.members:
            p.session = session
            self.settle_pending(p)
        for p in session.members[1:]:
            p.send(f"INVITE {session.id} {kind} {player.id}")

    def cmd_accept(self, player, sid):
        session = self.require_session(player, sid, started=False)
        if player is session.host:
            raise ValueError("not your invite")
        session.accepted.add(player)
        if len(session.accepted) < len(session.members):
            return
        session.started = True
        # Each game hears of the other side's leader, and its own link role.
        for role, p in enumerate(session.members):
            other = session.guest if role % 2 == 0 else session.host
            p.send(f"START {session.id} {session.kind} {other.id} {role}")

    def cmd_decline(self, player, sid):
        session = self.require_session(player, sid, started=False)
        self.sessions.pop(session.id, None)
        for p in session.members:
            p.session = None
            if p is not player:
                p.send(f"DECLINED {session.id}")

    def cmd_link(self, player, sid, payload):
        session = self.require_session(player, sid, started=True)
        if not HEX_RE.match(payload) or len(payload) % 2:
            raise ValueError("bad payload")
        for p in session.members:
            if p is not player:
                p.send(f"LINK {session.id} {payload}")

    def cmd_result(self, player, sid, outcome):
        session = self.require_session(player, sid, started=True)
        if session.kind not in RESULT_KINDS or outcome not in OUTCOMES:
            raise ValueError("bad result")
        session.reports.setdefault(player.key, outcome)
        if len(session.reports) == len(session.members) and not session.recorded:
            self.finish_battle(session)

    def cmd_quit(self, player, sid):
        session = self.sessions.get(int(sid))
        if session is None:
            # Already over: after a battle or trade both sides quit, and the
            # second QUIT finds nothing left to end.
            return
        if player not in session.members:
            raise ValueError("no such session")
        self.end_session(session, player)

    def cmd_emote(self, player, emote):
        emote = int(emote)
        if not 0 <= emote < MAX_EMOTE:
            raise ValueError("bad emote")
        now = time.monotonic()
        player.emote_tokens = min(EMOTE_BURST, player.emote_tokens + (now - player.emote_time) / EMOTE_REFILL)
        player.emote_time = now
        if player.map is None or player.emote_tokens < 1:
            return  # flooding: drop it
        player.emote_tokens -= 1
        self.broadcast_map(player.map, f"EMOTE {player.id} {emote}", exclude=player)

    def online_player(self, key):
        return next((p for p in self.players.values() if p.key == key), None)

    def cmd_friend(self, player, action, *args):
        """FRIEND ADD <player id> | FRIEND DEL <name> <tid>"""
        if action == "ADD" and len(args) == 1:
            other = self.players.get(int(args[0]))
            if other is None or other.name is None:
                raise ValueError("no such player")
            ok = self.friends.add(player.key, other.key)
            player.send(f"FRIENDADD {'ok' if ok else 'full'} {other.name}")
        elif action == "DEL" and len(args) == 2:
            self.friends.remove(player.key, (args[0], int(args[1])))
        else:
            raise ValueError("bad FRIEND")

    def cmd_friends(self, player):
        """One FRIEND line per friend: where they are when online, else 0s."""
        for name, tid in self.friends.list(player.key):
            other = self.online_player((name, tid))
            if other is not None and other.map is not None:
                x, y, _, _, gfx = other.pos
                busy = 1 if other.session is not None else 0
                player.send(f"FRIEND {name} {tid} {other.id} {other.map[0]} {other.map[1]} {x} {y} {gfx} {busy}")
            else:
                player.send(f"FRIEND {name} {tid} 0 0 0 0 0 0 0")
        player.send("FRIENDSEND")

    def cmd_season(self, player):
        """SEASON <number> <days left> <rated 0|1> <rating> <reward tier or -1> <reward season or 0>"""
        season, _ = self.seasons.current()
        rank, rating, *_ = self.league.rank_of(player.key)
        reward = self.seasons.pending_reward(player.key)
        reward_season, reward_tier = reward if reward else (0, -1)
        player.send(f"SEASON {season} {self.seasons.days_left()} {1 if rank else 0} {round(rating)}"
                    f" {reward_tier} {reward_season}")

    def cmd_claim(self, player):
        """CLAIMED <tier> <season>, or CLAIMED -1 0 when there is nothing to claim."""
        reward = self.seasons.claim(player.key)
        season, tier = reward if reward else (0, -1)
        player.send(f"CLAIMED {tier} {season}")

    def cmd_market(self, player, action, *args):
        """The market board: LIST <page>, SELL <species> <level> <price> <mon hex>,
        BUY <id>, CANCEL <id>, COLLECT."""
        if action == "LIST" and len(args) == 1:
            rows, page, pages = self.market.page(int(args[0]))
            for listing_id, name, tid, species, level, price in rows:
                mine = 1 if (name, tid) == player.key else 0
                player.send(f"MITEM {listing_id} {name} {species} {level} {price} {mine}")
            player.send(f"MARKETEND {page} {pages}")
        elif action == "SELL" and len(args) == 4:
            listing_id = self.market.sell(player.key, int(args[0]), int(args[1]), int(args[2]), args[3].lower())
            player.send(f"MLISTED ok {listing_id}" if listing_id else "MLISTED full 0")
        elif action == "BUY" and len(args) == 1:
            result, price, mon = self.market.buy(player.key, int(args[0]))
            player.send(f"MBOUGHT {result} {price} {mon or '-'}")
        elif action == "CANCEL" and len(args) == 1:
            mon = self.market.cancel(player.key, int(args[0]))
            player.send(f"MCANCELED ok {mon}" if mon else "MCANCELED gone -")
        elif action == "COLLECT" and not args:
            money, sales = self.market.collect(player.key)
            player.send(f"MCOLLECT {money} {sales}")
        else:
            raise ValueError("bad MARKET")

    # --- spectating -----------------------------------------------------

    def stop_watching(self, player):
        if player.watching is not None:
            player.watching.watchers.discard(player)
            player.watching = None

    def live_battle(self, sid):
        session = self.sessions.get(int(sid))
        if session is None or session.kind not in BATTLE_KINDS or not session.started:
            return None
        return session

    def cmd_live(self, player):
        """LIVE <session id> <host> <guest> per running battle, then LIVEEND."""
        battles = [s for s in self.sessions.values() if self.live_battle(s.id)]
        for session in battles[:MAX_LIVE]:
            player.send(f"LIVE {session.id} {session.host.name} {session.guest.name}")
        player.send("LIVEEND")

    def cmd_watch(self, player, sid):
        self.stop_watching(player)
        session = self.live_battle(sid)
        if session is None or player in session.members:
            player.send(f"WEND {int(sid)}")
            return
        session.watchers.add(player)
        player.watching = session
        player.send(f"WATCHING {session.id}")
        if session.state:
            player.send(f"WSTATE {session.id} {session.state}")
        if session.text:
            player.send(f"WTEXT {session.id} {session.text}")

    def cmd_unwatch(self, player):
        self.stop_watching(player)

    def battle_report(self, player, sid):
        """The battle `player` hosts, which only the host reports on."""
        session = self.require_session(player, sid, started=True)
        if session.kind not in BATTLE_KINDS or player is not session.host:
            raise ValueError("not the host of a battle")
        return session

    def cmd_bstate(self, player, sid, *fields):
        """BSTATE <sid>, then for the host and the guest: species level hp max-hp alive party-size."""
        session = self.battle_report(player, sid)
        if len(fields) != 12:
            raise ValueError("bad BSTATE")
        session.state = " ".join(str(int(f)) for f in fields)
        for watcher in session.watchers:
            watcher.send(f"WSTATE {session.id} {session.state}")

    def cmd_btext(self, player, sid, *words):
        session = self.battle_report(player, sid)
        session.text = " ".join(words)[:MAX_BATTLE_TEXT]
        for watcher in session.watchers:
            watcher.send(f"WTEXT {session.id} {session.text}")

    def cmd_myrank(self, player):
        rank, rating, w, l, d = self.league.rank_of((player.name, player.tid))
        player.send(f"MYRANK {rank} {round(rating)} {w} {l} {d}")

    def cmd_board(self, player):
        for i, (name, rating, w, l, d) in enumerate(self.league.top(), 1):
            player.send(f"RANK {i} {name} {round(rating)} {w} {l} {d}")
        player.send("BOARDEND")


# World features (followers, houses, leaderboards, hunts, photos): server/world.py.
import world  # noqa: E402

world.install(Server)

# Community features (tournaments, daycare, boss, cards, contests, chat,
# legality checks): server/community.py and server/community_*.py.
import community  # noqa: E402

community.install(Server)


async def serve(host, port, db_path, http_port=0, public_host="", season_days=28):
    server = Server(League(db_path), season_days)
    world.attach(server, db_path)
    community.attach(server, db_path)

    import ext
    import online

    online.install(server, ext.install(server))
    srv = await asyncio.start_server(server.handle, host, port)
    log.info("listening on %s", ", ".join(str(s.getsockname()) for s in srv.sockets))
    servers = [srv]
    if http_port:
        from web import Web

        web = await asyncio.start_server(Web(server, public_host, port).handle, host, http_port)
        log.info("web page on %s", ", ".join(str(s.getsockname()) for s in web.sockets))
        servers.append(web)
    await asyncio.gather(*(s.serve_forever() for s in servers))


def main():
    env = os.environ.get
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default=env("NETPLAY_HOST", "0.0.0.0"))
    parser.add_argument("--port", type=int, default=int(env("NETPLAY_PORT", "7777")),
                        help="game port the mGBA bridge connects to")
    parser.add_argument("--http-port", type=int, default=int(env("NETPLAY_HTTP_PORT", "0")),
                        help="port for the leaderboard page and bridge download (0: off)")
    parser.add_argument("--public-host", default=env("NETPLAY_PUBLIC_HOST", ""),
                        help="address players use to reach this server, written into the"
                             " downloaded bridge script (default: from the page request)")
    parser.add_argument("--db", default=env("NETPLAY_DB", "league.sqlite3"))
    parser.add_argument("--season-days", type=float, default=float(env("NETPLAY_SEASON_DAYS", "28")),
                        help="length of a ladder season")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s")
    asyncio.run(serve(args.host, args.port, args.db, args.http_port, args.public_host, args.season_days))


if __name__ == "__main__":
    main()
