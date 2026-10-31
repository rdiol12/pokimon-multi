"""Earlier shiny catches and legend sightings, then screenshots of the
website once TEST1's shiny is in the gallery."""
import os
import subprocess
import sys
import time
import urllib.request

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import MAPS, SPECIES  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))


def seed(port, out):
    for name, tid, shiny, place in (("Gary", 4242, "MAGIKARP", "ROUTE118"), ("Mia", 777, "RALTS", "ROUTE102"),
                                    ("Rex", 31, "ZIGZAGOON", "ROUTE103")):
        f = Fake(port, name, tid)  # noqa: F821 (run.py provides Fake)
        g, n = MAPS[place]
        f.ask(f"OEV CATCH {SPECIES[shiny]} 1 {g} {n} 0")
        f.close()
    for name, tid, species, place in (("Gary", 4242, "LATIOS", "ROUTE119"), ("Mia", 777, "RAIKOU", "ROUTE110")):
        f = Fake(port, name, tid)  # noqa: F821
        g, n = MAPS[place]
        f.ask(f"OEV SIGHT {SPECIES[species]} {g} {n} 1")
        f.close()
    return after


def after(port, out):
    base = f"http://127.0.0.1:{Fake.http}"  # noqa: F821
    for _ in range(600):
        time.sleep(1)
        try:
            if "TEST1" in urllib.request.urlopen(base + "/shinies").read().decode():
                break
        except OSError:
            pass
    time.sleep(6)  # both players' positions are fresh
    subprocess.call(["node", os.path.join(HERE, "web_shot.js"), base + "/map", os.path.join(out, "web-map.png"),
                     base + "/shinies", os.path.join(out, "web-shinies.png")])
    open(os.path.join(out, "web_done"), "w").close()
