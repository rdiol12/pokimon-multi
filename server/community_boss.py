"""The server boss (docs/community.md#server-boss).

One huge wild Pokémon at a time is shared by everyone on the server. Each
player battles it in their own game; whatever damage they deal comes off
its shared HP. When it falls, everyone who fought it can claim a reward
that grows with their share of the damage, and the next boss shows up
BOSS_REST_SECONDS later.
"""

import json
import logging
import os
import time

import community
from community import Answer, request

log = logging.getLogger("netplay.boss")

REQ_STATUS, REQ_DAMAGE, REQ_CLAIM = 20, 21, 22
ACTIVE, REWARD, RESTING = 2, 3, 4
EVENT_BOSS = 1  # COMMUNITY_EVENT_BOSS

# (name, species id from include/constants/species.h, level)
BOSSES = [
    ("WAILORD", 321, 70),
    ("SNORLAX", 143, 70),
    ("STEELIX", 208, 72),
    ("TYRANITAR", 248, 72),
    ("METAGROSS", 376, 74),
    ("DRAGONITE", 149, 74),
    ("GYARADOS", 130, 70),
    ("AGGRON", 306, 72),
]
BOSS_HP = int(os.environ.get("NETPLAY_BOSS_HP", "20000"))
REST_SECONDS = float(os.environ.get("NETPLAY_BOSS_REST_SECONDS", str(10 * 60)))
MAX_HIT = 1200  # more than one battle can deal: a report above this is cut down
# include/constants/items.h
ITEM_RARE_CANDY, ITEM_PP_MAX, ITEM_GOLD_BOTTLE_CAP = 102, 72, 134


class Boss:
    def __init__(self, path=None, clock=time.time):
        self.path = path
        self.clock = clock
        self.state = {"id": 0, "hp": 0, "max": BOSS_HP, "damage": {}, "claimed": [], "fell": 0}
        if path and os.path.exists(path):
            with open(path) as f:
                self.state = json.load(f)
        if self.state["id"] == 0:
            self.spawn()

    def save(self):
        if not self.path:
            return
        tmp = self.path + ".tmp"
        with open(tmp, "w") as f:
            json.dump(self.state, f)
        os.replace(tmp, self.path)

    def spawn(self):
        self.state = {"id": self.state["id"] + 1, "hp": BOSS_HP, "max": BOSS_HP, "damage": {}, "claimed": [], "fell": 0,
                      "last_damage": self.state.get("damage", {}), "last_claimed": self.state.get("claimed", []),
                      "last_id": self.state["id"]}
        self.save()
        log.info("boss %d: %s", self.state["id"], self.species()[0])

    def species(self, boss_id=None):
        return BOSSES[((boss_id or self.state["id"]) - 1) % len(BOSSES)]

    def tick(self):
        if self.state["hp"] <= 0 and self.clock() - self.state["fell"] >= REST_SECONDS:
            self.spawn()

    def alive(self):
        return self.state["hp"] > 0

    def hit(self, boss_id, ident, damage):
        """Returns True when this hit brought it down."""
        if boss_id != self.state["id"] or not self.alive():
            return False
        damage = max(0, min(int(damage), MAX_HIT))
        self.state["hp"] = max(0, self.state["hp"] - damage)
        self.state["damage"][ident] = self.state["damage"].get(ident, 0) + damage
        fell = self.state["hp"] == 0
        if fell:
            self.state["fell"] = self.clock()
            log.info("boss %d fell", self.state["id"])
        self.save()
        return fell

    def reward_for(self, ident):
        """(boss id, damage, share, rank) of a fallen boss `ident` fought and
        hasn't claimed for yet (this one once it's down, or the last one), or None."""
        st = self.state
        fights = [("last_", st.get("last_id", 0))]
        if not self.alive():
            fights.insert(0, ("", st["id"]))
        for prefix, boss_id in fights:
            damage = st.get(prefix + "damage", {})
            if ident in damage and ident not in st.get(prefix + "claimed", []):
                total = sum(damage.values()) or 1
                rank = sorted(damage.values(), reverse=True).index(damage[ident]) + 1
                return boss_id, damage[ident], damage[ident] / total, rank
        return None

    def claim(self, ident, boss_id):
        key = "claimed" if boss_id == self.state["id"] else "last_claimed"
        self.state.setdefault(key, []).append(ident)
        self.save()


def reward_items(share, rank):
    """(item, count) pairs: candies for everyone, more for a bigger share,
    and a Gold Bottle Cap for the top hitter."""
    items = [(ITEM_RARE_CANDY, min(10, 2 + int(share * 20)))]
    if rank == 1:
        items.append((ITEM_GOLD_BOTTLE_CAP, 1))
    elif share >= 0.1:
        items.append((ITEM_PP_MAX, 1))
    return items


def _boss(server):
    boss = server.community.get("boss")
    if boss is None:
        base = os.path.splitext(server.community_db_path)[0] if server.community_db_path else ""
        boss = server.community["boss"] = Boss(base + ".boss.json" if base else None)
    return boss


def ident(player):
    return f"{player.name}/{player.tid}"


def boss_event(boss, fell_by=""):
    name, species, level = boss.species()
    st = boss.state
    return (bytes([st["id"] & 0xFF, 1 if boss.alive() else 0]) + species.to_bytes(2, "little")
            + st["hp"].to_bytes(4, "little") + st["max"].to_bytes(4, "little") + community.encode_name(fell_by, 8))


@request(REQ_STATUS)
def status(server, player, args, data):
    boss = _boss(server)
    boss.tick()
    name, species, level = boss.species()
    st = boss.state
    reward = boss.reward_for(ident(player))
    if reward:
        boss_id, damage, share, rank = reward
        rname = boss.species(boss_id)[0]
        return Answer("SERVER BOSS", [f"{rname} has fallen!", f"You dealt {damage} damage,",
                                      f"{round(share * 100)}% of the total. Rank {rank}."],
                      status=REWARD, args=(boss_id, damage, rank))
    if not boss.alive():
        left = max(1, int((REST_SECONDS - (boss.clock() - st["fell"])) // 60) + 1)
        return Answer("SERVER BOSS", [f"{name} has fallen.", f"The next boss comes in {left} min.", ""],
                      status=RESTING, args=(species, level, 0, st["max"]))
    mine = st["damage"].get(ident(player), 0)
    hitters = len(st["damage"])
    return Answer("SERVER BOSS", [f"A giant {name} appeared!", f"{hitters} trainers have fought it.",
                                  f"Your damage so far: {mine}"],
                  status=ACTIVE, args=(species, level, st["hp"], st["max"]), data=boss_event(boss))


@request(REQ_DAMAGE)
def damage(server, player, args, data):
    boss = _boss(server)
    boss_id, dealt = args[0], args[1]
    fell = boss.hit(boss_id, ident(player), dealt)
    st = boss.state
    event = boss_event(boss, player.name if fell else "")
    for p in server.online_players():
        if community.is_community(p) and (fell or p is player):
            community.send_event(p, EVENT_BOSS, event)
    rows = [f"You dealt {min(dealt, MAX_HIT)} damage!", f"{boss.species()[0]} HP: {st['hp']}/{st['max']}"]
    if fell:
        rows = [f"You landed the final blow!", f"{boss.species()[0]} has fallen!"]
    return Answer("SERVER BOSS", rows, args=(st["hp"], st["max"], 1 if fell else 0))


@request(REQ_CLAIM)
def claim(server, player, args, data):
    boss = _boss(server)
    reward = boss.reward_for(ident(player))
    if not reward:
        return Answer("SERVER BOSS", ["Nothing to claim."], status=RESTING)
    boss_id, _, share, rank = reward
    boss.claim(ident(player), boss_id)
    items = reward_items(share, rank)
    flat = []
    for item, count in items[:2]:
        flat += [item, count]
    return Answer("SERVER BOSS", [], args=(flat + [0, 0, 0, 0])[:4])
