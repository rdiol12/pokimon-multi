#!/usr/bin/env python3
"""Gives a screenshot run's server database three past Rover League
champions (laurel.lua's Hall of Champions): Arlo, Cleo and Bram."""
import os
import sqlite3
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "server"))
import rovers  # noqa: E402
import rovers_league as L  # noqa: E402

db = sqlite3.connect(sys.argv[1])
L.setup(db)
week = L.week_of(time.time() + float(os.environ.get("ROVERS_TIME_OFFSET", "0")))
names = {c["name"]: i for i, c in enumerate(rovers.CAST)}
for ago, name in ((1, "Arlo"), (2, "Cleo"), (3, "Bram")):
    db.execute("INSERT OR REPLACE INTO rover_league_seats (week, seat, rover) VALUES (?, 0, ?)",
               (week - ago, names[name]))
    db.execute("INSERT OR REPLACE INTO rover_league_seats (week, seat, rover) VALUES (?, 1, ?)",
               (week - ago, (names[name] + 1) % len(rovers.CAST)))
    db.execute("INSERT INTO rover_league_matches (week, round, slot, a, b, winner, how) VALUES (?, 2, 0, 0, 1, 0, 'sim')",
               (week - ago,))
db.commit()
