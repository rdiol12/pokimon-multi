import asyncio
import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
from test_netplay_server import Client  # noqa: E402

MON = bytes(range(100))


class ExtTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        ext.install(self.server)
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

    async def test_unknown_and_bad_requests(self):
        a = await self.connect("Ash", 1)
        self.assertEqual((await self.ask(a, "NOPE"))[0], ext.STATUS_BAD)
        self.assertEqual((await self.ask(a, "BOUNTY POST x y"))[0], ext.STATUS_BAD)

    async def test_guild_found_join_and_ladder(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        self.assertEqual(await self.ask(a, "GUILD CREATE ace Ace Team"), (0, b"ACE|Ace Team"))
        status, answer = await self.ask(b, "GUILD JOIN ACE")
        self.assertEqual(status, 0)
        info = (await self.ask(b, "GUILD INFO"))[1].decode().split("\n")
        self.assertEqual(info[0].split("|")[:4], ["ACE", "Ace Team", "Ash", "2"])
        status, answer = await self.ask(a, "GUILD CREATE BCD Other")
        self.assertEqual(status, ext.STATUS_REFUSED)
        ladder = (await self.ask(a, "GUILD LIST"))[1].decode()
        self.assertIn("|ACE|Ace Team|2|", ladder)

    async def test_wonder_trade_swaps(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        self.assertEqual(await self.ask(a, "WONDER SEND 25", MON), (0, b""))
        self.assertEqual(await self.ask(a, "WONDER CHECK"), (0, b"WAITING"))
        other = bytes(reversed(MON))
        status, answer = await self.ask(b, "WONDER SEND 120", other)
        self.assertEqual(answer, b"Ash|25\0" + MON)
        status, answer = await self.ask(a, "WONDER CHECK")
        self.assertEqual(answer, b"Misty|120\0" + other)
        self.assertEqual(await self.ask(a, "WONDER CHECK"), (0, b""))

    async def test_bounty_fill_and_collect(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        self.assertEqual((await self.ask(a, "BOUNTY POST 261 1500"))[0], 0)
        bid = (await self.ask(b, "BOUNTY LIST"))[1].decode().split("|")[0]
        self.assertEqual((await self.ask(b, f"BOUNTY FILL {bid} 25", MON))[0], ext.STATUS_REFUSED)
        self.assertEqual(await self.ask(b, f"BOUNTY FILL {bid} 261", MON), (0, b"1500"))
        self.assertEqual((await self.ask(a, f"BOUNTY CANCEL {bid}"))[0], ext.STATUS_REFUSED)
        self.assertEqual(await self.ask(a, "BOUNTY COLLECT"), (0, b"Misty|261\0" + MON))

    async def test_mail_with_money(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        status, _ = await self.ask(b, "MAIL SEND Ash 1 0 500 0 0")
        self.assertEqual(status, 0)
        self.assertEqual(await a.recv(), "XNOTE MAIL")
        mid = (await self.ask(a, "MAIL INBOX"))[1].decode().split("|")[0]
        self.assertEqual(await self.ask(a, f"MAIL OPEN {mid}"), (0, b"Misty|0|500|0|0"))
        self.assertEqual((await self.ask(a, f"MAIL OPEN {mid}"))[0], ext.STATUS_REFUSED)

    async def test_tower_needs_other_teams(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        await self.ask(a, "TOWER START", MON)
        self.assertEqual((await self.ask(a, "TOWER NEXT"))[0], ext.STATUS_REFUSED)
        await self.ask(b, "TOWER START", MON)
        status, answer = await self.ask(b, "TOWER NEXT")
        self.assertEqual(answer, b"1|Ash\0" + MON)
        await self.ask(b, "TOWER WON")
        self.assertEqual((await self.ask(b, "TOWER STATE"))[1], b"2|1")

    async def test_hall_of_fame_wall(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        team = struct.pack("<HBB", 25, 50, 1) + struct.pack("<HBB", 1573, 42, 0) + bytes(16)
        self.assertEqual(await self.ask(a, "HOF ADD", team), (0, b"1|1"))
        # The same team again is the same entry; a new one goes up.
        self.assertEqual(await self.ask(a, "HOF ADD", team), (0, b"1|1"))
        self.assertEqual(await self.ask(b, "HOF ADD", struct.pack("<HBB", 1, 9, 0) + bytes(20)), (0, b"2|2"))
        status, answer = await self.ask(a, "HOF LIST")
        rows = [r.split("|") for r in answer.decode().split("\n")]
        self.assertEqual(rows[0], ["2", "1"])
        self.assertEqual([r[:2] for r in rows[1:]], [["2", "Misty"], ["1", "Ash"]])
        self.assertEqual(rows[2][3:], ["25.50.1", "1573.42.0"])
        self.assertEqual((await self.ask(a, "HOF ADD", bytes(24)))[0], ext.STATUS_BAD)
        self.assertEqual((await self.ask(a, "HOF ADD", struct.pack("<HBB", 25, 0, 0) + bytes(20)))[0], ext.STATUS_BAD)
        self.assertEqual((await self.ask(a, "HOF ADD", b"x"))[0], ext.STATUS_BAD)


if __name__ == "__main__":
    unittest.main()
