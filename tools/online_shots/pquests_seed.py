"""Gary and Mia have hidden treasures elsewhere already."""
import os
import sqlite3
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import ITEMS, MAPS  # noqa: E402

ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    for name, tid, place, x, y, item, count, hint in (
            ("Gary", 4242, "ROUTE119", 10, 40, "MASTER_BALL", 1, 1),
            ("Mia", 777, "FORTREE_CITY", 6, 12, "PP_UP", 2, 2)):
        g, n = MAPS[place]
        db.execute("INSERT INTO online_pquests (name, tid, map_group, map_num, x, y, item, count, hint)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (name, tid, g, n, x, y, ITEMS[item], count, hint))
    db.commit()
