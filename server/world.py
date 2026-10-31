"""World features on the netplay server (docs/world.md): what other players
look like (their following Pokémon and mount), open houses, leaderboards,
Safari hunts and the photo gallery.

`install(Server)` adds the cmd_* handlers below to the netplay server; the
mGBA side is tools/netplay_bridge/netplay_world.lua.

Protocol (one line each, client -> server):
    LOOK follower mount           what this player looks like (graphics ids)
    HOUSE open hexdecor           their house: open to visitors, 16 bytes
    SCORE board key value extra   a new result for a leaderboard
    WBOARD board key              ask for a leaderboard
    HUNT                          ask for the current Safari hunt
    PHOTO hexchunk                part of a PNG screenshot
    PHOTOEND group num            the photo is complete
    WORLD                         after WELCOME: send me everyone's LOOK and HOUSE, and HUNT
Server -> client:
    LOOK id follower mount        sent to everyone
    HOUSE id open hexdecor        likewise
    WROW board rank name value extra ... WBOARDEND board myrank
    HUNT id minutesleft           id 0: no hunt
"""

import json
import logging
import os
import re
import time

log = logging.getLogger("netplay.world")

BOARDS = {0: "fish", 1: "berries", 2: "hardcore", 3: "hunt", 8: "dungeon", 9: "treasure",
          10: "factory", 11: "arena", 12: "pyramid"}
BOARD_ROWS = 10
HUNT_SECONDS = 30 * 60
MAX_PHOTO_BYTES = 256 * 1024
HEX_RE = re.compile(r"^[0-9a-fA-F]*$")


def current_hunt(now=None):
    """Hunts run back to back, HUNT_SECONDS each. Ids are never 0."""
    now = time.time() if now is None else now
    slot = int(now // HUNT_SECONDS)
    hunt_id = slot % 0xFFFF + 1
    left = HUNT_SECONDS - (now - slot * HUNT_SECONDS)
    return hunt_id, max(1, int(left // 60))


class WorldStore:
    """Leaderboards, kept in a JSON file next to the league database. Each
    board keeps every player's best result."""

    def __init__(self, path):
        self.path = path
        self.boards = {}
        if path and os.path.exists(path):
            with open(path) as f:
                self.boards = json.load(f)

    def save(self):
        if not self.path:
            return
        tmp = self.path + ".tmp"
        with open(tmp, "w") as f:
            json.dump(self.boards, f)
        os.replace(tmp, self.path)

    def board(self, board, key):
        return self.boards.setdefault(f"{BOARDS[board]}:{key}", {})

    def submit(self, board, key, player_key, name, value, extra):
        rows = self.board(board, key)
        ident = f"{player_key[0]}/{player_key[1]}"
        best = rows.get(ident)
        if best is None or value > best["value"]:
            rows[ident] = {"name": name, "value": value, "extra": extra}
            self.save()
            return True
        return False

    def top(self, board, key):
        rows = self.board(board, key)
        return sorted(rows.items(), key=lambda kv: -kv[1]["value"])


class PlayerWorld:
    def __init__(self):
        self.look = (0, 0)
        self.house = (0, "00" * 16)
        self.photo = bytearray()


def _world(player):
    if not hasattr(player, "world"):
        player.world = PlayerWorld()
    return player.world


def _check_hex(text, length=None):
    if not HEX_RE.match(text) or len(text) % 2 or (length is not None and len(text) != length):
        raise ValueError("bad hex")
    return text.lower()


def cmd_look(self, player, follower, mount):
    w = _world(player)
    w.look = (int(follower) & 0xFFFF, int(mount) & 0xFFFF)
    self.broadcast_all(f"LOOK {player.id} {w.look[0]} {w.look[1]}", exclude=player)


def cmd_house(self, player, is_open, decor):
    w = _world(player)
    w.house = (1 if int(is_open) else 0, _check_hex(decor, 32))
    self.broadcast_all(f"HOUSE {player.id} {w.house[0]} {w.house[1]}", exclude=player)


def cmd_score(self, player, board, key, value, extra):
    board = int(board)
    if board not in BOARDS:
        raise ValueError("no such board")
    key = int(key) if board == 3 else 0
    if board == 3 and key != current_hunt()[0]:
        return  # that hunt is over
    self.world.submit(board, key, player.key, player.name, int(value) & 0xFFFFFFFF, int(extra) & 0xFFFFFFFF)


def cmd_wboard(self, player, board, key):
    board = int(board)
    if board not in BOARDS:
        raise ValueError("no such board")
    key = int(key) if board == 3 else 0
    rows = self.world.top(board, key)
    me = f"{player.key[0]}/{player.key[1]}"
    my_rank = next((i for i, (ident, _) in enumerate(rows, 1) if ident == me), 0)
    for i, (_, row) in enumerate(rows[:BOARD_ROWS], 1):
        player.send(f"WROW {board} {i} {row['name']} {row['value']} {row['extra']}")
    player.send(f"WBOARDEND {board} {min(my_rank, 255)}")


def cmd_hunt(self, player):
    hunt_id, left = current_hunt()
    player.send(f"HUNT {hunt_id} {left}")


def cmd_photo(self, player, chunk):
    w = _world(player)
    w.photo += bytes.fromhex(_check_hex(chunk))
    if len(w.photo) > MAX_PHOTO_BYTES:
        w.photo = bytearray()
        raise ValueError("photo too large")


def cmd_photoend(self, player, group, num):
    w = _world(player)
    data, w.photo = bytes(w.photo), bytearray()
    if not data.startswith(b"\x89PNG") or not self.gallery_dir:
        return
    os.makedirs(self.gallery_dir, exist_ok=True)
    stamp = time.strftime("%Y%m%d-%H%M%S")
    name = f"{stamp}-{player.name}-{player.id}.png"
    with open(os.path.join(self.gallery_dir, name), "wb") as f:
        f.write(data)
    index = os.path.join(self.gallery_dir, "index.json")
    entries = []
    if os.path.exists(index):
        with open(index) as f:
            entries = json.load(f)
    entries.append({"file": name, "player": player.name, "map": [int(group), int(num)], "time": int(time.time())})
    with open(index, "w") as f:
        json.dump(entries[-500:], f)
    log.info("photo from %s: %s", player.name, name)


def cmd_world(self, player):
    """Sent by the bridge after WELCOME: everyone's look and house, and
    the current hunt."""
    for p in self.players.values():
        if p is player or p.name is None or not hasattr(p, "world"):
            continue
        player.send(f"LOOK {p.id} {p.world.look[0]} {p.world.look[1]}")
        player.send(f"HOUSE {p.id} {p.world.house[0]} {p.world.house[1]}")
    cmd_hunt(self, player)


def broadcast_all(self, line, exclude=None):
    for p in self.players.values():
        if p is not exclude and p.name is not None:
            p.send(line)


def install(server_cls):
    """Adds the world commands to the netplay Server class."""
    for fn in (cmd_look, cmd_house, cmd_score, cmd_wboard, cmd_hunt, cmd_photo, cmd_photoend, cmd_world, broadcast_all):
        setattr(server_cls, fn.__name__, fn)


    init = server_cls.__init__

    def __init__(self, *args, **kwargs):
        init(self, *args, **kwargs)
        attach(self, None)

    server_cls.__init__ = __init__


def attach(server, db_path):
    """Gives a Server instance its world store and gallery folder."""
    base = os.path.splitext(db_path)[0] if db_path else ""
    server.world = WorldStore(base + ".world.json" if base else None)
    server.gallery_dir = base + "-gallery" if base else None
