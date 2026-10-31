"""Roaming trainers (rovers.py) on the market board (market.py) and at the
auction house (life_auction.py).

In towns with a Pokémon Mart, a rover with Pokémon at its ranch now and then
lists one on the market or puts it up for auction, and collects what it
earned. A rover with money to spare buys players' listings and bids on their
auctions when the price is fair for the level; what it gets goes on its team
or to its ranch, and outbid money comes back to it like a player's.

The server can't build a real struct Pokemon (the game encrypts it), so a
rover's Pokémon goes in the listing as a short note the game turns into the
real one (Rovers_FromListing in src/rovers.c): "RVR", 1, the rover id, the
rover's 16-byte Pokémon, zeros up to the 100 bytes of a struct Pokemon.

Rovers are sellers and buyers like players, keyed by their name and
TID_BASE + id. They don't buy from each other, and there are never more than
a few of their listings up at once, so the boards stay the players'.
"""

import sqlite3

try:
    import life_auction
except ImportError:  # the auction house is optional
    life_auction = None
try:
    from market import Market
except ImportError:
    Market = None

MON_SIZE = 100
MAGIC = b"RVR\x01"
MAX_ROVER_LISTINGS = 8   # on the market, all rovers together
MAX_ROVER_AUCTIONS = 4
AUCTION_HOURS = 8


def fair_price(level):
    """What a rover thinks a Pokémon of this level is worth."""
    return (400 + level * 120 + 99) // 100 * 100


def listing_blob(rid, mon, pack_mon):
    blob = MAGIC + bytes([rid]) + pack_mon(mon)
    return blob + bytes(MON_SIZE - len(blob))


def blob_level(blob):
    """A listed Pokémon's level: a rover's note, or a player's struct
    Pokemon (party data, not encrypted)."""
    if blob[:4] == MAGIC:
        return blob[7]
    return blob[84] if len(blob) >= 85 and 1 <= blob[84] <= 100 else 5


class Trader:
    def __init__(self, db, tid_base, news, new_mon, pack_mon, species_name, keep):
        self.db, self.tid_base, self.news = db, tid_base, news
        self.new_mon, self.pack_mon, self.species_name, self.keep = new_mon, pack_mon, species_name, keep
        self._market = None

    @property
    def market(self):
        """The market board's tables (made on first use)."""
        if self._market is None and Market is not None:
            try:
                self._market = Market(self.db)
            except sqlite3.Error:
                pass
        return self._market

    def key(self, r):
        return (r.name, self.tid_base + r.id)

    def get(self, r, mon, team_max):
        if len(r.team) < team_max:
            r.team.append(mon)
        else:
            self.keep(r, mon)

    # The market --------------------------------------------------------------

    def market_errands(self, r, rng, team_max):
        if self.market is None:
            return
        key = self.key(r)
        money, sales = self.market.collect(key)
        if sales:
            r.life["money"] += money
            self.news(self.db, f"{r.name} sold {sales} Pokémon on the market for {money}.")
        db = self.db
        mine = db.execute("SELECT COUNT(*) FROM market WHERE seller_name = ? AND seller_tid = ?", key).fetchone()[0]
        rovers_up = db.execute("SELECT COUNT(*) FROM market WHERE seller_tid >= ?", (self.tid_base,)).fetchone()[0]
        if r.life["ranch"] and not mine and rovers_up < MAX_ROVER_LISTINGS and rng.random() < 0.15:
            species, level = r.life["ranch"].pop(rng.randrange(len(r.life["ranch"])))
            blob = listing_blob(r.id, self.new_mon(species, level), self.pack_mon)
            price = fair_price(level)
            self.market.sell(key, species, level, price, blob.hex())
            self.news(db, f"{r.name} put a {self.species_name(species)} on the market for {price}.")
            return
        # Buying a player's Pokémon at a fair price.
        if rng.random() < 0.2 and r.life["money"] > 2000:
            rows = db.execute("SELECT id, seller_name, species, level, price FROM market WHERE seller_tid < ?"
                              " ORDER BY id DESC LIMIT 20", (self.tid_base,)).fetchall()
            good = [row for row in rows
                    if row[4] <= fair_price(row[3]) * 13 // 10 and row[4] + 1000 <= r.life["money"]]
            if good:
                lid, seller, species, level, price = rng.choice(good)
                result, price, _ = self.market.buy(key, lid)
                if result == "ok":
                    r.life["money"] -= price
                    self.get(r, self.new_mon(species, level), team_max)
                    self.news(db, f"{r.name} bought {seller}'s {self.species_name(species)} on the market.")

    # The auction house ---------------------------------------------------------

    def auction_errands(self, r, rng, team_max):
        if life_auction is None:
            return
        db = self.db
        try:
            life_auction.settle(db)
            self.collect_auctions(r, team_max)
        except sqlite3.Error:  # no auction tables on this server
            return
        key = self.key(r)
        open_mine = db.execute("SELECT COUNT(*) FROM auctions WHERE name = ? AND tid = ? AND status = 'open'",
                               key).fetchone()[0]
        rovers_up = db.execute("SELECT COUNT(*) FROM auctions WHERE tid >= ? AND status = 'open'",
                               (self.tid_base,)).fetchone()[0]
        if r.life["ranch"] and not open_mine and rovers_up < MAX_ROVER_AUCTIONS and rng.random() < 0.08:
            species, level = r.life["ranch"].pop(rng.randrange(len(r.life["ranch"])))
            blob = listing_blob(r.id, self.new_mon(species, level), self.pack_mon)
            start = max(life_auction.MIN_START, fair_price(level) * 6 // 10 // 100 * 100)
            db.execute("INSERT INTO auctions (name, tid, species, level, shiny, mon, start, ends_at)"
                       " VALUES (?, ?, ?, ?, 0, ?, ?, ?)",
                       (*key, species, level, blob, start, life_auction.now() + AUCTION_HOURS * 3600))
            self.news(db, f"{r.name} put a {self.species_name(species)} up for auction.")
            return
        if rng.random() < 0.15:
            self.bid(r, rng)

    def bid(self, r, rng):
        db, key = self.db, self.key(r)
        rows = db.execute(
            "SELECT id, name, species, level, start, bid, bidder_name, bidder_tid, ends_at FROM auctions"
            " WHERE status = 'open' AND tid < ? AND ends_at > ?", (self.tid_base, life_auction.now())).fetchall()
        for aid, seller, species, level, start, bid, bname, btid, ends in rng.sample(rows, len(rows)):
            if (bname, btid) == key:
                continue
            amount = life_auction.min_bid(start, bid)
            if amount > fair_price(level) * 3 // 2 or amount + 1000 > r.life["money"]:
                continue
            if bname is not None:
                life_auction._inbox(db, (bname, btid), "OUTBID", species, money=bid, other=r.name)
            r.life["money"] -= amount
            ends = max(ends, life_auction.now() + life_auction.SNIPE_WINDOW)
            db.execute("UPDATE auctions SET bid = ?, bids = bids + 1, bidder_name = ?, bidder_tid = ?, ends_at = ?"
                       " WHERE id = ?", (amount, *key, ends, aid))
            self.news(db, f"{r.name} bid {amount} on {seller}'s {self.species_name(species)}.")
            return

    def collect_auctions(self, r, team_max):
        db, key = self.db, self.key(r)
        rows = db.execute("SELECT id, kind, money, species, other, mon FROM auction_inbox"
                          " WHERE name = ? AND tid = ? ORDER BY id", key).fetchall()
        for rid, kind, money, species, other, mon in rows:
            db.execute("DELETE FROM auction_inbox WHERE id = ?", (rid,))
            level = blob_level(mon) if mon is not None else 5
            if kind == "WON":
                self.get(r, self.new_mon(species, level), team_max)
                self.news(db, f"{r.name} won {other}'s {self.species_name(species)} at the auction house.")
            elif kind in ("SOLD", "OUTBID"):
                r.life["money"] += money
                if kind == "SOLD":
                    self.news(db, f"{other} won {r.name}'s {self.species_name(species)} at the auction house.")
            elif kind in ("UNSOLD", "CANCELED"):
                self.keep(r, self.new_mon(species, level))
