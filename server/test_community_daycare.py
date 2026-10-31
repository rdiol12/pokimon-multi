import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import community_daycare as daycare  # noqa: E402
from test_community import CommunityCase  # noqa: E402


def parent(tag, species, nickname):
    return bytes([tag]) * daycare.BOX_MON_SIZE + species.to_bytes(2, "little") + community.encode_name(nickname, 12)


class DaycareTest(CommunityCase):
    async def test_two_trainers_pair_up_and_both_get_an_egg(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        _, status, _, _ = await self.ask(ash, daycare.REQ_STATUS)
        self.assertEqual(status, daycare.NONE)
        rows, _, _, _ = await self.ask(ash, daycare.REQ_LIST)
        self.assertEqual(rows, ["PAIR UP WITH", "MAY  (online)"])
        rows, _, _, _ = await self.ask(ash, daycare.REQ_LEAVE, (0,), parent(0xA1, 254, "Sceptile"))
        self.assertEqual(rows[1], "Now MAY has to leave")
        rows, status, _, _ = await self.ask(ash, daycare.REQ_STATUS)
        self.assertEqual((status, rows[1]), (daycare.WAITING, "Your Sceptile is waiting"))

        await self.ask(may, daycare.REQ_LIST)
        await self.ask(may, daycare.REQ_LEAVE, (0,), parent(0xB2, 132, "Ditto"))
        rows, status, _, _ = await self.ask(ash, daycare.REQ_STATUS)
        self.assertEqual((status, rows[1:3]), (daycare.BREEDING, ["Your Sceptile and MAY's", "Ditto are together."]))
        _, status, _, _ = await self.ask(ash, daycare.REQ_EGG)
        self.assertEqual(status, daycare.BREEDING)

        _daycare = daycare._daycare(self.server)
        _daycare.clock = lambda: 1e12
        for me, mine, theirs in ((ash, 0xA1, 0xB2), (may, 0xB2, 0xA1)):
            _, status, _, _ = await self.ask(me, daycare.REQ_STATUS)
            self.assertEqual(status, daycare.EGG)
            _, status, _, data = await self.ask(me, daycare.REQ_EGG)
            self.assertEqual((status, data), (daycare.EGG, bytes([mine]) * 80 + bytes([theirs]) * 80))
            await self.ask(me, daycare.REQ_EGG, (1,))  # collected
            _, status, _, _ = await self.ask(me, daycare.REQ_EGG)
            self.assertEqual(status, daycare.BREEDING)

        # MAY takes hers back: ASH's waits for a new partner.
        await self.ask(may, daycare.REQ_TAKE)
        _, status, _, _ = await self.ask(may, daycare.REQ_STATUS)
        self.assertEqual(status, daycare.NONE)
        _, status, _, _ = await self.ask(ash, daycare.REQ_STATUS)
        self.assertEqual(status, daycare.WAITING)

    async def test_bad_requests_are_refused(self):
        ash = await self.connect("ASH")
        rows, status, _, _ = await self.ask(ash, daycare.REQ_LEAVE, (3,), parent(1, 1, "X"))
        self.assertEqual((status, rows[-1]), (community.OFFLINE, "no such trainer"))
        await self.connect("MAY", 2)
        await asyncio.sleep(0.01)
        await self.ask(ash, daycare.REQ_LIST)
        rows, status, _, _ = await self.ask(ash, daycare.REQ_LEAVE, (0,), b"short")
        self.assertEqual((status, rows[-1]), (community.OFFLINE, "bad Pokémon"))


if __name__ == "__main__":
    unittest.main()
