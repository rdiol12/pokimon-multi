import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community_fusion as fusion  # noqa: E402
import community_legal as legal  # noqa: E402
from test_community import CommunityCase  # noqa: E402
from test_community_legal import make_mon  # noqa: E402

TREECKO, WINGULL, PIKACHU, DITTO, MEWTWO = 252, 278, 25, 132, 150
MOVE_POUND, MOVE_THUNDER_SHOCK, MOVE_SPORE = 1, 84, 147


class FusionRulesTest(unittest.TestCase):
    def test_name_types_and_stats(self):
        f = fusion.build(WINGULL, PIKACHU)
        self.assertEqual(f["name"], "Wingchu")
        self.assertEqual(f["type_names"], ["Water", "Electric"])
        # Wingull 40/30/30/85/55/30 and Pikachu 35/55/40/90/50/50, averaged, rounded up.
        self.assertEqual(f["stats"], [38, 43, 35, 88, 53, 40])
        self.assertTrue(1 <= len(f["colors"]) <= fusion.COLORS)

    def test_names_skip_punctuation(self):
        self.assertEqual(fusion.fusion_name("Wingull", "Mr. Mime"), "Wingmime")
        self.assertEqual(fusion.fusion_name("Pikachu", "Wingull"), "Pikaull")

    def test_matching_first_types_take_the_fathers_second(self):
        mother = {"type_ids": [12, 3]}
        self.assertEqual(fusion.fusion_types(mother, {"type_ids": [12, 10]}), [12, 10])
        self.assertEqual(fusion.fusion_types(mother, {"type_ids": [12, 12]}), [12, 3])

    def test_ditto_and_legendaries_dont_fuse(self):
        self.assertIsNone(fusion.build(WINGULL, DITTO))
        self.assertIsNone(fusion.build(WINGULL, MEWTWO))

    def test_answer_bytes(self):
        f = fusion.build(WINGULL, PIKACHU)
        raw = fusion.pack(WINGULL, PIKACHU, f)
        self.assertEqual(len(raw), 4 + 2 + fusion.COLORS * 4)
        species, partner, count = struct.unpack_from("<HHB", raw)
        self.assertEqual((species, partner, count), (WINGULL, PIKACHU, len(f["colors"])))
        self.assertEqual(list(struct.unpack_from("<%dH" % count, raw, 6)), f["colors"])


class FusionLegalityTest(unittest.TestCase):
    def test_partner_bits_decode(self):
        for partner in (1, PIKACHU, 0x7FF & 1234, 1500):
            self.assertEqual(legal.decode(make_mon(fusion=partner))["fusion_partner"], partner)

    def test_a_fusion_may_know_its_fathers_moves(self):
        self.assertIn("it can't learn Thundershock", legal.check(make_mon(moves=(MOVE_POUND, MOVE_THUNDER_SHOCK))))
        self.assertEqual(legal.check(make_mon(moves=(MOVE_POUND, MOVE_THUNDER_SHOCK), fusion=PIKACHU)), [])
        self.assertIn("it can't learn Spore", legal.check(make_mon(moves=(MOVE_POUND, MOVE_SPORE), fusion=PIKACHU)))

    def test_bad_partners_are_caught(self):
        self.assertIn("no such fusion", legal.check(make_mon(fusion=MEWTWO)))
        self.assertIn("no such fusion", legal.check(make_mon(fusion=2000)))


class FusionOnlineTest(CommunityCase):
    async def test_the_server_builds_a_fusion(self):
        c = await self.connect("TEST1")
        rows, status, args, raw = await self.ask(c, fusion.REQ_BUILD, (WINGULL, PIKACHU))
        self.assertEqual(status, 0)
        self.assertEqual(struct.unpack_from("<HH", raw), (WINGULL, PIKACHU))
        self.assertIn("Wingchu", str(rows))

    async def test_refuses_pairs_that_cant_fuse(self):
        c = await self.connect("TEST1")
        rows, status, args, raw = await self.ask(c, fusion.REQ_BUILD, (WINGULL, DITTO))
        self.assertEqual(status, fusion.CANT_FUSE)


if __name__ == "__main__":
    unittest.main()
