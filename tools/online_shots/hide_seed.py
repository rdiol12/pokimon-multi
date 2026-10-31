"""Hide and seek in Petalburg Woods: TEST1 seeks, TEST2 and TEST3 hide.
Others have points on the week's board already. Also plans each player's
walk (hide_path.lua in the output folder), as in racing_seed.py."""
import importlib.util
import os

# The game runs at its real speed: the server's clock runs the game.
ENV = {"NETPLAY_FISHT_LENGTH": 0, "NETPLAY_HIDE_LOBBY": 20, "NETPLAY_FPS": 60}
HERE = os.path.dirname(os.path.abspath(__file__))
# From the seeker's spot and the hiders' lanes (sArenas[0] in
# src/online_hide.c) to where each goes: next to TEST2's hiding place for
# the seeker.
AVOID = frozenset()
WALKS = [((12, 9), (7, 27)), ((8, 33), (6, 27)), ((10, 33), (40, 35))]


def seed(port, out):
    import sqlite3
    import time
    spec = importlib.util.spec_from_file_location("racing_seed", os.path.join(HERE, "racing_seed.py"))
    racing = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(racing)

    Fake(port, "Gary", 4242).close()  # noqa: F821 (the server makes its tables on start)
    db = sqlite3.connect(os.path.join(out, "league.sqlite3"))
    week = int(time.time() // (7 * 86400))
    db.executemany("INSERT INTO online_hide_points VALUES (?, ?, ?, ?)",
                   [(week, "Mia", 777, 9), (week, "Rex", 31337, 4)])
    db.commit()

    # The game's state (sHide in src/online_hide.c), for the scenario to follow.
    import subprocess
    nm = subprocess.run(["arm-none-eabi-nm", os.path.join(HERE, "..", "..", "pokeemerald.elf")],
                        capture_output=True, text=True).stdout
    hide = next(int(line.split()[0], 16) for line in nm.splitlines() if line.endswith(" sHide"))
    free = racing.route_grid("PetalburgWoods", ["primary/general", "secondary/rustboro"])
    with open(os.path.join(out, "hide_path.lua"), "w") as f:
        f.write(f"HIDE_STATE = 0x{hide:08x}\n")
        f.write("HIDE_PATH = {\n")
        for start, goal in WALKS:
            corners = racing.plan(free, start, avoid=AVOID, goal=lambda p, g=goal: p == g)[1]
            f.write("    {" + ", ".join(f"{{{x}, {y}}}" for x, y in corners) + "},\n")
        f.write("}\n")
