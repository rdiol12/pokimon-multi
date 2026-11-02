"""Other players' research on the server, for HUB > RESEARCH LEADERS."""

import time

# Noon on the server's clock, so the shots are by day.
_now = time.gmtime()
ENV = {"NETPLAY_FISHT_LENGTH": 0, "NETPLAY_ECO_MIGRATIONS": 0,
       "NETPLAY_CLOCK_UTC_OFFSET": (12 * 60 - (_now.tm_hour * 60 + _now.tm_min)) % 1440}


def seed(port, out):
    for name, tid, done, species in [("Gary", 4242, 42, 384), ("May", 77, 25, 282), ("Wally", 1310, 12, 281)]:
        p = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        p.ask(f"OEV RESEARCH {done} {species}")
        p.close()
