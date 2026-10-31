"""The Haven Reserve (docs/reserve.md): one guarded meadow shared by every
player, where trainers leave Pokémon they don't want.

The Pokémon left there "roam" it: when a game asks who's there (LIST), it
gets up to SHOWN of them, picked fresh every few minutes, so everyone in
the reserve sees much the same crowd. A player who battles one first
claims it (CLAIM), so two players can't fight the same Pokémon at once;
the claim runs out by itself if the game never reports back. Caught or
knocked out (RESULT), it leaves the reserve for everyone. The notice board
(LOG) shows the latest comings and goings.

The roaming NPC trainers (server/rovers.py) use the same reserve through
the functions at the bottom: drop_off(), pick_for_rover() and residents().
Their Pokémon don't need the game's raw bytes: a resident without them is
made fresh by the game from its species, level and personality.
"""

import os
import random
import time

import community
from community import Answer, request

REQ_LIST, REQ_LEAVE, REQ_CLAIM, REQ_RESULT, REQ_LOG = 70, 71, 72, 73, 74
GONE, FULL = 2, 3
END_STILL_HERE, END_CAUGHT, END_KNOCKED_OUT = 0, 1, 2
LISTED_MINE = 1

BOX_MON_SIZE = 80
NICK_SIZE = 12
SHOWN = 8
NUM_SPECIES_MAX = 0x7FFF
MAX_RESIDENTS = int(os.environ.get("NETPLAY_RESERVE_MAX", "600"))
CLAIM_SECONDS = float(os.environ.get("NETPLAY_RESERVE_CLAIM_SECONDS", str(10 * 60)))
ROTATE_SECONDS = float(os.environ.get("NETPLAY_RESERVE_ROTATE_SECONDS", str(5 * 60)))
LOG_SIZE = 11
NAME_LENGTH = 7

SCHEMA = """
CREATE TABLE IF NOT EXISTS reserve_mons (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    owner TEXT NOT NULL,
    owner_tid INTEGER NOT NULL,
    species INTEGER NOT NULL,
    level INTEGER NOT NULL,
    personality INTEGER NOT NULL,
    flags INTEGER NOT NULL DEFAULT 0,
    nickname TEXT NOT NULL DEFAULT '',
    mon BLOB,
    left_at REAL NOT NULL,
    claimed_by TEXT,
    claimed_tid INTEGER,
    claimed_until REAL
);
CREATE TABLE IF NOT EXISTS reserve_log (
    at REAL NOT NULL,
    line TEXT NOT NULL
);
"""


class Reserve:
    def __init__(self, db, clock=time.time):
        self.db = db
        self.clock = clock
        db.executescript(SCHEMA)
        db.commit()

    # --- residents ---------------------------------------------------------

    def count(self):
        return self.db.execute("SELECT COUNT(*) FROM reserve_mons").fetchone()[0]

    def add(self, owner, owner_tid, species, level, personality, flags=0, nickname="", mon=None):
        """Leaves a Pokémon at the reserve; returns its id, or None if full."""
        if self.count() >= MAX_RESIDENTS:
            return None
        cur = self.db.execute(
            "INSERT INTO reserve_mons (owner, owner_tid, species, level, personality, flags, nickname, mon, left_at)"
            " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (owner, owner_tid, species, level, personality & 0xFFFFFFFF, flags, nickname,
             bytes(mon) if mon is not None else None, self.clock()))
        self.log(f"{owner} left {nickname or 'a POKéMON'}")
        self.db.commit()
        return cur.lastrowid

    def get(self, mon_id):
        row = self.db.execute(
            "SELECT id, owner, owner_tid, species, level, personality, flags, nickname, mon, claimed_by, claimed_tid,"
            " claimed_until FROM reserve_mons WHERE id = ?", (mon_id,)).fetchone()
        if row is None:
            return None
        keys = ("id", "owner", "owner_tid", "species", "level", "personality", "flags", "nickname", "mon",
                "claimed_by", "claimed_tid", "claimed_until")
        return dict(zip(keys, row))

    def free(self, now=None):
        """Residents nobody is battling right now."""
        now = self.clock() if now is None else now
        return [r[0] for r in self.db.execute(
            "SELECT id FROM reserve_mons WHERE claimed_until IS NULL OR claimed_until < ? ORDER BY id", (now,))]

    def roaming(self, now=None):
        """The SHOWN residents roaming the meadow now. They change every
        ROTATE_SECONDS, and are the same for everyone in that time."""
        now = self.clock() if now is None else now
        ids = self.free(now)
        window = int(now // ROTATE_SECONDS) if ROTATE_SECONDS > 0 else 0
        random.Random(window).shuffle(ids)
        return [self.get(i) for i in ids[:SHOWN]]

    def claim(self, mon_id, key):
        """Starts a battle with one; returns it, or None if it's gone or
        someone else is battling it."""
        mon = self.get(mon_id)
        now = self.clock()
        if mon is None:
            return None
        if mon["claimed_until"] is not None and mon["claimed_until"] >= now and \
                (mon["claimed_by"], mon["claimed_tid"]) != tuple(key):
            return None
        self.db.execute("UPDATE reserve_mons SET claimed_by = ?, claimed_tid = ?, claimed_until = ? WHERE id = ?",
                        (key[0], key[1], now + CLAIM_SECONDS, mon_id))
        self.db.commit()
        return mon

    def finish(self, mon_id, key, end):
        """How the battle ended. Caught or knocked out, it leaves."""
        mon = self.get(mon_id)
        if mon is None or (mon["claimed_by"], mon["claimed_tid"]) != tuple(key):
            return False
        if end in (END_CAUGHT, END_KNOCKED_OUT):
            self.take(mon_id, key[0], end)
        else:
            self.db.execute("UPDATE reserve_mons SET claimed_by = NULL, claimed_tid = NULL, claimed_until = NULL"
                            " WHERE id = ?", (mon_id,))
            self.db.commit()
        return True

    def take(self, mon_id, by, end):
        mon = self.get(mon_id)
        if mon is None:
            return None
        self.db.execute("DELETE FROM reserve_mons WHERE id = ?", (mon_id,))
        name = mon["nickname"] or "a POKéMON"
        self.log(f"{by} caught {name}" if end == END_CAUGHT else f"{by} beat {name}")
        self.db.commit()
        return mon

    # --- the notice board --------------------------------------------------

    def log(self, line):
        self.db.execute("INSERT INTO reserve_log VALUES (?, ?)", (self.clock(), line[:community.ROW_LENGTH]))
        self.db.execute("DELETE FROM reserve_log WHERE rowid NOT IN"
                        " (SELECT rowid FROM reserve_log ORDER BY at DESC, rowid DESC LIMIT 50)")

    def news(self, n=LOG_SIZE):
        return [r[0] for r in self.db.execute("SELECT line FROM reserve_log ORDER BY at DESC, rowid DESC LIMIT ?",
                                              (n,))]


def _reserve(server):
    reserve = server.community.get("reserve")
    if reserve is None:
        reserve = server.community["reserve"] = Reserve(server.league.db)
    return reserve


def _u32(v):
    return (int(v) & 0xFFFFFFFF).to_bytes(4, "little")


@request(REQ_LIST)
def list_roaming(server, player, args, data):
    """data: 12 bytes each (id, personality, species u16, level, flags);
    rows 1..: who left each one; args[0]: how many, args[1]: residents."""
    reserve = _reserve(server)
    mons = reserve.roaming()
    out, owners = bytearray(), []
    for m in mons:
        flags = m["flags"] & ~LISTED_MINE
        if (m["owner"], m["owner_tid"]) == tuple(player.key):
            flags |= LISTED_MINE
        out += _u32(m["id"]) + _u32(m["personality"]) + (m["species"] & 0xFFFF).to_bytes(2, "little")
        out += bytes([max(1, min(100, m["level"])), flags & 0xFF])
        owners.append(m["owner"][:NAME_LENGTH])
    return Answer("HAVEN RESERVE", owners, args=(len(mons), reserve.count()), data=bytes(out))


@request(REQ_LEAVE)
def leave(server, player, args, data):
    """args: species, level, flags (shiny/female); data: the Pokémon's 80
    bytes and its nickname. Answers its id in args[0]."""
    if len(data) != BOX_MON_SIZE + NICK_SIZE:
        raise ValueError("bad Pokémon")
    species, level, flags = args[0], args[1], args[2] & 0x06
    if not 0 < species < NUM_SPECIES_MAX or not 1 <= level <= 100:
        raise ValueError("bad Pokémon")
    mon = data[:BOX_MON_SIZE]
    personality = int.from_bytes(mon[:4], "little")
    nickname = community.decode_text(data[BOX_MON_SIZE:]) or "POKéMON"
    reserve = _reserve(server)
    mon_id = reserve.add(player.name, player.key[1], species, level, personality, flags, nickname, mon)
    if mon_id is None:
        return Answer("HAVEN RESERVE", ["The reserve is full."], status=FULL)
    return Answer("HAVEN RESERVE", [f"{nickname} is roaming the reserve."], args=(mon_id,))


@request(REQ_CLAIM)
def claim(server, player, args, data):
    """data: its 80 bytes (none for a rover's Pokémon); args: species,
    level, personality."""
    mon = _reserve(server).claim(args[0], player.key)
    if mon is None:
        return Answer("HAVEN RESERVE", ["It's gone."], status=GONE)
    return Answer("HAVEN RESERVE", [], args=(mon["species"], mon["level"], mon["personality"]),
                  data=bytes(mon["mon"] or b""))


@request(REQ_RESULT)
def result(server, player, args, data):
    _reserve(server).finish(args[0], player.key, args[1])
    return Answer("HAVEN RESERVE", [])


@request(REQ_LOG)
def news(server, player, args, data):
    reserve = _reserve(server)
    lines = reserve.news()
    if not lines:
        lines = ["Nothing has happened yet."]
    return Answer("HAVEN RESERVE", lines, args=(reserve.count(),))


# --- for the roaming NPC trainers (server/rovers.py) ----------------------------


def drop_off(server, name, species, level, personality=None, nickname=None, mon=None, tid=0):
    """A rover leaves one of its Pokémon. Returns its id, or None if the
    reserve is full. `mon` (the game's 80 bytes) is optional."""
    if personality is None:
        personality = random.getrandbits(32)
    return _reserve(server).add(name, tid, int(species), max(1, min(100, int(level))), personality, 0,
                                nickname or "", mon)


def pick_for_rover(server, name, max_level=100, catch=True, rng=random):
    """A rover battles a Pokémon in the reserve: picks one it can handle
    (at most max_level), and catches it (catch=True) or knocks it out.
    Returns the Pokémon (a dict: species, level, personality, nickname,
    owner, mon) now gone from the reserve, or None if there's nothing."""
    reserve = _reserve(server)
    now = reserve.clock()
    ids = [i for i in reserve.free(now) if reserve.get(i)["level"] <= max_level]
    if not ids:
        return None
    return reserve.take(rng.choice(ids), name, END_CAUGHT if catch else END_KNOCKED_OUT)


def residents(server):
    """Everyone in the reserve: [(id, species, level, owner)]."""
    return [tuple(r) for r in _reserve(server).db.execute(
        "SELECT id, species, level, owner FROM reserve_mons ORDER BY id")]
