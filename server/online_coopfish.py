"""Co-op fishing: two players by the water reel in a giant Pokémon together.

One player picks another on the same map (HUB > CO-OP FISHING); the other
gets an invitation. Once both are in, each plays three rounds of the reeling
meter. With four hits or more between them the giant is landed, and both
get one (level 45, shiny one time in 64); the biggest landed are on the
giant catch board.
"""

import random

from online import Refused, answer, board
from online_ids import SPECIES, SPECIES_NAMES

GIANTS = ["WAILORD", "GYARADOS", "LAPRAS", "MILOTIC", "SHARPEDO", "KINGDRA", "HUNTAIL", "GOREBYSS",
          "RELICANTH", "DONDOZO", "AVALUGG", "DHELMISE"]
LEVEL = 45
HITS_NEEDED = 4
ROUNDS = 3
INVITE_SECONDS = 60
SHINY_ODDS = 64

WAITING, CAUGHT, GOT_AWAY, DECLINED = 0, 1, 2, 3


class Session:
    def __init__(self, sid, host, guest, species, now):
        self.id = sid
        self.host, self.guest = host, guest  # Player objects
        self.species = species
        self.started = now
        self.joined = False
        self.declined = False
        self.reels = {}
        self.result = None
        self.size = 0


_sessions = {}  # player id -> Session
_next = [1]
rng = random.Random()


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_giants (
            id INTEGER PRIMARY KEY, species INTEGER NOT NULL, size INTEGER NOT NULL,
            a TEXT NOT NULL, b TEXT NOT NULL, at REAL NOT NULL
        );
        """
    )


def mine(ctx):
    s = _sessions.get(ctx.player.id)
    if s and ctx.player.id not in ctx.server.players:
        s = None
    return s


def drop(s):
    for p in (s.host, s.guest):
        if _sessions.get(p.id) is s:
            del _sessions[p.id]


def species_name(sp):
    return SPECIES_NAMES.get(sp, "?").replace("_", " ")


def status(ctx):
    s = mine(ctx)
    if s and s.guest is ctx.player and not s.joined and not s.declined:
        if ctx.online.now() - s.started > INVITE_SECONDS or s.host.id not in ctx.server.players:
            drop(s)
            return []
        return [["COFISH", 1, s.host.name, s.id]]
    return []


def settle(ctx, s):
    if s.result is not None or len(s.reels) < 2:
        return
    hits = sum(s.reels.values())
    s.result = CAUGHT if hits >= HITS_NEEDED else GOT_AWAY
    if s.result == CAUGHT:
        s.size = 4000 + hits * 500 + rng.randrange(1500)  # mm
        ctx.db.execute("INSERT INTO online_giants (species, size, a, b, at) VALUES (?, ?, ?, ?, ?)",
                       (s.species, s.size, s.host.name, s.guest.name, ctx.online.now()))


def cmd_cofish(ctx, args, data):
    sub = args[0]
    me = ctx.player
    if sub == "WHO":
        others = [p for p in ctx.server.players.values()
                  if p is not me and p.name is not None and p.map == me.map and p.map is not None]
        if not others:
            raise Refused("Nobody else is here. Bring a friend to the water!")
        return answer(rows=[[p.name, p.id] for p in others])
    if sub == "ASK":
        other = ctx.server.players.get(int(args[1]))
        if other is None or other.name is None or other.map != me.map:
            raise Refused("They're not here any more.")
        if mine(ctx) or _sessions.get(other.id):
            raise Refused(f"{other.name} is busy fishing with someone else.")
        s = Session(_next[0], me, other, SPECIES[rng.choice(GIANTS)], ctx.online.now())
        _next[0] += 1
        _sessions[me.id] = _sessions[other.id] = s
        return answer(f"Waiting for {other.name}...", vars=[other.name])
    if sub == "BOARD":
        rows = ctx.db.execute("SELECT species, size, a, b FROM online_giants ORDER BY size DESC LIMIT 8").fetchall()
        return answer(rows=board("GIANT CATCHES", [(6, "POK#MON"), (72, "SIZE"), (112, "LANDED BY")],
                                 [[species_name(sp), f"{size / 1000:.1f} m", f"{a} & {b}"] for sp, size, a, b in rows]))
    s = mine(ctx)
    if s is None:
        raise Refused("The other player left.")
    if sub == "JOIN":
        s.joined = True
        return answer(vars=[s.host.name])
    if sub == "NO":
        s.declined = True
        if s.host.id not in ctx.server.players:
            drop(s)
        return answer()
    if sub == "WAIT":
        # The host waits for the answer: 0 still waiting, 1 joined, 3 declined.
        if s.declined:
            drop(s)
            return answer(f"{s.guest.name} said no.", code=DECLINED)
        if s.guest.id not in ctx.server.players:
            drop(s)
            raise Refused("The other player left.")
        if ctx.online.now() - s.started > INVITE_SECONDS:
            drop(s)
            raise Refused(f"{s.guest.name} didn't answer.")
        return answer(code=1 if s.joined else WAITING)
    if sub == "REEL":
        s.reels[me.id] = max(0, min(ROUNDS, int(args[1])))
        settle(ctx, s)
        return answer()
    if sub == "RESULT":
        partner = s.guest if me is s.host else s.host
        if s.result is None:
            if partner.id not in ctx.server.players:
                drop(s)
                raise Refused(f"{partner.name} left. The giant got away...")
            return answer(code=WAITING)
        hits = sum(s.reels.values())
        rows = []
        if s.result == CAUGHT:
            message = (f"You and {partner.name} landed a giant {species_name(s.species)}!~"
                       f"It's {s.size / 1000:.1f} m long! ({hits} hits)")
            rows.append(["+MON", s.species, LEVEL, 1 if rng.randrange(SHINY_ODDS) == 0 else 0])
        else:
            message = f"The giant {species_name(s.species)} got away... ({hits} of {HITS_NEEDED} hits)"
        _sessions.pop(me.id, None)  # the partner keeps theirs until they look
        return answer(message, code=s.result, vars=[partner.name, species_name(s.species)], rows=rows)
    raise ValueError("bad COFISH")


COMMANDS = {"COFISH": cmd_cofish}
