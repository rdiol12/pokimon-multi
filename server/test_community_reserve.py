import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import community_reserve as reserve  # noqa: E402
from test_community import CommunityCase  # noqa: E402


def left(tag, nickname, personality=0x12345678):
    mon = personality.to_bytes(4, "little") + bytes([tag]) * (reserve.BOX_MON_SIZE - 4)
    return mon + community.encode_name(nickname, reserve.NICK_SIZE)


def records(data):
    out = []
    for i in range(0, len(data), 12):
        r = data[i:i + 12]
        out.append((int.from_bytes(r[0:4], "little"), int.from_bytes(r[4:8], "little"),
                    int.from_bytes(r[8:10], "little"), r[10], r[11]))
    return out


class ReserveTest(CommunityCase):
    async def test_leave_roam_and_catch(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        rows, status, args, data = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual((status, args[0], data), (community.OK, 0, b""))

        # ASH leaves a shiny female Zigzagoon (flags 2|4).
        rows, status, args, _ = await self.ask(ash, reserve.REQ_LEAVE, (288, 12, 6), left(0xA1, "Ziggy"))
        self.assertEqual((status, rows[1]), (community.OK, "Ziggy is roaming the reserve."))
        zig = args[0]

        # Both see it; it's ASH's own for ASH.
        rows, _, args, data = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual((args[0], rows[1]), (1, "ASH"))
        self.assertEqual(records(data), [(zig, 0x12345678, 288, 12, 6 | reserve.LISTED_MINE)])
        rows, _, _, data = await self.ask(may, reserve.REQ_LIST)
        self.assertEqual(records(data)[0][4], 6)

        # MAY battles it: ASH can't at the same time.
        _, status, args, data = await self.ask(may, reserve.REQ_CLAIM, (zig,))
        self.assertEqual((status, args[:3], data[:5]), (community.OK, [288, 12, 0x12345678], left(0xA1, "")[:5]))
        self.assertEqual(len(data), reserve.BOX_MON_SIZE)
        _, status, _, _ = await self.ask(ash, reserve.REQ_CLAIM, (zig,))
        self.assertEqual(status, reserve.GONE)
        _, _, _, data = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual(data, b"")  # being battled: not roaming

        # MAY runs away: it's back. Then she catches it: gone for good.
        await self.ask(may, reserve.REQ_RESULT, (zig, reserve.END_STILL_HERE))
        _, _, args, _ = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual(args[0], 1)
        await self.ask(may, reserve.REQ_CLAIM, (zig,))
        await self.ask(ash, reserve.REQ_RESULT, (zig, reserve.END_CAUGHT))  # not ASH's battle: ignored
        _, _, args, _ = await self.ask(ash, reserve.REQ_LOG)
        self.assertEqual(args[0], 1)
        await self.ask(may, reserve.REQ_RESULT, (zig, reserve.END_CAUGHT))
        _, _, args, _ = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual(args[:2], [0, 0])
        _, status, _, _ = await self.ask(ash, reserve.REQ_CLAIM, (zig,))
        self.assertEqual(status, reserve.GONE)
        rows, _, _, _ = await self.ask(ash, reserve.REQ_LOG)
        self.assertEqual(rows[1:3], ["MAY caught Ziggy", "ASH left Ziggy"])

    async def test_claims_run_out(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        _, _, args, _ = await self.ask(ash, reserve.REQ_LEAVE, (300, 20, 0), left(1, "Skitty"))
        mon = args[0]
        await self.ask(ash, reserve.REQ_CLAIM, (mon,))
        _, status, _, _ = await self.ask(may, reserve.REQ_CLAIM, (mon,))
        self.assertEqual(status, reserve.GONE)
        r = reserve._reserve(self.server)
        start = r.clock()
        r.clock = lambda: start + reserve.CLAIM_SECONDS + 1
        _, status, _, _ = await self.ask(may, reserve.REQ_CLAIM, (mon,))
        self.assertEqual(status, community.OK)

    async def test_roaming_shows_at_most_shown_and_rotates(self):
        ash = await self.connect("ASH", 1)
        for i in range(20):
            await self.ask(ash, reserve.REQ_LEAVE, (1 + i, 5, 0), left(i, f"M{i}"))
        r = reserve._reserve(self.server)
        _, _, args, data = await self.ask(ash, reserve.REQ_LIST)
        self.assertEqual((args[0], args[1], len(data)), (reserve.SHOWN, 20, reserve.SHOWN * 12))
        first = {m["id"] for m in r.roaming(0)}
        self.assertEqual(first, {m["id"] for m in r.roaming(reserve.ROTATE_SECONDS - 1)})
        self.assertNotEqual(first, {m["id"] for m in r.roaming(reserve.ROTATE_SECONDS * 7)})

    async def test_rovers_drop_off_and_catch(self):
        ash = await self.connect("ASH", 1)
        mon = reserve.drop_off(self.server, "ROVER ED", 263, 9, personality=77, nickname="Zigzagoon")
        self.assertEqual(reserve.residents(self.server), [(mon, 263, 9, "ROVER ED")])
        # A player can battle it: no raw bytes, the game makes it fresh.
        _, status, args, data = await self.ask(ash, reserve.REQ_CLAIM, (mon,))
        self.assertEqual((status, args[:3], data), (community.OK, [263, 9, 77], b""))
        await self.ask(ash, reserve.REQ_RESULT, (mon, reserve.END_STILL_HERE))
        # A rover too low-levelled finds nothing; a stronger one catches it.
        self.assertIsNone(reserve.pick_for_rover(self.server, "ROVER JO", max_level=5))
        got = reserve.pick_for_rover(self.server, "ROVER JO", max_level=50)
        self.assertEqual((got["species"], got["owner"]), (263, "ROVER ED"))
        self.assertEqual(reserve.residents(self.server), [])
        rows, _, _, _ = await self.ask(ash, reserve.REQ_LOG)
        self.assertEqual(rows[1], "ROVER JO caught Zigzagoon")

    async def test_bad_and_full(self):
        ash = await self.connect("ASH", 1)
        rows, status, _, _ = await self.ask(ash, reserve.REQ_LEAVE, (288, 12, 0), b"short")
        self.assertEqual((status, rows[-1]), (community.OFFLINE, "bad Pokémon"))
        rows, status, _, _ = await self.ask(ash, reserve.REQ_LEAVE, (0, 12, 0), left(1, "X"))
        self.assertEqual(status, community.OFFLINE)
        old = reserve.MAX_RESIDENTS
        reserve.MAX_RESIDENTS = 1
        try:
            await self.ask(ash, reserve.REQ_LEAVE, (288, 12, 0), left(1, "A"))
            _, status, _, _ = await self.ask(ash, reserve.REQ_LEAVE, (288, 12, 0), left(1, "B"))
            self.assertEqual(status, reserve.FULL)
        finally:
            reserve.MAX_RESIDENTS = old


if __name__ == "__main__":
    unittest.main()
