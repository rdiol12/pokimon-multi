"""A tournament that starts as the players arrive and lasts LENGTH seconds;
Gary and Mia have landed a few already. The seed writes `ended` once it's
over, so the scenario moves on to the results."""
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import SPECIES  # noqa: E402

LENGTH = 90
START = time.time() + 12
ENV = {"NETPLAY_FISHT_START": START, "NETPLAY_FISHT_LENGTH": LENGTH, "NETPLAY_FISHT_PERIOD": 7200}


def seed(port, out):
    return after


def after(port, out):
    time.sleep(max(0, START - time.time()) + 1)
    for name, tid, catches in (("Gary", 4242, [("MAGIKARP", 820), ("TENTACOOL", 760)]),
                               ("Mia", 777, [("WINGULL", 540)])):
        f = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        for species, size in catches:
            f.ask(f"OEV FISH {SPECIES[species]} {size}")
        f.close()
    time.sleep(max(0, START + LENGTH - time.time()) + 1)
    open(os.path.join(out, "ended"), "w").close()
