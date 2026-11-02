import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import alive  # noqa: E402
import ext  # noqa: E402
import test_ext  # noqa: E402


def rows(answer):
    return [r.split("|") for r in answer.decode().split("\n") if r]


class AliveTest(unittest.IsolatedAsyncioTestCase):
    connect = test_ext.ExtTest.connect
    ask = test_ext.ExtTest.ask

    async def asyncSetUp(self):
        await test_ext.ExtTest.asyncSetUp(self)
        self.t = 1_000_000.0
        alive.now = lambda: self.t

    async def asyncTearDown(self):
        await test_ext.ExtTest.asyncTearDown(self)

    async def test_projects(self):
        a = await self.connect("May", 7)
        status, answer = await self.ask(a, "APROJ INFO")
        self.assertEqual(status, ext.STATUS_OK)
        info = rows(answer)
        self.assertEqual(len(info), len(alive.PROJECTS))
        self.assertEqual(info[0][1], "0")

        goal = alive.PROJECTS[7][2]
        status, answer = await self.ask(a, "APROJ GIVE 7 50000")
        self.assertEqual(status, ext.STATUS_OK)
        self.assertEqual(rows(answer)[0][:3], ["50000", str(goal), "0"])
        # A quarter funded makes the news.
        status, answer = await self.ask(a, "ANEWS")
        self.assertIn("25% funded", answer.decode())

        while True:
            status, answer = await self.ask(a, "APROJ GIVE 7 100000")
            if rows(answer)[0][2] == "1":
                break
        info = rows((await self.ask(a, "APROJ INFO"))[1])
        self.assertEqual(info[7][3], "1")
        self.assertEqual(info[7][4], "May")
        self.assertIn("is finished", (await self.ask(a, "ANEWS"))[1].decode())
        # Built: no more gifts, and bad requests are refused.
        self.assertNotEqual((await self.ask(a, "APROJ GIVE 7 100"))[0], ext.STATUS_OK)
        self.assertNotEqual((await self.ask(a, "APROJ GIVE 99 100"))[0], ext.STATUS_OK)
        self.assertNotEqual((await self.ask(a, "APROJ GIVE 1 9999999"))[0], ext.STATUS_OK)

    async def test_deeds_make_the_news(self):
        a = await self.connect("Brendan", 3)
        self.assertEqual((await self.ask(a, "AEV 1 0 16"))[0], ext.STATUS_OK)
        self.t += 600
        status, answer = await self.ask(a, "ANEWS")
        news = rows(answer)
        self.assertEqual(news[0][0], "10")
        self.assertIn("Brendan walked a lost child home", news[0][1])

    def test_plain(self):
        self.assertEqual(alive.plain("POKéMON\nare  here"), "POKeMON are here")


if __name__ == "__main__":
    unittest.main()
