import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import netplay_server as ns  # noqa: E402
import notify  # noqa: E402
from test_netplay_server import Client  # noqa: E402


class NotifyTest(unittest.IsolatedAsyncioTestCase):
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
        self.league.db.close()

    async def connect(self, name, tid=1, phone=False):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        c.id = int((await c.recv()).split()[1])
        await c.send("POS 0 9 5 5 1 0 100")
        if phone:
            await c.send("NOTIFY ON")
            line = await self.recv_kind(c, "NOTIFYKEY")
            _, n, t, c.token, last = line.split()
            self.assertEqual((n, int(t)), (name, tid))
            c.last = int(last)
        return c

    async def recv_kind(self, c, kind):
        while True:
            line = await c.recv()
            if line.split()[0] == kind:
                return line

    async def poll(self, name, tid, token, since=0):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"NOTIFYPOLL {name} {tid} {token} {since}")
        lines = []
        while True:
            line = await c.recv()
            lines.append(line)
            if line.startswith("NOTEEND"):
                return lines

    async def test_trade_offer_reaches_the_phone_live_and_by_poll(self):
        misty = await self.connect("Misty", 2, phone=True)
        ash = await self.connect("Ash")
        await ash.send(f"REQ trade {misty.id}")
        note = await self.recv_kind(misty, "NOTE")
        note_id, kind, text = note.split(" ", 3)[1:]
        self.assertEqual(kind, "trade")
        self.assertEqual(text, "Ash wants to trade with you!")
        lines = await self.poll("Misty", 2, misty.token)
        self.assertEqual(lines, [f"NOTE {note_id} trade Ash wants to trade with you!", f"NOTEEND {note_id}"])
        # Nothing new since then
        self.assertEqual(await self.poll("Misty", 2, misty.token, note_id), [f"NOTEEND {note_id}"])

    async def test_deal_offer_text(self):
        misty = await self.connect("Misty", 2, phone=True)
        ash = await self.connect("Ash")
        await ash.send(f"REQ deal {misty.id}")
        self.assertTrue((await self.recv_kind(misty, "NOTE")).endswith("Ash offers you a trade deal!"))

    async def test_battle_requests_are_not_notes(self):
        misty = await self.connect("Misty", 2, phone=True)
        ash = await self.connect("Ash")
        await ash.send(f"REQ battle {misty.id}")
        self.assertTrue((await self.recv_kind(misty, "INVITE")).startswith("INVITE"))
        self.assertEqual(await self.poll("Misty", 2, misty.token), ["NOTEEND 0"])

    async def test_wrong_token_gets_nothing(self):
        misty = await self.connect("Misty", 2, phone=True)
        ash = await self.connect("Ash")
        await ash.send(f"REQ trade {misty.id}")
        await self.recv_kind(misty, "NOTE")
        self.assertEqual(await self.poll("Misty", 2, "nope"), ["NOTEEND -1"])

    async def test_known_token_is_kept(self):
        misty = await self.connect("Misty", 2, phone=True)
        await misty.send(f"NOTIFY ON {misty.token}")
        self.assertEqual((await self.recv_kind(misty, "NOTIFYKEY")).split()[3], misty.token)

    async def test_friend_online_once_per_quiet_period(self):
        misty = await self.connect("Misty", 2, phone=True)
        ash = await self.connect("Ash")
        await misty.send(f"FRIEND ADD {ash.id}")
        await self.recv_kind(misty, "FRIENDADD")
        ash.writer.close()
        await asyncio.sleep(0.05)
        await self.connect("Ash")
        self.assertTrue((await self.recv_kind(misty, "NOTE")).endswith("friend Your friend Ash is online!"))
        await self.connect("Ash")  # back again right away: no second note
        await asyncio.sleep(0.05)
        rows = self.league.db.execute("SELECT COUNT(*) FROM notify_notes WHERE kind = 'friend'").fetchone()
        self.assertEqual(rows[0], 1)

    async def test_notes_only_for_players_with_a_phone(self):
        misty = await self.connect("Misty", 2)
        ash = await self.connect("Ash")
        await ash.send(f"REQ trade {misty.id}")
        await self.recv_kind(misty, "INVITE")
        self.assertEqual(self.league.db.execute("SELECT COUNT(*) FROM notify_notes").fetchone()[0], 0)

    async def test_tournament_start(self):
        import community_tournament as ct

        misty = await self.connect("Misty", 2, phone=True)
        tours = ct._tours(self.server)
        old = ct.SIGNUP_SECONDS
        ct.SIGNUP_SECONDS = 0
        try:
            tours.join(("Ash", 1))
            tours.join(("Misty", 2))
        finally:
            ct.SIGNUP_SECONDS = old
        note = await self.recv_kind(misty, "NOTE")
        self.assertIn(" tour The ", note)
        self.assertTrue(note.endswith("Your first battle is against Ash."), note)

    async def test_fishing_tournament_once(self):
        import online_fishing

        await self.connect("Misty", 2, phone=True)
        n = notify.notes(self.server)
        start = online_fishing.START
        online_fishing.START = n.clock() - 60
        try:
            notify.check_fishing(self.server)
            notify.check_fishing(self.server)
        finally:
            online_fishing.START = start
        rows = self.league.db.execute("SELECT kind FROM notify_notes").fetchall()
        self.assertEqual(rows, [("fishing",)])

    def test_old_notes_are_dropped(self):
        n = notify.Notes(self.league.db)
        token = n.register(("Ash", 1))
        for i in range(notify.NOTE_KEEP + 5):
            n.add(("Ash", 1), "trade", f"note {i}")
        rows = n.since(("Ash", 1), 0)
        self.assertEqual(len(rows), notify.NOTE_KEEP)
        self.assertEqual(rows[-1][2], f"note {notify.NOTE_KEEP + 4}")
        self.assertTrue(n.known(("Ash", 1), token))


if __name__ == "__main__":
    unittest.main()
