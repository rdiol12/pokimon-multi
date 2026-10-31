"""TEST1 and TEST3 are a team, TEST2 and TEST4 another; three more teams
have played already."""
ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    import os
    import sqlite3
    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    teams = [("TEST1", 4097, "TEST3", 4099, 1000, 0, 0), ("TEST2", 4098, "TEST4", 4100, 1000, 0, 0),
             ("Gary", 4242, "Mia", 777, 1074, 5, 2), ("Rex", 31337, "Lena", 512, 1022, 3, 2),
             ("Max", 90, "Ivy", 91, 948, 1, 4)]
    for an, at, bn, bt, rating, w, l in teams:
        db.execute("INSERT INTO online_teams (a_name, a_tid, b_name, b_tid, rating, wins, losses)"
                   " VALUES (?, ?, ?, ?, ?, ?, ?)", (an, at, bn, bt, rating, w, l))
    db.commit()
