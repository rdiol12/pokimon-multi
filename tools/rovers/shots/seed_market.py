#!/usr/bin/env python3
"""Puts a rover's Pokémon on the market board of a screenshot run's server
database (market.lua): Hal's Lv12 Ralts for 800."""
import os
import sqlite3
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "server"))
import rovers  # noqa: E402
import rovers_trade  # noqa: E402
from market import Market  # noqa: E402

db = sqlite3.connect(sys.argv[1])
blob = rovers_trade.listing_blob(12, rovers.new_mon(280, 12), rovers.pack_mon)
Market(db).sell((rovers.CAST[12]["name"], rovers.TID_BASE + 12), 280, 12, 800, blob.hex())
db.commit()
