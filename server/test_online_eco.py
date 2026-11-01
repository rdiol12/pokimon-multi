import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_eco  # noqa: E402
import online_research  # noqa: E402
import rovers  # noqa: E402
from online_ids import MAPS, SPECIES  # noqa: E402
from test_netplay_server import Client  # noqa: E402


def rows(out):
    return [r.split("|") for r in out.decode().split("\n")]


ROUTE102 = MAPS["ROUTE102"]
ROUTE102_KEY = f"{ROUTE102[0]}.{ROUTE102[1]}"


class EcoTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.now = 20_000 * 86400 + 3600
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server), [online_eco, online_research],
                                    clock=lambda: self.now)
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []
        self.seq = 0

    async def asyncTearDown(self):
        for c in self.clients:
            c.writer.close()
        self.srv.close()
        await self.srv.wait_closed()
        self.league.db.close()

    async def connect(self, name, tid):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        self.assertTrue((await c.recv()).startswith("WELCOME "))
        return c

    async def ask(self, c, request):
        self.seq += 1
        await c.send(f"X {self.seq} {request}")
        while True:
            line = await c.recv()
            if line.startswith(f"XR {self.seq} "):
                parts = line.split(" ")
                return int(parts[2]), bytes.fromhex(parts[3]) if len(parts) > 3 else b""

    def eco_status(self, out):
        return next((r for r in rows(out) if r[:2] == ["S", "ECO"]), None)

    async def test_catches_thin_flee_and_recover(self):
        old, online_eco.MIGRATIONS_PER_DAY = online_eco.MIGRATIONS_PER_DAY, 0
        self.addCleanup(setattr, online_eco, "MIGRATIONS_PER_DAY", old)
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await a.send(f"POS {ROUTE102[0]} {ROUTE102[1]} 5 5 1 3 0")
        await self.ask(b, "OSTAT")
        zigzagoon = SPECIES["ZIGZAGOON"]
        self.assertIn(zigzagoon, online_eco.home_species(ROUTE102_KEY))
        for _ in range(3):
            await self.ask(a, f"OEV CATCH {zigzagoon} 0 {ROUTE102[0]} {ROUTE102[1]} 0")
        status, out = await self.ask(a, "OSTAT")
        eco = self.eco_status(out)
        self.assertEqual(eco[2], ROUTE102_KEY)
        self.assertEqual(eco[3], f"{zigzagoon}:{100 - 3 * online_eco.CATCH_COST}")
        # Thinned below FLEE_AT: the rest flee to a linked route, and everyone hears.
        notes = []
        for _ in range(5):
            status, out = await self.ask(a, f"OEV CATCH {zigzagoon} 0 {ROUTE102[0]} {ROUTE102[1]} 0")
            notes += [r[1].replace("~", " ") for r in rows(out) if r[0] == "N"]
        self.assertTrue(any("fleeing" in n and "ZIGZAGOON" in n for n in notes), notes)
        status, out = await self.ask(b, "OSTAT")
        self.assertNotIn("N", [r[0] for r in rows(out)])  # only the catcher hears
        moves = online_eco.active_moves(self.league.db, self.now, src=ROUTE102_KEY)
        flee = [m for m in moves if m[0] == "flee"]
        self.assertEqual(len(flee), 1)
        dst = flee[0][3]
        # The player on the route it fled to gets it as a migrant.
        group, num = dst.split(".")
        await b.send(f"POS {group} {num} 5 5 1 3 0")
        status, out = await self.ask(b, "OSTAT")
        eco = self.eco_status(out)
        self.assertIn(f"{zigzagoon}:{online_eco.FLEE_RATE}", eco[5:7])
        self.assertIn("are hiding around here", " ".join(r[1].replace("~", " ") for r in rows(out) if r[0] == "N"))
        # The area report.
        status, out = await self.ask(a, "ECO AREA")
        lines = rows(out)
        self.assertEqual(lines[1], ["WILDLIFE: ROUTE 102"])
        self.assertIn(["ZIGZAGOON", "36%", "fled"], lines)
        # A day later, it's back.
        self.now += 86400
        status, out = await self.ask(a, "ECO AREA")
        self.assertIn(["ZIGZAGOON", "100%", ""], rows(out))

    async def test_daily_migrations_are_announced(self):
        a = await self.connect("Ann", 1)
        status, out = await self.ask(a, "OSTAT")
        self.assertNotIn("N", [r[0] for r in rows(out)])  # nobody is where they went
        planned = online_eco.plan_migrations(self.online.day())
        self.assertEqual(len(planned), online_eco.MIGRATIONS_PER_DAY)
        species, src, dst = planned[0]
        self.assertNotIn(species, online_eco.home_species(dst))
        self.assertLessEqual(online_eco.population(self.league.db, src, species, self.now),
                             online_eco.HOME_AFTER_MIGRATION)
        group, num = dst.split(".")
        await a.send(f"POS {group} {num} 5 5 1 3 0")
        status, out = await self.ask(a, "OSTAT")
        self.assertIn(f"{species}:{online_eco.MIGRANT_RATE}", self.eco_status(out)[5:7])
        notes = [r[1].replace("~", " ") for r in rows(out) if r[0] == "N"]
        self.assertEqual(len(notes), 1)
        self.assertIn(f"{online_eco.species_name(species)} has migrated here", notes[0])
        status, out = await self.ask(a, "ECO MOVES")
        self.assertEqual(rows(out)[1], ["MIGRATIONS NOW"])
        self.assertEqual(len(rows(out)) - 3, len(planned))
        # Told only once.
        status, out = await self.ask(a, "OSTAT")
        self.assertNotIn("N", [r[0] for r in rows(out)])

    async def test_rovers_thin_and_meet_migrants(self):
        await self.connect("Ann", 1)
        rovers.ECO = online_eco
        try:
            ctx = online_eco._RoverCtx(self.league.db, self.online)
            online_eco.ensure_day(ctx)
            zigzagoon = SPECIES["ZIGZAGOON"]
            online_eco.rover_catch(self.online, self.league.db, ROUTE102_KEY, zigzagoon, "Rover")
            self.assertEqual(online_eco.population(self.league.db, ROUTE102_KEY, zigzagoon, self.now),
                             100 - online_eco.ROVER_COST)
            species, src, dst = online_eco.plan_migrations(self.online.day())[0]
            weights = online_eco.rover_weights(self.online, self.league.db, dst, online_eco.WORLD[dst]["wild"])
            self.assertIn(species, [w[0] for w, _ in weights])
        finally:
            rovers.ECO = None

    async def test_research_leaders(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        await self.ask(a, f"OEV RESEARCH 9 {SPECIES['WURMPLE']}")
        await self.ask(a, f"OEV RESEARCH 10 {SPECIES['ZIGZAGOON']}")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn("Ann has completed the research on 10 Pok#mon species!", [r[1].replace("~", " ") for r in rows(out) if r[0] == "N"])
        status, out = await self.ask(b, "RESEARCH BOARD 3")
        lines = rows(out)
        self.assertEqual(lines[1], ["RESEARCH LEADERS"])
        self.assertEqual(lines[3], ["1. Ann", "10", "ZIGZAGOON"])
        self.assertEqual(lines[4], ["*2. Ben", "3", "-"])


if __name__ == "__main__":
    unittest.main()
