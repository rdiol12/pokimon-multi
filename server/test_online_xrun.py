"""The Stormwild extraction run (server/online_xrun.py)."""
import os
import sys
import types
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import online_xrun as xr  # noqa: E402
from online_ids import ITEMS, MAPS  # noqa: E402
from test_online import OnlineTest, rows  # noqa: E402

ZONE = " ".join(map(str, MAPS["STORMWILD"]))


class XRunTest(OnlineTest):
    async def asyncSetUp(self):
        import asyncio
        import ext
        import netplay_server as ns
        import online
        self.now = 1_000 * online.WEEK + 100
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server), [xr], clock=lambda: self.now)
        xr.install(self.server)
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []
        self.seq = 0

    def player(self, name):
        return next(p for p in self.server.players.values() if p.name == name)

    def xrun(self):
        return self.server.xruns[0]

    async def start(self, *names):
        cs = [await self.connect(n, i) for i, n in enumerate(names, 1)]
        for c in cs:
            status, out = await self.ask(c, "XRUN JOIN 3")
            self.assertEqual(status, 0, out)
        self.now += xr.LOBBY
        for c in cs:
            status, out = await self.ask(c, "XRUN GO")
            self.assertEqual(rows(out)[0][1], str(len(cs)))
        return cs

    async def test_too_many_pokemon(self):
        a = await self.connect("Ann", 1)
        status, out = await self.ask(a, "XRUN JOIN 4")
        self.assertEqual(status, 1)
        self.assertIn("Only 3", out.decode())

    async def test_runners_share_a_run_and_caches(self):
        a, b = await self.start("Ann", "Ben")
        self.assertEqual(len(self.server.xruns), 1)
        run = self.xrun()
        cache = run.caches[0]
        x, y = xr.CACHES[cache]
        # Too far away first.
        await a.send(f"POS {ZONE} 1 1 1 3 0")
        status, out = await self.ask(a, f"XRUN LOOT {cache}")
        self.assertEqual(status, 1)
        await a.send(f"POS {ZONE} {x} {y + 1} 1 3 0")
        await b.send(f"POS {ZONE} {x} {y + 1} 1 3 0")
        status, out = await self.ask(a, f"XRUN LOOT {cache}")
        self.assertEqual(status, 0, out)
        self.assertEqual(rows(out)[1][0], "+ITEM")
        status, out = await self.ask(b, f"XRUN LOOT {cache}")
        self.assertEqual(status, 1)
        self.assertIn("Ann got here first", out.decode())
        # The status shows the cache as taken.
        status, out = await self.ask(b, "OSTAT")
        row = next(r for r in rows(out) if r[:2] == ["S", "XRUN"])
        self.assertEqual(int(row[5]), 1 << cache)

    async def test_extract_only_at_a_lit_beacon(self):
        a, = await self.start("Ann")
        run = self.xrun()
        start, beacon = run.windows[0]
        bx, by = xr.BEACONS[beacon]
        await a.send(f"POS {ZONE} {bx} {by + 1} 1 3 0")
        status, out = await self.ask(a, f"XRUN OUT {beacon} 5000")
        self.assertEqual(status, 1)  # not lit yet
        self.now = run.start + start + 10
        status, out = await self.ask(a, "OSTAT")
        row = next(r for r in rows(out) if r[:2] == ["S", "XRUN"])
        self.assertEqual(int(row[4]) // 100, beacon)
        status, out = await self.ask(a, f"XRUN OUT {(beacon + 1) % 3} 5000")
        self.assertEqual(status, 1)  # another beacon
        status, out = await self.ask(a, f"XRUN OUT {beacon} 5000")
        self.assertEqual(status, 0, out)
        self.assertIn("$5000", out.decode())
        status, out = await self.ask(a, "XRUN BOARD")
        self.assertEqual(rows(out)[3], ["*1", "Ann", "5000", "1/1"])

    async def test_raid_spoils_wait_for_the_winner_to_get_out(self):
        a, b = await self.start("Ann", "Ben")
        ann, ben = self.player("Ann"), self.player("Ben")
        session = types.SimpleNamespace(kind="battle", host=ann, guest=ben, reports={ann.key: "win", ben.key: "loss"},
                                        id=1, recorded=False, members=[ann, ben])
        xr.settle_raid(self.server, session)
        status, out = await self.ask(b, "OSTAT")
        row = next(r for r in rows(out) if r[:2] == ["S", "XRUN"])
        self.assertEqual(row[6], "1")
        nugget = ITEMS["NUGGET"]
        status, out = await self.ask(b, f"XRUN ROB {nugget} 3 286 31 1")
        self.assertEqual(status, 0, out)
        self.assertIn("Ann took", out.decode())
        # Ben can't be robbed twice for one battle.
        self.assertEqual((await self.ask(b, f"XRUN ROB {nugget} 3 0 0 0"))[0], 1)
        # Ann gets them when she's out.
        run = self.xrun()
        start, beacon = run.windows[1]
        self.now = run.start + start + 1
        bx, by = xr.BEACONS[beacon]
        await a.send(f"POS {ZONE} {bx} {by} 1 3 0")
        status, out = await self.ask(a, f"XRUN OUT {beacon} 100")
        self.assertEqual(status, 0, out)
        self.assertIn(["+ITEM", str(nugget), "3"], rows(out))
        self.assertIn(["+MON", "286", "31", "1"], rows(out))

    async def test_failing_loses_the_spoils(self):
        a, b = await self.start("Ann", "Ben")
        run = self.xrun()
        run.spoils[tuple(self.player("Ann").key)] = [("+ITEM", 1, 1)]
        self.now = run.start + xr.RUN + 1
        status, out = await self.ask(a, "OSTAT")
        row = next(r for r in rows(out) if r[:2] == ["S", "XRUN"])
        self.assertEqual(row[3], "0")  # time's up
        status, out = await self.ask(a, "XRUN FAIL")
        self.assertIn("spoils were lost", out.decode())
        status, out = await self.ask(a, "XRUN BOARD")
        self.assertEqual(rows(out)[3], ["*1", "Ann", "0", "0/1"])

    def test_beacon_windows(self):
        import random
        for seed in range(50):
            windows = xr.beacon_windows(random.Random(seed))
            self.assertIn(len(windows), (3, 4))
            for (t0, b0), (t1, b1) in zip(windows, windows[1:]):
                self.assertNotEqual(b0, b1)
                self.assertGreaterEqual(t1, t0 + xr.BEACON_OPEN)
            self.assertLess(windows[-1][0] + xr.BEACON_OPEN, xr.RUN)


if __name__ == "__main__":
    unittest.main()
