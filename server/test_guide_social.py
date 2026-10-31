import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import asyncio  # noqa: E402
import ext  # noqa: E402
import guide_social  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_mentors  # noqa: E402
from online_ids import MAPS  # noqa: E402
from test_online import OnlineTest, rows  # noqa: E402


def map_of(name):
    return MAPS[name]


class GuideSocialTest(OnlineTest):
    async def asyncSetUp(self):
        self.now = 1_000 * online.WEEK + 100
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server),
                                    [online_mentors, guide_social], clock=lambda: self.now)
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []
        self.seq = 0

    async def at(self, c, name):
        g, n = map_of(name)
        await c.send(f"POS {g} {n} 5 5 0 0 0")
        await asyncio.sleep(0.02)

    async def notices(self, c):
        status, out = await self.ask(c, "OSTAT")
        return [r[1] for r in rows(out) if r[0] == "N"]

    async def test_invite_reaches_friends_online_and_later(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.at(a, "LITTLEROOT_TOWN")
        await self.at(b, "OLDALE_TOWN")
        self.assertEqual((await self.ask(a, "GUIDE INVITE"))[0], 1)  # no friends yet
        self.server.friends.add(("Ann", 1), ("Ben", 2))
        self.server.friends.add(("Cat", 3), ("Ann", 1))  # Cat follows Ann; Cat is offline
        status, out = await self.ask(a, "GUIDE INVITE")
        self.assertEqual(status, 0)
        self.assertEqual(rows(out)[0][0].replace("~", " "), "You invited 2 friends. Online now: Ben.")
        self.assertTrue(any("Ann is online at LITTLEROOT TOWN" in n.replace("~", " ") for n in await self.notices(b)))
        c = await self.connect("Cat", 3)
        self.assertTrue(any("invited you" in n.replace("~", " ") for n in await self.notices(c)))

    async def test_nearby_sorted_and_add_friend(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        c = await self.connect("Cid", 3)
        await self.at(a, "LITTLEROOT_TOWN")
        await self.at(b, "LILYCOVE_CITY")
        await self.at(c, "OLDALE_TOWN")
        status, out = await self.ask(a, "GUIDE NEARBY")
        r = rows(out)
        self.assertEqual(r[0][1], "2")
        self.assertIn("Cid", r[0][0])
        self.assertTrue(r[1][0].startswith("Cid: OLDALE TOWN"))
        self.assertTrue(r[2][0].startswith("Ben: LILYCOVE CITY"))
        status, out = await self.ask(a, f"GUIDE FRIEND {r[1][1]}")
        self.assertEqual(status, 0)
        self.assertIn(("Cid", 3), [tuple(f) for f in self.server.friends.list(("Ann", 1))])
        self.assertTrue(any("Ann added you" in n.replace("~", " ") for n in await self.notices(c)))

    async def test_nearby_notice_when_someone_comes_close(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.at(a, "LITTLEROOT_TOWN")
        await self.at(b, "LILYCOVE_CITY")
        self.assertEqual(await self.notices(a), [])  # first look
        await self.at(b, "ROUTE101")
        self.assertTrue(any("Ben is nearby" in n.replace("~", " ") for n in await self.notices(a)))
        self.assertEqual([n for n in await self.notices(a) if "nearby" in n], [])  # told once

    async def test_new_trainer_gets_a_mentor_until_they_say_goodbye(self):
        m = await self.connect("Max", 1)
        n = await self.connect("Nia", 2)
        await self.ask(m, "OEV BADGES 7")
        await self.notices(m)
        await self.ask(m, "MENTOR VOLUNTEER 7")
        await self.ask(n, "OEV BADGES 1")
        said = await self.notices(n)
        self.assertTrue(any("Meet your mentor, Max" in s.replace("~", " ") for s in said))
        self.assertEqual(rows((await self.ask(n, "MENTOR VIEW 1"))[1])[0][2], "Max")
        self.assertTrue(any("Nia is new here" in s.replace("~", " ") for s in await self.notices(m)))
        await self.ask(n, "MENTOR LEAVE 1")
        await self.notices(n)
        await self.notices(n)
        self.assertEqual(rows((await self.ask(n, "MENTOR VIEW 1"))[1])[0][1], "0")  # not paired again

    async def test_six_badges_told_they_can_mentor_once(self):
        a = await self.connect("Ann", 1)
        await self.ask(a, "OEV BADGES 6")
        self.assertTrue(any("you can mentor" in s.replace("~", " ") for s in await self.notices(a)))
        self.assertEqual([s for s in await self.notices(a) if "mentor" in s], [])


if __name__ == "__main__":
    unittest.main()
