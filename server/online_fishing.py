"""Fishing tournaments: every two hours, for half an hour, Pokémon landed
with a rod score points for the server-wide tournament. Each one has three
rare species that only bite while it's on (the game swaps one in for a
third of bites). Points are the catch's size in centimetres, plus 50 for a
rare one. When it ends, the top three win prizes and everyone who entered
gets Dive Balls; they collect them from HUB > FISHING TOURNEY.

NETPLAY_FISHT_PERIOD and NETPLAY_FISHT_LENGTH (seconds) change the
schedule, NETPLAY_FISHT_START sets when the first one starts (for tests).
"""

import math
import os

from online import Refused, answer, board
from online_ids import ITEMS, SPECIES, SPECIES_NAMES

PERIOD = float(os.environ.get("NETPLAY_FISHT_PERIOD", 2 * 3600))
LENGTH = float(os.environ.get("NETPLAY_FISHT_LENGTH", 30 * 60))
START = float(os.environ.get("NETPLAY_FISHT_START", 0))
RARE_BONUS = 50
BOARD_SIZE = 7

RARES = ["FEEBAS", "RELICANTH", "LUVDISC", "LAPRAS", "DRATINI", "MANTINE", "LANTURN", "CORSOLA", "CLAMPERL",
         "QWILFISH", "STARYU", "HORSEA", "REMORAID", "CHINCHOU", "WAILMER", "BARBOACH", "CORPHISH", "SKRELP",
         "CLAUNCHER", "MAREANIE", "DHELMISE", "ARROKUDA", "FINIZEN", "BRUXISH"]

# Place -> (item, count, money)
PRIZES = {1: ("RARE_CANDY", 5, 20000), 2: ("RARE_CANDY", 3, 10000), 3: ("ULTRA_BALL", 10, 5000),
          0: ("DIVE_BALL", 3, 0)}
PLACE_NAMES = {1: "1st", 2: "2nd", 3: "3rd"}

_announced = {"started": None, "finished": None}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_fish_scores (
            tourney INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            points INTEGER NOT NULL, catches INTEGER NOT NULL,
            best_species INTEGER NOT NULL, best_size INTEGER NOT NULL,
            PRIMARY KEY (tourney, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_fish_prizes (
            tourney INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            place INTEGER NOT NULL, claimed INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (tourney, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_fish_done (tourney INTEGER PRIMARY KEY);
        """
    )


def schedule(now):
    """(tourney number, its start, whether it's on now)."""
    k = math.floor((now - START) / PERIOD)
    start = START + k * PERIOD
    return k, start, now < start + LENGTH


def rares(k):
    n = len(RARES)
    return [RARES[(k * 3 + i * 7) % n] for i in range(3)]


def name(species):
    return SPECIES_NAMES.get(species, "?").replace("_", " ")


def standings(db, k):
    return db.execute(
        "SELECT name, tid, points, catches, best_species, best_size FROM online_fish_scores WHERE tourney = ?"
        " ORDER BY points DESC, best_size DESC, name", (k,)).fetchall()


def finish(ctx, k):
    """Hands out the prizes of tourney k, once."""
    db = ctx.db
    if k < 0 or db.execute("SELECT 1 FROM online_fish_done WHERE tourney = ?", (k,)).fetchone():
        return
    db.execute("INSERT INTO online_fish_done (tourney) VALUES (?)", (k,))
    rows = standings(db, k)
    for place, row in enumerate(rows, 1):
        db.execute("INSERT OR IGNORE INTO online_fish_prizes (tourney, name, tid, place) VALUES (?, ?, ?, ?)",
                   (k, row[0], row[1], place if place <= 3 else 0))
    if rows and _announced["finished"] != k:
        _announced["finished"] = k
        top = ", ".join(f"{PLACE_NAMES[i + 1]} {r[0]} ({r[2]})" for i, r in enumerate(rows[:3]))
        ctx.online.announce(f"The fishing tournament is over! {top}. Collect prizes in HUB > FISHING TOURNEY.")


def keep_up(ctx):
    """Announces a tournament that just started and settles the last one."""
    k, start, on = schedule(ctx.online.now())
    finish(ctx, k - 1)
    if not on:
        finish(ctx, k)
    elif _announced["started"] != k:
        _announced["started"] = k
        ctx.online.announce("A fishing tournament has started! Only now, "
                            f"{', '.join(rares(k))} bite. Land the biggest fish!")
    return k, start, on


def status(ctx):
    k, start, on = keep_up(ctx)
    left = math.ceil((start + LENGTH - ctx.online.now()) / 60) if on else 0
    return [["FISHT", 1 if on else 0, left, *[SPECIES[r] for r in rares(k)], k]]


def on_fish(ctx, args):
    """FISH species size-in-mm, for a Pokémon landed with a rod."""
    species, size = (args + [0, 0])[:2]
    k, _, on = keep_up(ctx)
    if not on or species <= 0:
        return
    points = size // 10 + (RARE_BONUS if name(species).replace(" ", "_") in rares(k) else 0)
    db = ctx.db
    row = db.execute("SELECT points, best_size FROM online_fish_scores WHERE tourney = ? AND name = ? AND tid = ?",
                     (k, *ctx.key)).fetchone()
    if row is None:
        db.execute("INSERT INTO online_fish_scores VALUES (?, ?, ?, ?, 1, ?, ?)", (k, *ctx.key, points, species, size))
    else:
        db.execute("UPDATE online_fish_scores SET points = points + ?, catches = catches + 1 WHERE tourney = ?"
                   " AND name = ? AND tid = ?", (points, k, *ctx.key))
        if size > row[1]:
            db.execute("UPDATE online_fish_scores SET best_species = ?, best_size = ? WHERE tourney = ?"
                       " AND name = ? AND tid = ?", (species, size, k, *ctx.key))
    ctx.online.notify(ctx.key, f"Tournament: +{points} points!")


def unclaimed(db, key):
    return db.execute("SELECT tourney, place FROM online_fish_prizes WHERE name = ? AND tid = ? AND claimed = 0",
                      (*key,)).fetchall()


def cmd_fisht(ctx, args, data):
    db, key = ctx.db, ctx.key
    k, start, on = keep_up(ctx)
    if args[0] == "BOARD":
        now = ctx.online.now()
        if on:
            shown = k
            left = math.ceil((start + LENGTH - now) / 60)
            message = f"Tournament on! {left} min left.~Rare: {', '.join(rares(k))}."
        else:
            shown = k if standings(db, k) else k - 1
            mins = math.ceil((start + PERIOD - now) / 60)
            message = f"Next tournament in {mins} min.~Rare: {', '.join(rares(k + 1))}."
        rows = []
        for place, (n, tid, points, catches, best, size) in enumerate(standings(db, shown), 1):
            mine = "*" if (n, tid) == key else ""
            rows.append([f"{mine}{place}", n, points, f"{name(best)} {size // 10}cm"])
        mine = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
        title = "FISHING TOURNEY - LIVE" if on else "FISHING TOURNEY - RESULTS"
        return answer(message, code=len(unclaimed(db, key)),
                      rows=board(title, [(6, "No."), (24, "TRAINER"), (84, "POINTS"), (124, "BEST CATCH")], rows))
    if args[0] == "CLAIM":
        prizes = unclaimed(db, key)
        if not prizes:
            raise Refused("You have no prizes to collect.")
        rows, said = [], []
        for tourney, place in prizes:
            item, count, money = PRIZES[place]
            db.execute("UPDATE online_fish_prizes SET claimed = 1 WHERE tourney = ? AND name = ? AND tid = ?",
                       (tourney, *key))
            rows.append(["+ITEM", ITEMS[item], count])
            if money:
                rows.append(["+MONEY", money])
            what = f"{count} {item.replace('_', ' ')}{'S' if count > 1 else ''}"
            what = what.replace("CANDYS", "CANDIES")
            said.append(f"{what}{f' and ${money}' if money else ''}"
                        f" ({PLACE_NAMES[place] + ' place' if place else 'for entering'})")
        return answer(f"{ctx.player.name} got {', '.join(said)}!", rows=rows)
    raise ValueError("bad FISHT")


COMMANDS = {"FISHT": cmd_fisht}
EVENTS = {"FISH": on_fish}
