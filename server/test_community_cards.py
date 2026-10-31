import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import community_cards as cards  # noqa: E402
from test_community import CommunityCase  # noqa: E402


def card(tag):
    return bytes([tag]) * cards.CARD_SIZE


class CardsTest(CommunityCase):
    async def test_publish_list_and_view_cards(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        await self.ask(ash, cards.REQ_PUBLISH, data=card(0xA))
        await self.ask(may, cards.REQ_PUBLISH, data=card(0xB))
        rows, _, _, _ = await self.ask(ash, cards.REQ_LIST)
        self.assertEqual(rows, ["TRAINER CARDS", "YOUR CARD", "MAY  (online)"])

        rows, _, args, data = await self.ask(ash, cards.REQ_VIEW, (1,))
        self.assertEqual((rows, args, data), (["TRAINER CARD", "NOT RANKED YET"], [0, 1000, 0, 0], card(0xB)))
        await self.battle(ash, may)
        rows, _, args, data = await self.ask(ash, cards.REQ_VIEW, (0,))
        self.assertEqual(args[0], 1)
        self.assertEqual(args[2:], [1, 0])
        self.assertTrue(rows[1].endswith(" TIER"))
        self.assertEqual(data, card(0xA))

    async def test_bad_cards_are_refused(self):
        ash = await self.connect("ASH")
        _, status, _, _ = await self.ask(ash, cards.REQ_PUBLISH, data=b"short")
        self.assertEqual(status, community.OFFLINE)
        rows, status, _, _ = await self.ask(ash, cards.REQ_VIEW, (5,))
        self.assertEqual((status, rows[-1]), (community.OFFLINE, "no such card"))


if __name__ == "__main__":
    unittest.main()
