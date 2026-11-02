"""Other players have events up: Gary's tournament is taking sign-ups, Lena
is running a catch hunt, and Ivy's four-player cup is under way."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import ITEMS, SPECIES  # noqa: E402

ENV = {"NETPLAY_FISHT_LENGTH": 0}


def host(port, name, tid, kind, choice, time, money, item=None):
    f = Fake(port, name, tid)  # noqa: F821
    f.ask(f"HEVENT NEW {kind}")
    f.ask(f"HEVENT {'TARGET' if kind == 0 else 'SIZE'} {choice}")
    f.ask(f"HEVENT TIME {time} 999999")
    f.ask(f"HEVENT MONEY {money}")
    if item:
        f.ask(f"HEVENT ITEM {ITEMS[item[0]]} {item[1]}")
    f.ask("HEVENT POST 999999")
    eid = f.ask("HEVENT LIST")[1].split("\n")[-1].split("|")[1]
    return f, eid


def seed(port, out):
    fakes = []
    f, gary = host(port, "Gary", 4242, 1, 8, 3, 50000)
    fakes.append(f)
    for name, tid in (("Mia", 777), ("Rex", 31)):
        p = Fake(port, name, tid)  # noqa: F821
        p.ask(f"HEVENT JOIN {gary}")
        fakes.append(p)
    f, _ = host(port, "Lena", 515, 0, SPECIES["ABSOL"], 2, 10000, ("MASTER_BALL", 1))
    fakes.append(f)
    f, ivy = host(port, "Ivy", 88, 1, 4, 1, 5000, ("NUGGET", 3))
    fakes.append(f)
    for name, tid in (("Kai", 12), ("Zoe", 13), ("Max", 14), ("Ada", 15)):
        p = Fake(port, name, tid)  # noqa: F821
        p.ask(f"HEVENT JOIN {ivy}")
        fakes.append(p)
    for f in fakes:
        f.close()
