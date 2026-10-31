import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import community_boss as boss  # noqa: E402
from test_community import CommunityCase  # noqa: E402


class BossTest(CommunityCase):
    async def asyncSetUp(self):
        await super().asyncSetUp()
        self.tmp = tempfile.TemporaryDirectory()
        self.server.community_db_path = os.path.join(self.tmp.name, "league.sqlite3")
        self.hp, boss.BOSS_HP = boss.BOSS_HP, 2000

    async def asyncTearDown(self):
        boss.BOSS_HP = self.hp
        self.tmp.cleanup()
        await super().asyncTearDown()

    async def test_everyone_wears_down_one_boss_and_shares_the_reward(self):
        ash, may = await self.connect("ASH", 1), await self.connect("MAY", 2)
        rows, status, args, data = await self.ask(ash, boss.REQ_STATUS)
        self.assertEqual((status, rows[1], args[2:]), (boss.ACTIVE, "A giant WAILORD appeared!", [2000, 2000]))
        self.assertEqual(data[:4], bytes([1, 1]) + (321).to_bytes(2, "little"))

        rows, _, args, _ = await self.ask(ash, boss.REQ_DAMAGE, (1, 5000))  # capped at one battle's worth
        self.assertEqual((rows[1], args), (f"You dealt {boss.MAX_HIT} damage!", [800, 2000, 0, 0]))
        rows, _, args, _ = await self.ask(may, boss.REQ_DAMAGE, (1, 900))
        self.assertEqual((rows[1:], args[2]), (["You landed the final blow!", "WAILORD has fallen!"], 1))
        rows, status, args, _ = await self.ask(ash, boss.REQ_STATUS)
        fell = [d for s, d in ash.events if s == boss.EVENT_BOSS]
        self.assertEqual((fell[-1][1], fell[-1][12:15]), (0, bytes([0xC7, 0xBB, 0xD3])))  # down, by MAY
        self.assertEqual((status, rows[2:4], args[2]), (boss.REWARD, ["You dealt 1200 damage,", "60% of the total. Rank 1."], 1))
        _, _, args, _ = await self.ask(ash, boss.REQ_CLAIM)
        self.assertEqual(args, [boss.ITEM_RARE_CANDY, 10, boss.ITEM_GOLD_BOTTLE_CAP, 1])
        _, status, _, _ = await self.ask(ash, boss.REQ_CLAIM)
        self.assertEqual(status, boss.RESTING)
        _, _, args, _ = await self.ask(may, boss.REQ_CLAIM)
        self.assertEqual(args, [boss.ITEM_RARE_CANDY, 10, boss.ITEM_PP_MAX, 1])

        # It rests, then the next one comes; the state survives a restart.
        _, status, _, _ = await self.ask(ash, boss.REQ_STATUS)
        self.assertEqual(status, boss.RESTING)
        state = boss._boss(self.server)
        state.clock = lambda: 1e12
        rows, status, _, _ = await self.ask(ash, boss.REQ_STATUS)
        self.assertEqual((status, rows[1]), (boss.ACTIVE, f"A giant {boss.BOSSES[1][0]} appeared!"))
        self.assertEqual(boss.Boss(state.path).state["id"], 2)


if __name__ == "__main__":
    unittest.main()
