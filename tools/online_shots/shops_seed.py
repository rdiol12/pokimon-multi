"""Gary and Mia already run shops, and Mia has bought from Gary."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "server"))
from online_ids import ITEMS  # noqa: E402

# No fishing tournament (its notice would cover the menus).
ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    gary = Fake(port, "Gary", 4242)  # noqa: F821 (run.py provides Fake)
    for item, count, price in (("RARE_CANDY", 3, 4800), ("ULTRA_BALL", 10, 1200), ("REVIVE", 4, 1500)):
        gary.ask(f"SHOP STOCK {ITEMS[item]} {count} {price}")
    mia = Fake(port, "Mia", 777)  # noqa: F821
    for item, count, price in (("MAX_REPEL", 5, 700), ("LEMONADE", 6, 350)):
        mia.ask(f"SHOP STOCK {ITEMS[item]} {count} {price}")
    shop = mia.ask("SHOP LIST")[1].split("\n")[1].split("|")[1]
    mia.ask(f"SHOP OPEN {shop}")
    shelf = mia.ask("SHOP ITEMS")[1].split("\n")[2].split("|")[1]
    mia.ask(f"SHOP BUY {ITEMS['ULTRA_BALL']} 2 {shelf} 9999")
    gary.close()
    mia.close()
