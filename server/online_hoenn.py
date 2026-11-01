"""Original Hoenn, adjusted for many players (docs/hoenn_multi.md):

- Shared milestones: everyone online hears when someone beats a gym, becomes
  Champion or catches a legendary, and the server keeps who cleared each gym
  first (HREC, shown by each gym's statues and HUB > HOENN RECORDS).
- Quick match: the left desk upstairs in every Pokémon Center finds another
  trainer who wants a battle or a trade (QM).
- Team up: lend your team to another trainer; it fights beside them in
  their big story battles and legendary battles (TEAMUP).
"""

import time

from online import Refused, answer, board
from online_ids import SPECIES_NAMES

# The gyms in badge order, then the League. (place, leader, badge)
GYMS = [
    ("RUSTBORO GYM", "ROXANNE", "STONE BADGE"),
    ("DEWFORD GYM", "BRAWLY", "KNUCKLE BADGE"),
    ("MAUVILLE GYM", "WATTSON", "DYNAMO BADGE"),
    ("LAVARIDGE GYM", "FLANNERY", "HEAT BADGE"),
    ("PETALBURG GYM", "NORMAN", "BALANCE BADGE"),
    ("FORTREE GYM", "WINONA", "FEATHER BADGE"),
    ("MOSSDEEP GYM", "TATE & LIZA", "MIND BADGE"),
    ("SOOTOPOLIS GYM", "JUAN", "RAIN BADGE"),
]
CHAMPION = len(GYMS)  # milestone index of becoming Champion
BOARD_LINES = 8

LEGENDS = {
    "ARTICUNO", "ZAPDOS", "MOLTRES", "MEWTWO", "MEW", "RAIKOU", "ENTEI", "SUICUNE", "LUGIA", "HO_OH", "CELEBI",
    "REGIROCK", "REGICE", "REGISTEEL", "LATIAS", "LATIOS", "KYOGRE", "GROUDON", "RAYQUAZA", "JIRACHI",
    "DEOXYS_NORMAL", "DEOXYS", "CRESSELIA", "HEATRAN", "DARKRAI", "SHAYMIN", "SHAYMIN_LAND",
}

# Quick match
QM_KINDS = {"0": "battle", "1": "trade"}
QM_WAIT = 10 * 60
MATCHED, WAITING = 1, 0  # VAR_0x800B after QM JOIN

# Team up: what VAR_0x800B is after TEAMUP VIEW.
TEAM_NONE, TEAM_INVITED, TEAM_ASKED, TEAM_READY, TEAM_LENT = 0, 1, 2, 3, 4
TEAM_INVITE_SECONDS = 15 * 60
RECORD_SIZE = 160  # struct SecretBase, as Online_AddTeamRecord sends it


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS hoenn_clears (
            idx INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, at REAL NOT NULL,
            PRIMARY KEY (idx, name, tid)
        );
        CREATE TABLE IF NOT EXISTS hoenn_legends (
            species INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, at REAL NOT NULL
        );
        CREATE TABLE IF NOT EXISTS hoenn_team_invites (
            from_name TEXT NOT NULL, from_tid INTEGER NOT NULL, to_name TEXT NOT NULL, to_tid INTEGER NOT NULL,
            at REAL NOT NULL, PRIMARY KEY (from_name, from_tid)
        );
        CREATE TABLE IF NOT EXISTS hoenn_teams (
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            partner_name TEXT NOT NULL, partner_tid INTEGER NOT NULL, team BLOB NOT NULL, at REAL NOT NULL,
            PRIMARY KEY (name, tid)
        );
        """
    )


def species_name(species):
    return SPECIES_NAMES.get(species, f"No.{species}").replace("_", " ")


def is_legend(species):
    return SPECIES_NAMES.get(species, "") in LEGENDS


def date(at):
    return time.strftime("%b %d", time.gmtime(at)).upper()


def milestone_name(idx):
    return "CHAMPION" if idx == CHAMPION else GYMS[idx][0]


# --- shared milestones --------------------------------------------------

def on_milestone(ctx, args):
    """MILESTONE badgeMask champion fresh: what the player has now. fresh is
    1 when it changed while they played online (announce it), 0 for the
    first report after connecting (just keep the records)."""
    mask, champion, fresh = (args + [0] * 3)[:3]
    db, key, now = ctx.db, ctx.key, ctx.online.now()
    have = [i for i in range(len(GYMS)) if mask & (1 << i)] + ([CHAMPION] if champion else [])
    for idx in have:
        if db.execute("SELECT 1 FROM hoenn_clears WHERE idx = ? AND name = ? AND tid = ?", (idx, *key)).fetchone():
            continue
        first = db.execute("SELECT COUNT(*) FROM hoenn_clears WHERE idx = ?", (idx,)).fetchone()[0] == 0
        db.execute("INSERT INTO hoenn_clears VALUES (?, ?, ?, ?)", (idx, *key, now))
        if not fresh:
            continue
        if idx == CHAMPION:
            msg = f"{ctx.player.name} became the Champion of Hoenn!"
        else:
            place, leader, badge = GYMS[idx]
            msg = f"{ctx.player.name} beat {leader} and earned the {badge}!"
        if first:
            msg += " The first trainer ever!"
        ctx.online.announce(msg)
    db.commit()


def on_catch(ctx, args):
    species = (args + [0])[0]
    if species <= 0 or not is_legend(species):
        return
    db, now = ctx.db, ctx.online.now()
    first = db.execute("SELECT COUNT(*) FROM hoenn_legends WHERE species = ?", (species,)).fetchone()[0] == 0
    db.execute("INSERT INTO hoenn_legends VALUES (?, ?, ?, ?)", (species, *ctx.key, now))
    msg = f"{ctx.player.name} caught {species_name(species)}!"
    if first:
        msg += " Nobody had caught one before!"
    ctx.online.announce(msg)


def clears(db, idx):
    return db.execute("SELECT name, at FROM hoenn_clears WHERE idx = ? ORDER BY at, rowid", (idx,)).fetchall()


def cmd_hrec(ctx, args, data):
    """HREC n: the records of gym n (0-7) or the League (8). HREC ALL: all of them."""
    db = ctx.db
    if args and args[0] == "ALL":
        rows = []
        for idx in range(CHAMPION + 1):
            got = clears(db, idx)
            first = got[0][0] if got else "-"
            mine = "*" if any(n == ctx.player.name for n, _ in got) else ""
            rows.append([mine + milestone_name(idx), first, len(got)])
        return answer(rows=board("HOENN RECORDS", [(6, "PLACE"), (100, "FIRST"), (180, "ALL")], rows))
    idx = int(args[0]) if args and args[0].isdigit() else -1
    if not 0 <= idx <= CHAMPION:
        raise ValueError("bad HREC")
    got = clears(db, idx)
    title = "CHAMPIONS OF HOENN" if idx == CHAMPION else f"{GYMS[idx][0]}: {GYMS[idx][1]}"
    if not got:
        return answer("Nobody has cleared it yet.~Be the first!", rows=board(title, [(6, "#"), (24, "TRAINER")], []))
    lines = []
    for i, (name, at) in enumerate(got[:BOARD_LINES], 1):
        lines.append([("*" if name == ctx.player.name else "") + str(i), name, date(at)])
    message = f"{len(got)} trainer{'s' if len(got) != 1 else ''} cleared it.~{got[0][0]} was the first."
    return answer(message, rows=board(title, [(6, "#"), (24, "TRAINER"), (150, "WHEN")], lines))


# --- quick match --------------------------------------------------------

QUEUE = {}  # kind -> {player id: time joined}


def cmd_qm(ctx, args, data):
    """QM JOIN kind (0 battle, 1 trade) / QM LEAVE."""
    me, now = ctx.player, ctx.online.now()
    word = args[0] if args else ""
    if word == "LEAVE":
        for waiting in QUEUE.values():
            waiting.pop(me.id, None)
        return answer("You're off the list.")
    if word != "JOIN" or len(args) < 2 or args[1] not in QM_KINDS:
        raise ValueError("bad QM")
    kind = QM_KINDS[args[1]]
    for waiting in QUEUE.values():
        waiting.pop(me.id, None)
    waiting = QUEUE.setdefault(kind, {})
    for pid, at in sorted(waiting.items(), key=lambda kv: kv[1]):
        other = ctx.server.players.get(pid)
        if other is None or other.name is None or now - at > QM_WAIT:
            del waiting[pid]
            continue
        if other.session:
            continue
        del waiting[pid]
        return answer(f"Found {other.name}, who wants a {kind} too!", code=MATCHED, vars=[other.name, other.id])
    waiting[me.id] = now
    return answer(f"Nobody else is looking for a {kind} yet.~You're on the list now. The next trainer~"
                  f"who looks will challenge you.", code=WAITING)


# --- team up ------------------------------------------------------------

def team_invite_to(db, key, now):
    return db.execute("SELECT from_name, from_tid FROM hoenn_team_invites WHERE to_name = ? AND to_tid = ? AND at > ?"
                      " ORDER BY at DESC", (*key, now - TEAM_INVITE_SECONDS)).fetchone()


def team_invite_from(db, key, now):
    return db.execute("SELECT to_name, to_tid FROM hoenn_team_invites WHERE from_name = ? AND from_tid = ? AND at > ?",
                      (*key, now - TEAM_INVITE_SECONDS)).fetchone()


def team_of(db, key):
    """(partner key, team) lent to `key`, or None."""
    row = db.execute("SELECT partner_name, partner_tid, team FROM hoenn_teams WHERE name = ? AND tid = ?",
                     (*key,)).fetchone()
    return ((row[0], row[1]), row[2]) if row else None


def lent_to(db, key):
    return db.execute("SELECT name, tid FROM hoenn_teams WHERE partner_name = ? AND partner_tid = ?",
                      (*key,)).fetchall()


def on_teamwin(ctx, args):
    """TEAMWIN: the player won a big battle with a partner's team."""
    got = team_of(ctx.db, ctx.key)
    if got:
        ctx.online.notify(got[0], f"{ctx.player.name} won a big battle with your team at their side!")


def cmd_teamup(ctx, args, data):
    db, key, now = ctx.db, ctx.key, ctx.online.now()
    word = args[0] if args else ""

    if word == "VIEW":
        got = team_of(db, key)
        if got:
            return answer(f"{got[0][0]}'s team fights beside you in big battles.", code=TEAM_READY,
                          vars=[got[0][0]])
        inv = team_invite_to(db, key, now)
        if inv:
            return answer(f"{inv[0]} asks for your team to fight beside them in big battles. Lend it?",
                          code=TEAM_INVITED, vars=[inv[0]])
        asked = team_invite_from(db, key, now)
        if asked:
            return answer(f"Waiting for {asked[0]} to answer.", code=TEAM_ASKED, vars=[asked[0]])
        lent = lent_to(db, key)
        if lent:
            return answer(f"Your team fights beside {lent[0][0]}.", code=TEAM_LENT, vars=[lent[0][0]])
        return answer("Ask another trainer to lend you their team. It fights beside you against TEAM AQUA, "
                      "TEAM MAGMA, your rivals and legendary POKéMON.", code=TEAM_NONE)

    if word == "WHO":
        rows = [[p.name, p.id] for p in ctx.server.players.values() if p.name is not None and p is not ctx.player]
        if not rows:
            raise Refused("Nobody else is online right now.")
        return answer("Whose team will you ask for?", rows=rows)

    if word == "ASK":
        target = ctx.server.players.get(int(args[1]) if len(args) > 1 and args[1].isdigit() else -1)
        if target is None or target.name is None:
            raise Refused("That trainer has gone offline.")
        db.execute("INSERT OR REPLACE INTO hoenn_team_invites VALUES (?, ?, ?, ?, ?)",
                   (*key, target.name, target.tid, now))
        ctx.online.notify((target.name, target.tid),
                          f"{ctx.player.name} asks you to team up! Answer in HUB > TEAM UP.")
        return answer(f"You asked {target.name} to team up.")

    if word in ("YES", "NO"):
        inv = team_invite_to(db, key, now)
        if inv is None:
            raise Refused("That invitation has run out.")
        db.execute("DELETE FROM hoenn_team_invites WHERE from_name = ? AND from_tid = ?", inv)
        if word == "NO":
            ctx.online.notify(inv, f"{ctx.player.name} can't team up right now.")
            return answer()
        if len(data) != RECORD_SIZE:
            raise ValueError("bad team")
        db.execute("INSERT OR REPLACE INTO hoenn_teams VALUES (?, ?, ?, ?, ?, ?)", (*inv, *key, data, now))
        ctx.online.notify(inv, f"{ctx.player.name} lent you their team! Open HUB > TEAM UP to bring it along.")
        return answer(f"Your team will fight beside {inv[0]}!")

    if word == "GET":
        got = team_of(db, key)
        if got is None:
            raise Refused("Nobody has lent you a team.")
        return answer(f"{got[0][0]}'s team will fight beside you in your next big battles!",
                      vars=[got[0][0]]) + b"\0" + got[1]

    if word == "END":
        got = team_of(db, key)
        if got:
            db.execute("DELETE FROM hoenn_teams WHERE name = ? AND tid = ?", key)
            ctx.online.notify(got[0], f"{ctx.player.name} gave your team back. Thanks for the help!")
            return answer(f"You gave {got[0][0]}'s team back.")
        lent = lent_to(db, key)
        if lent:
            db.execute("DELETE FROM hoenn_teams WHERE partner_name = ? AND partner_tid = ?", key)
            for who in lent:
                ctx.online.notify(tuple(who), f"{ctx.player.name} took their team back.")
            return answer("You took your team back.")
        raise Refused("You aren't teamed up.")

    raise ValueError("bad TEAMUP")


COMMANDS = {"HREC": cmd_hrec, "QM": cmd_qm, "TEAMUP": cmd_teamup}
EVENTS = {"MILESTONE": on_milestone, "CATCH": on_catch, "TEAMWIN": on_teamwin}
