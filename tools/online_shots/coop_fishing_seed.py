"""Earlier giant catches on the board."""
import sqlite3
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import SPECIES  # noqa: E402


def seed(port, out):
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    for sp, size, a, b in (("GYARADOS", 6100, "Gary", "Mia"), ("LAPRAS", 5200, "Rex", "Mia")):
        db.execute("INSERT INTO online_giants (species, size, a, b, at) VALUES (?, ?, ?, ?, ?)",
                   (SPECIES[sp], size, a, b, time.time()))
    db.commit()
