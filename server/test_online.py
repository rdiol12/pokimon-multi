import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_coopfish  # noqa: E402
import online_fishing  # noqa: E402
import online_goals  # noqa: E402
import online_sightings  # noqa: E402
import online_web  # noqa: E402
from online_ids import ITEMS, MAPS, SPECIES  # noqa: E402
from test_netplay_server import Client  # noqa: E402


def rows(out):
    return [r.split("|") for r in out.decode().split("\n")]


class OnlineTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.now = 1_000 * online.WEEK + 100
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server),
                                    [online_goals, online_sightings, online_web, online_fishing, online_coopfish], clock=lambda: self.now)
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

    async def ask(self, c, request, data=b""):
        """Sends a request like the bridge does; returns (status, answer)."""
        self.seq += 1
        for i in range(0, len(data), 100):
            await c.send("XD " + data[i:i + 100].hex())
        await c.send(f"X {self.seq} {request}")
        while True:
            line = await c.recv()
            if line.startswith(f"XR {self.seq} "):
                parts = line.split(" ")
                return int(parts[2]), bytes.fromhex(parts[3]) if len(parts) > 3 else b""


class GoalTest(OnlineTest):
    async def test_goals_fill_and_pay_helpers(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        old = online_goals.GOALS
        online_goals.GOALS = [("catch", "Catch {n} Pok#mon", 2, "ULTRA_BALL", 10), *old[1:]]
        try:
            featured = SPECIES[online_goals.featured(self.online.week())]
            self.assertEqual(await self.ask(a, f"OEV CATCH {featured} 0 0 9 1"), (0, b"|0"))
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[0], ["", "0"])
            self.assertEqual(rows(out)[3][:2], ["%", "50"])
            await self.ask(a, "OEV CATCH 1 0 0 9 0")
            # The goal is reached: both players hear of it, only Ann helped.
            status, out = await self.ask(b, "OSTAT")
            self.assertIn("N", [r[0] for r in rows(out)])
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[0][1], "1")
            self.assertEqual((await self.ask(b, "GOAL CLAIM"))[0], 1)
            status, out = await self.ask(a, "GOAL CLAIM")
            self.assertEqual(status, 0)
            self.assertIn(["+ITEM", str(ITEMS["ULTRA_BALL"]), "10"], rows(out))
            self.assertEqual((await self.ask(a, "GOAL CLAIM"))[0], 1)
            # Next week starts from zero.
            self.now += online.WEEK
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[3][:2], ["%", "0"])
        finally:
            online_goals.GOALS = old


class SightingTest(OnlineTest):
    async def test_sightings_board_and_notice(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        latios, route = SPECIES["LATIOS"], MAPS["ROUTE117"]
        await self.ask(a, f"OEV SIGHT {latios} {route[0]} {route[1]} 1")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn(["N", "Ann spotted LATIOS at ROUTE 117!"], rows(out))
        self.now += 300
        await self.ask(a, f"OEV SIGHT {latios} {route[0]} {route[1]} 1")
        status, out = await self.ask(b, "OSTAT")
        self.assertNotIn("N", [r[0] for r in rows(out)])  # announced recently
        status, out = await self.ask(b, "SIGHT BOARD")
        self.assertEqual(rows(out)[3], ["LATIOS", "ROUTE 117", "Ann", "now"])


class WebTest(OnlineTest):
    async def test_live_map_and_shiny_gallery(self):
        online_web.install(self.server)
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        g, n = MAPS["OLDALE_TOWN"]
        await a.send(f"POS {g} {n} 5 5 1 3 0")
        absol = SPECIES["ABSOL"]
        r120 = MAPS["ROUTE120"]
        await self.ask(a, f"OEV CATCH {absol} 1 {r120[0]} {r120[1]} 0")
        await self.ask(a, f"OEV CATCH {absol} 0 {r120[0]} {r120[1]} 0")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn(["N", "Ann caught a shiny ABSOL!"], rows(out))
        route = self.server.web_routes[0]
        data = __import__("json").loads(route("/map.json")[2])
        self.assertEqual([p["label"] for p in data["players"]], ["Ann"])
        self.assertIn("ABSOL", route("/shinies")[2])
        self.assertEqual(route("/shinies")[2].count('class="mon"'), 1)
        png = route(f"/sprite/{absol}.png")[2]
        self.assertTrue(png.startswith(b"\x89PNG") and b"tRNS" in png)
        self.assertIsNone(route("/sprite/999999.png"))


class FishingTest(OnlineTest):
    async def test_tournament_scores_and_prizes(self):
        online_fishing.START, online_fishing.PERIOD, online_fishing.LENGTH = self.now - 60, 7200, 600
        online_fishing._announced.update(started=None, finished=None)
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        status, out = await self.ask(a, "OSTAT")
        fisht = next(r for r in rows(out) if r[:2] == ["S", "FISHT"])
        self.assertEqual(fisht[2:4], ["1", "9"])
        k = int(fisht[7])
        rare = SPECIES[online_fishing.rares(k)[0]]
        self.assertIn("N", [r[0] for r in rows(out)])  # it started
        await self.ask(a, f"OEV FISH 129 900")      # Magikarp, 90 cm
        await self.ask(b, f"OEV FISH {rare} 450")   # rare, 45 cm + 50
        await self.ask(a, f"OEV FISH 129 300")
        status, out = await self.ask(a, "FISHT BOARD")
        lines = rows(out)
        self.assertEqual(lines[3][:3], ["*1", "Ann", "120"])
        self.assertEqual(lines[4][:3], ["2", "Ben", "95"])
        self.assertEqual(lines[0][1], "0")
        # Over: Ann is first, Ben second; both collect.
        self.now += 700
        status, out = await self.ask(a, "FISHT BOARD")
        self.assertEqual(rows(out)[0][1], "1")
        self.assertIn("RESULTS", rows(out)[1][0])
        status, out = await self.ask(b, "FISHT CLAIM")
        self.assertIn(["+ITEM", str(ITEMS["RARE_CANDY"]), "3"], rows(out))
        status, out = await self.ask(a, "FISHT CLAIM")
        self.assertIn(["+MONEY", "20000"], rows(out))
        self.assertEqual((await self.ask(a, "FISHT CLAIM"))[0], 1)
        # Reports outside a tournament don't count.
        await self.ask(a, "OEV FISH 129 900")
        self.assertEqual(self.league.db.execute("SELECT COUNT(*) FROM online_fish_scores").fetchone()[0], 2)


class CoFishTest(OnlineTest):
    async def test_two_players_land_a_giant(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        for c in (a, b):
            await c.send("POS 0 9 5 5 1 3 0")
            await self.ask(c, "OSTAT")
        status, out = await self.ask(a, "COFISH WHO")
        ben_id = rows(out)[1][1]
        self.assertEqual(rows(out)[1][0], "Ben")
        await self.ask(a, f"COFISH ASK {ben_id}")
        self.assertEqual(rows((await self.ask(a, "COFISH WAIT"))[1])[0][1], "0")
        status, out = await self.ask(b, "OSTAT")
        invite = next(r for r in rows(out) if r[:2] == ["S", "COFISH"])
        self.assertEqual(invite[2:4], ["1", "Ann"])
        await self.ask(b, "COFISH JOIN")
        self.assertEqual(rows((await self.ask(a, "COFISH WAIT"))[1])[0][1], "1")
        await self.ask(a, "COFISH REEL 3")
        self.assertEqual(rows((await self.ask(a, "COFISH RESULT"))[1])[0][1], "0")
        await self.ask(b, "COFISH REEL 2")
        for c in (a, b):
            status, out = await self.ask(c, "COFISH RESULT")
            self.assertEqual(rows(out)[0][1], "1")
            self.assertEqual(rows(out)[1][0], "+MON")
        status, out = await self.ask(a, "COFISH BOARD")
        self.assertEqual(rows(out)[3][2], "Ann & Ben")
        # Not enough hits: it gets away, and nobody gets anything.
        await self.ask(b, f"COFISH ASK {rows((await self.ask(b, 'COFISH WHO'))[1])[1][1]}")
        await self.ask(a, "COFISH JOIN")
        await self.ask(a, "COFISH REEL 1")
        await self.ask(b, "COFISH REEL 1")
        status, out = await self.ask(a, "COFISH RESULT")
        self.assertEqual(rows(out)[0][1], "2")
        self.assertEqual(len(rows(out)), 1)


if __name__ == "__main__":
    unittest.main()
