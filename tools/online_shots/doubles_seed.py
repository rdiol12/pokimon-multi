"""Gary, Mia and Rex have played ranked doubles already."""
ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    import os
    import sqlite3
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    for name, tid, rating, w, l in (("Gary", 4242, 1088, 6, 2), ("Mia", 777, 1031, 4, 3), ("Rex", 31337, 962, 2, 5)):
        db.execute("INSERT INTO online_doubles VALUES (?, ?, ?, ?, ?, 0)", (name, tid, rating, w, l))
    db.commit()
