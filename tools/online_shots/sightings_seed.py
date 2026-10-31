"""Other players met Raikou and Latias earlier."""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import MAPS, SPECIES  # noqa: E402


def seed(port, out):
    for name, tid, species, place in (("Gary", 4242, "RAIKOU", "ROUTE119"), ("Mia", 777, "LATIAS", "ROUTE120"),
                                      ("Rex", 31, "ENTEI", "ROUTE111")):
        f = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        g, n = MAPS[place]
        f.ask(f"OEV SIGHT {SPECIES[species]} {g} {n} 1")
        f.close()
