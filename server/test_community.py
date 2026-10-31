import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import netplay_server as ns  # noqa: E402
from test_netplay_server import Client  # noqa: E402


class CommunityCase(unittest.IsolatedAsyncioTestCase):
    """A netplay server on a free port, with helpers for community requests.
    The feature tests (test_community_*.py) build on it."""

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

    async def connect(self, name, tid=1):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        c.id = int((await c.recv()).split()[1])
        await c.send("CHELLO")
        c.events = []
        return c

    async def until(self, c, prefix):
        """The next line starting with `prefix`; CEV lines on the way are kept in c.events."""
        while True:
            line = await c.recv()
            if line.startswith("CEV "):
                _, section, data = line.split()
                c.events.append((int(section), bytes.fromhex(data)))
            if line.startswith(prefix):
                return line

    async def ask(self, c, req_type, args=(0, 0, 0, 0), data=b"", party=None):
        """Sends a request; returns (rows as text, status, args, data). party:
        raw Pokémon to answer CPARTY with."""
        for i in range(0, len(data), 96):
            await c.send(f"CDATA {data[i:i + 96].hex()}")
        a = (list(args) + [0, 0, 0, 0])[:4]
        await c.send(f"C {req_type} {' '.join(map(str, a))}")
        rows = {}
        while True:
            line = await c.recv()
            parts = line.split()
            if parts[0] == "CPARTY":
                for i, mon in enumerate(party or []):
                    await c.send(f"CPARTYMON {parts[1]} {i} {mon.hex()}")
                await c.send(f"CPARTYEND {parts[1]} {len(party or [])}")
            elif parts[0] == "CEV":
                c.events.append((int(parts[1]), bytes.fromhex(parts[2])))
            elif parts[0] == "CROW":
                rows[int(parts[1])] = community.decode_text(bytes.fromhex(parts[2]))
            elif parts[0] == "CRESP":
                self.assertEqual(int(parts[1]), req_type)
                resp = b"" if parts[7] == "-" else bytes.fromhex(parts[7])
                return [rows[i] for i in sorted(rows)], int(parts[2]), [int(x) for x in parts[3:7]], resp

    async def battle(self, host, guest, host_says="win", guest_says="loss"):
        """An online battle between two clients, both reporting a result."""
        await host.send(f"REQ battle {guest.id}")
        sid = (await self.until(host, "PENDING")).split()[1]
        await self.until(guest, "INVITE")
        await guest.send(f"ACCEPT {sid}")
        await self.until(host, "START")
        await self.until(guest, "START")
        await host.send(f"RESULT {sid} {host_says}")
        await guest.send(f"RESULT {sid} {guest_says}")
        await asyncio.sleep(0.05)
        for c in (host, guest):
            await c.send(f"QUIT {sid}")
        await asyncio.sleep(0.05)


class FoundationTest(CommunityCase):
    def test_text_round_trip(self):
        for text in ("PETALBURG CUP", "Your Sceptile's egg!", "No. 1: 50%", "POKéMON"):
            self.assertEqual(community.decode_text(community.encode_text(text)), text)
        self.assertEqual(community.encode_name("MAY", 8), bytes([0xC7, 0xBB, 0xD3]) + b"\xff" * 5)

    async def test_unknown_request_says_so(self):
        c = await self.connect("ASH")
        rows, status, _, _ = await self.ask(c, 250)
        self.assertEqual(status, community.OFFLINE)
        self.assertIn("The server doesn't have it yet.", rows)

    async def test_a_refused_request_answers_with_the_reason(self):
        @community.request(249)
        def refuse(server, player, args, data):
            raise ValueError("not today")
        try:
            c = await self.connect("ASH")
            rows, status, _, _ = await self.ask(c, 249)
            self.assertEqual((rows[-1], status), ("not today", community.OFFLINE))
        finally:
            del community.HANDLERS[249]

    async def test_request_data_and_the_party_reach_the_handler(self):
        seen = {}

        @community.request(248)
        def later(server, player, args, data):
            seen["args"], seen["data"] = args, data

            def got(server, player, party):
                community.send_answer(player, 248, community.Answer("PARTY", [f"{len(party)} mons"], args=(len(party),)))
            server.ask_party(player, got)
            return None  # answered once the party arrives
        try:
            c = await self.connect("ASH")
            data = bytes(range(200))
            rows, _, args, _ = await self.ask(c, 248, (1, 2, 3, 4), data, party=[b"\x01" * 100, b"\x02" * 100])
            self.assertEqual((seen["args"], seen["data"]), ([1, 2, 3, 4], data))
            self.assertEqual((rows, args[0]), (["PARTY", "2 mons"], 2))
        finally:
            del community.HANDLERS[248]


if __name__ == "__main__":
    unittest.main()
