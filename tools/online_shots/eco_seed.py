"""Other players have caught a lot of one species on Route 102: TEST1's
catch thins it below the point where the rest flee. Picks a species whose
flight goes to a route with known grass (written to eco_target.lua), and
makes the ones that fled show up in every encounter there."""
import os
import sys
import time

# Noon on the server's clock, so the shots are by day.
_now = time.gmtime()
ENV = {"NETPLAY_FISHT_LENGTH": 0, "NETPLAY_ECO_FLEE_RATE": 100,
       "NETPLAY_CLOCK_UTC_OFFSET": (12 * 60 - (_now.tm_hour * 60 + _now.tm_min)) % 1440}

GRASS = {"ROUTE101": (4, 4), "ROUTE104": (17, 63)}


def seed(port, out):
    sys.path.insert(0, "server")
    import online_eco
    from online_ids import MAPS, SPECIES

    src = "%d.%d" % tuple(MAPS["ROUTE102"])
    links = online_eco.linked_wild(src)
    names = {"%d.%d" % tuple(v): k for k, v in MAPS.items()}
    hour = int(time.time() // 3600)
    migrating = {m[0] for m in online_eco.plan_migrations(int(time.time() // 86400))}
    for name in ["SEEDOT", "RALTS", "LOTAD", "FLETCHLING"]:
        sp = SPECIES[name]
        dst = names[links[(sp + hour) % len(links)]]
        if dst in GRASS and sp not in migrating and sp not in online_eco.home_species("%d.%d" % tuple(MAPS[dst])):
            break
    x, y = GRASS[dst]
    with open(os.path.join(out, "eco_target.lua"), "w") as f:
        f.write(f'ECO = {{ species = "{name}", map = MAPS.MAP_{dst}, x = {x}, y = {y} }}\n')
    gary = Fake(port, "Gary", 4242)  # noqa: F821 (run.py provides Fake)
    for _ in range(7):
        gary.ask(f"OEV CATCH {sp} 0 {MAPS['ROUTE102'][0]} {MAPS['ROUTE102'][1]} 0")
    gary.close()
