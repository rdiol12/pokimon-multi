import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community_legal as legal  # noqa: E402

SPECIES_TREECKO, MOVE_POUND, MOVE_LEER, MOVE_ABSORB = 252, 1, 43, 71
MOVE_SPORE = 147  # Treecko can't learn it in any game


def make_mon(species=SPECIES_TREECKO, moves=(MOVE_POUND, MOVE_LEER), level=5, exp=None, evs=(0,) * 6,
             item=0, met_level=5, personality=0x12345678, ot_id=0x0BADF00D, edit=None):
    """A struct Pokemon (party, 100 bytes) the way the game stores it."""
    growth = legal.data()["growth"][species]
    if exp is None:
        exp = legal.exp_for_level(growth, level)
    subs = [bytearray(12) for _ in range(4)]
    struct.pack_into("<HHI", subs[0], 0, species, item, exp)
    struct.pack_into("<4H", subs[1], 0, *(list(moves) + [0] * 4)[:4])
    subs[2][0:6] = bytes(evs)
    struct.pack_into("<H", subs[3], 2, met_level)
    secure = bytearray(48)
    for t in range(4):
        i = legal.SUBSTRUCT_OFFSETS[t][personality % 24]
        secure[i * 12:(i + 1) * 12] = subs[t]
    words = struct.unpack("<12I", secure)
    checksum = sum(w + (w >> 16) for w in words) & 0xFFFF
    if edit:
        words = edit(list(words))
    key = personality ^ ot_id
    raw = bytearray(100)
    struct.pack_into("<II", raw, 0, personality, ot_id)
    raw[8:18] = bytes([0xCE, 0xCC, 0xBF, 0xBF, 0xBD, 0xC5, 0xC9]) + b"\xff" * 3  # TREECKO
    raw[19] = 2  # hasSpecies
    struct.pack_into("<H", raw, 28, checksum)
    struct.pack_into("<12I", raw, 32, *[w ^ key for w in words])
    raw[84] = level
    return bytes(raw)


class LegalityTest(unittest.TestCase):
    def test_growth_tables_match_the_game(self):
        self.assertEqual(legal.exp_for_level(0, 100), 1000000)
        self.assertEqual(legal.exp_for_level(1, 100), 600000)
        self.assertEqual(legal.exp_for_level(2, 100), 1640000)
        self.assertEqual(legal.exp_for_level(3, 100), 1059860)
        self.assertEqual(legal.exp_for_level(3, 2), 9)
        self.assertEqual(legal.level_from_exp(0, 999999), 99)

    def test_a_caught_pokemon_is_legal(self):
        self.assertEqual(legal.check(make_mon()), [])
        for p in range(24):
            self.assertEqual(legal.check(make_mon(personality=p, level=36, met_level=5)), [], p)
        self.assertEqual(legal.decode(make_mon())["nickname"], "TREECKO")

    def test_empty_slot_is_fine(self):
        self.assertEqual(legal.check(bytes(100)), [])

    def test_catches_edits(self):
        cases = {
            "its data was edited": make_mon(edit=lambda w: w[:3] + [w[3] ^ 1] + w[4:]),
            "it can't learn Spore": make_mon(moves=(MOVE_POUND, MOVE_SPORE)),
            "it knows a move twice": make_mon(moves=(MOVE_POUND, MOVE_POUND)),
            "too much effort (EVs)": make_mon(evs=(252, 252, 10, 0, 0, 0)),
            "its level doesn't match": make_mon(level=50, exp=100),
            "met above its level": make_mon(met_level=30),
            "no such item": make_mon(item=1000),
        }
        for reason, mon in cases.items():
            self.assertIn(reason, legal.check(mon), reason)
        self.assertIn("no such species", legal.check(_with_species(2000)))

    def test_moves_from_earlier_evolutions_are_legal(self):
        # Sceptile (254) keeps Absorb from Treecko's level-up moves.
        self.assertEqual(legal.check(make_mon(species=254, moves=(MOVE_ABSORB,), level=40)), [])


def _with_species(species):
    """A mon whose species field (re-checksummed) is out of range."""
    growth = legal.data()["growth"]
    growth.setdefault(species, 0)
    try:
        return make_mon(species=species)
    finally:
        growth.pop(species, None)


if __name__ == "__main__":
    unittest.main()
