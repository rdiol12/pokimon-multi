import asyncio
import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community_legal as legal  # noqa: E402
from test_community import CommunityCase  # noqa: E402

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


class LegalityOnlineTest(CommunityCase):
    async def answer_party(self, c, party):
        nonce = (await self.until(c, "CPARTY")).split()[1]
        for i, mon in enumerate(party):
            await c.send(f"CPARTYMON {nonce} {i} {mon.hex()}")
        await c.send(f"CPARTYEND {nonce} {len(party)}")

    async def start(self, kind, host, guest):
        await host.send(f"REQ {kind} {guest.id}")
        sid = (await self.until(host, "PENDING")).split()[1]
        await self.until(guest, "INVITE")
        await guest.send(f"ACCEPT {sid}")
        return sid

    async def test_team_check(self):
        ash = await self.connect("ASH")
        rows, status, args, _ = await self.ask(ash, legal.REQ_CHECK, party=[make_mon(), make_mon(level=12, met_level=3)])
        self.assertEqual((status, args[:2], rows[1]), (legal.ALL_LEGAL, [2, 0], "All 2 POKéMON are legal."))
        rows, status, args, _ = await self.ask(ash, legal.REQ_CHECK, party=[make_mon(), make_mon(moves=(MOVE_SPORE,))])
        self.assertEqual((status, args[:2], rows[1:3]), (legal.SOME_ILLEGAL, [2, 1], ["2. TREECKO:", "it can't learn Spore"]))
        reports = self.league.db.execute("SELECT name, species, reason, context FROM legality_reports").fetchall()
        self.assertEqual(reports, [("ASH", SPECIES_TREECKO, "it can't learn Spore", "team check")])

    async def test_an_illegal_party_stops_a_trade(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        sid = await self.start("trade", ash, may)
        await self.answer_party(ash, [make_mon()])
        await self.answer_party(may, [make_mon(evs=(255, 255, 0, 0, 0, 0))])
        self.assertEqual(await self.until(may, "ERR"), "ERR illegal Pokémon in your party")
        self.assertEqual(await self.until(may, "ENDED"), f"ENDED {sid}")
        self.assertEqual(await self.until(ash, "ENDED"), f"ENDED {sid}")
        self.assertEqual(self.server.sessions, {})

    async def test_an_illegal_party_keeps_a_battle_off_the_ladder(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        sid = await self.start("battle", ash, may)
        await self.answer_party(ash, [make_mon(level=50, exp=100)])
        await self.answer_party(may, [make_mon()])
        await asyncio.sleep(0.05)
        await ash.send(f"RESULT {sid} win")
        await may.send(f"RESULT {sid} loss")
        await asyncio.sleep(0.05)
        self.assertEqual(self.league.rank_of(("ASH", 1))[2:4], (0, 0))
        # A legal rematch counts.
        for c in (ash, may):
            await c.send(f"QUIT {sid}")
        sid = await self.start("battle", ash, may)
        await self.answer_party(ash, [make_mon()])
        await self.answer_party(may, [make_mon()])
        await asyncio.sleep(0.05)
        await ash.send(f"RESULT {sid} win")
        await may.send(f"RESULT {sid} loss")
        await asyncio.sleep(0.05)
        self.assertEqual(self.league.rank_of(("ASH", 1))[2:4], (1, 0))

    async def test_the_market_refuses_illegal_listings(self):
        ash = await self.connect("ASH")
        await ash.send(f"MARKET SELL 252 5 100 {make_mon(moves=(MOVE_SPORE,)).hex()}")
        self.assertEqual(await self.until(ash, "MLISTED"), "MLISTED full 0")
        await ash.send(f"MARKET SELL 252 5 100 {make_mon().hex()}")
        self.assertTrue((await self.until(ash, "MLISTED")).startswith("MLISTED ok"))


if __name__ == "__main__":
    unittest.main()
