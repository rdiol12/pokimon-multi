"""TEST1 leads the guild Aces [ACE], TEST2 is a member. Two rival guilds,
Star Squad [STAR] (whose two members guard its base) and Rockers [ROCK],
have war points this week; Aces placed second last week. The guild chest
has a few items."""
ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    import os
    import sqlite3
    import struct
    import time
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    guilds = [("ACE", "Aces", "TEST1", 4097), ("STAR", "Star Squad", "Mia", 777), ("ROCK", "Rockers", "Max", 90)]
    members = [("TEST1", 4097, "ACE"), ("TEST2", 4098, "ACE"), ("Mia", 777, "STAR"), ("Rex", 31337, "STAR"),
               ("Max", 90, "ROCK"), ("Ivy", 91, "ROCK")]
    db.executemany("INSERT INTO guilds (tag, name, leader_name, leader_tid) VALUES (?, ?, ?, ?)", guilds)
    db.executemany("INSERT INTO guild_members (name, tid, tag) VALUES (?, ?, ?)", members)
    week = int(time.time() // (7 * 86400))
    war = [(week, "STAR", "Mia", 777, 9), (week, "STAR", "Rex", 31337, 5), (week, "ROCK", "Max", 90, 6),
           (week - 1, "ROCK", "Max", 90, 12), (week - 1, "ACE", "TEST2", 4098, 8), (week - 1, "STAR", "Mia", 777, 3)]
    db.executemany("INSERT INTO online_guild_war VALUES (?, ?, ?, ?, ?)", war)
    import importlib.util
    spec = importlib.util.spec_from_file_location("ids", os.path.join(os.path.dirname(__file__), "..", "..",
                                                                       "server", "online_ids.py"))
    ids = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ids)
    db.executemany("INSERT INTO online_guild_chest VALUES ('ACE', ?, ?)",
                   [(ids.ITEMS["ULTRA_BALL"], 6), (ids.ITEMS["HYPER_POTION"], 3)])

    def team(name, mons):
        out = bytearray(160)
        out[2:9] = bytes([0xBB + ord(c) - 65 if c.isupper() else 0xD5 + ord(c) - 97 for c in name]) + \
            b"\xff" * (7 - len(name))
        for i, (species, level, moves) in enumerate(mons):
            struct.pack_into("<I", out, 0x34 + 4 * i, 0x1000 + i)
            struct.pack_into("<4H", out, 0x4C + 8 * i, *moves)
            struct.pack_into("<H", out, 0x7C + 2 * i, ids.SPECIES[species])
            out[0x94 + i] = level
        return bytes(out)

    db.executemany("INSERT INTO online_guild_defenders VALUES ('STAR', ?, ?, ?)", [
        ("Mia", 777, team("Mia", [("WINGULL", 11, (17, 55, 98, 0)), ("ZIGZAGOON", 8, (33, 29, 0, 0))])),
        ("Rex", 31337, team("Rex", [("ELECTRIKE", 13, (209, 98, 33, 0))])),
    ])
    db.commit()
