import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import life_auction  # noqa: E402
import test_ext  # noqa: E402
from test_ext import MON  # noqa: E402


class AuctionTest(unittest.IsolatedAsyncioTestCase):
    connect = test_ext.ExtTest.connect
    ask = test_ext.ExtTest.ask

    async def asyncSetUp(self):
        await test_ext.ExtTest.asyncSetUp(self)
        self.clock = 1_000_000.0
        self.real_now = life_auction.now
        life_auction.now = lambda: self.clock

    async def asyncTearDown(self):
        life_auction.now = self.real_now
        await test_ext.ExtTest.asyncTearDown(self)

    def rows(self, answer):
        return [r.split("|") for r in answer.decode().split("\n") if r]

    async def post(self, c, start=1000, hours=1, species=25):
        status, answer = await self.ask(c, f"AUCTION POST {species} 30 0 {start} {hours}", MON)
        self.assertEqual(status, 0, answer)
        return int(answer)

    async def test_bidding_and_winning(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        c = await self.connect("Brock", 3)
        aid = await self.post(a)
        status, answer = await self.ask(b, "AUCTION LIST")
        self.assertEqual(self.rows(answer), [[str(aid), "Ash", "25", "30", "1000", "0", "3600", "0"]])
        self.assertEqual((await self.ask(a, f"AUCTION BID {aid} 1000"))[0], 1)  # own auction
        self.assertEqual((await self.ask(b, f"AUCTION BID {aid} 900"))[0], 1)   # under the start
        self.assertEqual(await self.ask(b, f"AUCTION BID {aid} 1000"), (0, b"1000|3600"))
        self.assertEqual((await self.ask(b, f"AUCTION BID {aid} 2000"))[0], 1)  # already on top
        status, answer = await self.ask(c, f"AUCTION BID {aid} 1050")
        self.assertEqual(status, 1)
        self.assertIn(b"1100", answer)
        self.assertEqual((await self.ask(c, f"AUCTION BID {aid} 1100"))[0], 0)
        # Misty was outbid: her 1000 comes back.
        self.assertEqual(await self.ask(b, "AUCTION COLLECT"), (0, b"OUTBID|1000|25|Brock|1000"))
        self.assertEqual(await self.ask(b, "AUCTION COLLECT"), (0, b""))
        flags = self.rows((await self.ask(c, "AUCTION LIST"))[1])[0][7]
        self.assertEqual(int(flags) & life_auction.TOP, life_auction.TOP)
        # Time runs out: Brock gets the Pokémon, Ash gets 1100 less 5%.
        self.clock += 3601
        self.assertEqual((await self.ask(b, "AUCTION LIST"))[1], b"")
        self.assertEqual(await self.ask(c, "AUCTION COLLECT"), (0, b"WON|0|25|Ash|1100\0" + MON))
        self.assertEqual(await self.ask(a, "AUCTION COLLECT"), (0, b"SOLD|1045|25|Brock|1045"))
        self.assertEqual((await self.ask(c, f"AUCTION BID {aid} 5000"))[0], 1)

    async def test_late_bid_extends_the_end(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        aid = await self.post(a)
        self.clock += 3600 - 60
        self.assertEqual(await self.ask(b, f"AUCTION BID {aid} 1000"), (0, b"1000|300"))
        self.clock += 120
        self.assertEqual(len(self.rows((await self.ask(b, "AUCTION LIST"))[1])), 1)

    async def test_unsold_and_cancel(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        first = await self.post(a)
        second = await self.post(a, species=4)
        await self.post(a, species=7)
        self.assertEqual((await self.ask(a, "AUCTION POST 1 5 0 100 1", MON))[0], 1)  # 3 at once
        self.assertEqual((await self.ask(a, "AUCTION POST 1 5 0 100 2", MON))[0], 2)  # bad length
        self.assertEqual((await self.ask(b, f"AUCTION CANCEL {first}"))[0], 1)
        self.assertEqual(await self.ask(a, f"AUCTION CANCEL {first}"), (0, b""))
        self.assertEqual(await self.ask(a, "AUCTION COLLECT"), (0, b"CANCELED|0|25|-|0\0" + MON))
        await self.ask(b, f"AUCTION BID {second} 1000")
        self.assertEqual((await self.ask(a, f"AUCTION CANCEL {second}"))[0], 1)  # has a bid
        self.clock += 3601
        kinds = []
        while (answer := (await self.ask(a, "AUCTION COLLECT"))[1]):
            kinds.append(answer.split(b"|")[0])
        self.assertEqual(sorted(kinds), [b"SOLD", b"UNSOLD"])

    def test_min_bid(self):
        self.assertEqual(life_auction.min_bid(500, 0), 500)
        self.assertEqual(life_auction.min_bid(500, 500), 600)
        self.assertEqual(life_auction.min_bid(500, 10000), 10500)
        self.assertEqual(life_auction.min_bid(500, 10050), 10600)


if __name__ == "__main__":
    unittest.main()
