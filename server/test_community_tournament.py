import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community_tournament as tour  # noqa: E402
from test_community import CommunityCase  # noqa: E402


class TournamentTest(CommunityCase):
    async def asyncSetUp(self):
        await super().asyncSetUp()
        self.size, tour.SIZE = tour.SIZE, 4

    async def asyncTearDown(self):
        tour.SIZE = self.size
        await super().asyncTearDown()

    async def test_a_full_cup_plays_out_to_a_champion(self):
        players = [await self.connect(name, 10 + i) for i, name in enumerate(("ASH", "MAY", "RON", "SUN"))]
        rows, status, _, _ = await self.ask(players[0], tour.REQ_STATUS)
        self.assertEqual((status, rows[1]), (tour.NOT_IN, "Sign-ups are open. 0/4"))
        for c in players:
            _, status, _, _ = await self.ask(c, tour.REQ_JOIN)
            self.assertEqual(status, tour.SIGNED_UP)
        # Full: it started, and everyone heard who they face.
        rows, _, _, _ = await self.ask(players[0], tour.REQ_BRACKET)
        self.assertEqual(rows, ["PETALBURG CUP BRACKET", "- SEMIFINALS -", "ASH     vs  SUN", "MAY     vs  RON"])
        tour_events = [e for e in players[0].events if e[0] == 2]
        self.assertTrue(tour_events)
        self.assertEqual(tour_events[-1][1][:3], bytes([1, 1, 0xBB + ord("S") - 65]))

        rows, status, _, _ = await self.ask(players[0], tour.REQ_STATUS)
        self.assertEqual((status, rows[1:3]), (tour.PLAYING, ["You're in the SEMIFINALS!", "Your opponent: SUN."]))

        # A battle with someone else doesn't count; the cup matches do.
        await self.battle(players[0], players[1])
        rows, _, _, _ = await self.ask(players[0], tour.REQ_BRACKET)
        self.assertNotIn("▶", "".join(rows))
        await self.battle(players[3], players[0], "loss", "win")
        await self.battle(players[1], players[2], "win", "loss")
        rows, _, _, _ = await self.ask(players[0], tour.REQ_BRACKET)
        self.assertEqual(rows[2:], ["▶ASH     vs  SUN", "▶MAY     vs  RON", "- FINAL -", "ASH     vs  MAY"])
        rows, status, _, _ = await self.ask(players[3], tour.REQ_STATUS)
        self.assertEqual((status, rows[1]), (tour.NOT_IN, "You're out of PETALBURG CUP."))

        await self.battle(players[1], players[0], "loss", "win")
        rows, status, args, _ = await self.ask(players[0], tour.REQ_STATUS)
        self.assertEqual((status, args[1:3]), (tour.PRIZE, [1, tour.ITEM_MASTER_BALL]))
        _, status, args, _ = await self.ask(players[1], tour.REQ_STATUS)
        self.assertEqual((status, args[1:3]), (tour.PRIZE, [2, tour.ITEM_GOLD_BOTTLE_CAP]))
        # Claimed once; the next cup is open.
        rows, status, _, _ = await self.ask(players[0], tour.REQ_STATUS)
        self.assertEqual((status, rows[0], rows[3]), (tour.NOT_IN, "SLATEPORT CUP", "Last champion: ASH."))

        status, _, page = tour.page(self.server)
        self.assertEqual(status, 200)
        self.assertIn('<div class="p win">ASH</div>', page)
        self.assertIn("Champion: <b>ASH</b>", page)

    async def test_byes_and_withdrawing(self):
        a, b, c = [await self.connect(name, 20 + i) for i, name in enumerate(("ASH", "MAY", "RON"))]
        for x in (a, b, c):
            await self.ask(x, tour.REQ_JOIN)
        _, status, _, _ = await self.ask(c, tour.REQ_LEAVE)
        self.assertEqual(status, tour.NOT_IN)
        rows, status, args, _ = await self.ask(a, tour.REQ_STATUS)
        self.assertEqual((status, args[1]), (tour.SIGNED_UP, 2))
        # Sign-ups ran long enough: it starts with three, and the top seed gets a bye.
        await self.ask(c, tour.REQ_JOIN)
        tours = tour._tours(self.server)
        tours.clock = lambda: 1e12
        rows, status, _, _ = await self.ask(a, tour.REQ_STATUS)
        rows, _, _, _ = await self.ask(a, tour.REQ_BRACKET)
        self.assertIn("ASH     bye", rows)
        self.assertIn("MAY     vs  RON", rows)


if __name__ == "__main__":
    unittest.main()
