"""Battle replays: after a PvP battle the game uploads its battle recording
(the game's struct RecordedBattleSave) in parts; later the player lists their
replays and downloads one to watch it again in the game."""

from ext import Refused, text

UP_PART = 512
DOWN_PART = 1024
MAX_REPLAY = 8192
KEEP = 10


def setup(db):
    db.execute(
        """
        CREATE TABLE IF NOT EXISTS replays (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL, tid INTEGER NOT NULL,
            opponent TEXT NOT NULL, result TEXT NOT NULL,
            data BLOB NOT NULL,
            played_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        )
        """
    )


def cmd_replay(ctx, args, data):
    db, key, player = ctx.db, ctx.key, ctx.player
    sub = args[0]
    if sub == "PUT":
        part, parts, opponent, result = int(args[1]), int(args[2]), args[3], args[4]
        if result not in ("win", "loss", "draw") or not 0 <= part < parts <= MAX_REPLAY // UP_PART:
            raise ValueError("bad replay")
        if part == 0:
            player.replay_upload = b""
        buf = getattr(player, "replay_upload", None)
        if buf is None or len(buf) != part * UP_PART or len(data) > UP_PART:
            raise ValueError("replay part out of order")
        player.replay_upload = buf + data
        if part < parts - 1:
            return b""
        db.execute(
            "INSERT INTO replays (name, tid, opponent, result, data) VALUES (?, ?, ?, ?, ?)",
            (*key, opponent[:7], result, player.replay_upload),
        )
        player.replay_upload = None
        old = db.execute(
            "SELECT id FROM replays WHERE name = ? AND tid = ? ORDER BY id DESC LIMIT -1 OFFSET ?", (*key, KEEP)
        ).fetchall()
        db.executemany("DELETE FROM replays WHERE id = ?", old)
        return b""
    if sub == "LIST":
        # id|opponent|result|date|size, newest first
        rows = db.execute(
            "SELECT id, opponent, result, substr(played_at, 1, 10), length(data) FROM replays"
            " WHERE name = ? AND tid = ? ORDER BY id DESC", key
        ).fetchall()
        return text(rows)
    if sub == "GET":
        rid, part = int(args[1]), int(args[2])
        row = db.execute("SELECT data FROM replays WHERE id = ? AND name = ? AND tid = ?", (rid, *key)).fetchone()
        if row is None:
            raise Refused("That replay is gone.")
        return row[0][part * DOWN_PART:(part + 1) * DOWN_PART]
    raise ValueError("bad REPLAY request")


COMMANDS = {"REPLAY": cmd_replay}
