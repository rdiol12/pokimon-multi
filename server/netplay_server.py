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

log = logging.getLogger("netplay")

START_RATING = 1000
ELO_K = 32
NAME_RE = re.compile(r"^[A-Za-z0-9]{1,7}$")
HEX_RE = re.compile(r"^[0-9a-fA-F]{2,64}$")
KINDS = ("battle", "trade", "deal")
OUTCOMES = ("win", "loss", "draw")
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

    @property
    def key(self):
        return (self.name, self.tid)

    def send(self, line):
        self.writer.write((line + "\n").encode())

    def describe(self):
        x, y, facing, elevation, gfx = self.pos
        return f"PLAYER {self.id} {self.name} {self.map[0]} {self.map[1]} {x} {y} {facing} {elevation} {gfx}"


class Session:
    def __init__(self, sid, kind, host, guest):
        self.id = sid
        self.kind = kind
        self.host = host
        self.guest = guest
        self.started = False
        self.reports = {}
        self.recorded = False

    def peer(self, player):
        return self.guest if player is self.host else self.host


class Server:
    def __init__(self, league):
        self.league = league
        self.players = {}
        self.sessions = {}
        # Battles that ended before both players reported a result.
        self.pending = {}
        self._ids = itertools.count(1)
        self._sids = itertools.count(1)

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
        for session in list(self.sessions.values()):
            if player in (session.host, session.guest):
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
        peer = session.peer(leaver)
        if peer.id in self.players:
            peer.send(f"ENDED {session.id}")
        for p in (session.host, session.guest):
            if p.session is session:
                p.session = None
        if session.kind == "battle" and session.started and not session.recorded:
            self.pending[session.id] = session
            self.settle_pending(leaver)

    def settle_pending(self, player):
        """Close ended battles where `player` can no longer report: they
        disconnected, or moved on to another session. Whatever the other side
        reported then stands on its own."""
        for session in list(self.pending.values()):
            if player not in (session.host, session.guest) or player.key in session.reports:
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
        status = self.league.record(
            h.key, session.reports.get(h.key), g.key, session.reports.get(g.key)
        )
        log.info("battle %d %s vs %s: %s", session.id, h.name, g.name, status)

    def require_session(self, player, sid, started=None):
        session = self.sessions.get(int(sid))
        if session is None and started:
            session = self.pending.get(int(sid))
        if session is None or player not in (session.host, session.guest):
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
        handler = getattr(self, "cmd_" + cmd.lower(), None)
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
        session = Session(sid, kind, player, target)
        self.sessions[session.id] = session
        player.session = target.session = session
        self.settle_pending(player)
        self.settle_pending(target)
        target.send(f"INVITE {session.id} {kind} {player.id}")

    def cmd_accept(self, player, sid):
        session = self.require_session(player, sid, started=False)
        if player is not session.guest:
            raise ValueError("not your invite")
        session.started = True
        session.host.send(f"START {session.id} {session.kind} {session.guest.id} 0")
        session.guest.send(f"START {session.id} {session.kind} {session.host.id} 1")

    def cmd_decline(self, player, sid):
        session = self.require_session(player, sid, started=False)
        self.sessions.pop(session.id, None)
        session.host.session = session.guest.session = None
        session.peer(player).send(f"DECLINED {session.id}")

    def cmd_link(self, player, sid, payload):
        session = self.require_session(player, sid, started=True)
        if not HEX_RE.match(payload) or len(payload) % 2:
            raise ValueError("bad payload")
        session.peer(player).send(f"LINK {session.id} {payload}")

    def cmd_result(self, player, sid, outcome):
        session = self.require_session(player, sid, started=True)
        if session.kind != "battle" or outcome not in OUTCOMES:
            raise ValueError("bad result")
        session.reports.setdefault(player.key, outcome)
        if len(session.reports) == 2 and not session.recorded:
            self.finish_battle(session)

    def cmd_quit(self, player, sid):
        session = self.sessions.get(int(sid))
        if session is None:
            # Already over: after a battle or trade both sides quit, and the
            # second QUIT finds nothing left to end.
            return
        if player not in (session.host, session.guest):
            raise ValueError("no such session")
        self.end_session(session, player)

    def cmd_myrank(self, player):
        rank, rating, w, l, d = self.league.rank_of((player.name, player.tid))
        player.send(f"MYRANK {rank} {round(rating)} {w} {l} {d}")

    def cmd_board(self, player):
        for i, (name, rating, w, l, d) in enumerate(self.league.top(), 1):
            player.send(f"RANK {i} {name} {round(rating)} {w} {l} {d}")
        player.send("BOARDEND")


async def serve(host, port, db_path, http_port=0, public_host=""):
    server = Server(League(db_path))
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
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s")
    asyncio.run(serve(args.host, args.port, args.db, args.http_port, args.public_host))


if __name__ == "__main__":
    main()
