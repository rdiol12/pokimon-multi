"""The market board in Poké Marts. A player lists a Pokémon for a price and
can log off; whoever buys it pays the price, which waits on the server until
the seller collects it. Listings live in the league database, keyed by name +
trainer id like everything else.

The Pokémon itself is the game's struct Pokemon as hex. The server can't read
it (the game encrypts most of it), so the game sends species and level along
for the board."""

import re

MAX_LISTINGS = 5      # per seller
PAGE_SIZE = 5
MAX_PRICE = 999999
MON_HEX_RE = re.compile(r"^[0-9a-f]{200}$")  # sizeof(struct Pokemon) == 100


class Market:
    def __init__(self, db):
        self.db = db
        self.db.executescript(
            """
            CREATE TABLE IF NOT EXISTS market (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                seller_name TEXT NOT NULL,
                seller_tid INTEGER NOT NULL,
                species INTEGER NOT NULL,
                level INTEGER NOT NULL,
                price INTEGER NOT NULL,
                mon TEXT NOT NULL,
                listed_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
            );
            CREATE TABLE IF NOT EXISTS market_earnings (
                name TEXT NOT NULL,
                tid INTEGER NOT NULL,
                money INTEGER NOT NULL,
                sales INTEGER NOT NULL,
                PRIMARY KEY (name, tid)
            );
            """
        )
        self.db.commit()

    def page(self, number):
        """(rows, page count). Rows: id, seller name, seller tid, species, level, price."""
        total = self.db.execute("SELECT COUNT(*) FROM market").fetchone()[0]
        pages = max(1, (total + PAGE_SIZE - 1) // PAGE_SIZE)
        number = max(0, min(number, pages - 1))
        rows = self.db.execute(
            "SELECT id, seller_name, seller_tid, species, level, price FROM market"
            " ORDER BY id DESC LIMIT ? OFFSET ?",
            (PAGE_SIZE, number * PAGE_SIZE),
        ).fetchall()
        return rows, number, pages

    def sell(self, seller, species, level, price, mon):
        """The new listing's id, or None when the seller already has MAX_LISTINGS."""
        if not 0 < price <= MAX_PRICE or not MON_HEX_RE.match(mon):
            raise ValueError("bad listing")
        count = self.db.execute("SELECT COUNT(*) FROM market WHERE seller_name = ? AND seller_tid = ?",
                                seller).fetchone()[0]
        if count >= MAX_LISTINGS:
            return None
        cur = self.db.execute(
            "INSERT INTO market (seller_name, seller_tid, species, level, price, mon) VALUES (?, ?, ?, ?, ?, ?)",
            (*seller, species, level, price, mon),
        )
        self.db.commit()
        return cur.lastrowid

    def _take(self, listing_id):
        row = self.db.execute("SELECT seller_name, seller_tid, price, mon FROM market WHERE id = ?",
                              (listing_id,)).fetchone()
        if row is not None:
            self.db.execute("DELETE FROM market WHERE id = ?", (listing_id,))
        return row

    def buy(self, buyer, listing_id):
        """('ok', price, mon), ('gone', 0, None) or ('self', 0, None)."""
        row = self.db.execute("SELECT seller_name, seller_tid FROM market WHERE id = ?", (listing_id,)).fetchone()
        if row is None:
            return "gone", 0, None
        if tuple(row) == tuple(buyer):
            return "self", 0, None
        name, tid, price, mon = self._take(listing_id)
        self.db.execute(
            "INSERT INTO market_earnings (name, tid, money, sales) VALUES (?, ?, ?, 1)"
            " ON CONFLICT (name, tid) DO UPDATE SET money = money + excluded.money, sales = sales + 1",
            (name, tid, price),
        )
        self.db.commit()
        return "ok", price, mon

    def cancel(self, seller, listing_id):
        """The listed Pokémon, or None if it's gone or not the seller's."""
        row = self.db.execute("SELECT seller_name, seller_tid FROM market WHERE id = ?", (listing_id,)).fetchone()
        if row is None or tuple(row) != tuple(seller):
            return None
        mon = self._take(listing_id)[3]
        self.db.commit()
        return mon

    def collect(self, seller):
        """(money, sales) earned since the last collect."""
        row = self.db.execute("SELECT money, sales FROM market_earnings WHERE name = ? AND tid = ?",
                              seller).fetchone()
        if row is None:
            return 0, 0
        self.db.execute("DELETE FROM market_earnings WHERE name = ? AND tid = ?", seller)
        self.db.commit()
        return row
