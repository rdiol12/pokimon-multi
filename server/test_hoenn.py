"""Tests for server/online_hoenn.py and solo battles others can watch."""
import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_hoenn  # noqa: E402
from online_ids import SPECIES  # noqa: E402
from test_online import OnlineTest, rows  # noqa: E402


class HoennTest(OnlineTest):
    async def asyncSetUp(self):
        self.now = 1_000 * online.WEEK + 100
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server), [online_hoenn], clock=lambda: self.now)
        online_hoenn.QUEUE.clear()
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []
        self.seq = 0

    def notes(self, out):
        """The notices, with the game's line and page breaks as spaces."""
        return [r[1].replace("~", " ").replace("^", " ") for r in rows(out) if r[0] == "N"]


class MilestoneTest(HoennTest):
    async def test_badges_are_announced_and_recorded(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        # Ann's first report after connecting: records only, no notice.
        await self.ask(a, "OEV MILESTONE 1 0 0")
        status, out = await self.ask(b, "OSTAT")
        self.assertEqual(self.notes(out), [])
        # Ben earns the first two badges while online.
        await self.ask(b, "OEV MILESTONE 1 0 1")
        await self.ask(b, "OEV MILESTONE 3 0 1")
        status, out = await self.ask(a, "OSTAT")
        self.assertEqual(self.notes(out), ["Ben beat ROXANNE and earned the STONE BADGE!",
                                           "Ben beat BRAWLY and earned the KNUCKLE BADGE! The first trainer ever!"])
        await self.ask(a, "OEV MILESTONE 3 1 1")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn("Ann became the Champion of Hoenn! The first trainer ever!", self.notes(out))

        status, out = await self.ask(a, "HREC 0")
        r = rows(out)
        self.assertEqual(r[1], ["RUSTBORO GYM: ROXANNE"])
        self.assertEqual([x[1] for x in r[3:]], ["Ann", "Ben"])
        self.assertEqual(r[3][0], "*1")  # Ann's own line
        status, out = await self.ask(a, "HREC ALL")
        r = rows(out)
        self.assertEqual(r[3], ["*RUSTBORO GYM", "Ann", "2"])
        self.assertEqual(r[4], ["*DEWFORD GYM", "Ben", "2"])
        self.assertEqual(r[11], ["*CHAMPION", "Ann", "1"])

    async def test_legend_catch_is_announced(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        await self.ask(a, f"OEV CATCH {SPECIES['GROUDON']} 0 0 0 0")
        await self.ask(a, f"OEV CATCH {SPECIES['ZIGZAGOON']} 0 0 0 0")
        status, out = await self.ask(b, "OSTAT")
        self.assertEqual(self.notes(out), ["Ann caught GROUDON! Nobody had caught one before!"])


class QuickMatchTest(HoennTest):
    async def test_two_trainers_are_matched(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        status, out = await self.ask(a, "QM JOIN 0")
        self.assertEqual(rows(out)[0][1], str(online_hoenn.WAITING))
        status, out = await self.ask(b, "QM JOIN 1")  # a trade: nobody yet
        self.assertEqual(rows(out)[0][1], str(online_hoenn.WAITING))
        status, out = await self.ask(b, "QM JOIN 0")
        first = rows(out)[0]
        self.assertEqual(first[1], str(online_hoenn.MATCHED))
        self.assertEqual(first[2], "Ann")
        status, out = await self.ask(b, "QM JOIN 0")  # Ann was taken off the list
        self.assertEqual(rows(out)[0][1], str(online_hoenn.WAITING))


class TeamUpTest(HoennTest):
    async def test_lend_get_and_give_back(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        status, out = await self.ask(a, "TEAMUP WHO")
        ben_id = rows(out)[1][1]
        await self.ask(a, f"TEAMUP ASK {ben_id}")
        status, out = await self.ask(b, "TEAMUP VIEW")
        self.assertEqual(rows(out)[0][1], str(online_hoenn.TEAM_INVITED))
        team = bytes(range(160))
        status, out = await self.ask(b, "TEAMUP YES", team)
        self.assertEqual(status, 0)
        status, out = await self.ask(a, "TEAMUP VIEW")
        self.assertEqual(rows(out)[0][1], str(online_hoenn.TEAM_READY))
        self.assertIn("Ben lent you their team! Open HUB > TEAM UP to bring it along.", self.notes(out))
        status, out = await self.ask(a, "TEAMUP GET")
        self.assertTrue(out.endswith(b"\0" + team))
        await self.ask(a, "OEV TEAMWIN")
        status, out = await self.ask(b, "TEAMUP VIEW")
        self.assertEqual(rows(out)[0][1], str(online_hoenn.TEAM_LENT))
        self.assertIn("Ann won a big battle with your team at their side!", self.notes(out))
        await self.ask(b, "TEAMUP END")
        status, out = await self.ask(a, "TEAMUP VIEW")
        self.assertEqual(rows(out)[0][1], str(online_hoenn.TEAM_NONE))


class SoloWatchTest(unittest.IsolatedAsyncioTestCase):
    async def test_watch_a_gym_battle(self):
        from test_netplay_server import Client
        league = ns.League(":memory:")
        server = ns.Server(league)
        srv = await asyncio.start_server(server.handle, "127.0.0.1", 0)
        port = srv.sockets[0].getsockname()[1]
        clients = []

        async def connect(name, tid):
            c = Client(*await asyncio.open_connection("127.0.0.1", port))
            clients.append(c)
            await c.send(f"HELLO {name} {tid}")
            welcome = await c.recv()
            return c, int(welcome.split()[1])

        try:
            a, a_id = await connect("Ann", 1)
            b, _ = await connect("Ben", 2)
            await a.send("SOLO TATE&LIZA")
            await b.send("LIVE")
            sid = ns.SOLO_SID_BASE + a_id
            self.assertEqual(await b.recv(), f"LIVE {sid} Ann TATE&LIZA")
            self.assertEqual(await b.recv(), "LIVEEND")
            await b.send(f"WATCH {sid}")
            self.assertEqual(await b.recv(), f"WATCHING {sid}")
            await a.send("BSTATE 0 1 2 3 4 5 6 7 8 9 10 11 12")
            self.assertEqual(await b.recv(), f"WSTATE {sid} 1 2 3 4 5 6 7 8 9 10 11 12")
            await a.send("BTEXT 0 Solrock used Psychic!")
            self.assertEqual(await b.recv(), f"WTEXT {sid} Solrock used Psychic!")
            await a.send("SOLO END")
            self.assertEqual(await b.recv(), f"WEND {sid}")
        finally:
            for c in clients:
                c.writer.close()
            srv.close()
            await srv.wait_closed()
            league.db.close()


if __name__ == "__main__":
    unittest.main()
