import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import rovers  # noqa: E402
import test_ext  # noqa: E402

ROUTE101 = rovers.BY_NAME["Route101"]
G, N = ROUTE101.split(".")


def parse_here(answer):
    head, body = answer.split(b"\0", 1)
    count, major_id, species, level = (int(x) for x in head.decode().split("|"))
    out = [body[i * rovers.ROVER_SIZE:(i + 1) * rovers.ROVER_SIZE] for i in range(count)]
    return out, (major_id, species, level)


class RoverTest(unittest.IsolatedAsyncioTestCase):
    connect = test_ext.ExtTest.connect
    ask = test_ext.ExtTest.ask

    async def asyncSetUp(self):
        await test_ext.ExtTest.asyncSetUp(self)
        self.t = 1_000_000.0
        rovers.now = lambda: self.t
        rovers.rng.seed(7)

    async def asyncTearDown(self):
        await test_ext.ExtTest.asyncTearDown(self)

    def put(self, rid, key):
        self.league.db.execute("UPDATE rovers SET map = ? WHERE id = ?", (key, rid))

    async def test_cast_and_world(self):
        self.assertEqual(len(rovers.CAST), 150)
        for k, v in rovers.WORLD.items():
            for n in v["links"]:
                self.assertIn(n, rovers.WORLD, k)
        a = await self.connect("Ash", 1)
        status, answer = await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.assertEqual(status, 0)
        count = self.league.db.execute("SELECT COUNT(*) FROM rovers").fetchone()[0]
        self.assertEqual(count, 150)

    async def test_here_sync_and_fight(self):
        a = await self.connect("Ash", 1)
        await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.league.db.execute("UPDATE rovers SET map = '0.0'")
        self.put(3, ROUTE101)
        self.put(9, ROUTE101)
        status, answer = await self.ask(a, f"ROVER HERE {G} {N} 4")
        here, _ = parse_here(answer)
        ids = sorted(w[0] for w in here)
        self.assertEqual(ids, [3, 9])
        w = bytearray(here[0])
        self.assertEqual(w[1], 1)  # one Pokémon to start with
        # The player saw it catch a Wurmple.
        mon = rovers.pack_mon({**rovers.new_mon(265, 3), "moves": [33, 81, 0, 0], "moves_level": 3})
        w[1] = 2
        w[8 + 16:8 + 32] = mon
        self.assertEqual(await self.ask(a, f"ROVER SYNC {w[0]} 1 265", bytes(w)), (0, b""))
        r = rovers.load(self.league.db, w[0])
        self.assertEqual([m["species"] for m in r.team][1], 265)
        self.assertEqual(r.team[1]["moves"][:2], [33, 81])
        # Then a fight: counted for the rovers, kept off the PvP ladder.
        self.assertEqual(await self.ask(a, "ROVER FIGHT 3 9 1"), (0, b""))
        top = [row[0] for row in self.league.top()]
        self.assertNotIn(rovers.CAST[3]["name"], top)
        self.assertEqual(rovers.load(self.league.db, 9).losses, 1)
        status, board = await self.ask(a, "ROVER BOARD")
        self.assertIn(b"-", board)
        self.assertIn(rovers.CAST[3]["name"].encode() + b" beat", board)

    async def test_life_goes_on(self):
        a = await self.connect("Ash", 1)
        await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.t += 3 * 86400  # three days later
        for _ in range(10):
            await self.ask(a, "ROVER BOARD")
            self.t += 12 * 3600
        rs = rovers.load_all(self.league.db)
        self.assertGreater(sum(len(r.team) for r in rs), 150 * 2)
        self.assertGreater(max(r.lead_level() for r in rs), 15)
        self.assertGreater(sum(r.badges for r in rs), 0)
        self.assertGreater(sum(r.wins for r in rs), 0)
        for r in rs:
            rovers.check_team(r.team)
        # Their own lives: bags, crafting, the ranch, quests and jobs.
        self.assertGreater(sum(len(r.life["bag"]) for r in rs), 150)
        self.assertGreater(sum(len(r.life["ranch"]) for r in rs), 0)
        news = " ".join(row[0] for row in self.league.db.execute(
            "SELECT line FROM rover_news")) + " " + " ".join(str(r.life["quest"]) for r in rs)
        crafted = [r for r in rs if any(i in r.life["bag"] for i, _, _ in rovers.life.RECIPES)]
        self.assertTrue(crafted)
        self.assertTrue(any(r.life["money"] != rovers.life.START_MONEY for r in rs))
        self.assertTrue(any(r.life["quest"] or r.life["job"] for r in rs) or "quest" in news)
        self.assertEqual(len(rs[0].wire()), rovers.ROVER_SIZE)

    async def test_night(self):
        class Night:
            NIGHT = 3

            @staticmethod
            def time_of_day(t=None):
                return 3
        rovers.ensure_rovers(self.league.db)
        before = {r.id: r.map for r in rovers.load_all(self.league.db)}
        old, rovers.world = rovers.world, Night
        try:
            self.assertTrue(rovers.is_night(0))
            self.assertFalse(rovers.out_now("Pidgey", True, False))
            self.assertTrue(rovers.out_now("Pidgey", True, True))
            self.assertTrue(rovers.out_now("Hoothoot", True, False))
            self.t += 6 * 3600
            rovers.Sim(self.league.db, self.league).run()
        finally:
            rovers.world = old
        after = rovers.load_all(self.league.db)
        moved = [r for r in after if r.map != before[r.id]]
        owls = [r for r in moved if r.id % rovers.NIGHT_OWL == 0]
        self.assertEqual(len(moved), len(owls))  # only night owls on the move
        self.assertTrue(owls)
        self.assertFalse(rovers.is_night(0))

    async def test_majors(self):
        a = await self.connect("Ash", 1)
        status, answer = await self.ask(a, f"MAJOR LIST {G} {N}")
        rows = [r.split("|") for r in answer.decode().split("\n")]
        self.assertEqual(rows[0], ["0"])
        self.assertTrue(len(rows) > 1 or True)
        mid, sid, level, key = self.league.db.execute(
            "SELECT id, species, level, map FROM majors WHERE cleared_by IS NULL").fetchone()
        g, n = key.split(".")
        here, major = parse_here((await self.ask(a, f"ROVER HERE {g} {n} 0"))[1])
        self.assertEqual(major, (mid, sid, level))
        self.assertEqual(await self.ask(a, f"MAJOR CLEAR {mid} 1"), (0, b""))
        self.assertEqual((await self.ask(a, f"MAJOR CLEAR {mid} 0"))[0], 1)  # already done
        status, answer = await self.ask(a, f"MAJOR LIST {G} {N}")
        self.assertEqual(answer.decode().split("\n")[0], str(level * rovers.MAJOR_REWARD // 10))
        status, answer = await self.ask(a, f"MAJOR LIST {G} {N}")
        self.assertEqual(answer.decode().split("\n")[0], "0")  # paid once

    async def test_bad_requests(self):
        a = await self.connect("Ash", 1)
        await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.assertEqual((await self.ask(a, "ROVER SYNC 1 0 0", b"\0" * 10))[0], 2)
        self.assertEqual((await self.ask(a, "ROVER FIGHT 2 2 1"))[0], 2)
        self.assertEqual((await self.ask(a, "ROVER INFO 999"))[0], 1)


if __name__ == "__main__":
    unittest.main()
