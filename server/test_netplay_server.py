import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import netplay_server as ns  # noqa: E402


class Client:
    def __init__(self, reader, writer):
        self.reader = reader
        self.writer = writer

    async def send(self, line):
        self.writer.write((line + "\n").encode())
        await self.writer.drain()

    async def recv(self):
        line = await asyncio.wait_for(self.reader.readline(), 2)
        return line.decode().strip()

    async def expect_silence(self):
        try:
            line = await asyncio.wait_for(self.reader.readline(), 0.1)
        except asyncio.TimeoutError:
            return
        raise AssertionError(f"unexpected message: {line!r}")


class ServerTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []

    async def asyncTearDown(self):
        for c in self.clients:
            c.writer.close()
        self.srv.close()
        await self.srv.wait_closed()

    async def connect(self, name, tid=1, map_key=(0, 9), xy=(5, 5)):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        welcome = await c.recv()
        self.assertTrue(welcome.startswith("WELCOME "))
        c.id = int(welcome.split()[1])
        if map_key is not None:
            await c.send(f"POS {map_key[0]} {map_key[1]} {xy[0]} {xy[1]} 1 0")
        return c

    async def test_presence_same_map_only(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        # B arriving on A's map: B sees A, A sees B.
        self.assertEqual(await b.recv(), f"PLAYER {a.id} Ash 0 9 5 5 1 0")
        self.assertEqual(await a.recv(), f"PLAYER {b.id} Misty 0 9 5 5 1 0")
        await b.send("POS 0 9 6 5 4 0")
        self.assertEqual(await a.recv(), f"PLAYER {b.id} Misty 0 9 6 5 4 0")
        # B leaves for another map: both get GONE for each other.
        await b.send("POS 1 2 3 4 1 0")
        self.assertEqual(await a.recv(), f"GONE {b.id}")
        self.assertEqual(await b.recv(), f"GONE {a.id}")
        await a.send("POS 0 9 7 5 1 0")
        await b.expect_silence()

    async def test_disconnect_broadcasts_gone(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.recv()
        await b.recv()
        b.writer.close()
        self.assertEqual(await a.recv(), f"GONE {b.id}")

    async def test_rejects_commands_before_hello(self):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send("POS 0 0 0 0 0 0")
        self.assertTrue((await c.recv()).startswith("ERR"))
        await c.send("HELLO bad-name 1")
        self.assertTrue((await c.recv()).startswith("ERR"))

    async def start_session(self, kind="battle"):
        a = await self.connect("Ash", 11)
        b = await self.connect("Misty", 22)
        await a.recv()
        await b.recv()
        await a.send(f"REQ {kind} {b.id}")
        invite = await b.recv()
        _, sid, k, frm = invite.split()
        self.assertEqual((k, int(frm)), (kind, a.id))
        await b.send(f"ACCEPT {sid}")
        self.assertEqual(await a.recv(), f"START {sid} {kind} {b.id} 0")
        self.assertEqual(await b.recv(), f"START {sid} {kind} {a.id} 1")
        return a, b, sid

    async def test_link_relay(self):
        a, b, sid = await self.start_session("trade")
        await a.send(f"LINK {sid} 00112233445566778899aabbccddeeff")
        self.assertEqual(await b.recv(), f"LINK {sid} 00112233445566778899aabbccddeeff")
        await b.send(f"LINK {sid} ff")
        self.assertEqual(await a.recv(), f"LINK {sid} ff")

    async def test_decline(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.recv()
        await b.recv()
        await a.send(f"REQ trade {b.id}")
        sid = (await b.recv()).split()[1]
        await b.send(f"DECLINE {sid}")
        self.assertEqual(await a.recv(), f"DECLINED {sid}")
        # Both are free again.
        await b.send(f"REQ battle {a.id}")
        self.assertTrue((await a.recv()).startswith("INVITE"))

    async def board(self, client):
        await client.send("BOARD")
        rows = []
        while (line := await client.recv()) != "BOARDEND":
            rows.append(line)
        return rows

    async def test_matching_results_update_leaderboard(self):
        a, b, sid = await self.start_session()
        await a.send(f"RESULT {sid} win")
        await b.send(f"RESULT {sid} loss")
        await a.send(f"QUIT {sid}")
        self.assertEqual(await b.recv(), f"ENDED {sid}")
        self.assertEqual(await self.board(b), [
            "RANK 1 Ash 1016 1 0 0",
            "RANK 2 Misty 984 0 1 0",
        ])

    async def test_disputed_results_do_not_count(self):
        a, b, sid = await self.start_session()
        await a.send(f"RESULT {sid} win")
        await b.send(f"RESULT {sid} win")
        await a.send(f"QUIT {sid}")
        await b.recv()
        self.assertEqual(await self.board(b), [])
        status = self.league.db.execute("SELECT status FROM matches").fetchone()[0]
        self.assertEqual(status, "disputed")

    async def test_rage_quit_counts_as_loss(self):
        a, b, sid = await self.start_session()
        await a.send(f"RESULT {sid} win")
        b.writer.close()
        self.assertEqual(await a.recv(), f"ENDED {sid}")
        self.assertEqual(await a.recv(), f"GONE {b.id}")
        self.assertEqual(await self.board(a), [
            "RANK 1 Ash 1016 1 0 0",
            "RANK 2 Misty 984 0 1 0",
        ])

    async def test_report_after_peer_quit_still_counts(self):
        a, b, sid = await self.start_session()
        await a.send(f"RESULT {sid} loss")
        await a.send(f"QUIT {sid}")
        self.assertEqual(await b.recv(), f"ENDED {sid}")
        self.assertEqual(await self.board(b), [])
        await b.send(f"RESULT {sid} win")
        self.assertEqual(await self.board(b), [
            "RANK 1 Misty 1016 1 0 0",
            "RANK 2 Ash 984 0 1 0",
        ])

    async def test_trades_do_not_touch_leaderboard(self):
        a, b, sid = await self.start_session("trade")
        await a.send(f"RESULT {sid} win")
        self.assertTrue((await a.recv()).startswith("ERR"))

    async def test_link_requires_started_session(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.recv()
        await b.recv()
        await a.send(f"REQ battle {b.id}")
        sid = (await b.recv()).split()[1]
        await a.send(f"LINK {sid} 00")
        self.assertTrue((await a.recv()).startswith("ERR"))


class LeagueTest(unittest.TestCase):
    def test_draw_between_equals_keeps_rating(self):
        league = ns.League(":memory:")
        self.assertEqual(league.record(("A", 1), "draw", ("B", 2), "draw"), "ok")
        self.assertEqual(
            [(n, r, w, l, d) for n, r, w, l, d in league.top()],
            [("A", 1000, 0, 0, 1), ("B", 1000, 0, 0, 1)],
        )


if __name__ == "__main__":
    unittest.main()
