import asyncio
import json
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import netplay_server as ns  # noqa: E402
import world  # noqa: E402
from test_netplay_server import Client  # noqa: E402


class WorldTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        world.attach(self.server, os.path.join(self.tmp.name, "league.sqlite3"))
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []

    async def asyncTearDown(self):
        for c in self.clients:
            c.writer.close()
        self.srv.close()
        await self.srv.wait_closed()
        self.league.db.close()
        self.tmp.cleanup()

    async def connect(self, name, tid=1):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        c.id = int((await c.recv()).split()[1])
        return c

    async def test_look_and_house_reach_everyone_and_newcomers(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.send("LOOK 16409 0")
        self.assertEqual(await b.recv(), f"LOOK {a.id} 16409 0 0")
        # The follower's accessory (Pokémon fashion) goes with the look.
        await a.send("LOOK 16409 0 6")
        self.assertEqual(await b.recv(), f"LOOK {a.id} 16409 0 6")
        await a.send("HOUSE 1 " + "03" * 2 + "00" * 14)
        self.assertEqual(await b.recv(), f"HOUSE {a.id} 1 0303" + "00" * 14)
        c = await self.connect("Brock")
        await c.send("WORLD")
        self.assertEqual(await c.recv(), f"LOOK {a.id} 16409 0 6")
        self.assertEqual(await c.recv(), f"HOUSE {a.id} 1 0303" + "00" * 14)
        self.assertTrue((await c.recv()).startswith("HUNT "))
        self.assertTrue((await c.recv()).startswith("CLOCK "))

    async def test_everyone_gets_the_same_clock(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.send("CLOCK")
        await b.send("CLOCK")
        ta = int((await a.recv()).split()[1])
        tb = int((await b.recv()).split()[1])
        self.assertLessEqual(abs(ta - tb), 1)
        self.assertTrue(0 <= ta < 86400)

    def test_clock_follows_the_configured_offset(self):
        noon_utc = 1700000000 - 1700000000 % 86400 + 12 * 3600
        old = os.environ.get("NETPLAY_CLOCK_UTC_OFFSET")
        try:
            os.environ["NETPLAY_CLOCK_UTC_OFFSET"] = "120"
            self.assertEqual(world.clock_seconds(noon_utc), 14 * 3600)
            self.assertEqual(world.time_of_day(noon_utc), world.DAY)
            os.environ["NETPLAY_CLOCK_UTC_OFFSET"] = "540"  # 21:00
            self.assertEqual(world.time_of_day(noon_utc), world.NIGHT)
            os.environ["NETPLAY_CLOCK_UTC_OFFSET"] = "-300"  # 7:00
            self.assertEqual(world.time_of_day(noon_utc), world.MORNING)
        finally:
            if old is None:
                os.environ.pop("NETPLAY_CLOCK_UTC_OFFSET", None)
            else:
                os.environ["NETPLAY_CLOCK_UTC_OFFSET"] = old

    async def test_board_keeps_each_players_best(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.send("SCORE 0 0 900 129")
        await a.send("SCORE 0 0 700 129")  # not better: ignored
        await b.send("SCORE 0 0 1200 72")
        await b.send("HUNT")
        await b.recv()  # Misty's score is in
        await a.send("WBOARD 0 0")
        self.assertEqual(await a.recv(), "WROW 0 1 Misty 1200 72")
        self.assertEqual(await a.recv(), "WROW 0 2 Ash 900 129")
        self.assertEqual(await a.recv(), "WBOARDEND 0 2")
        saved = json.load(open(os.path.join(self.tmp.name, "league.world.json")))
        self.assertEqual(len(saved["fish:0"]), 2)

    async def test_hunt_scores_count_only_for_the_current_hunt(self):
        a = await self.connect("Ash")
        hunt_id, _ = world.current_hunt()
        await a.send(f"SCORE 3 {hunt_id + 1} 500 263")
        await a.send(f"SCORE 3 {hunt_id} 400 263")
        await a.send(f"WBOARD 3 {hunt_id}")
        self.assertEqual(await a.recv(), "WROW 3 1 Ash 400 263")
        self.assertEqual(await a.recv(), "WBOARDEND 3 1")

    async def test_dungeon_board_ranks_the_deepest_floor(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.send("SCORE 8 0 12 3")
        await a.send("HUNT")
        await a.recv()  # Ash's score is in
        await b.send("SCORE 8 0 8 1")
        await b.send("WBOARD 8 0")
        self.assertEqual(await b.recv(), "WROW 8 1 Ash 12 3")
        self.assertEqual(await b.recv(), "WROW 8 2 Misty 8 1")
        self.assertEqual(await b.recv(), "WBOARDEND 8 2")

    async def test_treasure_board_counts_treasures_found(self):
        a = await self.connect("Ash")
        await a.send("SCORE 9 0 1 0")
        await a.send("SCORE 9 0 2 4")
        await a.send("WBOARD 9 0")
        self.assertEqual(await a.recv(), "WROW 9 1 Ash 2 4")
        self.assertEqual(await a.recv(), "WBOARDEND 9 1")

    async def test_frontier_boards_keep_the_best_streak(self):
        a = await self.connect("Ash")
        for board in (10, 11, 12):
            await a.send(f"SCORE {board} 0 20 0")
            await a.send(f"SCORE {board} 0 7 1")  # a worse streak: ignored
            await a.send(f"WBOARD {board} 0")
            self.assertEqual(await a.recv(), f"WROW {board} 1 Ash 20 0")
            self.assertEqual(await a.recv(), f"WBOARDEND {board} 1")

    async def test_photo_goes_to_the_gallery(self):
        a = await self.connect("Ash")
        png = b"\x89PNG\r\n\x1a\n" + bytes(range(200))
        hexdata = png.hex()
        for i in range(0, len(hexdata), 200):
            await a.send("PHOTO " + hexdata[i:i + 200])
        await a.send("PHOTOEND 0 9")
        await a.send("HUNT")
        await a.recv()  # the server has handled everything before this
        gallery = os.path.join(self.tmp.name, "league-gallery")
        index = json.load(open(os.path.join(gallery, "index.json")))
        self.assertEqual(index[0]["player"], "Ash")
        with open(os.path.join(gallery, index[0]["file"]), "rb") as f:
            self.assertEqual(f.read(), png)

    def test_hunts_change_every_half_hour(self):
        first, left = world.current_hunt(1800 * 1000 + 60)
        self.assertEqual(left, 29)
        self.assertEqual(world.current_hunt(1800 * 1001 + 1)[0], first + 1)


if __name__ == "__main__":
    unittest.main()
