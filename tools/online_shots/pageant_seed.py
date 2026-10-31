"""Gary, Mia and Rex have entered this week's pageant and voted; TEST2 won
last week's, so has a prize waiting."""
import os
import sqlite3
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import SPECIES  # noqa: E402

ENV = {"NETPLAY_FISHT_LENGTH": 0}
WEEK = 7 * 24 * 3600


def seed(port, out):
    players = [("Gary", 4242, "MILOTIC", 45, 1, 0x1234, "Aurora"),
               ("Mia", 777, "SYLVEON", 38, 0, 0x0101, "Ribbons"),
               ("Rex", 31337, "ALTARIA", 41, 0, 0x2222, "Cloud")]
    fakes = []
    for name, tid, species, level, shiny, pers, nick in players:
        f = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        f.ask(f"PAGEANT ENTER {SPECIES[species]} {level} {shiny} {pers} {nick}")
        fakes.append(f)
    # Gary and Rex vote for Mia's, Mia for Gary's.
    for voter, i in ((0, 0), (2, 1), (1, 0)):
        entry = fakes[voter].ask(f"PAGEANT SHOW {i}")[1].split("\n")[1].split("|")[3]
        fakes[voter].ask(f"PAGEANT VOTE 0 {entry}")
    for f in fakes:
        f.close()
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    db.execute("INSERT INTO online_pageant (week, name, tid, species, level, shiny, personality, nickname, votes)"
               " VALUES (?, 'TEST2', 4098, ?, 30, 0, 7, 'Sparky', 5)", (int(time.time() // WEEK) - 1, SPECIES["PIKACHU"]))
    db.commit()
