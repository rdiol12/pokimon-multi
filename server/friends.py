"""Friend lists. A player adds someone they meet online; the list is one-way
(like following) and lives in the league database, keyed by name + trainer
id like everything else."""

MAX_FRIENDS = 10


class Friends:
    def __init__(self, db):
        self.db = db
        self.db.executescript(
            """
            CREATE TABLE IF NOT EXISTS friends (
                owner_name TEXT NOT NULL,
                owner_tid INTEGER NOT NULL,
                friend_name TEXT NOT NULL,
                friend_tid INTEGER NOT NULL,
                added_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
                PRIMARY KEY (owner_name, owner_tid, friend_name, friend_tid)
            );
            """
        )
        self.db.commit()

    def list(self, owner):
        return self.db.execute(
            "SELECT friend_name, friend_tid FROM friends WHERE owner_name = ? AND owner_tid = ?"
            " ORDER BY added_at, friend_name",
            owner,
        ).fetchall()

    def add(self, owner, friend):
        """False when the list is full; adding someone twice is fine."""
        if owner == friend:
            raise ValueError("can't add yourself")
        current = self.list(owner)
        if tuple(friend) in [tuple(f) for f in current]:
            return True
        if len(current) >= MAX_FRIENDS:
            return False
        self.db.execute("INSERT INTO friends (owner_name, owner_tid, friend_name, friend_tid) VALUES (?, ?, ?, ?)",
                        (*owner, *friend))
        self.db.commit()
        return True

    def remove(self, owner, friend):
        self.db.execute(
            "DELETE FROM friends WHERE owner_name = ? AND owner_tid = ? AND friend_name = ? AND friend_tid = ?",
            (*owner, *friend),
        )
        self.db.commit()
