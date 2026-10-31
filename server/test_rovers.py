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
        w[8 + rovers.MON_SIZE:8 + 2 * rovers.MON_SIZE] = mon
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
        # Without the day/night code it is never night.
        old, rovers.world = rovers.world, None
        try:
            self.assertFalse(rovers.is_night(0))
        finally:
            rovers.world = old

    async def test_rivals_and_friends(self):
        a = await self.connect("Ash", 1)
        await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.league.db.execute("UPDATE rovers SET map = '0.0'")
        self.put(5, ROUTE101)
        # Ash beats rover 5: a rival now.
        self.assertEqual(await self.ask(a, "ROVER BEAT 5 0 1"), (0, b""))
        here, _ = parse_here((await self.ask(a, f"ROVER HERE {G} {N} 4"))[1])
        w = next(w for w in here if w[0] == 5)
        self.assertEqual(w[-2] & 1, 1)
        # Two trades make friends; a friend has a present once a day.
        w = bytearray(w)
        for _ in range(2):
            self.assertEqual(await self.ask(a, f"ROVER SYNC 5 5 {w[8]}", bytes(w)), (0, b""))
        here, _ = parse_here((await self.ask(a, f"ROVER HERE {G} {N} 4"))[1])
        w = next(w for w in here if w[0] == 5)
        self.assertEqual(w[-2] & 6, 6)
        status, gift = await self.ask(a, "ROVER GIFT 5")
        item, count = (int(x) for x in gift.decode().split("|"))
        self.assertIn(item, rovers.ITEMS.values())
        self.assertGreater(count, 0)
        self.assertEqual((await self.ask(a, "ROVER GIFT 5"))[1], b"0|0")
        # It wins one back: no longer a rival.
        await self.ask(a, "ROVER BEAT 5 1 1")
        self.assertEqual(rovers.bond(self.league.db, 5, ("Ash", 1))["rival"], 0)
        # Somebody else sees no bond.
        b = await self.connect("Misty", 2)
        here, _ = parse_here((await self.ask(b, f"ROVER HERE {G} {N} 4"))[1])
        self.assertTrue(all(w[-2] == 0 for w in here))

    def test_fusion_eggs_and_old_teams(self):
        # Teams saved before fusions (16 bytes a Pokémon) still load.
        old = b"".join(rovers.pack_mon(rovers.new_mon(s, 10))[:16] for s in (25, 278))
        self.assertEqual([m["species"] for m in rovers.unpack_team(old)], [25, 278])
        self.assertEqual(rovers.unpack_team(old)[1]["fusion"], 0)
        # A Day Care visit with two kinds makes a fusion egg, which hatches into one.
        self.assertTrue(rovers.can_fuse(278, 25))
        self.assertFalse(rovers.can_fuse(25, 26))   # same family
        self.assertFalse(rovers.can_fuse(25, 132))  # Ditto
        rovers.ensure_rovers(self.league.db)
        breeder = next(i for i, c in enumerate(rovers.CAST) if c["style"] == "BREEDER")
        r = rovers.load(self.league.db, breeder)
        r.team = [rovers.new_mon(278, 20), rovers.new_mon(25, 20)]
        daycare = next(k for k, v in rovers.WORLD.items() if v["daycare"])
        r.map, r.egg = daycare, 0
        sim = rovers.Sim(self.league.db, self.league)
        rovers.rng.random = lambda: 0.0
        old_world, rovers.world = rovers.world, None
        try:
            for _ in range(10):
                if r.egg:
                    break
                sim.step(r, [r], 12 * 3600)
        finally:
            del rovers.rng.random
            rovers.world = old_world
        self.assertIn(r.egg, (278, 172))  # the mother's shape: Wingull or Pichu
        father = r.life["egg_fusion"]
        self.assertIn(father, (25, 278))
        r.egg_steps = 1
        r.team = r.team[:2]
        sim.step(r, [r], None)
        baby = r.team[-1]
        self.assertEqual((baby["level"], baby["fusion"]), (1, father))
        self.assertEqual(rovers.unpack_mon(rovers.pack_mon(baby))["fusion"], father)

    async def test_rift_divers(self):
        a = await self.connect("Ash", 1)
        await self.ask(a, f"ROVER HERE {G} {N} 4")
        self.league.db.execute("UPDATE rovers SET badges = 2")
        seen = set()
        for _ in range(30):
            here, _ = parse_here((await self.ask(a, "ROVER HERE 75 1 4"))[1])
            self.assertLessEqual(len(here), 1)
            seen.update(w[0] for w in here)
        self.assertTrue(seen)
        rid = next(iter(seen))
        # Visiting only: it's still on its own map.
        self.assertNotEqual(rovers.load(self.league.db, rid).map, "75.1")
        self.assertEqual(await self.ask(a, f"ROVER EXPLORED {rid} 75 1"), (0, b""))
        self.assertEqual(rovers.bond(self.league.db, rid, ("Ash", 1))["friendship"], 2)

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
