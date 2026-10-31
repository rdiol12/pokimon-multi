"""Chat and lobby rooms (docs/community/chat.md).

Players talk in rooms with Easy Chat phrases (six words from the game's
word list, so there's nothing to filter). Rooms: everyone's PLAZA, TRADING
and BATTLES, one for the area the player is in (by its map name), and one
per ladder tier, open only to players in that tier. Each room keeps its last
few messages, and everyone in a room gets them when someone speaks.
"""

import collections
import time

import community
from community import Answer, listen, request
from seasons import TIERS, tier_of

REQ_ROOMS, REQ_JOIN, REQ_SAY, REQ_LEAVE = 50, 51, 52, 53
LOCKED, TOO_FAST = 2, 3
HISTORY = 4
WORDS = 6
EMPTY_WORD = 0xFFFF
EC_NUM_GROUPS, EC_MASK_BITS = 22, 9  # include/constants/easy_chat.h
SAY_INTERVAL = 2.0
EVENT_CHAT = 0  # COMMUNITY_EVENT_CHAT

# Room ids (u8): fixed rooms, then the area room, then one per tier.
PLAZA, TRADING, BATTLES, AREA = 0, 1, 2, 3
TIER_BASE = 10
FIXED = {PLAZA: "PLAZA", TRADING: "TRADING", BATTLES: "BATTLES"}


def area_name(data):
    return community.decode_text(data[:20]).upper() or "SOMEWHERE"


def room_key(room_id, player, area):
    if room_id in FIXED:
        return FIXED[room_id]
    if room_id == AREA:
        return "AREA:" + area
    if TIER_BASE <= room_id < TIER_BASE + len(TIERS):
        return "TIER:" + TIERS[room_id - TIER_BASE][0]
    raise ValueError("no such room")


def room_title(key):
    if key.startswith("AREA:"):
        return key[5:]
    if key.startswith("TIER:"):
        return key[5:] + " TIER"
    return key


def _chat(server):
    chat = server.community.get("chat")
    if chat is None:
        chat = server.community["chat"] = {"history": collections.defaultdict(lambda: collections.deque(maxlen=HISTORY))}
    return chat


def members(server, key):
    return [p for p in server.online_players() if getattr(p, "chat_room", None) == key]


def my_tier(server, player):
    rank = server.league.rank_of(player.key)
    return tier_of(rank[1]) if rank[0] else 0


@request(REQ_ROOMS)
def rooms(server, player, args, data):
    area = area_name(data)
    tier = my_tier(server, player)
    ids = [PLAZA, TRADING, BATTLES, AREA, TIER_BASE + tier]
    rows = []
    for room_id in ids:
        key = room_key(room_id, player, area)
        rows.append(f"{room_title(key)} ({len(members(server, key))})")
    return Answer("CHAT ROOMS", rows, data=bytes(ids))


def history_event(server, key):
    history = _chat(server)["history"][key]
    data = bytearray([len(history)])
    for name, words in history:
        data += community.encode_name(name, 8)
        for w in words:
            data += w.to_bytes(2, "little")
    return bytes(data)


def tell_room(server, key):
    data = history_event(server, key)
    for p in members(server, key):
        if community.is_community(p):
            community.send_event(p, EVENT_CHAT, data)


@request(REQ_JOIN)
def join(server, player, args, data):
    room_id = args[0]
    key = room_key(room_id, player, area_name(data))
    if key.startswith("TIER:") and room_id - TIER_BASE != my_tier(server, player):
        return Answer(room_title(key), ["That room is for another tier."], status=LOCKED)
    player.chat_room = key
    community.send_event(player, EVENT_CHAT, history_event(server, key))
    return Answer(room_title(key), [f"{len(members(server, key))} here"])


@request(REQ_SAY)
def say(server, player, args, data):
    key = getattr(player, "chat_room", None)
    if key is None:
        raise ValueError("not in a room")
    if len(data) != WORDS * 2:
        raise ValueError("bad phrase")
    words = [int.from_bytes(data[i:i + 2], "little") for i in range(0, len(data), 2)]
    if all(w == EMPTY_WORD for w in words):
        return Answer(room_title(key), [])
    if any(w != EMPTY_WORD and (w >> EC_MASK_BITS) >= EC_NUM_GROUPS for w in words):
        raise ValueError("bad word")
    now = time.monotonic()
    if now - getattr(player, "chat_time", 0) < SAY_INTERVAL:
        return Answer(room_title(key), ["Slow down a little."], status=TOO_FAST)
    player.chat_time = now
    _chat(server)["history"][key].append((player.name, words))
    tell_room(server, key)
    return Answer(room_title(key), [])


@request(REQ_LEAVE)
def leave(server, player, args, data):
    player.chat_room = None
    return Answer("", [])
