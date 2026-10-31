"""Hide and seek: HUB > HIDE AND SEEK. The player who starts a game is the
seeker; others who join within LOBBY seconds hide. Everyone is taken to the
game's arena (src/online_hide.c). Hiders get HIDE seconds to hide while the
seeker counts, then the seeker has SEEK seconds to find them: the seeker only
sees hiders within SIGHT tiles (Server.can_see), and finds one by talking to
them. A hider who leaves the arena's map is out. Found hiders and the seeker
score points on a weekly board, and win money.

The lead Pokémon helps (the game works out how from its ability or type and
sends it with HIDE JOIN). A seeker's lead may have sharp eyes (sees hiders
from farther away) or sense hiders (the game shows HOT, WARM or COLD for the
closest one); a hider's may blend in (the seeker has to come closer to see
them). The server's clock runs the game: the status row tells the game how
long is left, so a game whose clock runs fast or slow keeps in step.
"""

import math
import os

from online import Refused, answer, board
from online_ids import SPECIES_NAMES

ARENAS = ["PETALBURG WOODS", "ROUTE 117"]  # as sArenas in src/online_hide.c
ARENA_MAPS = ["PETALBURG_WOODS", "ROUTE117"]
MAX_HIDERS = 5
LOBBY = float(os.environ.get("NETPLAY_HIDE_LOBBY", 20))
HIDE = 30
SEEK = 120
SIGHT = 3  # tiles, in both directions
TAG_REACH = 2  # how close the seeker must be to find someone (positions lag a little)
TAG_EARLY = 5  # the seeker's game may finish counting this many seconds early
# Talents (HIDE JOIN's arguments 2 and 3, as in src/online_hide.c): what the
# lead does for a seeker, and for a hider.
SEEK_NONE, SHARP_EYES, SENSE = 0, 1, 2
HIDE_NONE, BLEND_IN = 0, 1
SHARP_SIGHT = 2  # sharp eyes see this many tiles farther
BLEND_SIGHT = 2  # the seeker sees those who blend in this many tiles closer
HOT, WARM = 5, 12  # sensing: the closest hider within this many tiles (else COLD)
FIND_MONEY, ALL_FOUND_MONEY, HIDDEN_MONEY = 500, 1000, 1500
FIND_POINTS, HIDDEN_POINTS = 2, 3
BOARD_SIZE = 8

# Roles (data row 0, field 0, of HIDE JOIN).
SEEKER, HIDER = 0, 1


class Game:
    count = 0

    def __init__(self, arena, seeker, start):
        Game.count += 1
        self.id = Game.count & 0x7FFF
        self.arena = arena
        self.seeker = seeker  # key
        self.hiders = []  # keys, in lane order
        self.names = {}
        self.start = start  # when hiding starts
        self.found = {}  # hider key -> "found" or "left"
        self.paid = set()
        self.seek_talent = SEEK_NONE
        self.blend = set()  # hiders who blend in

    def phase(self, now):
        if now < self.start:
            return "lobby"
        if now < self.start + HIDE:
            return "hide"
        if now < self.start + HIDE + SEEK and len(self.found) < len(self.hiders):
            return "seek"
        return "over"

    def players(self):
        return [self.seeker, *self.hiders]


def setup(db):
    db.execute(
        """CREATE TABLE IF NOT EXISTS online_hide_points (
            week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, points INTEGER NOT NULL,
            PRIMARY KEY (week, name, tid)
        )"""
    )


def games(server):
    if not hasattr(server, "hide_games"):
        server.hide_games = []
    return server.hide_games


def game_of(server, key):
    for game in games(server):
        if tuple(key) in game.players() and tuple(key) not in game.paid:
            return game
    return None


def arena_map(game):
    from online_ids import MAPS
    return tuple(MAPS[ARENA_MAPS[game.arena]])


def install(server):
    def can_see(viewer, other):
        game = game_of(server, viewer.key)
        if game is None or viewer.key != game.seeker or other.key not in game.hiders or other.key in game.found:
            return True
        if not hasattr(server, "online") or game.phase(server.online.now()) not in ("hide", "seek"):
            return True
        return distance(viewer, other) <= sight(game, other.key)

    server.can_see = can_see
    pos = server.cmd_pos

    def cmd_pos(player, *args):
        pos(player, *args)
        # A hider who leaves the arena is out.
        game = game_of(server, player.key)
        if game and player.key in game.hiders and player.key not in game.found and hasattr(server, "online") \
                and game.phase(server.online.now()) in ("hide", "seek") and player.map != arena_map(game):
            game.found[player.key] = "left"
            server.online.notify(game.seeker, f"{player.name} left the {ARENAS[game.arena].title()}!")

    server.cmd_pos = cmd_pos


def distance(a, b):
    return max(abs(a.pos[0] - b.pos[0]), abs(a.pos[1] - b.pos[1]))


def sight(game, hider):
    """How close the seeker must be to see a hider."""
    tiles = SIGHT + (SHARP_SIGHT if game.seek_talent == SHARP_EYES else 0)
    return max(1, tiles - (BLEND_SIGHT if hider in game.blend else 0))


def talent_text(species, talent, seeker):
    name = SPECIES_NAMES.get(species, "").replace("_", " ")
    if not name:
        return ""
    if seeker and talent == SHARP_EYES:
        return f" Your {name}'s sharp eyes spot hiders from farther away!"
    if seeker and talent == SENSE:
        return f" Your {name} can sense if hiders are close!"
    if not seeker and talent == BLEND_IN:
        return f" Your {name} helps you blend in!"
    return ""


def num_arg(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        return 0


def add_points(db, week, key, points):
    db.execute("INSERT INTO online_hide_points VALUES (?, ?, ?, ?) ON CONFLICT (week, name, tid)"
               " DO UPDATE SET points = points + excluded.points", (week, *key, points))


def cmd_hide(ctx, args, data):
    db, key = ctx.db, tuple(ctx.key)
    now = ctx.online.now()
    word = args[0]
    # Games everyone has collected from are gone; so are long-over ones.
    for game in [g for g in games(ctx.server) if g.phase(now) == "over"
                 and (set(g.players()) <= g.paid or now > g.start + HIDE + SEEK + 600)]:
        games(ctx.server).remove(game)

    if word == "LIST":
        rows = []
        for i, name in enumerate(ARENAS):
            open_game = next((g for g in games(ctx.server) if g.arena == i and g.phase(now) == "lobby"
                              and len(g.hiders) < MAX_HIDERS), None)
            rows.append([f"{name}: JOIN {open_game.names[open_game.seeker]}" if open_game else f"{name}: SEEK", i])
        return answer("Where will you play?", rows=rows)

    if word == "JOIN":
        try:
            arena = int(args[1])
        except (IndexError, ValueError):
            arena = -1
        if not 0 <= arena < len(ARENAS):
            raise Refused("That place is closed.")
        if game_of(ctx.server, key):
            raise Refused("You're in a game already!")
        seek_talent, hide_talent, species = num_arg(args, 2), num_arg(args, 3), num_arg(args, 4)
        game = next((g for g in games(ctx.server) if g.arena == arena and g.phase(now) == "lobby"
                     and now < g.start - 3 and len(g.hiders) < MAX_HIDERS), None)
        if game is None:
            game = Game(arena, key, now + LOBBY)
            game.names[key] = ctx.player.name
            games(ctx.server).append(game)
            ctx.online.announce(f"{ctx.player.name} is seeking in {ARENAS[arena]}! Hide from them in"
                                " HUB > HIDE AND SEEK.", exclude=key)
            role, lane = SEEKER, 0
            game.seek_talent = seek_talent if seek_talent in (SHARP_EYES, SENSE) else SEEK_NONE
            said = f"You're the seeker! Others have {round(game.start - now)} seconds to join."
            said += talent_text(species, game.seek_talent, True)
        else:
            role, lane = HIDER, len(game.hiders)
            game.hiders.append(key)
            game.names[key] = ctx.player.name
            ctx.online.notify(game.seeker, f"{ctx.player.name} will hide!")
            said = f"{game.names[game.seeker]} is the seeker. Get ready to hide!"
            if hide_talent == BLEND_IN:
                game.blend.add(key)
                said += talent_text(species, BLEND_IN, False)
        seconds = max(1, round(game.start - now))
        return answer(said, code=seconds, vars=[ARENAS[arena]], rows=[[role, arena, lane, HIDE, SEEK, game.id]])

    game = game_of(ctx.server, key)
    if game is None:
        raise Refused("That game is over.")

    if word == "GO":
        # Hiding starts: VAR_0x800B is how many hide (0: the game is off).
        if not game.hiders:
            games(ctx.server).remove(game)
            return answer("Nobody came to hide... Maybe next time!", code=0)
        n = len(game.hiders)
        if key == game.seeker:
            return answer(f"Close your eyes and count to {HIDE}! Then find {n} hider{'s' if n > 1 else ''}"
                          f" in {SEEK // 60} minutes.", code=n)
        return answer(f"Hide! {game.names[game.seeker]} counts to {HIDE}. Don't leave"
                      f" {ARENAS[game.arena].title()}!", code=n)

    if word == "TAG":
        # The seeker talked to a player (their id).
        if key != game.seeker or game.phase(now) not in ("hide", "seek") or now < game.start + HIDE - TAG_EARLY:
            raise Refused("Not now!")
        target = ctx.server.players.get(int(args[1]) if len(args) > 1 and args[1].isdigit() else -1)
        if target is None or target.key not in game.hiders:
            raise Refused("They aren't playing.")
        if target.key in game.found:
            raise Refused(f"You found {target.name} already!")
        me = ctx.player.pos
        if max(abs(me[0] - target.pos[0]), abs(me[1] - target.pos[1])) > TAG_REACH or target.map != ctx.player.map:
            raise Refused("Too far away!")
        game.found[target.key] = "found"
        ctx.online.notify(target.key, f"{ctx.player.name} found you!")
        left = len(game.hiders) - len(game.found)
        if left:
            return answer(f"Found {target.name}! {left} still hiding.", code=left)
        for k in game.hiders:
            if k != target.key:
                ctx.online.notify(k, f"{ctx.player.name} found everyone!")
        return answer(f"Found {target.name}! That's everyone!", code=0)

    if word == "END":
        # The game's clock may run a little ahead of the server's.
        if game.phase(now) != "over" and now < game.start + HIDE + SEEK - 10:
            raise Refused("The game is still on!")
        game.paid.add(key)
        week = ctx.online.week()
        rows = []
        if key == game.seeker:
            finds = sum(1 for v in game.found.values() if v == "found")
            money = FIND_MONEY * finds + (ALL_FOUND_MONEY if finds and finds == len(game.hiders) else 0)
            if finds:
                add_points(db, week, key, FIND_POINTS * finds)
            said = (f"You found everyone! ${money}!" if finds == len(game.hiders)
                    else f"You found {finds} of {len(game.hiders)}." + (f" ${money}!" if money else ""))
        elif game.found.get(key) == "found":
            money, said = 0, "You were found! Better luck next time."
        elif game.found.get(key) == "left":
            money, said = 0, f"You left {ARENAS[game.arena].title()}, so you're out!"
        else:
            money, said = HIDDEN_MONEY, f"Nobody found you! ${HIDDEN_MONEY}!"
            add_points(db, week, key, HIDDEN_POINTS)
        if money:
            rows.append(["+MONEY", money])
        return answer(said, rows=rows)

    if word == "QUIT":
        game.paid.add(key)
        if key == game.seeker:
            for k in game.hiders:
                ctx.online.notify(k, f"{game.names[key]} stopped seeking. The game is off.")
            games(ctx.server).remove(game)
        elif key in game.hiders:
            game.found.setdefault(key, "left")
        return answer()

    raise ValueError("bad HIDE")


def cmd_hideboard(ctx, args, data):
    week = ctx.online.week()
    rows = []
    for place, (name, tid, points) in enumerate(ctx.db.execute(
            "SELECT name, tid, points FROM online_hide_points WHERE week = ? ORDER BY points DESC, name", (week,)), 1):
        rows.append([f"{'*' if (name, tid) == tuple(ctx.key) else ''}{place}", name, points])
    mine = [r for r in rows if r[0].startswith("*")]
    rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
    return answer("Points: 2 for each hider found, 3 for staying hidden.",
                  rows=board("HIDE AND SEEK: THIS WEEK", [(6, "No."), (30, "TRAINER"), (130, "POINTS")], rows))


def status(ctx):
    game = game_of(ctx.server, ctx.key)
    if game is None:
        return []
    now = ctx.online.now()
    left = len(game.hiders) - len(game.found)
    seconds = max(0, math.ceil(game.start + HIDE + SEEK - now))
    # Sensing: how close the closest hider still hiding is (0: no sense, or
    # not seeking; 1 COLD, 2 WARM, 3 HOT).
    hint = 0
    if tuple(ctx.key) == game.seeker and game.seek_talent == SENSE and game.phase(now) == "seek":
        near = [distance(ctx.player, p) for p in ctx.server.players.values()
                if p.key in game.hiders and p.key not in game.found and p.map == ctx.player.map]
        if near:
            hint = 3 if min(near) <= HOT else 2 if min(near) <= WARM else 1
    return [["HIDE", 1 if tuple(ctx.key) in game.found else 0, left, 1 if game.phase(now) == "over" else 0, game.id,
             seconds, hint]]


COMMANDS = {"HIDE": cmd_hide, "HIDEBOARD": cmd_hideboard}
