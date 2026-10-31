"""Online contests (docs/community/contests.md).

A player enters one Pokémon in a category (COOL, BEAUTY, CUTE, SMART,
TOUGH). It is judged against the latest entries other players made in that
category (and the contest regulars when fewer than three have entered):
three rounds, each scoring the Pokémon's condition for the category, its
sheen, how well its moves suit the category, and the crowd's mood. The
winner's Pokémon gets the category's Master Rank ribbon. Every entry is
kept, so the next player faces it.
"""

import random
import time

import community
from community import Answer, request

REQ_ENTER, REQ_BOARD = 40, 41
WON, PLACED = 0, 2
CATEGORIES = ("COOL", "BEAUTY", "CUTE", "SMART", "TOUGH")
ENTRY_SIZE = 2 + 1 + 1 + 1 + 1 + 12  # species, condition, sheen, level, move appeal, nickname
ROUNDS = 3
MAX_MOVE_APPEAL = 64

# Who fills the stage until enough players have entered: (trainer, nickname,
# species id from include/constants/species.h, condition, sheen, move appeal).
REGULARS = {
    0: [("LIAM", "Blaziken", 257, 120, 40, 30), ("ROSA", "Absol", 359, 100, 30, 26), ("DEX", "Manectric", 310, 90, 20, 24)],
    1: [("MIRA", "Milotic", 350, 140, 60, 32), ("JUNE", "Roselia", 315, 90, 30, 22), ("ARIA", "Altaria", 334, 110, 40, 26)],
    2: [("PIP", "Skitty", 300, 120, 40, 28), ("NELL", "Plusle", 311, 100, 30, 24), ("TOBY", "Azumarill", 184, 90, 20, 22)],
    3: [("SAGE", "Alakazam", 65, 120, 40, 30), ("IVY", "Xatu", 178, 100, 30, 26), ("OREN", "Claydol", 344, 90, 20, 22)],
    4: [("BRAM", "Hariyama", 297, 120, 40, 30), ("KIRA", "Aggron", 306, 110, 40, 26), ("GUS", "Torkoal", 324, 90, 20, 22)],
}

SCHEMA = """
CREATE TABLE IF NOT EXISTS contest_entries (
    id INTEGER PRIMARY KEY,
    category INTEGER NOT NULL,
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    nickname TEXT NOT NULL,
    species INTEGER NOT NULL,
    condition INTEGER NOT NULL,
    sheen INTEGER NOT NULL,
    appeal INTEGER NOT NULL,
    score INTEGER,
    entered REAL NOT NULL
);
"""


def _db(server):
    if not server.community.get("contest_ready"):
        server.league.db.executescript(SCHEMA)
        server.community["contest_ready"] = True
    return server.league.db


def judge(entrants, rng):
    """entrants: dicts with condition, sheen, appeal. Adds `rounds` and
    `score`; returns them best first."""
    for e in entrants:
        base = e["condition"] * 0.5 + e["sheen"] * 0.25 + min(e["appeal"], MAX_MOVE_APPEAL)
        e["rounds"] = [int(base / ROUNDS + rng.randint(0, 25)) for _ in range(ROUNDS)]
        e["score"] = sum(e["rounds"])
    return sorted(entrants, key=lambda e: -e["score"])


@request(REQ_ENTER)
def enter(server, player, args, data):
    category = args[0]
    if not 0 <= category < len(CATEGORIES) or len(data) != ENTRY_SIZE:
        raise ValueError("bad entry")
    db = _db(server)
    species = int.from_bytes(data[0:2], "little")
    me = {"name": player.name, "nickname": community.decode_text(data[6:]) or "POKéMON", "species": species,
          "condition": data[2], "sheen": data[3], "appeal": data[5], "me": True}
    rivals = db.execute(
        "SELECT name, nickname, species, condition, sheen, appeal FROM contest_entries"
        " WHERE category = ? AND NOT (name = ? AND tid = ?) GROUP BY name, tid ORDER BY MAX(id) DESC LIMIT 3",
        (category, *player.key)).fetchall()
    entrants = [me] + [dict(zip(("name", "nickname", "species", "condition", "sheen", "appeal"), r)) for r in rivals]
    for regular in REGULARS[category]:
        if len(entrants) >= 4:
            break
        entrants.append(dict(zip(("name", "nickname", "species", "condition", "sheen", "appeal"), regular)))
    ranked = judge(entrants, random.Random())
    place = next(i for i, e in enumerate(ranked, 1) if e.get("me"))
    db.execute("INSERT INTO contest_entries (category, name, tid, nickname, species, condition, sheen, appeal, score,"
               " entered) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
               (category, player.name, player.tid, me["nickname"], species, me["condition"], me["sheen"],
                min(me["appeal"], MAX_MOVE_APPEAL), me["score"], time.time()))
    db.commit()
    results = bytearray()
    for e in ranked:
        results += community.encode_name(e["name"], 8) + community.encode_name(e["nickname"], 12)
        results += e["species"].to_bytes(2, "little") + bytes(e["rounds"]) + bytes([1 if e.get("me") else 0])
    title = f"{CATEGORIES[category]} CONTEST RESULTS"
    return Answer(title, [], status=WON if place == 1 else PLACED, args=(place, me["score"], len(ranked)),
                  data=bytes(results))


@request(REQ_BOARD)
def board(server, player, args, data):
    category = args[0] % len(CATEGORIES)
    rows = _db(server).execute(
        "SELECT name, nickname, MAX(score) FROM contest_entries WHERE category = ?"
        " GROUP BY name, tid ORDER BY MAX(score) DESC LIMIT 10", (category,)).fetchall()
    return Answer(f"{CATEGORIES[category]} TOP SCORES", [f"{s:>3}  {n}'s {nick}" for n, nick, s in rows])
