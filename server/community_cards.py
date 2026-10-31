"""Trainer cards (docs/community.md#trainer-cards).

Opening CARDS sends the player's card (name, look, badges, Pokédex, play
time, team) to the server. Anyone can then look at the cards of the players
online and of recent ones; the server adds their ladder standing and how
many cups they have won.
"""

import time

import community
from community import Answer, request
from seasons import TIERS, tier_of

REQ_PUBLISH, REQ_LIST, REQ_VIEW = 30, 31, 32
CARD_SIZE = 40
LIST_SIZE = 11

SCHEMA = """
CREATE TABLE IF NOT EXISTS cards (
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    card BLOB NOT NULL,
    updated REAL NOT NULL,
    PRIMARY KEY (name, tid)
);
"""


def _db(server):
    if not server.community.get("cards_ready"):
        server.league.db.executescript(SCHEMA)
        server.community["cards_ready"] = True
    return server.league.db


@request(REQ_PUBLISH)
def publish(server, player, args, data):
    if len(data) != CARD_SIZE:
        raise ValueError("bad card")
    db = _db(server)
    db.execute("INSERT OR REPLACE INTO cards (name, tid, card, updated) VALUES (?, ?, ?, ?)",
               (player.name, player.tid, bytes(data), time.time()))
    db.commit()
    return Answer("", [])


@request(REQ_LIST)
def card_list(server, player, args, data):
    db = _db(server)
    online = {p.key for p in server.online_players()}
    rows = db.execute("SELECT name, tid FROM cards ORDER BY updated DESC LIMIT 50").fetchall()
    keys = [player.key] + [k for k in rows if k in online and k != player.key]
    keys += [k for k in rows if k not in online and k != player.key]
    keys = keys[:LIST_SIZE]
    player.card_list = keys
    labels = ["YOUR CARD"] + [f"{n}{'  (online)' if (n, t) in online else ''}" for n, t in keys[1:]]
    return Answer("TRAINER CARDS", labels)


def cups_won(db, key):
    try:
        return db.execute("SELECT COUNT(*) FROM tour_entrants WHERE name = ? AND tid = ? AND place = 1", key).fetchone()[0]
    except Exception:  # no tournaments on this server
        return 0


@request(REQ_VIEW)
def view(server, player, args, data):
    keys = getattr(player, "card_list", [])
    if not 0 <= args[0] < len(keys):
        raise ValueError("no such card")
    key = keys[args[0]]
    db = _db(server)
    row = db.execute("SELECT card FROM cards WHERE name = ? AND tid = ?", key).fetchone()
    if row is None:
        raise ValueError("no such card")
    rank, rating, wins, losses, _ = server.league.rank_of(key)
    extra = [f"{TIERS[tier_of(rating)][0]} TIER" if rank else "NOT RANKED YET"]
    cups = cups_won(db, key)
    if cups:
        extra.append(f"CUPS WON: {cups}")
    return Answer("TRAINER CARD", extra, args=(rank, round(rating), wins, losses), data=row[0])
