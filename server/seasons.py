"""Ladder seasons. A season lasts a fixed number of days. When it ends,
everyone who played a rated battle gets the reward for the tier they
finished in, and ratings are pulled halfway back to the starting rating
for a fresh climb."""

import time

# Tiers by rating; a player's tier is the last one whose floor they reach.
# The ROM has the same table (sSeasonTiers in src/netplay.c).
TIERS = [("BRONZE", 0), ("SILVER", 1000), ("GOLD", 1100), ("PLATINUM", 1200), ("DIAMOND", 1300), ("MASTER", 1400)]
DAY = 24 * 60 * 60


def tier_of(rating):
    tier = 0
    for i, (_, floor) in enumerate(TIERS):
        if rating >= floor:
            tier = i
    return tier


class Seasons:
    def __init__(self, db, start_rating, days=28, clock=time.time):
        self.db = db
        self.start_rating = start_rating
        self.length = days * DAY
        self.clock = clock
        self.db.executescript(
            """
            CREATE TABLE IF NOT EXISTS seasons (
                id INTEGER PRIMARY KEY,
                started_at REAL NOT NULL,
                ends_at REAL NOT NULL
            );
            CREATE TABLE IF NOT EXISTS season_rewards (
                season INTEGER NOT NULL,
                name TEXT NOT NULL,
                tid INTEGER NOT NULL,
                tier INTEGER NOT NULL,
                rating REAL NOT NULL,
                claimed INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (season, name, tid)
            );
            """
        )
        if self.current() is None:
            now = self.clock()
            self.db.execute("INSERT INTO seasons (id, started_at, ends_at) VALUES (1, ?, ?)", (now, now + self.length))
        self.db.commit()

    def current(self):
        """(season number, ends_at)"""
        return self.db.execute("SELECT id, ends_at FROM seasons ORDER BY id DESC LIMIT 1").fetchone()

    def roll_if_due(self):
        """End every season whose time is up. Returns how many ended."""
        ended = 0
        while True:
            season, ends_at = self.current()
            if self.clock() < ends_at:
                return ended
            self.end_season(season, ends_at)
            ended += 1

    def end_season(self, season, ends_at):
        rows = self.db.execute(
            "SELECT name, tid, rating FROM players WHERE wins + losses + draws > 0"
        ).fetchall()
        self.db.executemany(
            "INSERT OR REPLACE INTO season_rewards (season, name, tid, tier, rating) VALUES (?, ?, ?, ?, ?)",
            [(season, name, tid, tier_of(rating), rating) for name, tid, rating in rows],
        )
        self.db.execute(
            "UPDATE players SET rating = ? + (rating - ?) / 2, wins = 0, losses = 0, draws = 0",
            (self.start_rating, self.start_rating),
        )
        # Seasons keep their schedule; one ended early starts the next now.
        start = min(ends_at, self.clock())
        self.db.execute("INSERT INTO seasons (id, started_at, ends_at) VALUES (?, ?, ?)",
                        (season + 1, start, start + self.length))
        self.db.commit()

    def days_left(self):
        _, ends_at = self.current()
        return max(0, int((ends_at - self.clock() + DAY - 1) // DAY))

    def pending_reward(self, key):
        """(season, tier) of the oldest unclaimed reward, or None."""
        return self.db.execute(
            "SELECT season, tier FROM season_rewards WHERE name = ? AND tid = ? AND claimed = 0"
            " ORDER BY season LIMIT 1",
            key,
        ).fetchone()

    def claim(self, key):
        reward = self.pending_reward(key)
        if reward is None:
            return None
        self.db.execute("UPDATE season_rewards SET claimed = 1 WHERE season = ? AND name = ? AND tid = ?",
                        (reward[0], *key))
        self.db.commit()
        return reward
