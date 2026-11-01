#!/usr/bin/env python3
"""Rovers at the auction house of a screenshot run's server database
(auction.lua): Dax's Lv18 Aron is up for auction, and the player has just
won Hal's Lv14 Makuhita. Args: the database, the player's name and trainer id."""
import os
import sqlite3
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "server"))
import rovers  # noqa: E402
import rovers_trade  # noqa: E402

db = sqlite3.connect(sys.argv[1])
name, tid = sys.argv[2], int(sys.argv[3])
aron = rovers_trade.listing_blob(7, rovers.new_mon(304, 18), rovers.pack_mon)
db.execute("INSERT INTO auctions (name, tid, species, level, shiny, mon, start, ends_at) VALUES (?, ?, 304, 18, 0, ?, 1300, ?)",
           (rovers.CAST[7]["name"], rovers.TID_BASE + 7, aron, time.time() + 8 * 3600))
makuhita = rovers_trade.listing_blob(12, rovers.new_mon(296, 14), rovers.pack_mon)
db.execute("INSERT INTO auction_inbox (name, tid, kind, money, species, other, mon) VALUES (?, ?, 'WON', 1500, 296, ?, ?)",
           (name, tid, rovers.CAST[12]["name"], makuhita))
db.commit()
