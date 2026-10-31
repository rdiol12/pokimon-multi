"""What the roaming trainers (rovers.py) do besides battling: they carry
money and a bag, gather Apricorns, Berries and Shards, craft them into Poké
Balls and medicine with the crafting recipes (src/crafting.c), keep the
Pokémon they can't carry at a ranch where they bring back finds like the
Hoenn Ranch's (src/ranch.c), take the daily quests in Petalburg, Slateport
and Fortree (src/daily_quest.c), and send Pokémon on jobs from Mauville
(src/life_jobs.c).

The state lives in the rovers table's `life` column as JSON:
{"money": int, "bag": {item: count}, "ranch": [[species, level], ...],
 "quest": {"town", "kind", "need", "done", "reward"} or None,
 "job": {"mon": mon, "steps": int} or None}

Item names are the game's ITEM_ constants without the prefix. The game is
only sent the battle medicine and how many balls there are (see BAG_WIRE).
"""

import json

START_MONEY = 3000
RANCH_SIZE = 6
BALL_PRICE = 200
POTION_PRICE = 300

APRICORNS = ["RED_APRICORN", "BLUE_APRICORN", "YELLOW_APRICORN", "GREEN_APRICORN",
             "PINK_APRICORN", "WHITE_APRICORN", "BLACK_APRICORN"]
BERRIES = ["ORAN_BERRY", "ORAN_BERRY", "PECHA_BERRY", "CHESTO_BERRY", "RAWST_BERRY",
           "SITRUS_BERRY", "LEPPA_BERRY"]
CAVE_FINDS = ["RED_SHARD", "BLUE_SHARD", "YELLOW_SHARD", "GREEN_SHARD", "TINY_MUSHROOM",
              "TINY_MUSHROOM", "BIG_MUSHROOM", "STARDUST"]

# src/crafting.c's recipes that rovers use: (item, count made, needs).
RECIPES = [
    ("LEVEL_BALL", 1, {"RED_APRICORN": 2}),
    ("LURE_BALL", 1, {"BLUE_APRICORN": 2}),
    ("MOON_BALL", 1, {"YELLOW_APRICORN": 2}),
    ("FRIEND_BALL", 1, {"GREEN_APRICORN": 2}),
    ("LOVE_BALL", 1, {"PINK_APRICORN": 2}),
    ("FAST_BALL", 1, {"WHITE_APRICORN": 2}),
    ("HEAVY_BALL", 1, {"BLACK_APRICORN": 2}),
    ("GREAT_BALL", 1, {"POKE_BALL": 2, "BLUE_APRICORN": 1}),
    ("SUPER_POTION", 1, {"ORAN_BERRY": 2, "SITRUS_BERRY": 1}),
    ("POTION", 2, {"ORAN_BERRY": 2}),
    ("FULL_HEAL", 1, {"PECHA_BERRY": 1, "CHESTO_BERRY": 1, "RAWST_BERRY": 1}),
    ("REVIVE", 1, {"TINY_MUSHROOM": 2, "SITRUS_BERRY": 1}),
    ("FIRE_STONE", 1, {"RED_SHARD": 4}),
    ("WATER_STONE", 1, {"BLUE_SHARD": 4}),
    ("THUNDER_STONE", 1, {"YELLOW_SHARD": 4}),
    ("LEAF_STONE", 1, {"GREEN_SHARD": 4}),
]
BALLS = ["POKE_BALL", "GREAT_BALL", "ULTRA_BALL", "LEVEL_BALL", "LURE_BALL", "MOON_BALL",
         "FRIEND_BALL", "LOVE_BALL", "FAST_BALL", "HEAVY_BALL"]
# Sent to the game, in this order, for the rover's battle items.
BAG_WIRE = ["POTION", "SUPER_POTION", "FULL_HEAL", "REVIVE"]

# src/ranch.c's finds by type (common, common, rare); rovers' ranch
# Pokémon bring back the ones for their trainer's favourite type.
TYPE_FINDS = {
    "NORMAL": ("WHITE_APRICORN", "GREEN_APRICORN", "PRETTY_FEATHER"),
    "FIGHTING": ("WHITE_APRICORN", "RED_APRICORN", "PROTEIN"),
    "FLYING": ("GREEN_APRICORN", "WHITE_APRICORN", "PRETTY_FEATHER"),
    "POISON": ("BLACK_APRICORN", "TINY_MUSHROOM", "BIG_MUSHROOM"),
    "GROUND": ("RED_SHARD", "YELLOW_SHARD", "SOFT_SAND"),
    "ROCK": ("BLUE_SHARD", "GREEN_SHARD", "HARD_STONE"),
    "BUG": ("TINY_MUSHROOM", "GREEN_APRICORN", "SILVER_POWDER"),
    "GHOST": ("BLACK_APRICORN", "TINY_MUSHROOM", "SPELL_TAG"),
    "STEEL": ("RED_SHARD", "BLUE_SHARD", "METAL_COAT"),
    "FIRE": ("RED_APRICORN", "RED_SHARD", "CHARCOAL"),
    "WATER": ("BLUE_APRICORN", "HEART_SCALE", "PEARL"),
    "GRASS": ("GREEN_APRICORN", "TINY_MUSHROOM", "SITRUS_BERRY"),
    "ELECTRIC": ("YELLOW_APRICORN", "YELLOW_SHARD", "MAGNET"),
    "PSYCHIC": ("PINK_APRICORN", "STARDUST", "STAR_PIECE"),
    "ICE": ("BLUE_APRICORN", "BLUE_SHARD", "NEVER_MELT_ICE"),
    "DRAGON": ("BLACK_APRICORN", "YELLOW_APRICORN", "DRAGON_SCALE"),
    "DARK": ("BLACK_APRICORN", "BLACK_APRICORN", "BLACK_GLASSES"),
    "FAIRY": ("PINK_APRICORN", "STARDUST", "STAR_PIECE"),
}

# The daily quest givers' towns and the kinds of favour rovers take on.
QUEST_TOWNS = {"PetalburgCity": "Petalburg", "SlateportCity": "Slateport", "FortreeCity": "Fortree"}
QUEST_KINDS = [("catch", 2, 5, 1000), ("train", 3, 6, 1200), ("battle", 1, 3, 1500)]
JOB_TOWN = "MauvilleCity"


def empty():
    return {"money": START_MONEY, "bag": {}, "ranch": [], "quest": None, "job": None}


def load(blob):
    life = empty()
    if blob:
        try:
            life.update(json.loads(blob))
        except ValueError:
            pass
    return life


def dump(life):
    return json.dumps(life, separators=(",", ":"))


def item_name(item):
    """SUPER_POTION -> Super Potion (news lines are plain ASCII)."""
    return " ".join(w.capitalize() for w in item.split("_"))


def add(life, item, n=1):
    life["bag"][item] = life["bag"].get(item, 0) + n


def take(life, item, n=1):
    have = life["bag"].get(item, 0)
    if have < n:
        return False
    if have == n:
        del life["bag"][item]
    else:
        life["bag"][item] = have - n
    return True


def ball_count(life):
    return sum(life["bag"].get(b, 0) for b in BALLS)


def use_ball(life, rng):
    """Throws one of the rover's balls (crafted ones first). Returns its
    name, or None if the bag has none."""
    have = [b for b in BALLS if life["bag"].get(b)]
    if not have:
        return None
    crafted = [b for b in have if b not in ("POKE_BALL", "GREAT_BALL", "ULTRA_BALL")]
    ball = rng.choice(crafted) if crafted else have[-1]
    take(life, ball)
    return ball


def bag_wire(life):
    """The battle medicine counts, the number of balls, how many Pokémon
    are at the ranch, money (u16, capped) and the quest's progress."""
    counts = [min(life["bag"].get(i, 0), 255) for i in BAG_WIRE]
    return bytes(counts + [min(ball_count(life), 255), min(len(life["ranch"]), 255)]) + \
        min(life["money"], 0xFFFF).to_bytes(2, "little")


def gather(life, kind, rng):
    """One find on the map: Apricorns and Berries outside, Shards and
    Mushrooms in caves."""
    if kind == "cave":
        item = rng.choice(CAVE_FINDS)
    elif kind == "route":
        item = rng.choice(APRICORNS if rng.random() < 0.5 else BERRIES)
    else:
        return None
    add(life, item)
    return item


def craft(life):
    """Crafts whatever the bag allows, best first. Returns the items made."""
    made = []
    for item, count, needs in RECIPES:
        while all(life["bag"].get(k, 0) >= n for k, n in needs.items()):
            for k, n in needs.items():
                take(life, k, n)
            add(life, item, count)
            made.append(item)
            if len(made) >= 8:
                return made
    return made


def ranch_finds(life, fav_type, rng):
    """Each Pokémon at the ranch may bring something back."""
    found = []
    common_a, common_b, rare = TYPE_FINDS.get(fav_type, TYPE_FINDS["NORMAL"])
    for _ in life["ranch"]:
        roll = rng.random()
        if roll < 0.02:
            item = rare
        elif roll < 0.12:
            item = common_a if rng.random() < 0.5 else common_b
        else:
            continue
        add(life, item)
        found.append(item)
    return found


def send_to_ranch(life, mon):
    """Keeps a Pokémon the rover can't carry. Returns False if the ranch
    is full (the caller lets it go)."""
    if len(life["ranch"]) >= RANCH_SIZE:
        return False
    life["ranch"].append([mon["species"], mon["level"]])
    return True


def shop(life):
    """In a town with a Pokémon Mart: buy balls and Potions if short."""
    bought = 0
    while ball_count(life) < 5 and life["money"] >= BALL_PRICE + 1000:
        life["money"] -= BALL_PRICE
        add(life, "POKE_BALL")
        bought += 1
    while life["bag"].get("POTION", 0) < 2 and life["money"] >= POTION_PRICE + 1000:
        life["money"] -= POTION_PRICE
        add(life, "POTION")
        bought += 1
    return bought


def take_quest(life, town, rng):
    kind, lo, hi, reward = rng.choice(QUEST_KINDS)
    life["quest"] = {"town": town, "kind": kind, "need": rng.randint(lo, hi), "done": 0, "reward": reward}
    return life["quest"]


def quest_progress(life, kind, n=1):
    """Counts something toward the quest. Returns the finished quest, if
    this finished it (the reward is paid)."""
    q = life["quest"]
    if not q or q["kind"] != kind:
        return None
    q["done"] += n
    if q["done"] < q["need"]:
        return None
    life["money"] += q["reward"]
    life["quest"] = None
    return q


def quest_text(q):
    what = {"catch": "catch {} Pokémon", "train": "train up {} levels", "battle": "win {} battles"}[q["kind"]]
    return what.format(q["need"])
