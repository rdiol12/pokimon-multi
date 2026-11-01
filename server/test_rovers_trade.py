"""Rovers on the market board and at the auction house (rovers_trade.py)."""
import random
import sqlite3
import struct
import types
import unittest

import life_auction
import rovers
import rovers_life as life
import rovers_trade
from market import Market


def rover(rid, name, money=0, ranch=()):
    r = types.SimpleNamespace(id=rid, name=name, team=[rovers.new_mon(25, 20)], life=life.empty())
    r.life["money"] = money
    r.life["ranch"] = [list(m) for m in ranch]
    return r


class TraderTest(unittest.TestCase):
    def setUp(self):
        self.db = sqlite3.connect(":memory:")
        life_auction.setup(self.db)
        rovers.setup(self.db)
        self.t = 1_000_000.0
        self.old_now = life_auction.now
        life_auction.now = lambda: self.t
        self.kept = []
        self.trader = rovers_trade.Trader(self.db, rovers.TID_BASE, rovers.news, rovers.new_mon, rovers.pack_mon,
                                          rovers.species_name, lambda r, m: self.kept.append(m))
        self.rng = random.Random(3)

    def tearDown(self):
        life_auction.now = self.old_now

    def test_blob_matches_the_game(self):
        blob = rovers_trade.listing_blob(12, rovers.new_mon(280, 17), rovers.pack_mon)
        self.assertEqual(len(blob), 100)
        self.assertEqual(blob[:5], b"RVR\x01\x0c")
        self.assertEqual(struct.unpack_from("<H", blob, 5)[0], 280)
        self.assertEqual(rovers_trade.blob_level(blob), 17)

    def test_sells_and_buys_on_the_market(self):
        r = rover(3, "Hal", ranch=[(280, 15)])
        self.rng.random = lambda: 0.0
        self.trader.market_errands(r, self.rng, 6)
        (lid, name, tid, species, level, price), = Market(self.db).page(0)[0]
        self.assertEqual((name, tid, species, level), ("Hal", rovers.TID_BASE + 3, 280, 15))
        self.assertEqual(price, rovers_trade.fair_price(15))
        self.assertEqual(r.life["ranch"], [])
        # A player buys it; the rover collects the money next time.
        result, paid, mon = Market(self.db).buy(("Ash", 1), lid)
        self.assertEqual(bytes.fromhex(mon)[:3], b"RVR")
        # A player's listing at a fair price: the rover buys it.
        player_mon = bytes(84) + bytes([30]) + bytes(15)
        Market(self.db).sell(("Ash", 1), 1, 30, rovers_trade.fair_price(30), player_mon.hex())
        r.life["money"] = 20000
        self.trader.market_errands(r, self.rng, 6)
        self.assertEqual(r.life["money"], 20000 + paid - rovers_trade.fair_price(30))
        self.assertEqual([m["species"] for m in r.team], [25, 1])
        self.assertEqual(r.team[1]["level"], 30)
        # It never buys from another rover.
        other = rover(4, "Gia", ranch=[(281, 20)])
        self.trader.market_errands(other, self.rng, 6)
        self.trader.market_errands(r, self.rng, 6)
        self.assertEqual(len(Market(self.db).page(0)[0]), 1)

    def test_auctions(self):
        seller = rover(5, "Rue", ranch=[(283, 12)])
        self.rng.random = lambda: 0.0
        self.trader.auction_errands(seller, self.rng, 6)
        aid, start = self.db.execute("SELECT id, start FROM auctions").fetchone()
        # Ash bids and wins once the time is up.
        self.db.execute("UPDATE auctions SET bid = ?, bids = 1, bidder_name = 'Ash', bidder_tid = 1 WHERE id = ?",
                        (start, aid))
        self.t += 9 * 3600
        self.trader.auction_errands(seller, self.rng, 6)
        self.assertEqual(seller.life["money"], start - start * life_auction.HOUSE_CUT // 100)
        kind, mon = self.db.execute("SELECT kind, mon FROM auction_inbox WHERE name = 'Ash'").fetchone()
        self.assertEqual((kind, bytes(mon[:3])), ("WON", b"RVR"))
        # A rover bids on a player's auction, is outbid, and gets its money back.
        self.db.execute("INSERT INTO auctions (name, tid, species, level, shiny, mon, start, ends_at)"
                        " VALUES ('Ash', 1, 1, 20, 0, ?, 500, ?)", (bytes(84) + bytes([20]) + bytes(15), self.t + 3600))
        buyer = rover(6, "Kit", money=10000)
        self.trader.bid(buyer, self.rng)
        self.assertEqual(buyer.life["money"], 9500)
        self.db.execute("UPDATE auctions SET bid = 900, bidder_name = 'May', bidder_tid = 2 WHERE status = 'open'")
        life_auction._inbox(self.db, ("Kit", rovers.TID_BASE + 6), "OUTBID", 1, money=500)
        self.trader.collect_auctions(buyer, 6)
        self.assertEqual(buyer.life["money"], 10000)
        # It bids again and wins this time.
        self.trader.bid(buyer, self.rng)
        self.t += 2 * 3600
        life_auction.settle(self.db)
        self.trader.collect_auctions(buyer, 6)
        self.assertEqual([m["species"] for m in buyer.team], [25, 1])


if __name__ == "__main__":
    unittest.main()
