"""Player-hosted events (server/online_hevents.py)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import online  # noqa: E402
import online_hevents as hev  # noqa: E402
from online_ids import ITEMS, SPECIES  # noqa: E402
from test_online import OnlineTest, rows  # noqa: E402


def said(out):
    """Every notice in an answer, line breaks back to spaces."""
    return " ".join(r[-1] for r in rows(out) if r[0] == "N").replace("~", " ").replace("^", " ")


class HostedEventTest(OnlineTest):
    async def asyncSetUp(self):
        await super().asyncSetUp()
        # Only this module, so other features' notices stay out of the way.
        self.online = online.Online(self.server, ext.install(self.server), [hev], clock=lambda: self.now)
        hev.install(self.server)

    async def battle(self, host, guest, winner):
        guest_id = next(p.id for p in self.server.players.values() if p.name == guest.name)
        await host.send(f"REQ battle {guest_id}")
        while not (line := await guest.recv()).startswith("INVITE "):
            pass
        sid = line.split()[1]
        await guest.send(f"ACCEPT {sid}")
        while not (await host.recv()).startswith(f"START {sid} "):
            pass
        await host.send(f"RESULT {sid} {'win' if winner is host else 'loss'}")
        await guest.send(f"RESULT {sid} {'win' if winner is guest else 'loss'}")
        for p in (host, guest):
            await p.send(f"QUIT {sid}")
        await self.ask(host, "OSTAT")

    async def connect(self, name, tid):
        c = await super().connect(name, tid)
        c.name = name
        return c

    async def host_hunt(self, c, species="ZIGZAGOON", money=5000, item=None):
        self.assertEqual((await self.ask(c, f"HEVENT NEW {hev.HUNT}"))[0], 0)
        await self.ask(c, f"HEVENT TARGET {SPECIES[species]}")
        status, out = await self.ask(c, "HEVENT TIME 1 6000")
        self.assertEqual([r[0] for r in rows(out)[1:]], ["NO MONEY", "$1000", "$5000"])  # what you can afford
        await self.ask(c, f"HEVENT MONEY {money}")
        if item:
            await self.ask(c, f"HEVENT ITEM {ITEMS[item[0]]} {item[1]}")
        status, out = await self.ask(c, "HEVENT CHECK")
        self.assertEqual(status, 0)
        return await self.ask(c, "HEVENT POST 6000")

    async def test_hunt_first_catch_wins_the_escrowed_prize(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        c = await self.connect("Cy", 3)
        await self.ask(b, "OSTAT")
        status, out = await self.host_hunt(a, item=("RARE_CANDY", 3))
        self.assertEqual(status, 0)
        self.assertIn(["-MONEY", "5000"], rows(out))
        self.assertIn(["-ITEM", str(ITEMS["RARE_CANDY"]), "3"], rows(out))
        self.assertEqual((await self.ask(a, f"HEVENT NEW {hev.CUP}"))[0], 1)  # one at a time
        status, out = await self.ask(b, "OSTAT")
        self.assertIn("hosting a CATCH HUNT for ZIGZAGOON", said(out))
        status, out = await self.ask(b, "HEVENT BOARD")
        self.assertEqual(rows(out)[3], ["Ann", "HUNT ZIGZAGOON", "$5000+", "1h"])
        status, out = await self.ask(b, "HEVENT LIST")
        eid = rows(out)[1][1]
        status, out = await self.ask(b, f"HEVENT SHOW {eid}")
        self.assertEqual(rows(out)[0][1], "0")
        await self.ask(b, f"HEVENT JOIN {eid}")
        await self.ask(c, f"HEVENT JOIN {eid}")
        self.assertEqual((await self.ask(a, "HEVENT CANCEL"))[0], 1)  # players are in
        # Catching something else, or before joining, doesn't count.
        await self.ask(b, f"OEV CATCH {SPECIES['WURMPLE']} 0 0 9 0")
        await self.ask(c, f"OEV CATCH {SPECIES['ZIGZAGOON']} 0 0 9 0")
        status, out = await self.ask(b, "HEVENT VIEW")
        self.assertEqual(rows(out)[0][1], "0")
        self.assertIn("Cy caught a ZIGZAGOON and won Ann's HUNT!", said(out))
        # Cy got it first.
        self.assertEqual(self.league.db.execute("SELECT winner_name FROM online_hevents").fetchone()[0], "Cy")
        status, out = await self.ask(c, "HEVENT VIEW")
        self.assertEqual(rows(out)[0][1], "1")
        self.assertIn(["+MONEY", "5000"], rows(out))
        self.assertIn(["+ITEM", str(ITEMS["RARE_CANDY"]), "3"], rows(out))
        self.assertNotIn("+MONEY", [r[0] for r in rows((await self.ask(c, "HEVENT VIEW"))[1])])  # paid once

    async def test_hunt_with_no_winner_refunds_the_host(self):
        a = await self.connect("Ann", 1)
        await self.host_hunt(a, money=1000)
        self.now += 3601
        status, out = await self.ask(a, "HEVENT VIEW")
        self.assertIn(["+MONEY", "1000"], rows(out))
        self.assertIn("ended with no winner", rows(out)[0][0])

    async def test_cancel_before_anyone_joins(self):
        a = await self.connect("Ann", 1)
        self.assertEqual((await self.ask(a, f"HEVENT NEW {hev.HUNT}"))[0], 0)
        await self.ask(a, f"HEVENT TARGET {SPECIES['ABSOL']}")
        await self.ask(a, "HEVENT TIME 0 500")
        await self.ask(a, "HEVENT MONEY 0")
        self.assertEqual((await self.ask(a, "HEVENT CHECK"))[0], 1)  # no prize
        await self.ask(a, f"HEVENT ITEM {ITEMS['NUGGET']} 2")
        await self.ask(a, "HEVENT POST 500")
        status, out = await self.ask(a, "HEVENT MINE")
        self.assertEqual(rows(out)[0][1], "1")
        status, out = await self.ask(a, "HEVENT CANCEL")
        self.assertIn(["+ITEM", str(ITEMS["NUGGET"]), "2"], rows(out))
        self.assertEqual((await self.ask(a, "HEVENT LIST"))[0], 1)  # the board is empty

    async def test_cup_bracket_battles_and_prize(self):
        host = await self.connect("Hal", 9)
        ps = [await self.connect(n, i) for i, n in enumerate(["Ann", "Ben", "Cy", "Di"], 1)]
        await self.ask(host, f"HEVENT NEW {hev.CUP}")
        await self.ask(host, "HEVENT SIZE 4")
        await self.ask(host, "HEVENT TIME 1 100000")
        await self.ask(host, "HEVENT MONEY 50000")
        status, out = await self.ask(host, "HEVENT POST 100000")
        self.assertIn(["-MONEY", "50000"], rows(out))
        eid = rows((await self.ask(ps[0], "HEVENT LIST"))[1])[1][1]
        for p in ps[:3]:
            await self.ask(p, f"HEVENT JOIN {eid}")
        status, out = await self.ask(ps[3], f"HEVENT JOIN {eid}")
        self.assertIn("starts now", rows(out)[0][0])
        # Equal ratings: seeds follow sign-up order, 1 v 4 and 2 v 3.
        status, out = await self.ask(ps[0], "HEVENT MINE")
        self.assertIn("battle Di next", rows(out)[0][0])
        await self.battle(ps[0], ps[3], winner=ps[3])
        await self.battle(ps[1], ps[2], winner=ps[1])
        status, out = await self.ask(ps[3], "HEVENT MINE")
        self.assertIn("battle Ben next", rows(out)[0][0])
        status, out = await self.ask(ps[0], f"HEVENT SHOW {eid}")
        self.assertEqual(rows(out)[0][1], "3")  # a bracket
        self.assertIn(["Ann", "vs", ">Di"], rows(out))
        # A battle between players who don't meet now changes nothing.
        await self.battle(ps[0], ps[2], winner=ps[0])
        await self.battle(ps[1], ps[3], winner=ps[1])
        status, out = await self.ask(ps[1], "HEVENT VIEW")
        self.assertIn(["+MONEY", "50000"], rows(out))
        self.assertIn("You won Hal's CUP!", rows(out)[0][0])

    async def test_cup_round_times_out_and_small_cup_is_called_off(self):
        host = await self.connect("Hal", 9)
        ps = [await self.connect(n, i) for i, n in enumerate(["Ann", "Ben"], 1)]
        await self.ask(host, f"HEVENT NEW {hev.CUP}")
        await self.ask(host, "HEVENT SIZE 8")
        await self.ask(host, "HEVENT TIME 0 1000")
        await self.ask(host, "HEVENT MONEY 1000")
        await self.ask(host, "HEVENT POST 1000")
        eid = rows((await self.ask(ps[0], "HEVENT LIST"))[1])[1][1]
        await self.ask(ps[0], f"HEVENT JOIN {eid}")
        self.now += 1801
        status, out = await self.ask(host, "HEVENT VIEW")
        self.assertIn(["+MONEY", "1000"], rows(out))  # one player isn't a cup
        # Again with two: sign-ups close, nobody battles, the top seed wins.
        await self.ask(host, f"HEVENT NEW {hev.CUP}")
        await self.ask(host, "HEVENT SIZE 4")
        await self.ask(host, "HEVENT TIME 0 1000")
        await self.ask(host, "HEVENT MONEY 1000")
        await self.ask(host, "HEVENT POST 1000")
        eid = rows((await self.ask(ps[0], "HEVENT LIST"))[1])[1][1]
        await self.ask(ps[0], f"HEVENT JOIN {eid}")
        await self.ask(ps[1], f"HEVENT JOIN {eid}")
        self.now += 1801
        await self.ask(host, "OSTAT")
        status, out = await self.ask(ps[1], "HEVENT MINE")
        self.assertIn("battle Ann next", rows(out)[0][0])
        self.now += hev.ROUND_SECONDS + 1
        status, out = await self.ask(ps[0], "HEVENT VIEW")
        self.assertIn(["+MONEY", "1000"], rows(out))


if __name__ == "__main__":
    unittest.main()
