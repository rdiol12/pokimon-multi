"""Once TEST1 and TEST2 team up, they've already caught eight of the ten
Pokémon of their first duo quest (so one catch each finishes it)."""
import os
import sqlite3
import time

ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    return after


def after(port, out):
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"), timeout=10)
    for _ in range(1200):
        try:
            if db.execute("SELECT 1 FROM online_duos").fetchone():
                break
        except sqlite3.OperationalError:
            pass
        time.sleep(0.5)
    db.execute("UPDATE online_duos SET prog_a = 5, prog_b = 3")
    db.commit()
