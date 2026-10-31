"""Legality checks (docs/community/legality.md).

The server reads the raw Pokémon data the games send and checks that it
could have come from the game: a valid checksum, a real species, moves it
can learn (itself or anything it evolves from, in any game), no move twice,
items and moves that exist, effort values within the caps, and a level that
matches its experience. What the game can learn and how fast each species
grows come from server/data/legal.json.gz (tools/community/make_legal_data.py).

Where it applies:
- Ladder battles, trades, deals and co-op battles: when one starts, the
  server asks both games for their party. An illegal party ends a trade or
  deal on the spot, and keeps a battle off the ladder.
- The market board refuses to list an illegal Pokémon.
These run for games with the community bridge (netplay_community.lua, part
of the bridge download), which is what answers the party requests.
- START > ONLINE > COMMUNITY > CHECK TEAM checks your own party.
Every illegal Pokémon seen is logged in the legality_reports table.
"""

import gzip
import json
import logging
import os
import struct
import time

import community
import community_fusion
from community import Answer, listen, request

log = logging.getLogger("netplay.legal")

REQ_CHECK = 60
ALL_LEGAL, SOME_ILLEGAL = 0, 2
BOX_MON_SIZE = 80
PARTY_MON_SIZE = 100
MAX_LEVEL = 100
EV_TOTAL, EV_MAX = 510, 252
DATA_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "legal.json.gz")

# src/pokemon.c sSubstructOffsets: where substruct type t is, by personality % 24.
SUBSTRUCT_OFFSETS = [
    [0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3],
    [1, 1, 2, 3, 2, 3, 0, 0, 0, 0, 0, 0, 2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2],
    [2, 3, 1, 1, 3, 2, 2, 3, 1, 1, 3, 2, 0, 0, 0, 0, 0, 0, 3, 2, 3, 2, 1, 1],
    [3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 3, 2, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0],
]

SCHEMA = """
CREATE TABLE IF NOT EXISTS legality_reports (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    species INTEGER,
    reason TEXT NOT NULL,
    context TEXT NOT NULL,
    seen REAL NOT NULL
);
"""

_data = None


def data():
    global _data
    if _data is None:
        with gzip.open(DATA_PATH, "rt") as f:
            raw = json.load(f)
        raw["growth"] = {int(k): v for k, v in raw["growth"].items()}
        raw["learn"] = {int(k): set(v) for k, v in raw["learn"].items()}
        _data = raw
    return _data


def exp_for_level(growth, n):
    """gExperienceTables (src/data/pokemon/experience_tables.h)."""
    if n <= 1:
        return 0
    cube = n * n * n
    if growth == 0:
        return cube
    if growth == 1:
        if n <= 50:
            return (100 - n) * cube // 50
        if n <= 68:
            return (150 - n) * cube // 100
        if n <= 98:
            return ((1911 - 10 * n) // 3) * cube // 500
        return (160 - n) * cube // 100
    if growth == 2:
        if n <= 15:
            return ((n + 1) // 3 + 24) * cube // 50
        if n <= 36:
            return (n + 14) * cube // 50
        return ((n // 2) + 32) * cube // 50
    if growth == 3:
        return 6 * cube // 5 - 15 * n * n + 100 * n - 140
    if growth == 4:
        return 4 * cube // 5
    return 5 * cube // 4


def level_from_exp(growth, exp):
    level = 1
    while level < MAX_LEVEL and exp_for_level(growth, level + 1) <= exp:
        level += 1
    return level


def decode(raw):
    """The fields the checks need from a struct BoxPokemon (+ party data),
    or raises ValueError."""
    if len(raw) < BOX_MON_SIZE:
        raise ValueError("too short")
    personality, ot_id = struct.unpack_from("<II", raw, 0)
    flags = raw[19]
    checksum = struct.unpack_from("<H", raw, 28)[0]
    key = personality ^ ot_id
    words = [w ^ key for w in struct.unpack_from("<12I", raw, 32)]
    calc = 0
    for w in words:
        calc += w + (w >> 16)
    secure = struct.pack("<12I", *words)

    def sub(t):
        i = SUBSTRUCT_OFFSETS[t][personality % 24]
        return secure[i * 12:(i + 1) * 12]

    s0, s1, s2, s3 = sub(0), sub(1), sub(2), sub(3)
    a, b = struct.unpack_from("<HH", s0, 0)
    exp_word = struct.unpack_from("<I", s0, 4)[0]
    moves = [w & 0x7FF for w in struct.unpack_from("<4H", s1, 0)]
    ivs = struct.unpack_from("<I", s3, 4)[0]
    mon = {
        "personality": personality,
        "bad_egg": bool(flags & 1),
        "has_species": bool(flags & 2),
        "is_egg": bool(flags & 4),
        "checksum_ok": (calc & 0xFFFF) == checksum,
        "species": a & 0x7FF,
        "item": b & 0x3FF,
        "exp": exp_word & 0x1FFFFF,
        "moves": moves,
        "evs": list(s2[0:6]),
        "ivs": [(ivs >> (5 * i)) & 31 for i in range(6)],
        "met_level": struct.unpack_from("<H", s3, 2)[0] & 0x7F,
        "ability_num": (struct.unpack_from("<I", s3, 8)[0] >> 29) & 3,
        # Fusion breeding's partner species (src/breed_fusion.c), spread over spare bits.
        "fusion_partner": ((struct.unpack_from("<I", s3, 8)[0] >> 20) & 0x7F)
                          | (((struct.unpack_from("<H", s0, 10)[0] >> 14) & 3) << 7)
                          | (((struct.unpack_from("<I", s3, 8)[0] >> 28) & 1) << 9)
                          | (((flags >> 7) & 1) << 10),
        "level": raw[84] if len(raw) >= PARTY_MON_SIZE else None,
        "nickname": community.decode_text(raw[8:18]),
    }
    return mon


def check(raw):
    """A list of reasons this Pokémon can't be legitimate (empty if it's fine)."""
    d = data()
    try:
        mon = decode(raw)
    except (ValueError, struct.error):
        return ["unreadable data"]
    if not mon["has_species"] and mon["species"] == 0:
        return []  # an empty slot
    reasons = []
    if not mon["checksum_ok"]:
        reasons.append("its data was edited")
    if mon["bad_egg"]:
        reasons.append("it's a Bad Egg")
    species = mon["species"]
    if not 0 < species < d["species_count"] or species == d["species_egg"]:
        reasons.append("no such species")
        return reasons
    partner = mon["fusion_partner"]
    if partner and not (0 < partner < d["species_count"] and partner != d["species_egg"]
                        and community_fusion.can_fuse_with(partner)):
        reasons.append("no such fusion")
        partner = 0
    if mon["is_egg"]:
        return reasons
    if not 0 <= mon["item"] < d["items_count"]:
        reasons.append("no such item")
    moves = [m for m in mon["moves"] if m]
    if not moves:
        reasons.append("it knows no moves")
    if len(set(moves)) != len(moves):
        reasons.append("it knows a move twice")
    learnable = d["learn"].get(species)
    if learnable is not None and partner:
        # A fusion also knows moves from its father's side.
        learnable = learnable | d["learn"].get(partner, set())
    for move in moves:
        if move >= d["moves_count"]:
            reasons.append("no such move")
        elif learnable is not None and move not in learnable:
            reasons.append(f"it can't learn {d['move_names'].get(str(move), move)}")
    if sum(mon["evs"]) > EV_TOTAL or max(mon["evs"]) > EV_MAX:
        reasons.append("too much effort (EVs)")
    if mon["ability_num"] > 2:
        reasons.append("no such ability")
    growth = d["growth"].get(species)
    if growth is not None:
        if mon["exp"] > exp_for_level(growth, MAX_LEVEL):
            reasons.append("too much experience")
        level = level_from_exp(growth, mon["exp"])
        if mon["level"] is not None and mon["level"] != level:
            reasons.append("its level doesn't match")
        if mon["met_level"] > level:
            reasons.append("met above its level")
    return reasons


def _db(server):
    if not server.community.get("legal_ready"):
        server.league.db.executescript(SCHEMA)
        server.community["legal_ready"] = True
    return server.league.db


def report(server, player, raw, reasons, context):
    try:
        species = decode(raw)["species"]
    except (ValueError, struct.error):
        species = None
    db = _db(server)
    for reason in reasons:
        db.execute("INSERT INTO legality_reports (name, tid, species, reason, context, seen) VALUES (?, ?, ?, ?, ?, ?)",
                   (player.name, player.tid, species, reason, context, time.time()))
    db.commit()
    log.warning("illegal Pokémon from %s (%s): %s", player.name, context, ", ".join(reasons))


def check_party(server, player, party, context):
    """[(slot, nickname, reasons)] for the illegal ones."""
    bad = []
    for slot, raw in enumerate(party):
        reasons = check(raw)
        if reasons:
            report(server, player, raw, reasons, context)
            try:
                name = decode(raw)["nickname"] or "?"
            except (ValueError, struct.error):
                name = "?"
            bad.append((slot, name, reasons))
    return bad


# --- in-game check -------------------------------------------------------------

@request(REQ_CHECK)
def check_team(server, player, args, data_):
    def checked(server, player, party):
        bad = check_party(server, player, party, "team check")
        if not bad:
            answer = Answer("TEAM CHECK", [f"All {len(party)} POKéMON are legal.", "They can battle and trade", "online."],
                            status=ALL_LEGAL, args=(len(party), 0))
        else:
            rows = []
            for slot, name, reasons in bad:
                rows.append(f"{slot + 1}. {name}:")
                rows += [f"  {r}" for r in reasons[:2]]
            answer = Answer("TEAM CHECK", rows[:community.ROWS - 1], status=SOME_ILLEGAL, args=(len(party), len(bad)))
        community.send_answer(player, REQ_CHECK, answer)

    server.ask_party(player, checked)
    return None  # answered once the party arrives


# --- battles, trades and deals --------------------------------------------------------

@listen("session_started")
def on_session(server, session):
    for p in (session.host, session.guest):
        if not community.is_community(p):
            continue

        def checked(server, player, party, session=session):
            bad = check_party(server, player, party, session.kind)
            if not bad:
                return
            session.illegal = True
            if session.kind in ("trade", "deal") and session.id in server.sessions:
                log.warning("%s %d stopped: %s has an illegal Pokémon", session.kind, session.id, player.name)
                player.send("ERR illegal Pokémon in your party")
                server.end_session(session, player)
                player.send(f"ENDED {session.id}")

        server.ask_party(p, checked)


def install(server_cls):
    finish_battle = server_cls.finish_battle

    def finish_legal_battle(self, session):
        if getattr(session, "illegal", False):
            session.recorded = True
            self.pending.pop(session.id, None)
            log.warning("battle %d not rated: illegal Pokémon", session.id)
            return
        finish_battle(self, session)

    server_cls.finish_battle = finish_legal_battle

    cmd_market = server_cls.cmd_market

    def legal_market(self, player, action, *args):
        if action == "SELL" and len(args) == 4 and community.is_community(player):
            try:
                raw = bytes.fromhex(args[3])
            except ValueError:
                raw = b""
            reasons = check(raw)
            if reasons:
                report(self, player, raw, reasons, "market")
                player.send("MLISTED full 0")
                return
        cmd_market(self, player, action, *args)

    server_cls.cmd_market = legal_market
