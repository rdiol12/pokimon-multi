"""Pokémon pageants: every week, players enter one Pokémon from their party
and everyone votes for their favourites (three votes a week, never for
their own). Each week has a theme, after the contest categories. When the
week is over, the three with the most votes win prizes, collected from
HUB > POKéMON PAGEANT.
"""

from online import Refused, answer, board
from online_ids import ITEMS, SPECIES_NAMES

VOTES = 3
BOARD_SIZE = 8
THEMES = ["Cutest", "Coolest", "Most Beautiful", "Smartest", "Toughest"]
# Place -> (item, count, money)
PRIZES = {1: ("RARE_CANDY", 3, 15000), 2: ("RARE_CANDY", 2, 8000), 3: ("RARE_CANDY", 1, 4000)}
PLACE_NAMES = {1: "1st", 2: "2nd", 3: "3rd"}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_pageant (
            id INTEGER PRIMARY KEY, week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            species INTEGER NOT NULL, level INTEGER NOT NULL, shiny INTEGER NOT NULL,
            personality INTEGER NOT NULL, nickname TEXT NOT NULL, votes INTEGER NOT NULL DEFAULT 0,
            claimed INTEGER NOT NULL DEFAULT 0,
            UNIQUE (week, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_pageant_votes (
            week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, entry INTEGER NOT NULL,
            PRIMARY KEY (week, name, tid, entry)
        );
        """
    )


def theme(week):
    return THEMES[week % len(THEMES)]


def species_name(species):
    return SPECIES_NAMES.get(species, "?").replace("_", " ")


def entries(db, week):
    return db.execute("SELECT id, name, tid, species, level, shiny, personality, nickname, votes FROM online_pageant"
                      " WHERE week = ? ORDER BY id", (week,)).fetchall()


def standings(db, week):
    return db.execute("SELECT id, name, tid, species, nickname, votes FROM online_pageant WHERE week = ?"
                      " ORDER BY votes DESC, id", (week,)).fetchall()


def votes_left(db, week, key):
    used = db.execute("SELECT COUNT(*) FROM online_pageant_votes WHERE week = ? AND name = ? AND tid = ?",
                      (week, *key)).fetchone()[0]
    return VOTES - used


def unclaimed(db, key, week):
    """(entry id, place) of past weeks' top three entries of this player."""
    out = []
    for (w,) in db.execute("SELECT DISTINCT week FROM online_pageant WHERE name = ? AND tid = ? AND week < ?"
                           " AND claimed = 0", (*key, week)).fetchall():
        for place, row in enumerate(standings(db, w)[:3], 1):
            if (row[1], row[2]) == tuple(key) and row[5] > 0:
                out.append((row[0], place))
    return out


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def cmd_pageant(ctx, args, data):
    db, key = ctx.db, ctx.key
    week = ctx.online.week()
    word = args[0]

    if word == "ENTER":
        # species level shiny personality nickname...
        species, level, shiny, personality = (number(args, i) for i in range(1, 5))
        nickname = " ".join(args[5:])[:10] or species_name(species)
        if species_name(species) in ("?", "EGG", "NONE"):
            raise Refused("Only a hatched POKéMON can enter!")
        db.execute("DELETE FROM online_pageant_votes WHERE entry IN"
                   " (SELECT id FROM online_pageant WHERE week = ? AND name = ? AND tid = ?)", (week, *key))
        db.execute("DELETE FROM online_pageant WHERE week = ? AND name = ? AND tid = ?", (week, *key))
        db.execute("INSERT INTO online_pageant (week, name, tid, species, level, shiny, personality, nickname)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?)", (week, *key, species, level, shiny, personality, nickname))
        ctx.online.announce(f"{ctx.player.name} entered {nickname} in the {theme(week)} POKéMON pageant!",
                            exclude=key)
        return answer(f"{nickname} is entered in this week's {theme(week)} POKéMON pageant! Good luck!")

    if word in ("SHOW", "VOTE"):
        # SHOW index: the entry at index (wrapping), for the picture and vote.
        # VOTE index entry
        listed = [e for e in entries(db, week) if (e[1], e[2]) != tuple(key)]
        if not listed:
            raise Refused(f"Nobody else has entered the {theme(week)} POKéMON pageant yet.")
        if word == "VOTE":
            entry = number(args, 2)
            if entry not in [e[0] for e in listed]:
                raise Refused("You can't vote for that one.")
            if db.execute("SELECT 1 FROM online_pageant_votes WHERE week = ? AND name = ? AND tid = ? AND entry = ?",
                          (week, *key, entry)).fetchone():
                raise Refused("You already voted for this one!")
            if votes_left(db, week, key) <= 0:
                raise Refused(f"You've used your {VOTES} votes this week.")
            db.execute("INSERT INTO online_pageant_votes VALUES (?, ?, ?, ?)", (week, *key, entry))
            db.execute("UPDATE online_pageant SET votes = votes + 1 WHERE id = ?", (entry,))
            e = next(e for e in listed if e[0] == entry)
            ctx.online.notify((e[1], e[2]), f"{ctx.player.name} voted for your {e[7]} in the pageant!")
            left = votes_left(db, week, key)
            return answer(f"You voted for {e[1]}'s {e[7]}! {left} vote{'' if left == 1 else 's'} left.")
        i = number(args, 1) % len(listed)
        eid, name, tid, species, level, shiny, personality, nickname, votes = listed[i]
        voted = db.execute("SELECT 1 FROM online_pageant_votes WHERE week = ? AND name = ? AND tid = ? AND entry = ?",
                           (week, *key, eid)).fetchone()
        mark = ", voted!" if voted else "."
        who = f"{name}'s {nickname} ({species_name(species)})"
        if len(who) > 30:
            who = f"{name}'s {nickname}"
        # Two lines: the menu comes up next to it.
        return answer(f"{who}~{i + 1}/{len(listed)}, Lv. {level}, {votes} vote{'' if votes == 1 else 's'}{mark}",
                      code=votes_left(db, week, key),
                      rows=[[species, shiny, personality, eid, (i + 1) % len(listed)]])

    if word == "BOARD":
        rows = []
        for place, (eid, name, tid, species, nickname, votes) in enumerate(standings(db, week), 1):
            mine = "*" if (name, tid) == tuple(key) else ""
            rows.append([f"{mine}{place}", nickname, name, votes])
        mine = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
        left = votes_left(db, week, key)
        message = f"This week: the {theme(week)} POKéMON!~You have {left} vote{'' if left == 1 else 's'} left."
        return answer(message, code=len(unclaimed(db, key, week)),
                      rows=board(f"{theme(week).upper()} POKéMON PAGEANT",
                                 [(6, "No."), (24, "POKéMON"), (96, "TRAINER"), (164, "VOTES")], rows))

    if word == "CLAIM":
        prizes = unclaimed(db, key, week)
        if not prizes:
            raise Refused("You have no prizes to collect.")
        rows, said = [], []
        for eid, place in prizes:
            item, count, money = PRIZES[place]
            db.execute("UPDATE online_pageant SET claimed = 1 WHERE id = ?", (eid,))
            rows += [["+ITEM", ITEMS[item], count], ["+MONEY", money]]
            said.append(f"{PLACE_NAMES[place]} place: {count} RARE {'CANDIES' if count > 1 else 'CANDY'} and ${money}")
        # Weeks without a prize are settled too.
        db.execute("UPDATE online_pageant SET claimed = 1 WHERE name = ? AND tid = ? AND week < ?", (*key, week))
        return answer(f"{'; '.join(said)}!", rows=rows)

    raise ValueError("bad PAGEANT")


COMMANDS = {"PAGEANT": cmd_pageant}
