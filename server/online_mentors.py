"""Mentors: trainers with six badges or more can volunteer as mentors, and
newer trainers (fewer than six) pick one in HUB > MENTORS. Each time an
apprentice wins a badge, both get a gift; once a day the mentor can cheer
an apprentice on (a small gift too). An apprentice with all eight badges
graduates: both get a big gift and the mentor's graduate count goes up.

The game sends its badge count with every MENTOR request and reports it
(OEV BADGES n) whenever it changes.
"""

from online import Refused, answer, board
from online_ids import ITEMS

MENTOR_BADGES = 6
GRADUATE_BADGES = 8
MAX_APPRENTICES = 4

# Badge count reached -> (apprentice gift, mentor gift); a gift is (item, count, money).
def badge_gifts(badges):
    if badges >= GRADUATE_BADGES:
        return ("RARE_CANDY", 5, 10000), ("RARE_CANDY", 3, 10000)
    if badges >= 6:
        return ("ULTRA_BALL", 10, 3000), ("RARE_CANDY", 1, 3000)
    if badges >= 3:
        return ("HYPER_POTION", 5, 2000), ("ULTRA_BALL", 3, 2000)
    return ("SUPER_POTION", 5, 1000), ("GREAT_BALL", 3, 1000)


CHEER_GIFT = ("SUPER_POTION", 3, 0)

# VAR_0x800B of MENTOR VIEW.
NEW, APPRENTICE, MENTOR, MENTOR_RESTING = 0, 1, 2, 3


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_badges (
            name TEXT NOT NULL, tid INTEGER NOT NULL, badges INTEGER NOT NULL, PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_mentors (
            name TEXT NOT NULL, tid INTEGER NOT NULL, volunteer INTEGER NOT NULL DEFAULT 1,
            graduates INTEGER NOT NULL DEFAULT 0, PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_apprentices (
            name TEXT NOT NULL, tid INTEGER NOT NULL, mentor_name TEXT NOT NULL, mentor_tid INTEGER NOT NULL,
            last_cheer INTEGER NOT NULL DEFAULT -1, PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_mentor_gifts (
            name TEXT NOT NULL, tid INTEGER NOT NULL, item TEXT NOT NULL, count INTEGER NOT NULL,
            money INTEGER NOT NULL
        );
        """
    )


def badges_of(db, key):
    row = db.execute("SELECT badges FROM online_badges WHERE name = ? AND tid = ?", (*key,)).fetchone()
    return row[0] if row else 0


def mentor_of(db, key):
    row = db.execute("SELECT mentor_name, mentor_tid FROM online_apprentices WHERE name = ? AND tid = ?",
                     (*key,)).fetchone()
    return tuple(row) if row else None


def apprentices(db, key):
    return db.execute("SELECT a.name, a.tid, a.last_cheer, COALESCE(b.badges, 0) FROM online_apprentices a"
                      " LEFT JOIN online_badges b ON b.name = a.name AND b.tid = a.tid"
                      " WHERE a.mentor_name = ? AND a.mentor_tid = ? ORDER BY a.name", (*key,)).fetchall()


def gifts(db, key):
    return db.execute("SELECT rowid, item, count, money FROM online_mentor_gifts WHERE name = ? AND tid = ?",
                      (*key,)).fetchall()


def give(db, key, gift):
    db.execute("INSERT INTO online_mentor_gifts VALUES (?, ?, ?, ?, ?)", (*key, *gift))


def set_badges(ctx, badges):
    """Keeps the player's badge count; a new badge brings gifts."""
    db, key = ctx.db, ctx.key
    if not 0 <= badges <= GRADUATE_BADGES:
        return
    old = db.execute("SELECT badges FROM online_badges WHERE name = ? AND tid = ?", (*key,)).fetchone()
    db.execute("INSERT OR REPLACE INTO online_badges VALUES (?, ?, ?)", (*key, badges))
    if old is None or badges <= old[0]:
        return
    mentor = mentor_of(db, key)
    if mentor is None:
        return
    mine, theirs = badge_gifts(badges)
    give(db, key, mine)
    give(db, mentor, theirs)
    name = ctx.player.name
    if badges >= GRADUATE_BADGES:
        db.execute("DELETE FROM online_apprentices WHERE name = ? AND tid = ?", (*key,))
        db.execute("UPDATE online_mentors SET graduates = graduates + 1 WHERE name = ? AND tid = ?", (*mentor,))
        ctx.online.notify(key, f"You graduated! {mentor[0]} left you a gift in HUB > MENTORS.")
        ctx.online.notify(mentor, f"Your apprentice {name} graduated with 8 badges! A gift is in HUB > MENTORS.")
        ctx.online.announce(f"{name} graduated, mentored by {mentor[0]}!", exclude=key)
    else:
        ctx.online.notify(key, f"Badge {badges}! Your mentor {mentor[0]} left you a gift in HUB > MENTORS.")
        ctx.online.notify(mentor, f"Your apprentice {name} won badge {badges}! A gift is in HUB > MENTORS.")


def on_badges(ctx, args):
    if args:
        set_badges(ctx, args[0])


def gift_text(item, count, money):
    name = item.replace("_", " ")
    if count > 1:
        name = name[:-1] + "IES" if name.endswith("Y") else name + "S"
    return f"{count} {name}" + (f" and ${money}" if money else "")


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def cmd_mentor(ctx, args, data):
    db, key = ctx.db, ctx.key
    word = args[0]
    # Every request ends with the game's badge count.
    set_badges(ctx, number(args, len(args) - 1))
    args = args[:-1]
    badges = badges_of(db, key)
    waiting = len(gifts(db, key))
    note = f"~A gift is waiting for you!" if waiting else ""

    if word == "VIEW":
        mentor = mentor_of(db, key)
        if mentor:
            mb = badges_of(db, mentor)
            return answer(f"Your mentor is {mentor[0]} ({mb} badges).{note}", code=APPRENTICE, vars=[mentor[0]])
        if badges >= MENTOR_BADGES:
            row = db.execute("SELECT volunteer, graduates FROM online_mentors WHERE name = ? AND tid = ?",
                             (*key,)).fetchone()
            mine = apprentices(db, key)
            lines = [[n, f"{b} badges", "yes" if c == ctx.online.day() else ""] for n, t, c, b in mine]
            rows = board("MY APPRENTICES", [(6, "TRAINER"), (90, "BADGES"), (150, "CHEERED")], lines)
            if row and row[0]:
                message = f"Mentor: {len(mine)} apprentice{'' if len(mine) == 1 else 's'}, " \
                          f"{row[1]} graduated.{note}"
                return answer(message, code=MENTOR, rows=rows)
            return answer(f"With {badges} badges, you can mentor~newer trainers!" + (" A gift is waiting!" if waiting else ""),
                          code=MENTOR_RESTING, rows=rows)
        return answer("A mentor helps you on your way.~You both get gifts for your badges!",
                      code=NEW)

    if word == "FIND":
        if badges >= MENTOR_BADGES:
            raise Refused("You have enough badges to be a mentor yourself!")
        rows = []
        for n, t, grads in db.execute("SELECT name, tid, graduates FROM online_mentors WHERE volunteer = 1"
                                      " ORDER BY graduates DESC, name").fetchall():
            if (n, t) == tuple(key) or len(apprentices(db, (n, t))) >= MAX_APPRENTICES:
                continue
            rows.append([f"{n}, {badges_of(db, (n, t))} badges", f"{n}:{t}"])
        if not rows:
            raise Refused("No mentors are free right now. Try again later!")
        return answer("Who will be your mentor?", rows=rows[:20])

    if word == "PICK":
        try:
            n, t = args[1].rsplit(":", 1)
            mentor = (n, int(t))
        except (IndexError, ValueError):
            raise Refused("That mentor isn't free.")
        if badges >= MENTOR_BADGES or mentor_of(db, key):
            raise Refused("You can't pick a mentor now.")
        db.execute("INSERT INTO online_apprentices (name, tid, mentor_name, mentor_tid) VALUES (?, ?, ?, ?)",
                   (*key, *mentor))
        ctx.online.notify(mentor, f"{ctx.player.name} picked you as their mentor! See HUB > MENTORS.")
        return answer(f"{mentor[0]} is your mentor now!~You'll both get gifts for badges.")

    if word == "VOLUNTEER":
        if badges < MENTOR_BADGES:
            raise Refused(f"Mentors need {MENTOR_BADGES} badges.")
        db.execute("INSERT INTO online_mentors (name, tid) VALUES (?, ?) ON CONFLICT (name, tid)"
                   " DO UPDATE SET volunteer = 1 - volunteer", (*key,))
        on = db.execute("SELECT volunteer FROM online_mentors WHERE name = ? AND tid = ?", (*key,)).fetchone()[0]
        if on:
            return answer("You're on the mentor list. Newer trainers can pick you now!")
        return answer("You're off the mentor list. Your apprentices stay with you.")

    if word == "WHO":
        rows = [[f"{n}, {b} badges", f"{n}:{t}"] for n, t, c, b in apprentices(db, key)]
        if not rows:
            raise Refused("You have no apprentices yet.")
        return answer("Cheer on which apprentice?", rows=rows)

    if word == "CHEER":
        try:
            n, t = args[1].rsplit(":", 1)
            app = (n, int(t))
        except (IndexError, ValueError):
            raise Refused("That didn't work.")
        row = db.execute("SELECT last_cheer FROM online_apprentices WHERE name = ? AND tid = ? AND mentor_name = ?"
                         " AND mentor_tid = ?", (*app, *key)).fetchone()
        if row is None:
            raise Refused("That isn't your apprentice.")
        if row[0] == ctx.online.day():
            raise Refused(f"You already cheered {app[0]} on today.")
        db.execute("UPDATE online_apprentices SET last_cheer = ? WHERE name = ? AND tid = ?",
                   (ctx.online.day(), *app))
        give(db, app, CHEER_GIFT)
        ctx.online.notify(app, f"Your mentor {ctx.player.name} cheers you on! A gift is in HUB > MENTORS.")
        return answer(f"You cheered {app[0]} on, with {gift_text(*CHEER_GIFT)}!")

    if word == "GIFTS":
        got = gifts(db, key)
        if not got:
            raise Refused("No gifts are waiting.")
        items, money = {}, 0
        for rowid, item, count, cash in got:
            db.execute("DELETE FROM online_mentor_gifts WHERE rowid = ?", (rowid,))
            items[item] = items.get(item, 0) + count
            money += cash
        rows = [["+ITEM", ITEMS[item], count] for item, count in items.items()]
        said = [gift_text(item, count, 0) for item, count in items.items()]
        if money:
            rows.append(["+MONEY", money])
            said.append(f"${money}")
        return answer(f"{ctx.player.name} got {' and '.join([', '.join(said[:-1]), said[-1]] if len(said) > 1 else said)}!",
                      rows=rows)

    if word == "LEAVE":
        mentor = mentor_of(db, key)
        if mentor is None:
            raise Refused("You don't have a mentor.")
        db.execute("DELETE FROM online_apprentices WHERE name = ? AND tid = ?", (*key,))
        ctx.online.notify(mentor, f"{ctx.player.name} has left your mentoring.")
        return answer(f"You said goodbye to {mentor[0]}.")

    raise ValueError("bad MENTOR")


COMMANDS = {"MENTOR": cmd_mentor}
EVENTS = {"BADGES": on_badges}
