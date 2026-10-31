"""Duo quests: two players team up and work on quests together. One asks
another (HUB > DUO QUESTS); once the other says yes, the pair gets its first
quest. Both players' catches count towards it, and when it's done both get
the reward and the next, slightly bigger, quest starts. The pair stays
together until one of them splits it up.
"""

import json

from online import Refused, answer, board
from online_ids import ITEMS

INVITE_SECONDS = 15 * 60

# kind, base target, text ({n} is the target), reward item, count, money
QUESTS = [
    ("catch", 10, "Catch {n} POKéMON", "ULTRA_BALL", 5, 3000),
    ("fish", 5, "Land {n} POKéMON with a rod", "DIVE_BALL", 5, 3000),
    ("species", 6, "Catch {n} different species", "TIMER_BALL", 5, 4000),
    ("places", 4, "Catch POKéMON in {n} places", "NEST_BALL", 5, 4000),
    ("catch", 20, "Catch {n} POKéMON", "RARE_CANDY", 2, 6000),
]

# What VAR_0x800B is after DUO VIEW.
NONE, INVITED, ASKED, ACTIVE, REWARD = 0, 1, 2, 3, 4  # REWARD: active, and a reward waiting


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_duos (
            id INTEGER PRIMARY KEY,
            a_name TEXT NOT NULL, a_tid INTEGER NOT NULL, b_name TEXT NOT NULL, b_tid INTEGER NOT NULL,
            round INTEGER NOT NULL DEFAULT 0,
            prog_a INTEGER NOT NULL DEFAULT 0, prog_b INTEGER NOT NULL DEFAULT 0,
            seen TEXT NOT NULL DEFAULT '[]',
            ended INTEGER NOT NULL DEFAULT 0
        );
        CREATE TABLE IF NOT EXISTS online_duo_rewards (
            name TEXT NOT NULL, tid INTEGER NOT NULL, round INTEGER NOT NULL, partner TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS online_duo_invites (
            from_name TEXT NOT NULL, from_tid INTEGER NOT NULL, to_name TEXT NOT NULL, to_tid INTEGER NOT NULL,
            at REAL NOT NULL, PRIMARY KEY (from_name, from_tid)
        );
        """
    )


def quest(rnd):
    kind, base, text, item, count, money = QUESTS[rnd % len(QUESTS)]
    n = base + base * (rnd // len(QUESTS)) // 2
    return kind, n, text.format(n=n), item, count, money


def duo_of(db, key):
    """(id, partner key, my side 'a'/'b', round, my progress, partner progress, seen) or None."""
    row = db.execute("SELECT id, a_name, a_tid, b_name, b_tid, round, prog_a, prog_b, seen FROM online_duos"
                     " WHERE ended = 0 AND ((a_name = ? AND a_tid = ?) OR (b_name = ? AND b_tid = ?))",
                     (*key, *key)).fetchone()
    if row is None:
        return None
    did, an, at, bn, bt, rnd, pa, pb, seen = row
    if (an, at) == tuple(key):
        return did, (bn, bt), "a", rnd, pa, pb, json.loads(seen)
    return did, (an, at), "b", rnd, pb, pa, json.loads(seen)


def invite_to(db, key, now):
    return db.execute("SELECT from_name, from_tid FROM online_duo_invites WHERE to_name = ? AND to_tid = ? AND at > ?"
                      " ORDER BY at DESC", (*key, now - INVITE_SECONDS)).fetchone()


def invite_from(db, key, now):
    return db.execute("SELECT to_name, to_tid FROM online_duo_invites WHERE from_name = ? AND from_tid = ? AND at > ?",
                      (*key, now - INVITE_SECONDS)).fetchone()


def rewards(db, key):
    return db.execute("SELECT rowid, round, partner FROM online_duo_rewards WHERE name = ? AND tid = ?",
                      (*key,)).fetchall()


def progress(ctx, kind, amount=1, value=None):
    """Counts something one of the pair did towards their quest."""
    db, key = ctx.db, ctx.key
    duo = duo_of(db, key)
    if duo is None:
        return
    did, partner, side, rnd, mine, theirs, seen = duo
    qkind, n, text, *_ = quest(rnd)
    if qkind != kind:
        return
    if value is not None:
        if value in seen:
            return
        seen.append(value)
    mine += amount
    db.execute(f"UPDATE online_duos SET prog_{side} = ?, seen = ? WHERE id = ?", (mine, json.dumps(seen), did))
    total = mine + theirs
    if total < n:
        if total in (n // 2, n - 1):
            msg = f"Duo quest: {total}/{n}!"
            ctx.online.notify(key, msg)
            ctx.online.notify(partner, msg)
        return
    # Done: both get the reward, and the next quest starts.
    for who, other in ((key, partner[0]), (partner, ctx.player.name)):
        db.execute("INSERT INTO online_duo_rewards VALUES (?, ?, ?, ?)", (*who, rnd, other))
        ctx.online.notify(who, f"Duo quest done with {other}! Your reward is in HUB > DUO QUESTS.")
    db.execute("UPDATE online_duos SET round = round + 1, prog_a = 0, prog_b = 0, seen = '[]' WHERE id = ?", (did,))


def on_catch(ctx, args):
    species, shiny, group, num, fished = (args + [0] * 5)[:5]
    if species <= 0:
        return
    progress(ctx, "catch")
    progress(ctx, "species", value=species)
    progress(ctx, "places", value=group * 256 + num)


def on_fish(ctx, args):
    progress(ctx, "fish")


def prize_name(item, count):
    name = item.replace("_", " ")
    return f"{count} {name[:-1] + 'IES' if name.endswith('Y') else name + 'S'}" if count > 1 else f"1 {name}"


def online_players(ctx):
    return [p for p in ctx.server.players.values() if p.name is not None and p is not ctx.player]


def cmd_duo(ctx, args, data):
    db, key = ctx.db, ctx.key
    now = ctx.online.now()
    word = args[0]

    if word == "VIEW":
        duo = duo_of(db, key)
        if duo:
            did, partner, side, rnd, mine, theirs, seen = duo
            kind, n, text, item, count, money = quest(rnd)
            total = min(mine + theirs, n)
            rows = board(f"DUO QUEST {rnd + 1}: {text}",
                         [(6, "TRAINER"), (150, "COUNTED")],
                         [["%", total * 100 // n, f"{ctx.player.name} & {partner[0]}", "", f"{total}/{n}"],
                          [f"*{ctx.player.name}", mine], [partner[0], theirs]])
            got = len(rewards(db, key))
            message = f"Teamed up with {partner[0]}: {total}/{n}.~Reward: {prize_name(item, count)}, ${money}."
            return answer(message, code=REWARD if got else ACTIVE, vars=[partner[0], got], rows=rows)
        inv = invite_to(db, key, now)
        if inv:
            return answer(f"{inv[0]} wants to team up~for duo quests. Will you?", code=INVITED, vars=[inv[0]])
        asked = invite_from(db, key, now)
        if asked:
            return answer(f"Waiting for {asked[0]} to answer.",
                          code=ASKED, vars=[asked[0]])
        return answer("Team up with another trainer and catch POKéMON together!", code=NONE)

    if word == "WHO":
        busy = set()
        for (an, at, bn, bt) in db.execute("SELECT a_name, a_tid, b_name, b_tid FROM online_duos WHERE ended = 0"):
            busy |= {(an, at), (bn, bt)}
        rows = [[p.name, p.id] for p in online_players(ctx) if (p.name, p.tid) not in busy]
        if not rows:
            raise Refused("Nobody online is free to team up right now.")
        return answer("Who will you team up with?", rows=rows)

    if word == "ASK":
        target = ctx.server.players.get(int(args[1]) if len(args) > 1 and args[1].isdigit() else -1)
        if target is None or target.name is None:
            raise Refused("That trainer has gone offline.")
        if duo_of(db, (target.name, target.tid)):
            raise Refused(f"{target.name} already has a partner.")
        db.execute("INSERT OR REPLACE INTO online_duo_invites VALUES (?, ?, ?, ?, ?)",
                   (*key, target.name, target.tid, now))
        ctx.online.notify((target.name, target.tid),
                          f"{ctx.player.name} wants to team up! Answer in HUB > DUO QUESTS.")
        return answer(f"You asked {target.name} to team up.")

    if word in ("YES", "NO"):
        inv = invite_to(db, key, now)
        if inv is None:
            raise Refused("That invitation has run out.")
        db.execute("DELETE FROM online_duo_invites WHERE (from_name = ? AND from_tid = ?)"
                   " OR (from_name = ? AND from_tid = ?)", (*inv, *key))
        if word == "NO":
            ctx.online.notify(inv, f"{ctx.player.name} can't team up right now.")
            return answer()
        if duo_of(db, inv):
            raise Refused(f"{inv[0]} has teamed up with someone else.")
        db.execute("INSERT INTO online_duos (a_name, a_tid, b_name, b_tid) VALUES (?, ?, ?, ?)", (*inv, *key))
        text = quest(0)[2]
        ctx.online.notify(inv, f"{ctx.player.name} teamed up with you! First duo quest: {text}.")
        return answer(f"You teamed up with {inv[0]}!~First quest: {text}.")

    if word == "CLAIM":
        got = rewards(db, key)
        if not got:
            raise Refused("No rewards to collect yet.")
        rows, said = [], []
        for rowid, rnd, partner in got:
            kind, n, text, item, count, money = quest(rnd)
            db.execute("DELETE FROM online_duo_rewards WHERE rowid = ?", (rowid,))
            rows += [["+ITEM", ITEMS[item], count], ["+MONEY", money]]
            said.append(f"{prize_name(item, count)} and ${money}")
        duo = duo_of(db, key)
        return answer(f"{ctx.player.name} got {', '.join(said)}!", vars=[duo[1][0] if duo else ""], rows=rows)

    if word == "LEAVE":
        duo = duo_of(db, key)
        if duo is None:
            raise Refused("You don't have a partner.")
        db.execute("UPDATE online_duos SET ended = 1 WHERE id = ?", (duo[0],))
        ctx.online.notify(duo[1], f"{ctx.player.name} split up your duo.")
        return answer(f"You split up with {duo[1][0]}.")

    raise ValueError("bad DUO")


COMMANDS = {"DUO": cmd_duo}
EVENTS = {"CATCH": on_catch, "FISH": on_fish}
