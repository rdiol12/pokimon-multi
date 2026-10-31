import os
import random
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import community_contest as contest  # noqa: E402
from test_community import CommunityCase  # noqa: E402


def entry(species, condition, sheen, appeal, nickname):
    return (species.to_bytes(2, "little") + bytes([condition, sheen, 50, appeal])
            + community.encode_name(nickname, 12))


class ContestTest(CommunityCase):
    def test_judging_favours_condition_sheen_and_moves(self):
        rng = random.Random(1)
        strong = {"condition": 255, "sheen": 255, "appeal": 64}
        weak = {"condition": 0, "sheen": 0, "appeal": 0}
        wins = sum(contest.judge([dict(strong), dict(weak)], rng)[0]["condition"] == 255 for _ in range(50))
        self.assertEqual(wins, 50)

    async def test_entries_face_other_players_then_the_regulars(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        rows, status, args, data = await self.ask(ash, contest.REQ_ENTER, (2,), entry(300, 255, 200, 40, "Skitty"))
        self.assertEqual(rows, ["CUTE CONTEST RESULTS"])
        self.assertEqual((status, args[0], args[2]), (contest.WON, 1, 4))
        first = data[:26]
        self.assertEqual((community.decode_text(first[:8]), community.decode_text(first[8:20]), first[25]), ("ASH", "Skitty", 1))

        # MAY now faces ASH's entry, plus two regulars.
        _, status, args, data = await self.ask(may, contest.REQ_ENTER, (2,), entry(184, 0, 0, 0, "Marill"))
        self.assertEqual((status, args[0]), (contest.PLACED, 4))
        names = [community.decode_text(data[i * 26:i * 26 + 8]) for i in range(4)]
        self.assertEqual(names[0], "ASH")
        rows, _, _, _ = await self.ask(may, contest.REQ_BOARD, (2,))
        self.assertEqual(rows[1].split()[1:], ["ASH's", "Skitty"])

    async def test_bad_entries_are_refused(self):
        ash = await self.connect("ASH")
        _, status, _, _ = await self.ask(ash, contest.REQ_ENTER, (9,), entry(1, 1, 1, 1, "X"))
        self.assertEqual(status, community.OFFLINE)


if __name__ == "__main__":
    unittest.main()
