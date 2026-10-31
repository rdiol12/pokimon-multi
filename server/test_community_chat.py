import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community  # noqa: E402
import community_chat as chat  # noqa: E402
from test_community import CommunityCase  # noqa: E402


def phrase(*words):
    words = list(words) + [chat.EMPTY_WORD] * (chat.WORDS - len(words))
    return b"".join(w.to_bytes(2, "little") for w in words)


class ChatTest(CommunityCase):
    async def test_rooms_and_messages(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        area = community.encode_name("ROUTE 117", 20)
        rows, _, _, ids = await self.ask(ash, chat.REQ_ROOMS, data=area)
        self.assertEqual(rows, ["CHAT ROOMS", "PLAZA (0)", "TRADING (0)", "BATTLES (0)", "ROUTE 117 (0)", "BRONZE TIER (0)"])
        self.assertEqual(list(ids), [0, 1, 2, 3, 10])
        for c in (ash, may):
            rows, _, _, _ = await self.ask(c, chat.REQ_JOIN, (chat.TRADING,))
        self.assertEqual(rows, ["TRADING", "2 here"])

        await self.ask(ash, chat.REQ_SAY, data=phrase(0x0001, 0x0203))
        await self.until(may, "CEV 0")
        _, data = may.events[-1]
        self.assertEqual(data[0], 1)
        self.assertEqual(community.decode_text(data[1:9]), "ASH")
        self.assertEqual(data[9:13], bytes([1, 0, 3, 2]))

        rows, status, _, _ = await self.ask(ash, chat.REQ_SAY, data=phrase(0x0001))
        self.assertEqual((status, rows[1]), (chat.TOO_FAST, "Slow down a little."))
        _, status, _, _ = await self.ask(ash, chat.REQ_SAY, data=phrase(0xFFFE))
        self.assertEqual(status, community.OFFLINE)  # not an Easy Chat word

        rows, status, _, _ = await self.ask(ash, chat.REQ_JOIN, (chat.TIER_BASE + 3,))
        self.assertEqual((status, rows[1]), (chat.LOCKED, "That room is for another tier."))
        await self.ask(may, chat.REQ_LEAVE)
        rows, _, _, _ = await self.ask(ash, chat.REQ_ROOMS, data=area)
        self.assertEqual(rows[2], "TRADING (1)")


if __name__ == "__main__":
    unittest.main()
