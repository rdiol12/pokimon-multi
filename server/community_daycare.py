"""The shared daycare (docs/community.md#shared-daycare).

Two trainers each sign up one Pokémon as a parent (a copy: the Pokémon
stays in their party). Once both parents are in, an egg is ready after
DAYCARE_EGG_SECONDS, and each trainer can collect their own egg from the
pair; then the next one starts. The games work out the egg themselves from
both parents, with the game's own breeding rules.
"""

import os
import time

import community
from community import Answer, request

REQ_STATUS, REQ_LIST, REQ_LEAVE, REQ_EGG, REQ_TAKE = 10, 11, 12, 13, 14
NONE, WAITING, BREEDING, EGG = 2, 3, 4, 5
BOX_MON_SIZE = 80
NICK_SIZE = 12
EGG_SECONDS = float(os.environ.get("NETPLAY_DAYCARE_EGG_SECONDS", str(5 * 60)))
LIST_SIZE = 11

SCHEMA = """
CREATE TABLE IF NOT EXISTS daycare_parents (
    name TEXT NOT NULL,
    tid INTEGER NOT NULL,
    partner_name TEXT NOT NULL,
    partner_tid INTEGER NOT NULL,
    mon BLOB NOT NULL,
    species INTEGER NOT NULL,
    nickname TEXT NOT NULL,
    next_egg REAL,
    PRIMARY KEY (name, tid)
);
"""


class Daycare:
    def __init__(self, db, clock=time.time):
        self.db = db
        self.clock = clock
        db.executescript(SCHEMA)
        db.commit()

    def mine(self, key):
        return self.db.execute(
            "SELECT partner_name, partner_tid, mon, species, nickname, next_egg FROM daycare_parents"
            " WHERE name = ? AND tid = ?", key).fetchone()

    def partner_of(self, key):
        """The partner's row, if they signed up with us too."""
        row = self.mine(key)
        if row is None:
            return None
        other = self.mine((row[0], row[1]))
        if other is None or (other[0], other[1]) != key:
            return None
        return other

    def leave(self, key, partner, mon, species, nickname):
        self.db.execute("INSERT OR REPLACE INTO daycare_parents VALUES (?, ?, ?, ?, ?, ?, ?, NULL)",
                        (*key, *partner, mon, species, nickname))
        self.start_if_paired(key)
        self.db.commit()

    def start_if_paired(self, key):
        if self.partner_of(key) is None:
            return
        row = self.mine(key)
        ready = self.clock() + EGG_SECONDS
        for k in (key, (row[0], row[1])):
            self.db.execute("UPDATE daycare_parents SET next_egg = ? WHERE name = ? AND tid = ? AND next_egg IS NULL",
                            (ready, *k))

    def collected(self, key):
        self.db.execute("UPDATE daycare_parents SET next_egg = ? WHERE name = ? AND tid = ?",
                        (self.clock() + EGG_SECONDS, *key))
        self.db.commit()

    def take(self, key):
        row = self.mine(key)
        self.db.execute("DELETE FROM daycare_parents WHERE name = ? AND tid = ?", key)
        if row is not None:  # the partner's egg timer stops until someone new joins
            self.db.execute("UPDATE daycare_parents SET next_egg = NULL WHERE name = ? AND tid = ?", (row[0], row[1]))
        self.db.commit()


def _daycare(server):
    daycare = server.community.get("daycare")
    if daycare is None:
        daycare = server.community["daycare"] = Daycare(server.league.db)
    return daycare


@request(REQ_STATUS)
def status(server, player, args, data):
    daycare = _daycare(server)
    row = daycare.mine(player.key)
    if row is None:
        return Answer("SHARED DAYCARE", ["Pair up with another trainer.", "Each of you leaves one parent,",
                                         "and you both get an egg."], status=NONE)
    partner_name, _, _, _, nickname, next_egg = row
    other = daycare.partner_of(player.key)
    if other is None:
        return Answer("SHARED DAYCARE", [f"Your {nickname} is waiting", f"for {partner_name} to leave", "a Pokémon too."],
                      status=WAITING)
    if next_egg is not None and daycare.clock() >= next_egg:
        return Answer("SHARED DAYCARE", [f"Your {nickname} and {partner_name}'s", f"{other[4]} had an egg!", ""],
                      status=EGG)
    left = max(1, int(((next_egg or daycare.clock()) - daycare.clock()) // 60) + 1)
    return Answer("SHARED DAYCARE", [f"Your {nickname} and {partner_name}'s", f"{other[4]} are together.",
                                     f"An egg may come in {left} min."], status=BREEDING)


@request(REQ_LIST)
def partners(server, player, args, data):
    keys = [p.key for p in server.online_players() if p.key != player.key]
    for key in server.friends.list(player.key):
        if tuple(key) not in keys and tuple(key) != player.key:
            keys.append(tuple(key))
    keys = keys[:LIST_SIZE]
    player.daycare_choices = keys
    online = {p.key for p in server.online_players()}
    return Answer("PAIR UP WITH", [f"{n}{'  (online)' if (n, t) in online else ''}" for n, t in keys])


@request(REQ_LEAVE)
def leave(server, player, args, data):
    choices = getattr(player, "daycare_choices", [])
    if not 0 <= args[0] < len(choices):
        raise ValueError("no such trainer")
    if len(data) != BOX_MON_SIZE + 2 + NICK_SIZE:
        raise ValueError("bad Pokémon")
    mon, species = data[:BOX_MON_SIZE], int.from_bytes(data[BOX_MON_SIZE:BOX_MON_SIZE + 2], "little")
    nickname = community.decode_text(data[BOX_MON_SIZE + 2:]) or "POKéMON"
    partner = choices[args[0]]
    daycare = _daycare(server)
    daycare.leave(player.key, partner, mon, species, nickname)
    if daycare.partner_of(player.key):
        return Answer("SHARED DAYCARE", [f"{partner[0]}'s Pokémon is already", "there. They'll get along!"])
    return Answer("SHARED DAYCARE", [f"Now {partner[0]} has to leave", "a Pokémon with yours."])


@request(REQ_EGG)
def egg(server, player, args, data):
    """args[0] 0: both parents' data; 1: the game made the egg."""
    daycare = _daycare(server)
    row, other = daycare.mine(player.key), daycare.partner_of(player.key)
    if row is None or other is None or row[5] is None or daycare.clock() < row[5]:
        return Answer("SHARED DAYCARE", ["No egg yet."], status=BREEDING)
    if args[0] == 1:
        daycare.collected(player.key)
        return Answer("SHARED DAYCARE", [])
    return Answer("SHARED DAYCARE", [], status=EGG, data=bytes(row[2]) + bytes(other[2]))


@request(REQ_TAKE)
def take(server, player, args, data):
    _daycare(server).take(player.key)
    return Answer("SHARED DAYCARE", ["Your Pokémon came back."])
