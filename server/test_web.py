import asyncio
import json
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import netplay_server as ns  # noqa: E402
from web import Web  # noqa: E402


class WebTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.web = await asyncio.start_server(Web(self.server, "", 7777).handle, "127.0.0.1", 0)
        self.port = self.web.sockets[0].getsockname()[1]

    async def asyncTearDown(self):
        self.web.close()
        await self.web.wait_closed()
        self.league.db.close()

    async def get(self, path, host="play.example.org:8080"):
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        writer.write(f"GET {path} HTTP/1.1\r\nHost: {host}\r\n\r\n".encode())
        await writer.drain()
        data = await asyncio.wait_for(reader.read(), 2)
        writer.close()
        head, _, body = data.partition(b"\r\n\r\n")
        return int(head.split()[1]), body.decode()

    async def test_leaderboard_page_and_json(self):
        self.league.record(("Ash", 1), "win", ("Misty", 2), "loss")
        status, body = await self.get("/")
        self.assertEqual(status, 200)
        self.assertIn("Ash", body)
        self.assertIn("play.example.org:7777", body)
        status, body = await self.get("/leaderboard.json")
        self.assertEqual(json.loads(body)[0], {"rank": 1, "name": "Ash", "rating": 1016,
                                               "wins": 1, "losses": 0, "draws": 0})

    async def test_bridge_download_points_at_this_server(self):
        status, body = await self.get("/netplay_bridge.lua")
        self.assertEqual(status, 200)
        self.assertTrue(body.startswith(
            'NETPLAY_HOST = NETPLAY_HOST or "play.example.org"\nNETPLAY_PORT = NETPLAY_PORT or 7777\n'))
        self.assertIn("function Bridge:onFrame", body)

    async def test_health_and_404(self):
        status, body = await self.get("/health")
        self.assertEqual((status, json.loads(body)), (200, {"ok": True, "online": 0}))
        status, _ = await self.get("/nope")
        self.assertEqual(status, 404)


if __name__ == "__main__":
    unittest.main()
