"""Shared wild spawns and map channels (docs/shared_map.md).

Shared wild spawns: every game online makes the visible wild Pokémon near
its player (the expansion's overworld encounters) and reports them here.
Everyone else on the same map and channel sees them too: one set of wild
Pokémon for everybody. The first player to battle one claims it; the others
see it frozen in place with a VS bubble, and talking to it offers to watch
the battle (WATCH, like a gym battle). Rare spawns (shiny, or a slot of the
map's table at 5% or less, or a roamer, swarm or Feebas) are announced.

Map channels: a map holds CHANNEL_SIZE players at a time (you plus the 24
others a game can show). More than that and the server opens channel 2, 3...
Players only see, trade and battle the players and wild Pokémon of their own
channel. Arriving on a map you go to the channel where most of your friends
are, else the first one with room; START > ONLINE > CHANNEL lists them and
moves you to another (a friend's, say).

Client -> server:
    WSPAWN uid gfx level flags x y facing    one of my spawns appeared or moved
    WGONE uid                                one of my spawns is gone
    WCLAIM owner uid                         I want to battle this one
    WEND owner uid result                    my battle with it ended: 1 caught, 2 beaten, 3 ran
    CHAN                                     list the channels of my map
    CHAN JOIN n                              move me to channel n
Server -> client:
    WILD owner uid gfx level flags x y facing group num claim claimer
                                             a spawn of someone on my map and channel (claim 1: being battled)
    WILDGONE owner uid
    WCLAIMED owner uid ok|taken|gone claimer the answer to WCLAIM
    WRARE owner uid                          a rare one just appeared here
    CHANNEL n count                          my channel on this map, and how many there are
    CHANROW n players friends friendname     one per channel ("-": no friend there), then
    CHANEND n count
    CHANFULL n                               the answer to CHAN JOIN when n has no room

`install(Server)` adds this to the netplay server class.
"""

import logging
import os
import time

log = logging.getLogger("netplay.shared_map")

CHANNEL_SIZE = max(1, int(os.environ.get("NETPLAY_CHANNEL_SIZE", "25")))  # you + NETPLAY_MAX_REMOTES
MAX_SPAWNS = 6           # per player (the game makes at most OWE_SPAWNS_MAX = 4)
RARE_AGAIN = 120         # seconds before the same player's rare spawn is announced again

OBJ_EVENT_MON = 1 << 14
OBJ_EVENT_MON_SHINY = 1 << 13
SPECIES_MASK = OBJ_EVENT_MON_SHINY - 1 - (1 << 12)  # OBJ_EVENT_MON_SPECIES_MASK
FLAG_RARE = 1
FLAG_ASLEEP = 2
FLAG_WATER = 4
FLAGS = FLAG_RARE | FLAG_ASLEEP | FLAG_WATER
RESULT_CAUGHT, RESULT_BEATEN, RESULT_RAN = 1, 2, 3


class Spawn:
    __slots__ = ("owner", "uid", "gfx", "level", "flags", "x", "y", "facing", "claimer", "map", "room")

    def __init__(self, owner, uid):
        self.owner = owner
        self.uid = uid
        self.claimer = None
        self.gfx = self.level = self.flags = self.x = self.y = self.facing = 0
        self.map = owner.map
        self.room = owner.room

    @property
    def species(self):
        return self.gfx & SPECIES_MASK

    @property
    def shiny(self):
        return bool(self.gfx & OBJ_EVENT_MON_SHINY)

    def line(self):
        claimer = self.claimer.name if self.claimer is not None else "-"
        return (f"WILD {self.owner.id} {self.uid} {self.gfx} {self.level} {self.flags} {self.x} {self.y}"
                f" {self.facing} {self.map[0]} {self.map[1]} {1 if self.claimer else 0} {claimer}")


def _names():
    try:
        from online_sightings import place_name, species_name
        return place_name, species_name
    except Exception:  # the id tables are optional
        return (lambda g, n: "somewhere"), (lambda s: f"No.{s}")


# --- channels ---------------------------------------------------------------

def channel_of(room):
    return room[1] if isinstance(room, tuple) and len(room) == 2 else 1


def base_of(room):
    return room[0] if isinstance(room, tuple) and len(room) == 2 else room


def channel_counts(server, map_key, base, exclude=None):
    """Players in each channel of the map (within the same base room)."""
    counts = {}
    for p in server.players.values():
        if p is exclude or p.name is None or p.map != map_key or base_of(p.room) != base:
            continue
        ch = channel_of(p.room)
        counts[ch] = counts.get(ch, 0) + 1
    return counts


def friend_channels(server, player, map_key, base):
    """channel -> [names of the player's friends there]."""
    try:
        keys = set(server.friends.list(player.key))
    except Exception:
        return {}
    out = {}
    for p in server.players.values():
        if p is not player and p.key in keys and p.map == map_key and base_of(p.room) == base:
            out.setdefault(channel_of(p.room), []).append(p.name)
    return out


def place(self, player, map_key):
    """The room (base room, channel) for `player` on `map_key`."""
    base = self.room_of(player, map_key)
    if player.map == map_key and isinstance(player.room, tuple) and base_of(player.room) == base:
        return player.room  # staying put
    counts = channel_counts(self, map_key, base, exclude=player)
    pick = getattr(player, "channel_pick", None)
    if pick and pick[0] == map_key and counts.get(pick[1], 0) < CHANNEL_SIZE:
        return (base, pick[1])
    friends = friend_channels(self, player, map_key, base)
    for ch in sorted(friends, key=lambda c: (-len(friends[c]), c)):
        if counts.get(ch, 0) < CHANNEL_SIZE:
            return (base, ch)
    ch = 1
    while counts.get(ch, 0) >= CHANNEL_SIZE:
        ch += 1
    return (base, ch)


def channel_count(self, map_key, base):
    counts = channel_counts(self, map_key, base)
    return max(counts) if counts else 1


def tell_channel(self, player):
    """CHANNEL n count, when the map has more than one channel (the bridge
    takes channel 1 of 1 on every new map until told otherwise)."""
    if player.map is None:
        return
    ch, count = channel_of(player.room), channel_count(self, player.map, base_of(player.room))
    if ch != 1 or count > 1:
        player.send(f"CHANNEL {ch} {count}")


def cmd_chan(self, player, *args):
    if player.map is None:
        raise ValueError("not on a map")
    base = base_of(player.room)
    if not args:
        counts = channel_counts(self, player.map, base)
        friends = friend_channels(self, player, player.map, base)
        for ch in range(1, max(counts) + 1 if counts else 2):
            names = friends.get(ch, [])
            player.send(f"CHANROW {ch} {counts.get(ch, 0)} {len(names)} {names[0] if names else '-'}")
        player.send(f"CHANEND {channel_of(player.room)} {max(counts) if counts else 1}")
        return
    if args[0] != "JOIN" or len(args) != 2:
        raise ValueError("bad CHAN")
    ch = int(args[1])
    counts = channel_counts(self, player.map, base, exclude=player)
    if ch < 1 or ch > max(list(counts) + [channel_of(player.room)]) or counts.get(ch, 0) >= CHANNEL_SIZE:
        player.send(f"CHANFULL {ch}")
        return
    player.channel_pick = (player.map, ch)
    if ch != channel_of(player.room):
        self.relocate(player, player.map, (base, ch))  # says CHANNEL unless it's 1 of 1
        if channel_count(self, player.map, base) > 1 or ch != 1:
            return
    player.send(f"CHANNEL {ch} {channel_count(self, player.map, base)}")


# --- spawns -------------------------------------------------------------------

def spawns_of(self, player):
    return [s for s in self.spawns.values() if s.owner is player]


def drop_spawns(self, player):
    """The player left their map (or the server): their spawns go with them,
    and anything they had claimed is free again."""
    for s in spawns_of(self, player):
        self.spawns.pop((player.id, s.uid), None)
        self.broadcast_map(s.map, f"WILDGONE {player.id} {s.uid}", exclude=player, room=s.room)
    release_claims(self, player)


def release_claims(self, player):
    for s in self.spawns.values():
        if s.claimer is player:
            s.claimer = None
            self.broadcast_map(s.map, s.line(), room=s.room)


def same_place(a, b):
    return a.map is not None and a.map == b.map and a.room == b.room


def cmd_wspawn(self, player, uid, gfx, level, flags, x, y, facing):
    uid, gfx, level, flags = int(uid), int(gfx), int(level), int(flags)
    x, y, facing = int(x), int(y), int(facing)
    if player.map is None:
        raise ValueError("not on a map")
    if not (0 < uid <= 0xFFFF) or not (gfx & OBJ_EVENT_MON) or not (1 <= level <= 100):
        raise ValueError("bad WSPAWN")
    if abs(x) > 1024 or abs(y) > 1024 or not (0 <= facing <= 8):
        raise ValueError("bad WSPAWN")
    key = (player.id, uid)
    spawn = self.spawns.get(key)
    new = spawn is None
    if new:
        mine = spawns_of(self, player)
        if len(mine) >= MAX_SPAWNS:
            oldest = mine[0]
            self.spawns.pop((player.id, oldest.uid), None)
            self.broadcast_map(oldest.map, f"WILDGONE {player.id} {oldest.uid}", exclude=player, room=oldest.room)
        spawn = self.spawns[key] = Spawn(player, uid)
    spawn.gfx, spawn.level, spawn.flags = gfx, level, flags & FLAGS
    spawn.x, spawn.y, spawn.facing = x, y, facing
    spawn.map, spawn.room = player.map, player.room
    self.broadcast_map(player.map, spawn.line(), exclude=player, room=player.room)
    if new and (spawn.shiny or spawn.flags & FLAG_RARE):
        announce_rare(self, player, spawn)


def announce_rare(self, player, spawn):
    self.broadcast_map(player.map, f"WRARE {player.id} {spawn.uid}", exclude=player, room=player.room)
    now = self.clock() if callable(getattr(self, "clock", None)) else time.time()
    last = getattr(player, "rare_said", None)
    if last is not None and now - last < RARE_AGAIN:
        return
    player.rare_said = now
    online = getattr(self, "online", None)
    if online is None:
        return
    place_name, species_name = _names()
    what = ("A shiny " if spawn.shiny else "A rare ") + species_name(spawn.species)
    message = f"{what} appeared at {place_name(*spawn.map)}! (near {player.name})"
    # Everyone online elsewhere; those right here got WRARE (a toast) instead.
    for p in list(self.players.values()):
        if p.name is not None and p is not player and not same_place(p, player):
            online.notify(p.key, message)


def cmd_wgone(self, player, uid):
    spawn = self.spawns.pop((player.id, int(uid)), None)
    if spawn is not None:
        self.broadcast_map(spawn.map, f"WILDGONE {player.id} {spawn.uid}", exclude=player, room=spawn.room)


def cmd_wclaim(self, player, owner, uid):
    owner, uid = int(owner), int(uid)
    spawn = self.spawns.get((owner, uid))
    if spawn is None or spawn.map != player.map or spawn.room != player.room:
        player.send(f"WCLAIMED {owner} {uid} gone -")
        return
    if spawn.claimer is not None and spawn.claimer is not player:
        player.send(f"WCLAIMED {owner} {uid} taken {spawn.claimer.name}")
        return
    spawn.claimer = player
    player.send(f"WCLAIMED {owner} {uid} ok {player.name}")
    self.broadcast_map(spawn.map, spawn.line(), exclude=player, room=spawn.room)


def cmd_wend(self, player, owner, uid, result):
    owner, uid, result = int(owner), int(uid), int(result)
    spawn = self.spawns.get((owner, uid))
    if spawn is None or spawn.claimer is not player:
        return
    if result == RESULT_RAN:
        spawn.claimer = None
        self.broadcast_map(spawn.map, spawn.line(), exclude=player, room=spawn.room)
        return
    if result not in (RESULT_CAUGHT, RESULT_BEATEN):
        raise ValueError("bad WEND")
    self.spawns.pop((owner, uid), None)
    # The owner's game removes it too.
    self.broadcast_map(spawn.map, f"WILDGONE {owner} {uid}", exclude=player, room=spawn.room)
    if result == RESULT_CAUGHT and (spawn.shiny or spawn.flags & FLAG_RARE):
        online = getattr(self, "online", None)
        if online is not None:
            _, species_name = _names()
            what = ("the shiny " if spawn.shiny else "the rare ") + species_name(spawn.species)
            online.announce(f"{player.name} caught {what}!", exclude=player.key)


def show_spawns(self, player):
    """Everything on the player's map and channel that isn't theirs."""
    for s in self.spawns.values():
        if s.owner is not player and s.map == player.map and s.room == player.room:
            player.send(s.line())


def install(server_cls):
    for fn in (place, channel_count, tell_channel, cmd_chan, cmd_wspawn, cmd_wgone, cmd_wclaim, cmd_wend,
               show_spawns):
        setattr(server_cls, fn.__name__, fn)

    init = server_cls.__init__

    def __init__(self, *args, **kwargs):
        init(self, *args, **kwargs)
        self.spawns = {}  # (owner id, uid) -> Spawn

    server_cls.__init__ = __init__

    relocate = server_cls.relocate

    def relocate_with_spawns(self, player, new_map, new_room):
        if (player.map, player.room) == (new_map, new_room):
            return relocate(self, player, new_map, new_room)
        seen = [s for s in self.spawns.values()
                if s.owner is not player and s.map == player.map and s.room == player.room]
        if player.map == new_map:
            # Another channel of the same map: my spawns come along.
            mine = spawns_of(self, player)
            for s in mine:
                self.broadcast_map(s.map, f"WILDGONE {player.id} {s.uid}", exclude=player, room=s.room)
                s.room = new_room
            release_claims(self, player)
        else:
            mine = []
            drop_spawns(self, player)
        for s in seen:
            player.send(f"WILDGONE {s.owner.id} {s.uid}")
        relocate(self, player, new_map, new_room)
        for s in mine:
            self.broadcast_map(s.map, s.line(), exclude=player, room=s.room)
        show_spawns(self, player)
        tell_channel(self, player)

    server_cls.relocate = relocate_with_spawns

    disconnect = server_cls.disconnect

    def disconnect_with_spawns(self, player):
        if player.id in self.players:
            drop_spawns(self, player)
        disconnect(self, player)

    server_cls.disconnect = disconnect_with_spawns
