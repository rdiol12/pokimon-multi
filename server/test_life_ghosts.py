import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import life_ghosts  # noqa: E402
import test_ext  # noqa: E402


def mon(species, level=50):
    data = bytearray(life_ghosts.MON_SIZE)
    data[0:2] = species.to_bytes(2, "little")
    data[14] = level
    return bytes(data)


class GhostTest(unittest.IsolatedAsyncioTestCase):
    connect = test_ext.ExtTest.connect
    ask = test_ext.ExtTest.ask

    async def asyncSetUp(self):
        await test_ext.ExtTest.asyncSetUp(self)

    async def asyncTearDown(self):
        await test_ext.ExtTest.asyncTearDown(self)

    async def test_meet_and_battle_a_ghost(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        self.assertEqual(await self.ask(a, "GHOST GET"), (0, b""))  # nobody yet
        team = mon(25) + mon(6, 60)
        self.assertEqual(await self.ask(a, "GHOST PUT 0 2", team), (0, b"0|0|0|0"))
        self.assertEqual(await self.ask(a, "GHOST GET"), (0, b""))  # not your own
        status, answer = await self.ask(b, "GHOST GET")
        self.assertEqual(status, 0)
        head, party = answer.split(b"\0", 1)
        gid, name, gender, wins, losses, count = head.decode().split("|")
        self.assertEqual((name, gender, wins, losses, count), ("Ash", "0", "0", "0", "2"))
        self.assertEqual(party, team)
        self.assertEqual(await self.ask(b, f"GHOST RESULT {gid} 0"), (0, b""))  # Misty lost
        self.assertEqual(await self.ask(b, f"GHOST RESULT {gid} 1"), (0, b""))  # then won
        self.assertEqual((await self.ask(a, f"GHOST RESULT {gid} 1"))[0], 1)    # own ghost
        # Ash hears about it on the next upload; then it starts over.
        self.assertEqual(await self.ask(a, "GHOST PUT 0 1", mon(25)), (0, b"1|1|1|1"))
        self.assertEqual(await self.ask(a, "GHOST PUT 0 1", mon(25)), (0, b"0|0|1|1"))

    async def test_bad_teams(self):
        a = await self.connect("Ash", 1)
        self.assertEqual((await self.ask(a, "GHOST PUT 0 2", mon(25)))[0], 2)        # short
        self.assertEqual((await self.ask(a, "GHOST PUT 0 1", mon(0)))[0], 2)         # no species
        self.assertEqual((await self.ask(a, "GHOST PUT 0 1", mon(25, 0)))[0], 2)     # level 0
        self.assertEqual((await self.ask(a, "GHOST PUT 0 7", mon(25) * 7))[0], 2)    # too many
        self.assertEqual((await self.ask(a, "GHOST RESULT 99 1"))[0], 1)

    async def test_old_ghosts_fade(self):
        a = await self.connect("Ash", 1)
        b = await self.connect("Misty", 2)
        await self.ask(a, "GHOST PUT 1 1", mon(25))
        real = life_ghosts.now
        life_ghosts.now = lambda: real() + 31 * 86400
        try:
            self.assertEqual(await self.ask(b, "GHOST GET"), (0, b""))
        finally:
            life_ghosts.now = real


if __name__ == "__main__":
    unittest.main()
