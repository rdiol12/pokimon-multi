"""Draft battles: everyone drafts from the same pool, so matches are fair.

The pool changes every day and is the same for every player: six picks from
the game's own list of draft Pokémon (the game sends the list's size and the
server answers with indexes). Each player builds a team of three from it,
then queues; the server pairs two queued players and starts an ordinary
battle session between them, which counts on the PvP ladder."""

import datetime
import random
import sys

from ext import Refused, text

POOL_SIZE = 6


def pool(list_size, day=None):
    day = day or datetime.date.today().isoformat()
    rng = random.Random(f"draft:{day}")
    return day, rng.sample(range(list_size), min(POOL_SIZE, list_size))


def cmd_draft(ctx, args, data):
    server, player = ctx.server, ctx.player
    queue = server.__dict__.setdefault("draft_queue", [])
    sub = args[0]
    if sub == "POOL":
        list_size = int(args[1])
        if not POOL_SIZE <= list_size <= 500:
            raise ValueError("bad list size")
        day, picks = pool(list_size)
        return text([[day] + picks])
    if sub == "QUEUE":
        if player.session:
            raise Refused("You're busy.")
        queue[:] = [p for p in queue if p.id in server.players and p.session is None and p is not player]
        if not queue:
            queue.append(player)
            return b"QUEUED"
        other = queue.pop(0)
        Session = sys.modules[type(server).__module__].Session
        session = Session(next(server._sids), "battle", other, player)
        session.started = True
        server.sessions[session.id] = session
        other.session = player.session = session
        other.send(f"START {session.id} battle {player.id} 0")
        player.send(f"START {session.id} battle {other.id} 1")
        return b"MATCHED"
    if sub == "LEAVE":
        if player in queue:
            queue.remove(player)
        return b""
    raise ValueError("bad DRAFT request")


COMMANDS = {"DRAFT": cmd_draft}
