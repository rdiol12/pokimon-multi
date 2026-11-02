import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import shared_map  # noqa: E402
import test_netplay_server  # noqa: E402

_Base = test_netplay_server.ServerTest


class ServerTest(unittest.IsolatedAsyncioTestCase):
    """The netplay server tests' set-up, without their tests."""
    asyncSetUp = _Base.asyncSetUp
    asyncTearDown = _Base.asyncTearDown
    connect = _Base.connect


del _Base

ABSOL = 359
GFX = (1 << 14) | ABSOL
SHINY_GFX = GFX | (1 << 13)


class Announcer:
    def __init__(self):
        self.said = []

    def announce(self, message, exclude=None):
        self.said.append((message, exclude))

    def notify(self, key, message):
        self.said.append((message, key))


class SharedSpawnTest(ServerTest):
    async def pair(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await b.recv()  # PLAYER a
        await a.recv()  # PLAYER b
        return a, b

    async def test_spawns_reach_others_on_the_map_and_newcomers(self):
        a, b = await self.pair()
        await a.send(f"WSPAWN 7 {GFX} 12 0 10 11 1")
        self.assertEqual(await b.recv(), f"WILD {a.id} 7 {GFX} 12 0 10 11 1 0 9 0 -")
        await a.expect_silence()
        # It walks: the same line, new tile.
        await a.send(f"WSPAWN 7 {GFX} 12 2 11 11 4")
        self.assertEqual(await b.recv(), f"WILD {a.id} 7 {GFX} 12 2 11 11 4 0 9 0 -")
        # Someone arriving sees it at once.
        c = await self.connect("Brock")
        lines = {await c.recv(), await c.recv(), await c.recv()}
        self.assertIn(f"WILD {a.id} 7 {GFX} 12 2 11 11 4 0 9 0 -", lines)
        # Gone from the owner's game: gone for everyone.
        await a.recv(); await b.recv()  # PLAYER c
        await a.send("WGONE 7")
        self.assertEqual(await b.recv(), f"WILDGONE {a.id} 7")
        self.assertEqual(await c.recv(), f"WILDGONE {a.id} 7")

    async def test_owner_leaving_takes_their_spawns(self):
        a, b = await self.pair()
        await a.send(f"WSPAWN 1 {GFX} 5 0 3 3 1")
        await b.recv()
        await a.send("POS 1 2 3 4 1 0 100")
        self.assertEqual(await b.recv(), f"WILDGONE {a.id} 1")
        self.assertEqual(await b.recv(), f"GONE {a.id}")
        self.assertEqual(self.server.spawns, {})

    async def test_first_claim_wins_and_the_rest_watch(self):
        a, b = await self.pair()
        c = await self.connect("Brock")
        for x in (a, b):
            await x.recv()  # PLAYER c
        await c.recv(); await c.recv()
        await a.send(f"WSPAWN 3 {GFX} 9 0 4 4 1")
        await b.recv(); await c.recv()
        await b.send(f"WCLAIM {a.id} 3")
        self.assertEqual(await b.recv(), f"WCLAIMED {a.id} 3 ok Misty")
        # The owner and everyone else see it being battled.
        self.assertEqual(await a.recv(), f"WILD {a.id} 3 {GFX} 9 0 4 4 1 0 9 1 Misty")
        self.assertEqual(await c.recv(), f"WILD {a.id} 3 {GFX} 9 0 4 4 1 0 9 1 Misty")
        await c.send(f"WCLAIM {a.id} 3")
        self.assertEqual(await c.recv(), f"WCLAIMED {a.id} 3 taken Misty")
        await a.send(f"WCLAIM {a.id} 3")
        self.assertEqual(await a.recv(), f"WCLAIMED {a.id} 3 taken Misty")
        # Misty ran: it's free again.
        await b.send(f"WEND {a.id} 3 3")
        self.assertEqual(await a.recv(), f"WILD {a.id} 3 {GFX} 9 0 4 4 1 0 9 0 -")
        self.assertEqual(await c.recv(), f"WILD {a.id} 3 {GFX} 9 0 4 4 1 0 9 0 -")
        # Brock catches it: gone, the owner's game removes it.
        await c.send(f"WCLAIM {a.id} 3")
        self.assertEqual(await c.recv(), f"WCLAIMED {a.id} 3 ok Brock")
        await a.recv(); await b.recv()
        await c.send(f"WEND {a.id} 3 1")
        self.assertEqual(await a.recv(), f"WILDGONE {a.id} 3")
        self.assertEqual(await b.recv(), f"WILDGONE {a.id} 3")
        await c.send(f"WCLAIM {a.id} 3")
        self.assertEqual(await c.recv(), f"WCLAIMED {a.id} 3 gone -")

    async def test_claim_is_freed_when_the_claimer_leaves(self):
        a, b = await self.pair()
        await a.send(f"WSPAWN 3 {GFX} 9 0 4 4 1")
        await b.recv()
        await b.send(f"WCLAIM {a.id} 3")
        await b.recv(); await a.recv()
        b.writer.close()
        self.assertEqual(await a.recv(), f"WILD {a.id} 3 {GFX} 9 0 4 4 1 0 9 0 -")
        self.assertEqual(await a.recv(), f"GONE {b.id}")

    async def test_rare_spawns_are_announced(self):
        self.server.online = Announcer()
        a, b = await self.pair()
        await self.connect("Brock", map_key=(1, 2))
        await a.send(f"WSPAWN 2 {SHINY_GFX} 30 0 4 4 1")
        self.assertTrue((await b.recv()).startswith(f"WILD {a.id} 2 "))
        # Misty is right there: a toast. Brock is elsewhere: a notice.
        self.assertEqual(await b.recv(), f"WRARE {a.id} 2")
        self.assertEqual(len(self.server.online.said), 1)
        message, key = self.server.online.said[0]
        self.assertIn("shiny", message)
        self.assertIn("near Ash", message)
        self.assertEqual(key, ("Brock", 1))
        # A plain one isn't.
        await a.send(f"WSPAWN 4 {GFX} 30 0 6 4 1")
        await b.recv()
        await b.expect_silence()
        self.assertEqual(len(self.server.online.said), 1)
        # Catching the shiny is news too.
        await b.send(f"WCLAIM {a.id} 2")
        await b.recv(); await a.recv()
        await b.send(f"WEND {a.id} 2 1")
        await a.recv()
        self.assertIn("Misty caught the shiny", self.server.online.said[-1][0])

    async def test_bad_spawns_are_refused(self):
        a = await self.connect("Ash")
        await a.send("WSPAWN 0 16384 5 0 1 1 1")
        self.assertTrue((await a.recv()).startswith("ERR"))
        await a.send("WSPAWN 1 100 5 0 1 1 1")  # not a Pokémon
        self.assertTrue((await a.recv()).startswith("ERR"))


class ChannelTest(ServerTest):
    def setUp(self):
        self.size = shared_map.CHANNEL_SIZE
        shared_map.CHANNEL_SIZE = 2

    def tearDown(self):
        shared_map.CHANNEL_SIZE = self.size

    async def test_a_full_map_opens_a_new_channel(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.recv(); await b.recv()
        c = await self.connect("Brock")
        # Channel 1 holds two: Brock is alone in channel 2.
        self.assertEqual(await c.recv(), "CHANNEL 2 2")
        await a.expect_silence()
        await b.expect_silence()
        await c.send("CHAN")
        self.assertEqual(await c.recv(), "CHANROW 1 2 0 -")
        self.assertEqual(await c.recv(), "CHANROW 2 1 0 -")
        self.assertEqual(await c.recv(), "CHANEND 2 2")
        # Channel 1 is full.
        await c.send("CHAN JOIN 1")
        self.assertEqual(await c.recv(), "CHANFULL 1")
        # Spawns stay in their channel.
        await c.send(f"WSPAWN 1 {GFX} 5 0 3 3 1")
        await a.expect_silence()
        # Misty leaves; Brock can join Ash.
        await b.send("POS 1 2 3 4 1 0 100")
        await a.recv(); await b.recv()
        await c.send("CHAN JOIN 1")
        # Brock's spawn comes along to channel 1.
        self.assertEqual(await a.recv(), f"PLAYER {c.id} Brock 0 9 5 5 1 0 100")
        self.assertEqual(await a.recv(), f"WILD {c.id} 1 {GFX} 5 0 3 3 1 0 9 0 -")
        self.assertEqual(await c.recv(), f"PLAYER {a.id} Ash 0 9 5 5 1 0 100")
        self.assertEqual(await c.recv(), "CHANNEL 1 1")

    async def test_you_go_where_your_friends_are(self):
        a = await self.connect("Ash")
        b = await self.connect("Misty")
        await a.recv(); await b.recv()
        c = await self.connect("Brock")
        self.assertEqual(await c.recv(), "CHANNEL 2 2")
        # Gary is Brock's friend: he goes to channel 2 though channel 1 ...
        await a.send("POS 1 2 3 4 1 0 100")  # ... has room again
        await b.recv(); await a.recv()
        self.server.friends.add(("Gary", 1), ("Brock", 1))
        d = await self.connect("Gary")
        self.assertEqual(await d.recv(), f"PLAYER {c.id} Brock 0 9 5 5 1 0 100")
        self.assertEqual(await d.recv(), "CHANNEL 2 2")
        await d.send("CHAN")
        self.assertEqual(await d.recv(), "CHANROW 1 1 0 -")
        self.assertEqual(await d.recv(), "CHANROW 2 2 1 Brock")


if __name__ == "__main__":
    unittest.main()
