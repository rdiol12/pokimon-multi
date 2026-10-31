"""Gary and Mia start a PvP battle once both games are online; Rex bets
500 COINS on Mia. Once TEST1 and TEST2 have bet too, Gary wins."""
import os
import sqlite3
import time

ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    return after


def wait_for(db, sql, n, timeout=600):
    for _ in range(timeout * 2):
        try:
            if db.execute(sql).fetchone()[0] >= n:
                return True
        except sqlite3.OperationalError:
            pass
        time.sleep(0.5)
    return False


def after(port, out):
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"), timeout=10)
    # Both games are online (they've asked for the status).
    time.sleep(25)
    gary, mia, rex = (Fake(port, n, t) for n, t in (("Gary", 4242), ("Mia", 777), ("Rex", 31337)))  # noqa: F821
    gary.say(f"REQ battle {mia.id}")
    for line in mia.f:
        if line.startswith("INVITE "):
            sid = line.split()[1]
            break
    mia.say(f"ACCEPT {sid}")
    time.sleep(1)
    rex.ask("BET LIST 9999")
    rex.ask(f"BET SHOW {sid}")
    rex.ask("BET PLACE 500 g 9999")
    wait_for(db, "SELECT COUNT(*) FROM online_bets", 3)
    time.sleep(3)
    gary.say(f"RESULT {sid} win")
    mia.say(f"RESULT {sid} loss")
    gary.say(f"QUIT {sid}")
    mia.say(f"QUIT {sid}")
    time.sleep(120)
    for f in (gary, mia, rex):
        f.close()
