"""Player-made quests: a player hides items from their bag at the spot where
they stand, with a hint, and other players go and find them
(HUB > PLAYER QUESTS). A player takes one quest at a time; the game knows
where its treasure is (from the status) and, once the player is on that
map, says so, then that something glints nearby, and on the very spot asks
the server for the treasure. The maker hears who found it.

Spots are the server's last POS of the maker, in the same map coordinates
the game checks.
"""

from online import Refused, answer
from online_ids import ITEMS
from online_sightings import place_name

MAX_HIDDEN = 3
FIND_DISTANCE = 2  # the game asks on the spot; POS may lag a step behind
HINTS = ["Near the water", "In the tall grass", "By a tree", "Next to a building", "On the path",
         "In a quiet corner", "Close to a sign", "Where the flowers grow", "By a rock", "At a dead end"]
ITEM_NAMES = {v: k for k, v in ITEMS.items()}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_pquests (
            id INTEGER PRIMARY KEY, name TEXT NOT NULL, tid INTEGER NOT NULL,
            map_group INTEGER NOT NULL, map_num INTEGER NOT NULL, x INTEGER NOT NULL, y INTEGER NOT NULL,
            item INTEGER NOT NULL, count INTEGER NOT NULL, hint INTEGER NOT NULL,
            found_by TEXT
        );
        CREATE TABLE IF NOT EXISTS online_pquest_takers (
            name TEXT NOT NULL, tid INTEGER NOT NULL, quest INTEGER NOT NULL, PRIMARY KEY (name, tid)
        );
        """
    )


def item_text(item, count):
    name = ITEM_NAMES.get(item, "?").replace("_", " ")
    if count > 1:
        name = name[:-1] + "IES" if name.endswith("Y") else name + "S"
    return f"{count} {name}"


def open_quest(db, qid):
    return db.execute("SELECT id, name, tid, map_group, map_num, x, y, item, count, hint FROM online_pquests"
                      " WHERE id = ? AND found_by IS NULL", (qid,)).fetchone()


def taken(db, key):
    row = db.execute("SELECT quest FROM online_pquest_takers WHERE name = ? AND tid = ?", (*key,)).fetchone()
    quest = open_quest(db, row[0]) if row else None
    if row and quest is None:
        db.execute("DELETE FROM online_pquest_takers WHERE name = ? AND tid = ?", (*key,))
    return quest


def describe(q):
    qid, name, tid, group, num, x, y, item, count, hint = q
    return f"{name} hid {item_text(item, count)} in {place_name(group, num)}.~Hint: {HINTS[hint % len(HINTS)]}."


def chosen(args, i=1):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def status(ctx):
    q = taken(ctx.db, ctx.key)
    return [["PQUEST", q[0], q[3], q[4], q[5], q[6]]] if q else []


def cmd_pquest(ctx, args, data):
    db, key = ctx.db, ctx.key
    word = args[0]

    if word == "LIST":
        mine = taken(db, key)
        if mine:
            return answer(describe(mine), code=1)
        quests = db.execute("SELECT id, name, map_group, map_num FROM online_pquests WHERE found_by IS NULL"
                            " AND NOT (name = ? AND tid = ?) ORDER BY id DESC LIMIT 20", (*key,)).fetchall()
        if not quests:
            raise Refused("Nobody has hidden anything yet. Why not hide a treasure yourself?")
        rows = [[f"{name}: {place_name(g, n)}", qid] for qid, name, g, n in quests]
        return answer("Which treasure will you look for?", rows=rows)

    if word == "SHOW":
        q = open_quest(db, chosen(args))
        if q is None:
            raise Refused("Somebody found that one already!")
        return answer(describe(q))

    if word == "TAKE":
        q = open_quest(db, chosen(args))
        if q is None:
            raise Refused("Somebody found that one already!")
        db.execute("INSERT OR REPLACE INTO online_pquest_takers VALUES (?, ?, ?)", (*key, q[0]))
        return answer(f"Quest taken! Head to {place_name(q[3], q[4])}.")

    if word == "DROP":
        db.execute("DELETE FROM online_pquest_takers WHERE name = ? AND tid = ?", (*key,))
        return answer("You gave up the quest.")

    if word == "HINTS":
        return answer("What's the hint?", rows=[[h, i] for i, h in enumerate(HINTS)])

    if word == "MAKE":
        # item count hint
        item, count, hint = chosen(args, 1), chosen(args, 2), chosen(args, 3)
        if ctx.player.map is None or getattr(ctx.player, "pos", None) is None:
            raise Refused("The server doesn't know where you are yet. Take a step and try again.")
        if item not in ITEM_NAMES or not 1 <= count <= 99 or not 0 <= hint < len(HINTS):
            raise Refused("That can't be hidden.")
        n = db.execute("SELECT COUNT(*) FROM online_pquests WHERE name = ? AND tid = ? AND found_by IS NULL",
                       (*key,)).fetchone()[0]
        if n >= MAX_HIDDEN:
            raise Refused(f"You can only have {MAX_HIDDEN} treasures hidden at once.")
        (group, num), (x, y) = ctx.player.map, ctx.player.pos[:2]
        db.execute("INSERT INTO online_pquests (name, tid, map_group, map_num, x, y, item, count, hint)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (*key, group, num, x, y, item, count, hint))
        ctx.online.announce(f"{ctx.player.name} hid a treasure in {place_name(group, num)}!"
                            " Find it in HUB > PLAYER QUESTS.", exclude=key)
        return answer(f"You hid {item_text(item, count)} right here! Others can look for it now.",
                      rows=[["-ITEM", item, count]])

    if word == "MINE":
        quests = db.execute("SELECT id, map_group, map_num, item, count FROM online_pquests WHERE name = ? AND tid = ?"
                            " AND found_by IS NULL ORDER BY id", (*key,)).fetchall()
        found = db.execute("SELECT COUNT(*) FROM online_pquests WHERE name = ? AND tid = ? AND found_by IS NOT NULL",
                           (*key,)).fetchone()[0]
        rows = [[f"{place_name(g, n)}: {item_text(item, count)}", qid] for qid, g, n, item, count in quests]
        return answer(f"Hidden: {len(rows)} of {MAX_HIDDEN}. Found by others: {found}.", rows=rows)

    if word == "CANCEL":
        qid = chosen(args)
        q = open_quest(db, qid)
        if q is None or (q[1], q[2]) != tuple(key):
            raise Refused("That treasure is gone.")
        db.execute("DELETE FROM online_pquests WHERE id = ?", (qid,))
        return answer(f"You dug up your {item_text(q[7], q[8])} again.", rows=[["+ITEM", q[7], q[8]]])

    if word == "FIND":
        q = taken(db, key)
        if q is None:
            raise Refused("Somebody found it first!")
        qid, name, tid, group, num, x, y, item, count, hint = q
        pos = getattr(ctx.player, "pos", None)
        if ctx.player.map != (group, num) or pos is None or max(abs(pos[0] - x), abs(pos[1] - y)) > FIND_DISTANCE:
            raise Refused("Hmm... nothing here.")
        db.execute("UPDATE online_pquests SET found_by = ? WHERE id = ?", (ctx.player.name, qid))
        db.execute("DELETE FROM online_pquest_takers WHERE quest = ?", (qid,))
        ctx.online.notify((name, tid), f"{ctx.player.name} found the {item_text(item, count)} you hid in "
                                       f"{place_name(group, num)}!")
        return answer(f"{ctx.player.name} found {name}'s treasure: {item_text(item, count)}!",
                      rows=[["+ITEM", item, count]])

    raise ValueError("bad PQUEST")


COMMANDS = {"PQUEST": cmd_pquest}
