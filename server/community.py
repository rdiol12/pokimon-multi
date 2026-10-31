"""Community features on the netplay server (docs/community.md): the shared
request/answer plumbing that tournaments, the shared daycare, the server
boss, trainer cards, contests, chat rooms and legality checks sit on.

`install(Server)` adds the commands below to the netplay server, and loads
each feature module in FEATURES that is present (server/community_*.py).
A feature registers request handlers with `@request(type)` and listens for
server events with `@listen(name)`. The mGBA side is
tools/netplay_bridge/netplay_community.lua.

Protocol (one line each, client -> server):
    CHELLO                        after WELCOME: the community mailbox is there
    CDATA hex                     part of the next request's bytes
    C type a0 a1 a2 a3            a request (COMMUNITY_REQ_* in include/constants/community.h)
    CPARTYMON nonce index hex     one party Pokémon's raw bytes, asked for by CPARTY
    CPARTYEND nonce count
Server -> client:
    CROW index hex                a text row of the answer, in the game's charset
    CRESP type status a0 a1 a2 a3 hex   the answer
    CEV section hex               rewrite an event section (COMMUNITY_EVENT_*)
    CPARTY nonce                  send me your party

Events features can listen for: hello(server, player), gone(server, player),
battle_finished(server, session), session_started(server, session).
"""

import importlib
import itertools
import logging
import os
import re

log = logging.getLogger("netplay.community")

FEATURES = (
    "community_tournament",
    "community_daycare",
    "community_boss",
    "community_cards",
    "community_contest",
    "community_chat",
    "community_legal",
)

OK, OFFLINE = 0, 1
ROWS = 12
ROW_LENGTH = 31  # characters; the ROM's rows are 32 bytes with the end mark
DATA_SIZE = 0x100
EVENT_SIZE = 0xC0 - 1
MAX_PARTY = 6
HEX_RE = re.compile(r"^[0-9a-fA-F]*$")

HANDLERS = {}
LISTENERS = {}
WEB_ROUTES = {}  # path -> (link title, fn(server) -> (status, content type, body))

# --- the game's charset ------------------------------------------------------

_TO_GAME = {" ": 0x00, "é": 0x1B, "&": 0x2D, "+": 0x2E, "=": 0x35, "%": 0x5B, "(": 0x5C, ")": 0x5D,
            "!": 0xAB, "?": 0xAC, ".": 0xAD, "-": 0xAE, "·": 0xAF, '"': 0xB1, "'": 0xB4, "♂": 0xB5,
            "♀": 0xB6, ",": 0xB8, "/": 0xBA, ":": 0xF0, ">": 0x86, "▶": 0xEF}
_FROM_GAME = {v: k for k, v in _TO_GAME.items()}


def encode_text(text, length=None):
    """ASCII (plus é ♂ ♀) to game bytes, without the end mark."""
    out = bytearray()
    for ch in text:
        b = ord(ch)
        if 48 <= b <= 57:
            out.append(0xA1 + b - 48)
        elif 65 <= b <= 90:
            out.append(0xBB + b - 65)
        elif 97 <= b <= 122:
            out.append(0xD5 + b - 97)
        else:
            out.append(_TO_GAME.get(ch, 0x00))
    return bytes(out[:length] if length is not None else out)


def encode_name(text, length=8):
    """A fixed-size game string padded with end marks (0xFF)."""
    data = encode_text(text, length - 1)
    return data + b"\xff" * (length - len(data))


def decode_text(data):
    out = []
    for b in data:
        if b == 0xFF:
            break
        if 0xA1 <= b <= 0xAA:
            out.append(chr(48 + b - 0xA1))
        elif 0xBB <= b <= 0xD4:
            out.append(chr(65 + b - 0xBB))
        elif 0xD5 <= b <= 0xEE:
            out.append(chr(97 + b - 0xD5))
        else:
            out.append(_FROM_GAME.get(b, ""))
    return "".join(out).strip()


# --- answers ------------------------------------------------------------------


class Answer:
    """What a request handler returns: a title, text rows, a status, four
    numbers and raw bytes."""

    def __init__(self, title="", rows=(), status=OK, args=(), data=b""):
        self.title = title
        self.rows = list(rows)
        self.status = status
        self.args = (list(args) + [0, 0, 0, 0])[:4]
        self.data = bytes(data)


def request(req_type):
    def register(fn):
        HANDLERS[req_type] = fn
        return fn
    return register


def listen(event):
    def register(fn):
        LISTENERS.setdefault(event, []).append(fn)
        return fn
    return register


def emit(event, *args):
    for fn in LISTENERS.get(event, ()):
        try:
            fn(*args)
        except Exception:  # one feature's bug shouldn't take the others down
            log.exception("community listener %s failed", event)


def send_answer(player, req_type, answer):
    rows = ([answer.title] if answer.title is not None else []) + answer.rows
    for i, row in enumerate(rows[:ROWS]):
        player.send(f"CROW {i} {encode_text(str(row), ROW_LENGTH).hex() or '00'}")
    data = answer.data[:DATA_SIZE].hex() or "-"
    args = " ".join(str(int(a) & 0xFFFFFFFF) for a in answer.args)
    player.send(f"CRESP {req_type} {answer.status} {args} {data}")


def send_event(player, section, data):
    player.send(f"CEV {section} {bytes(data[:EVENT_SIZE]).hex() or '00'}")


def is_community(player):
    return getattr(player, "community", False)


# --- commands -------------------------------------------------------------------

_nonces = itertools.count(1)


def _check_hex(text):
    if not HEX_RE.match(text) or len(text) % 2:
        raise ValueError("bad hex")
    return bytes.fromhex(text)


def cmd_chello(self, player):
    player.community = True
    emit("hello", self, player)


def cmd_cdata(self, player, chunk):
    buf = getattr(player, "cdata", b"") + _check_hex(chunk)
    if len(buf) > DATA_SIZE:
        player.cdata = b""
        raise ValueError("request too large")
    player.cdata = buf


def cmd_c(self, player, req_type, a0, a1, a2, a3):
    req_type = int(req_type)
    data, player.cdata = getattr(player, "cdata", b""), b""
    args = [int(a) for a in (a0, a1, a2, a3)]
    handler = HANDLERS.get(req_type)
    if handler is None:
        answer = Answer("COMMUNITY", ["The server doesn't have it yet."], status=OFFLINE)
    else:
        try:
            answer = handler(self, player, args, data)
        except ValueError as e:
            log.info("community request %d from %s refused: %s", req_type, player.name, e)
            answer = Answer("COMMUNITY", [str(e)[:ROW_LENGTH]], status=OFFLINE)
    if answer is not None:  # None: the handler answers later itself
        send_answer(player, req_type, answer)


def ask_party(self, player, callback):
    """Asks `player`'s bridge for the raw bytes of their party, then calls
    callback(server, player, [bytes per Pokémon])."""
    nonce = next(_nonces)
    if not hasattr(player, "party_waits"):
        player.party_waits = {}
    player.party_waits[nonce] = ({}, callback)
    player.send(f"CPARTY {nonce}")
    return nonce


def cmd_cpartymon(self, player, nonce, index, hex_data):
    wait = getattr(player, "party_waits", {}).get(int(nonce))
    index = int(index)
    if wait is None or not 0 <= index < MAX_PARTY:
        return
    wait[0][index] = _check_hex(hex_data)


def cmd_cpartyend(self, player, nonce, count):
    wait = getattr(player, "party_waits", {}).pop(int(nonce), None)
    if wait is None:
        return
    mons, callback = wait
    party = [mons[i] for i in sorted(mons) if i < int(count)]
    try:
        callback(self, player, party)
    except Exception:
        log.exception("party callback failed")


def online_players(self):
    return [p for p in self.players.values() if p.name is not None]


def install(server_cls):
    """Adds the community commands to the netplay Server class, and wraps the
    places features listen to."""
    for fn in (cmd_chello, cmd_cdata, cmd_c, cmd_cpartymon, cmd_cpartyend, ask_party, online_players):
        setattr(server_cls, fn.__name__, fn)

    finish_battle = server_cls.finish_battle

    def finish_battle_and_tell(self, session):
        finish_battle(self, session)
        emit("battle_finished", self, session)

    server_cls.finish_battle = finish_battle_and_tell

    cmd_accept = server_cls.cmd_accept

    def accept_and_tell(self, player, sid):
        cmd_accept(self, player, sid)
        session = self.sessions.get(int(sid))
        if session is not None and session.started:
            emit("session_started", self, session)

    server_cls.cmd_accept = accept_and_tell

    disconnect = server_cls.disconnect

    def disconnect_and_tell(self, player):
        known = player.id in self.players
        disconnect(self, player)
        if known and player.name is not None:
            emit("gone", self, player)

    server_cls.disconnect = disconnect_and_tell

    init = server_cls.__init__

    def __init__(self, *args, **kwargs):
        init(self, *args, **kwargs)
        attach(self, None)

    server_cls.__init__ = __init__

    for name in FEATURES:
        if not os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), name + ".py")):
            continue
        module = importlib.import_module(name)
        if hasattr(module, "install"):
            module.install(server_cls)
        log.debug("community feature %s loaded", name)


def attach(server, db_path):
    """Gives a Server instance its community state; features keep theirs in
    the league database (server.league.db) or under server.community."""
    server.community = {}
    server.community_db_path = db_path
    emit("attach", server)
