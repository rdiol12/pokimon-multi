"""Gary (eight badges, three graduates) and Mia (seven badges) are on the
mentor list."""
import os
import sqlite3

ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    for name, tid, badges in (("Gary", 4242, 8), ("Mia", 777, 7)):
        f = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        f.ask(f"MENTOR VOLUNTEER {badges}")
        f.close()
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    db.execute("UPDATE online_mentors SET graduates = 3 WHERE name = 'Gary'")
    db.commit()
