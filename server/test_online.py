import asyncio
import os
import re
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_betting  # noqa: E402
import online_coopfish  # noqa: E402
import online_doubles  # noqa: E402
import online_duo  # noqa: E402
import online_fishing  # noqa: E402
import online_goals  # noqa: E402
import online_guilds  # noqa: E402
import online_hide  # noqa: E402
import online_mentors  # noqa: E402
import online_pageant  # noqa: E402
import online_pquests  # noqa: E402
import online_racing  # noqa: E402
import online_shops  # noqa: E402
import online_sightings  # noqa: E402
import online_teams  # noqa: E402
import online_web  # noqa: E402
from online_ids import ITEMS, MAPS, SPECIES  # noqa: E402
from test_netplay_server import Client  # noqa: E402


def rows(out):
    return [r.split("|") for r in out.decode().split("\n")]


class OnlineTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.now = 1_000 * online.WEEK + 100
        self.league = ns.League(":memory:")
        self.server = ns.Server(self.league)
        self.online = online.Online(self.server, ext.install(self.server),
                                    [online_goals, online_sightings, online_web, online_fishing, online_coopfish, online_shops,
                                     online_pageant, online_duo, online_mentors,
                                     online_pquests, online_betting,
                                     online_doubles, online_teams, online_guilds, online_racing, online_hide],
                                    clock=lambda: self.now)
        online_betting.install(self.server)
        online_doubles.install(self.server)
        online_teams.install(self.server)
        online_guilds.install(self.server)
        online_hide.install(self.server)
        self.srv = await asyncio.start_server(self.server.handle, "127.0.0.1", 0)
        self.port = self.srv.sockets[0].getsockname()[1]
        self.clients = []
        self.seq = 0

    async def asyncTearDown(self):
        for c in self.clients:
            c.writer.close()
        self.srv.close()
        await self.srv.wait_closed()
        self.league.db.close()

    async def connect(self, name, tid):
        c = Client(*await asyncio.open_connection("127.0.0.1", self.port))
        self.clients.append(c)
        await c.send(f"HELLO {name} {tid}")
        self.assertTrue((await c.recv()).startswith("WELCOME "))
        return c

    async def ask(self, c, request, data=b""):
        """Sends a request like the bridge does; returns (status, answer)."""
        self.seq += 1
        for i in range(0, len(data), 100):
            await c.send("XD " + data[i:i + 100].hex())
        await c.send(f"X {self.seq} {request}")
        while True:
            line = await c.recv()
            if line.startswith(f"XR {self.seq} "):
                parts = line.split(" ")
                return int(parts[2]), bytes.fromhex(parts[3]) if len(parts) > 3 else b""


class GoalTest(OnlineTest):
    async def test_goals_fill_and_pay_helpers(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        old = online_goals.GOALS
        online_goals.GOALS = [("catch", "Catch {n} Pok#mon", 2, "ULTRA_BALL", 10), *old[1:]]
        try:
            featured = SPECIES[online_goals.featured(self.online.week())]
            self.assertEqual(await self.ask(a, f"OEV CATCH {featured} 0 0 9 1"), (0, b"|0"))
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[0], ["", "0"])
            self.assertEqual(rows(out)[3][:2], ["%", "50"])
            await self.ask(a, "OEV CATCH 1 0 0 9 0")
            # The goal is reached: both players hear of it, only Ann helped.
            status, out = await self.ask(b, "OSTAT")
            self.assertIn("N", [r[0] for r in rows(out)])
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[0][1], "1")
            self.assertEqual((await self.ask(b, "GOAL CLAIM"))[0], 1)
            status, out = await self.ask(a, "GOAL CLAIM")
            self.assertEqual(status, 0)
            self.assertIn(["+ITEM", str(ITEMS["ULTRA_BALL"]), "10"], rows(out))
            self.assertEqual((await self.ask(a, "GOAL CLAIM"))[0], 1)
            # Next week starts from zero.
            self.now += online.WEEK
            status, out = await self.ask(a, "GOAL VIEW")
            self.assertEqual(rows(out)[3][:2], ["%", "0"])
        finally:
            online_goals.GOALS = old


class SightingTest(OnlineTest):
    async def test_sightings_board_and_notice(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        latios, route = SPECIES["LATIOS"], MAPS["ROUTE117"]
        await self.ask(a, f"OEV SIGHT {latios} {route[0]} {route[1]} 1")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn(["N", "Ann spotted LATIOS at ROUTE 117!"], rows(out))
        self.now += 300
        await self.ask(a, f"OEV SIGHT {latios} {route[0]} {route[1]} 1")
        status, out = await self.ask(b, "OSTAT")
        self.assertNotIn("N", [r[0] for r in rows(out)])  # announced recently
        status, out = await self.ask(b, "SIGHT BOARD")
        self.assertEqual(rows(out)[3], ["LATIOS", "ROUTE 117", "Ann", "now"])


class WebTest(OnlineTest):
    async def test_live_map_and_shiny_gallery(self):
        online_web.install(self.server)
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(b, "OSTAT")
        g, n = MAPS["OLDALE_TOWN"]
        await a.send(f"POS {g} {n} 5 5 1 3 0")
        absol = SPECIES["ABSOL"]
        r120 = MAPS["ROUTE120"]
        await self.ask(a, f"OEV CATCH {absol} 1 {r120[0]} {r120[1]} 0")
        await self.ask(a, f"OEV CATCH {absol} 0 {r120[0]} {r120[1]} 0")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn(["N", "Ann caught a shiny ABSOL!"], rows(out))
        route = self.server.web_routes[0]
        data = __import__("json").loads(route("/map.json")[2])
        self.assertEqual([p["label"] for p in data["players"]], ["Ann"])
        self.assertIn("ABSOL", route("/shinies")[2])
        self.assertEqual(route("/shinies")[2].count('class="mon"'), 1)
        png = route(f"/sprite/{absol}.png")[2]
        self.assertTrue(png.startswith(b"\x89PNG") and b"tRNS" in png)
        self.assertIsNone(route("/sprite/999999.png"))


class FishingTest(OnlineTest):
    async def test_tournament_scores_and_prizes(self):
        online_fishing.START, online_fishing.PERIOD, online_fishing.LENGTH = self.now - 60, 7200, 600
        online_fishing._announced.update(started=None, finished=None)
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        status, out = await self.ask(a, "OSTAT")
        fisht = next(r for r in rows(out) if r[:2] == ["S", "FISHT"])
        self.assertEqual(fisht[2:4], ["1", "9"])
        k = int(fisht[7])
        rare = SPECIES[online_fishing.rares(k)[0]]
        self.assertIn("N", [r[0] for r in rows(out)])  # it started
        await self.ask(a, f"OEV FISH 129 900")      # Magikarp, 90 cm
        await self.ask(b, f"OEV FISH {rare} 450")   # rare, 45 cm + 50
        await self.ask(a, f"OEV FISH 129 300")
        status, out = await self.ask(a, "FISHT BOARD")
        lines = rows(out)
        self.assertEqual(lines[3][:3], ["*1", "Ann", "120"])
        self.assertEqual(lines[4][:3], ["2", "Ben", "95"])
        self.assertEqual(lines[0][1], "0")
        # Over: Ann is first, Ben second; both collect.
        self.now += 700
        status, out = await self.ask(a, "FISHT BOARD")
        self.assertEqual(rows(out)[0][1], "1")
        self.assertIn("RESULTS", rows(out)[1][0])
        status, out = await self.ask(b, "FISHT CLAIM")
        self.assertIn(["+ITEM", str(ITEMS["RARE_CANDY"]), "3"], rows(out))
        status, out = await self.ask(a, "FISHT CLAIM")
        self.assertIn(["+MONEY", "20000"], rows(out))
        self.assertEqual((await self.ask(a, "FISHT CLAIM"))[0], 1)
        # Reports outside a tournament don't count.
        await self.ask(a, "OEV FISH 129 900")
        self.assertEqual(self.league.db.execute("SELECT COUNT(*) FROM online_fish_scores").fetchone()[0], 2)


class SpeciesNamesTest(unittest.TestCase):
    def test_every_named_species_exists(self):
        for name in online_fishing.RARES + online_coopfish.GIANTS:
            self.assertIn(name, SPECIES)


class CoFishTest(OnlineTest):
    async def test_two_players_land_a_giant(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        for c in (a, b):
            await c.send("POS 0 9 5 5 1 3 0")
            await self.ask(c, "OSTAT")
        status, out = await self.ask(a, "COFISH WHO")
        ben_id = rows(out)[1][1]
        self.assertEqual(rows(out)[1][0], "Ben")
        await self.ask(a, f"COFISH ASK {ben_id}")
        self.assertEqual(rows((await self.ask(a, "COFISH WAIT"))[1])[0][1], "0")
        status, out = await self.ask(b, "OSTAT")
        invite = next(r for r in rows(out) if r[:2] == ["S", "COFISH"])
        self.assertEqual(invite[2:4], ["1", "Ann"])
        await self.ask(b, "COFISH JOIN")
        self.assertEqual(rows((await self.ask(a, "COFISH WAIT"))[1])[0][1], "1")
        await self.ask(a, "COFISH REEL 3")
        self.assertEqual(rows((await self.ask(a, "COFISH RESULT"))[1])[0][1], "0")
        await self.ask(b, "COFISH REEL 2")
        for c in (a, b):
            status, out = await self.ask(c, "COFISH RESULT")
            self.assertEqual(rows(out)[0][1], "1")
            self.assertEqual(rows(out)[1][0], "+MON")
        status, out = await self.ask(a, "COFISH BOARD")
        self.assertEqual(rows(out)[3][2], "Ann & Ben")
        # Not enough hits: it gets away, and nobody gets anything.
        await self.ask(b, f"COFISH ASK {rows((await self.ask(b, 'COFISH WHO'))[1])[1][1]}")
        await self.ask(a, "COFISH JOIN")
        await self.ask(a, "COFISH REEL 1")
        await self.ask(b, "COFISH REEL 1")
        status, out = await self.ask(a, "COFISH RESULT")
        self.assertEqual(rows(out)[0][1], "2")
        self.assertEqual(len(rows(out)), 1)


class ShopTest(OnlineTest):
    async def test_stock_buy_and_collect(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        potion = ITEMS["POTION"]
        status, out = await self.ask(b, "SHOP LIST")
        self.assertEqual(status, 1)
        status, out = await self.ask(a, f"SHOP PRICES {potion} 5 300")
        self.assertEqual([r[1] for r in rows(out)[1:]], ["150", "300", "450", "600"])
        status, out = await self.ask(a, f"SHOP STOCK {potion} 5 450")
        self.assertEqual(rows(out)[1], ["-ITEM", str(potion), "5"])
        await self.ask(a, f"SHOP STOCK {potion} 2 450")  # same price: same shelf
        status, out = await self.ask(b, "SHOP LIST")
        self.assertEqual(rows(out)[1][0], "Ann's Shop")
        shop = rows(out)[1][1]
        status, out = await self.ask(b, f"SHOP OPEN {shop}")
        self.assertEqual(rows(out)[3], ["Potion", "7", "$450"])
        status, out = await self.ask(b, "SHOP ITEMS")
        shelf = rows(out)[1][1]
        # Not enough money, then 3 of them.
        self.assertEqual((await self.ask(b, f"SHOP BUY {potion} 3 {shelf} 1000"))[0], 1)
        status, out = await self.ask(b, f"SHOP BUY {potion} 3 {shelf} 5000")
        self.assertEqual(rows(out)[1:3], [["-MONEY", "1350"], ["+ITEM", str(potion), "3"]])
        # Asking for more than is left buys what's left.
        status, out = await self.ask(b, f"SHOP BUY {potion} 99 {shelf} 5000")
        self.assertEqual(rows(out)[2], ["+ITEM", str(potion), "4"])
        self.assertEqual((await self.ask(b, "SHOP LIST"))[0], 1)
        # Ann hears of the sales and collects.
        status, out = await self.ask(a, "SHOP MINE")
        self.assertEqual(rows(out)[0][1], "3150")
        self.assertIn("N", [r[0] for r in rows(out)])
        status, out = await self.ask(a, "SHOP COLLECT")
        self.assertEqual(rows(out)[1], ["+MONEY", "3150"])
        self.assertEqual((await self.ask(a, "SHOP COLLECT"))[0], 1)

    async def test_shelves_and_take_back(self):
        a = await self.connect("Ann", 1)
        for i, item in enumerate(["POTION", "ANTIDOTE", "POKE_BALL", "GREAT_BALL", "REPEL", "ESCAPE_ROPE"]):
            await self.ask(a, f"SHOP STOCK {ITEMS[item]} 1 100")
        status, out = await self.ask(a, f"SHOP STOCK {ITEMS['ETHER']} 1 100")
        self.assertEqual(status, 1)
        status, out = await self.ask(a, "SHOP MINE")
        self.assertEqual(rows(out)[3][0], "Pok# Ball x1 $100")
        status, out = await self.ask(a, f"SHOP TAKE {rows(out)[1][1]}")
        self.assertEqual(rows(out)[1], ["+ITEM", str(ITEMS["POTION"]), "1"])


class PageantTest(OnlineTest):
    async def test_enter_vote_and_win(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        c = await self.connect("Cy", 3)
        self.assertEqual((await self.ask(a, "PAGEANT SHOW 0"))[0], 1)
        milotic, eevee = SPECIES["MILOTIC"], SPECIES["EEVEE"]
        await self.ask(a, f"PAGEANT ENTER {milotic} 40 1 1234 Lady")
        await self.ask(b, f"PAGEANT ENTER {eevee} 12 0 99 MR. FLUFF")
        # Ann only sees Ben's entry, and can't vote for her own.
        status, out = await self.ask(a, "PAGEANT SHOW 5")
        self.assertEqual(rows(out)[1], [str(eevee), "0", "99", rows(out)[1][3], "0"])
        self.assertIn("MR. FLUFF", rows(out)[0][0])
        ben_entry = rows(out)[1][3]
        status, out = await self.ask(b, "PAGEANT SHOW 0")
        ann_entry = rows(out)[1][3]
        self.assertEqual((await self.ask(a, f"PAGEANT VOTE 0 {ann_entry}"))[0], 1)
        self.assertEqual((await self.ask(a, f"PAGEANT VOTE 0 {ben_entry}"))[0], 0)
        self.assertEqual((await self.ask(a, f"PAGEANT VOTE 0 {ben_entry}"))[0], 1)  # once each
        status, out = await self.ask(c, "PAGEANT SHOW 0")
        self.assertEqual(rows(out)[0][1], "3")  # votes left
        for entry in (ann_entry, ben_entry):
            await self.ask(c, f"PAGEANT VOTE 0 {entry}")
        status, out = await self.ask(b, "PAGEANT BOARD")
        self.assertEqual(rows(out)[3], ["*1", "MR. FLUFF", "Ben", "2"])
        self.assertEqual(rows(out)[4], ["2", "Lady", "Ann", "1"])
        # Next week Ben collects 1st place, Ann 2nd; only once.
        self.now += online.WEEK
        status, out = await self.ask(b, "PAGEANT BOARD")
        self.assertEqual(rows(out)[0][1], "1")
        status, out = await self.ask(b, "PAGEANT CLAIM")
        self.assertEqual(rows(out)[2], ["+MONEY", "15000"])
        self.assertEqual((await self.ask(b, "PAGEANT CLAIM"))[0], 1)
        status, out = await self.ask(a, "PAGEANT CLAIM")
        self.assertEqual(rows(out)[1], ["+ITEM", str(ITEMS["RARE_CANDY"]), "2"])


class DuoTest(OnlineTest):
    async def test_team_up_and_finish_quests(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        await self.ask(c := await self.connect("Cy", 3), "OSTAT")
        self.assertEqual(rows((await self.ask(a, "DUO VIEW"))[1])[0][1], "0")
        status, out = await self.ask(a, "DUO WHO")
        ben = next(r for r in rows(out)[1:] if r[0] == "Ben")
        await self.ask(a, f"DUO ASK {ben[1]}")
        self.assertEqual(rows((await self.ask(a, "DUO VIEW"))[1])[0][1], "2")
        status, out = await self.ask(b, "DUO VIEW")
        self.assertEqual(rows(out)[0][1:3], ["1", "Ann"])
        self.assertIn("N", [r[0] for r in rows(out)])  # the invitation notice
        await self.ask(b, "DUO YES")
        # Ben is busy now: Cy can't ask him.
        self.assertNotIn("Ben", [r[0] for r in rows((await self.ask(c, "DUO WHO"))[1])[1:]])
        kind, n, *_ = online_duo.quest(0)
        self.assertEqual(kind, "catch")
        for i in range(n):
            await self.ask(a if i % 2 else b, f"OEV CATCH {i + 1} 0 0 9 0")
        status, out = await self.ask(a, "DUO VIEW")
        self.assertEqual(rows(out)[0][1], "4")  # a reward waiting
        self.assertEqual(rows(out)[0][3], "1")
        self.assertEqual(rows(out)[3][:2], ["%", "0"])  # the next quest
        status, out = await self.ask(b, "DUO CLAIM")
        self.assertEqual(rows(out)[1], ["+ITEM", str(ITEMS["ULTRA_BALL"]), "5"])
        self.assertEqual((await self.ask(b, "DUO CLAIM"))[0], 1)
        # Quest 2 is fishing: catches don't count, fished ones do.
        await self.ask(a, "OEV CATCH 5 0 0 9 1")
        await self.ask(a, f"OEV FISH 5 400")
        status, out = await self.ask(b, "DUO VIEW")
        self.assertEqual(rows(out)[3][4], f"1/{online_duo.quest(1)[1]}")
        await self.ask(b, "DUO LEAVE")
        self.assertEqual(rows((await self.ask(a, "DUO VIEW"))[1])[0][1], "0")

    async def test_species_quest_counts_each_once(self):
        db = self.league.db
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        db.execute("INSERT INTO online_duos (a_name, a_tid, b_name, b_tid, round) VALUES ('Ann', 1, 'Ben', 2, 2)")
        for sp in (7, 7, 8):
            await self.ask(a, f"OEV CATCH {sp} 0 0 9 0")
        await self.ask(b, "OEV CATCH 7 0 0 9 0")
        status, out = await self.ask(a, "DUO VIEW")
        self.assertEqual(rows(out)[3][4], "2/6")


class MentorTest(OnlineTest):
    async def test_mentor_apprentice_badges_and_graduation(self):
        a = await self.connect("Ann", 1)  # the mentor
        b = await self.connect("Ben", 2)
        self.assertEqual(rows((await self.ask(a, "MENTOR VIEW 8"))[1])[0][1], "3")  # can volunteer
        self.assertEqual((await self.ask(a, "MENTOR FIND 8"))[0], 1)
        await self.ask(a, "MENTOR VOLUNTEER 8")
        self.assertEqual(rows((await self.ask(a, "MENTOR VIEW 8"))[1])[0][1], "2")
        self.assertEqual(rows((await self.ask(b, "MENTOR VIEW 1"))[1])[0][1], "0")
        status, out = await self.ask(b, "MENTOR FIND 1")
        self.assertEqual(rows(out)[1], ["Ann, 8 badges", "Ann:1"])
        await self.ask(b, "MENTOR PICK Ann:1 1")
        status, out = await self.ask(b, "MENTOR VIEW 1")
        self.assertEqual(rows(out)[0][1:3], ["1", "Ann"])
        # A cheer a day.
        status, out = await self.ask(a, "MENTOR WHO 8")
        await self.ask(a, f"MENTOR CHEER {rows(out)[1][1]} 8")
        self.assertEqual((await self.ask(a, "MENTOR CHEER Ben:2 8"))[0], 1)
        status, out = await self.ask(a, "MENTOR VIEW 8")
        self.assertEqual(rows(out)[3], ["Ben", "1 badges", "yes"])
        # Ben wins a badge (reported in the background): gifts for both.
        await self.ask(b, "OEV BADGES 2")
        status, out = await self.ask(b, "MENTOR GIFTS 2")
        self.assertEqual([r for r in rows(out)[1:] if r[0] != "N"],
                         [["+ITEM", str(ITEMS["SUPER_POTION"]), "8"], ["+MONEY", "1000"]])
        self.assertEqual(rows(out)[0][0], "Ben got 8 SUPER POTIONS and $1000!")
        status, out = await self.ask(a, "MENTOR GIFTS 8")
        self.assertEqual(rows(out)[1], ["+ITEM", str(ITEMS["GREAT_BALL"]), "3"])
        # The same count again brings nothing.
        await self.ask(b, "OEV BADGES 2")
        self.assertEqual((await self.ask(b, "MENTOR GIFTS 2"))[0], 1)
        # Eight badges: Ben graduates.
        await self.ask(b, "MENTOR VIEW 8")
        self.assertEqual(rows((await self.ask(b, "MENTOR VIEW 8"))[1])[0][1], "3")
        status, out = await self.ask(a, "MENTOR VIEW 8")
        self.assertIn("1 graduated", rows(out)[0][0])


class PlayerQuestTest(OnlineTest):
    async def test_hide_take_and_find(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        potion = ITEMS["POTION"]
        self.assertEqual((await self.ask(a, f"PQUEST MAKE {potion} 3 1"))[0], 1)  # no position yet
        await a.send("POS 0 9 12 7 1 3 0")
        status, out = await self.ask(a, f"PQUEST MAKE {potion} 3 1")
        self.assertEqual(rows(out)[1], ["-ITEM", str(potion), "3"])
        status, out = await self.ask(b, "PQUEST LIST")
        qid = rows(out)[1][1]
        self.assertIn("Hint: In the tall grass", rows((await self.ask(b, f"PQUEST SHOW {qid}"))[1])[0][0])
        await self.ask(b, f"PQUEST TAKE {qid}")
        status, out = await self.ask(b, "OSTAT")
        self.assertIn(["S", "PQUEST", qid, "0", "9", "12", "7"], rows(out))
        self.assertEqual(rows((await self.ask(b, "PQUEST LIST"))[1])[0][1], "1")  # busy with it
        # Too far, then on the spot.
        await b.send("POS 0 9 20 7 1 3 0")
        self.assertEqual((await self.ask(b, "PQUEST FIND"))[0], 1)
        await b.send("POS 0 9 13 7 1 3 0")
        status, out = await self.ask(b, "PQUEST FIND")
        self.assertEqual(rows(out)[1], ["+ITEM", str(potion), "3"])
        status, out = await self.ask(a, "PQUEST MINE")
        self.assertIn("Found by others: 1", rows(out)[0][0])
        self.assertIn("N", [r[0] for r in rows(out)])
        self.assertNotIn("PQUEST", [r[1] for r in rows((await self.ask(b, "OSTAT"))[1]) if r[0] == "S"])

    async def test_cancel_gives_items_back(self):
        a = await self.connect("Ann", 1)
        await a.send("POS 0 9 12 7 1 3 0")
        await self.ask(a, f"PQUEST MAKE {ITEMS['ULTRA_BALL']} 2 0")
        status, out = await self.ask(a, "PQUEST MINE")
        status, out = await self.ask(a, f"PQUEST CANCEL {rows(out)[1][1]}")
        self.assertEqual(rows(out)[1], ["+ITEM", str(ITEMS["ULTRA_BALL"]), "2"])


class BettingTest(OnlineTest):
    async def battle(self, host, guest, kind="battle"):
        guest_id = next(p.id for p in self.server.players.values() if p.name == guest.name)
        await host.send(f"REQ {kind} {guest_id}")
        while not (line := await guest.recv()).startswith("INVITE "):
            pass
        sid = line.split()[1]
        await guest.send(f"ACCEPT {sid}")
        while not (await host.recv()).startswith(f"START {sid} "):
            pass
        return sid

    async def test_bet_and_win(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        c = await self.connect("Cy", 3)
        d = await self.connect("Di", 4)
        a.name, b.name = "Ann", "Ben"
        # No coins: the house gives 100 once a day.
        status, out = await self.ask(c, "BET LIST 0")
        self.assertEqual(rows(out)[0][1], "0")
        self.assertIn(["+COINS", "100"], rows(out))
        self.assertNotIn(["+COINS", "100"], rows((await self.ask(c, "BET LIST 0"))[1]))
        sid = await self.battle(a, b)
        status, out = await self.ask(c, "BET LIST 100")
        self.assertEqual(rows(out)[1][:2], ["Ann vs Ben", sid])
        self.assertIn("N", [r[0] for r in rows(out)])  # heard it started
        self.assertEqual(rows((await self.ask(a, "BET LIST 100"))[1])[0][1], "0")  # not on your own
        await self.ask(c, f"BET SHOW {sid}")
        self.assertEqual((await self.ask(c, "BET PLACE 500 h 100"))[0], 1)  # too few coins
        status, out = await self.ask(c, "BET PLACE 100 h 100")
        self.assertEqual(rows(out)[1], ["-COINS", "100"])
        await self.ask(d, f"BET SHOW {sid}")
        await self.ask(d, "BET PLACE 50 g 500")
        self.assertEqual((await self.ask(d, "BET PLACE 10 h 500"))[0], 1)  # one side only
        status, out = await self.ask(d, f"BET SHOW {sid}")
        self.assertEqual(rows(out)[1][0], "Ann (x1.5)")
        # Ann wins: Cy gets the whole pool.
        await a.send(f"RESULT {sid} win")
        await b.send(f"RESULT {sid} loss")
        for p in (a, b):
            await p.send(f"QUIT {sid}")
        await self.ask(a, "OSTAT")
        status, out = await self.ask(c, "BET COLLECT")
        self.assertEqual(rows(out)[1], ["+COINS", "150"])
        self.assertEqual((await self.ask(d, "BET COLLECT"))[0], 1)
        status, out = await self.ask(d, "BET BOARD")
        self.assertEqual(rows(out)[3][1:], ["Cy", "1", "+50"])
        self.assertEqual(rows(out)[4][1:], ["Di", "1", "-50"])
        # Bets close after two minutes.
        sid = await self.battle(b, a)
        self.now += online_betting.BET_SECONDS + 1
        out = (await self.ask(c, "BET LIST 50"))[1]
        self.assertEqual(rows(out)[0][1], "0", out)


class DoublesTest(BettingTest):
    async def test_doubles_have_their_own_ladder(self):
        a = await self.connect("Ann", 1)
        b = await self.connect("Ben", 2)
        a.name, b.name = "Ann", "Ben"
        self.assertIn("RANKED DOUBLE", rows((await self.ask(a, "DOUBLES BOARD"))[1])[0][0])
        sid = await self.battle(a, b, "double")
        await a.send(f"RESULT {sid} loss")
        await b.send(f"RESULT {sid} win")
        await self.ask(b, "OSTAT")
        status, out = await self.ask(a, "DOUBLES BOARD")
        self.assertEqual(rows(out)[3], ["1", "Ben", "1016", "1-0-0"])
        self.assertEqual(rows(out)[4], ["*2", "Ann", "984", "0-1-0"])
        self.assertIn("N", [r[0] for r in rows(out)])
        # The singles ladder didn't change.
        self.assertEqual(self.league.db.execute("SELECT COUNT(*) FROM matches").fetchone()[0], 0)


class TeamsTest(OnlineTest):
    def pid(self, name):
        return next(p.id for p in self.server.players.values() if p.name == name)

    async def team_up(self, a, b, b_name):
        status, out = await self.ask(a, "TEAM WHO")
        self.assertIn(b_name, [r[0] for r in rows(out)[1:]])
        await self.ask(a, f"TEAM ASK {self.pid(b_name)}")
        status, out = await self.ask(b, "TEAM VIEW")
        self.assertEqual(rows(out)[0][1], str(online_teams.INVITED))
        status, out = await self.ask(b, "TEAM YES")
        self.assertEqual(status, 0)

    async def recv_until(self, c, prefix):
        while not (line := await c.recv()).startswith(prefix):
            pass
        return line

    async def test_team_battle(self):
        a, b, c, d = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat", "Dan"), 1)]
        # Without a team, a team battle is declined.
        await a.send(f"REQ multi {self.pid('Ben')}")
        await self.recv_until(a, "DECLINED ")
        await self.team_up(a, c, "Cat")
        await self.team_up(b, d, "Dan")
        status, out = await self.ask(a, "TEAM VIEW")
        self.assertEqual(rows(out)[0][1], str(online_teams.TEAMED))
        self.assertIn("Ann & Cat", rows(out)[0][0])

        await a.send(f"REQ multi {self.pid('Ben')}")
        invites = [await self.recv_until(x, "INVITE ") for x in (b, c, d)]
        sid = invites[0].split()[1]
        self.assertTrue(all(i.split()[1:3] == [sid, "multi"] for i in invites))
        for x in (b, c, d):
            await x.send(f"ACCEPT {sid}")
        starts = [await self.recv_until(x, f"START {sid} ") for x in (a, b, c, d)]
        self.assertEqual([s.split()[4] for s in starts], ["0", "1", "2", "3"])
        self.assertEqual(starts[2].split()[3], str(self.pid("Ben")))  # the other side's leader

        # Link packets go to the three others.
        await c.send(f"LINK {sid} 01020304")
        for x in (a, b, d):
            self.assertEqual(await self.recv_until(x, "LINK "), f"LINK {sid} 01020304")

        for x, outcome in ((a, "loss"), (c, "loss"), (b, "win"), (d, "win")):
            await x.send(f"RESULT {sid} {outcome}")
        await self.ask(d, "OSTAT")
        status, out = await self.ask(b, "TEAM BOARD")
        self.assertEqual(rows(out)[3], ["*1", "Ben & Dan", "1016", "1-0-0"])
        self.assertEqual(rows(out)[4], ["2", "Ann & Cat", "984", "0-1-0"])

    async def test_partner_leaving_ends_it_for_all(self):
        a, b, c, d = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat", "Dan"), 1)]
        await self.team_up(a, c, "Cat")
        await self.team_up(b, d, "Dan")
        await a.send(f"REQ multi {self.pid('Ben')}")
        sid = (await self.recv_until(d, "INVITE ")).split()[1]
        await d.send(f"DECLINE {sid}")
        for x in (a, b, c):
            await self.recv_until(x, f"DECLINED {sid}")
        self.assertIsNone(self.server.players[self.pid("Cat")].session)



def team_record(levels):
    """A team record (struct SecretBase) with Pokémon of these levels."""
    out = bytearray(online_guilds.RECORD_SIZE)
    out[2:9] = b"\xbb\xff\xff\xff\xff\xff\xff"
    for i, level in enumerate(levels):
        out[online_guilds.SPECIES + 2 * i:online_guilds.SPECIES + 2 * i + 2] = (SPECIES["MUDKIP"] + i).to_bytes(2, "little")
        out[online_guilds.LEVELS + i] = level
    return bytes(out)


class GuildsTest(OnlineTest):
    async def guild(self, c, tag, join=False):
        word = f"GUILD JOIN {tag}" if join else f"GUILD CREATE {tag} Team {tag}"
        self.assertEqual((await self.ask(c, word))[0], 0)

    async def test_hall_is_one_room_per_guild(self):
        a, b, c = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat"), 1)]
        status, out = await self.ask(a, "GHALL VIEW")
        self.assertEqual(rows(out)[0][1], str(online_guilds.NO_GUILD))
        await self.guild(a, "RED")
        await self.guild(b, "RED", join=True)
        await self.guild(c, "BLUE")
        status, out = await self.ask(a, "GHALL VIEW")
        self.assertEqual(rows(out)[0][1:], [str(online_guilds.LEADER), "RED", "Team RED"])
        self.assertIn("2 of 2 members", rows(out)[0][0])
        hall = " ".join(map(str, online_guilds.HALL))
        for x in (a, b, c):
            await x.send(f"POS {hall} 6 12 1 0 0")
        # Ann sees Ben (same guild), never Cat.
        seen = []
        while True:
            try:
                seen.append(await asyncio.wait_for(a.recv(), 0.3))
            except asyncio.TimeoutError:
                break
        names = " ".join(seen)
        self.assertIn("Ben", names)
        self.assertNotIn("Cat", names)

    async def test_chest(self):
        a, b = await self.connect("Ann", 1), await self.connect("Ben", 2)
        await self.guild(a, "RED")
        await self.guild(b, "RED", join=True)
        potion = ITEMS["POTION"]
        status, out = await self.ask(a, f"GHALL PUT {potion} 5")
        self.assertEqual(rows(out)[1], ["-ITEM", str(potion), "5"])
        status, out = await self.ask(b, "GHALL CHEST")
        self.assertEqual(rows(out)[1], ["5 Potions", str(potion), str(potion), "5"])
        status, out = await self.ask(b, f"GHALL TAKE {potion}")
        self.assertEqual(rows(out)[1], ["+ITEM", str(potion), "5"])
        status, out = await self.ask(a, f"GHALL TAKE {potion}")
        self.assertEqual(status, 1)  # gone

    async def test_pvp_win_scores_and_prizes_next_week(self):
        a, b = await self.connect("Ann", 1), await self.connect("Ben", 2)
        await self.guild(a, "RED")
        await self.guild(b, "BLUE")
        await a.send(f"REQ battle {next(p.id for p in self.server.players.values() if p.name == 'Ben')}")
        sid = (await self.recv_until(b, "INVITE ")).split()[1]
        await b.send(f"ACCEPT {sid}")
        await self.recv_until(a, f"START {sid}")
        await a.send(f"RESULT {sid} win")
        await b.send(f"RESULT {sid} loss")
        await self.ask(a, "OSTAT")
        status, out = await self.ask(a, "WAR BOARD")
        self.assertEqual(rows(out)[3], ["*1", "(RED) Team RED", str(online_guilds.WAR_WIN)])
        self.assertEqual(rows(out)[0][1], "0")
        self.now += online.WEEK
        status, out = await self.ask(a, "WAR BOARD")
        self.assertEqual(rows(out)[0][1], "1")  # a prize is waiting
        status, out = await self.ask(a, "WAR CLAIM")
        self.assertIn(["+ITEM", str(ITEMS["RARE_CANDY"]), "3"], rows(out))
        self.assertEqual((await self.ask(a, "WAR CLAIM"))[0], 1)
        # Ben's guild placed second.
        status, out = await self.ask(b, "WAR CLAIM")
        self.assertEqual(status, 1)  # BLUE never scored: no place

    async def recv_until(self, c, prefix):
        while not (line := await c.recv()).startswith(prefix):
            pass
        return line

    async def test_raid(self):
        a, b, c = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat"), 1)]
        await self.guild(a, "RED")
        await self.guild(b, "BLUE")
        await self.guild(c, "BLUE", join=True)
        self.assertEqual((await self.ask(a, "RAID LIST"))[0], 1)  # nobody defends
        await self.ask(b, "RAID DEFEND", team_record([20, 35, 10]))
        status, out = await self.ask(c, "RAID DEFEND", team_record([40]))
        self.assertIn("Defenders: 2", out.decode())
        status, out = await self.ask(a, "RAID LIST")
        self.assertEqual(rows(out)[1], ["(BLUE) Team BLUE", "BLUE"])
        status, out = await self.ask(a, "RAID GET BLUE")
        self.assertEqual(status, 0)
        text_part, team = out.split(b"\0", 1)
        self.assertEqual(len(team), online_guilds.RECORD_SIZE)
        # Each defender's best: Cat's Lv. 40, then Ben's Lv. 35.
        self.assertEqual(list(team[online_guilds.LEVELS:online_guilds.LEVELS + 3]), [40, 35, 0])
        self.assertEqual(team[2:6], bytes([0xBC, 0xC6, 0xCF, 0xBF]))  # "BLUE"
        status, out = await self.ask(a, "RAID RESULT 1")
        self.assertIn("fell", out.decode())
        status, out = await self.ask(a, "WAR BOARD")
        self.assertEqual(rows(out)[3][2], str(online_guilds.WAR_RAID))
        # The defenders hear of it.
        status, out = await self.ask(b, "WAR BOARD")
        self.assertTrue(any(r[0] == "N" and "raided" in r[1] for r in rows(out)))
        # Three raids a day.
        for _ in range(2):
            await self.ask(a, "RAID GET BLUE")
            await self.ask(a, "RAID RESULT 0")
        self.assertEqual((await self.ask(a, "RAID GET BLUE"))[0], 1)
        status, out = await self.ask(b, "WAR BOARD")
        self.assertIn(["*2", "(BLUE) Team BLUE", str(2 * online_guilds.WAR_DEFENDED)], rows(out))



class RacingTest(OnlineTest):
    async def test_heat(self):
        a, b, c = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat"), 1)]
        status, out = await self.ask(a, "RACE LIST")
        self.assertEqual(rows(out)[1], ["PETALBURG RUN", "0"])
        status, out = await self.ask(a, "RACE JOIN 0")
        self.assertEqual(rows(out)[0][1:3], [str(int(online_racing.HEAT_WAIT)), "PETALBURG RUN"])
        self.assertEqual(rows(out)[1], ["0"])  # lane
        self.now += 5
        status, out = await self.ask(b, "RACE LIST")
        self.assertEqual(rows(out)[1][0], "PETALBURG RUN 1 waiting")
        status, out = await self.ask(b, "RACE JOIN 0")
        self.assertEqual(rows(out)[0][1], str(int(online_racing.HEAT_WAIT) - 5))
        self.assertEqual(rows(out)[1], ["1"])
        # Too close to the start: Cat gets a heat of her own.
        self.now += online_racing.HEAT_WAIT - 6
        status, out = await self.ask(c, "RACE JOIN 0")
        self.assertEqual(rows(out)[1], ["0"])
        self.now += 60
        status, out = await self.ask(b, "RACE FINISH 0 2400")
        self.assertIn(["+MONEY", str(online_racing.WIN_MONEY)], rows(out))
        self.assertIn("0:40.00", rows(out)[0][0])
        status, out = await self.ask(a, "RACE FINISH 0 2460")
        self.assertIn("2nd", rows(out)[0][0])
        self.assertNotIn("+MONEY", [r[0] for r in rows(out)])
        notes = [r[1].replace("~", " ").replace("^", " ") for r in rows(out) if r[0] == "N"]
        self.assertTrue(any("results: 1st Ben 0:40.00, 2nd Ann 0:41.00" in n for n in notes), notes)
        # Alone in a heat: a time, no prize.
        status, out = await self.ask(c, "RACE FINISH 0 3000")
        self.assertEqual(rows(out)[0][0].split("^")[0].split("~")[0], "Your time: 0:50.00.")
        status, out = await self.ask(a, "RACE BOARD 0")
        self.assertEqual(rows(out)[3:6], [["1", "Ben", "0:40.00"], ["*2", "Ann", "0:41.00"], ["3", "Cat", "0:50.00"]])
        # Finishing again, outside a race, doesn't count.
        self.assertEqual((await self.ask(a, "RACE FINISH 0 1000"))[0], 1)

    async def test_quit(self):
        a, b = await self.connect("Ann", 1), await self.connect("Ben", 2)
        await self.ask(a, "RACE JOIN 1")
        await self.ask(b, "RACE JOIN 1")
        await self.ask(a, "RACE QUIT")
        self.now += 60
        status, out = await self.ask(b, "RACE FINISH 1 4000")
        self.assertIn(["+MONEY", str(online_racing.WIN_MONEY)], rows(out))
        self.assertTrue(any(r[0] == "N" and "gave up" in r[1] for r in rows(out)))
        self.assertEqual(self.server.race_heats, [])



class HideTest(OnlineTest):
    async def drain(self, c):
        lines = []
        while True:
            try:
                lines.append(await asyncio.wait_for(c.recv(), 0.2))
            except asyncio.TimeoutError:
                return lines

    async def test_game(self):
        a, b, c = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat"), 1)]
        woods = " ".join(map(str, MAPS["PETALBURG_WOODS"]))
        for x, at in ((a, "24 10"), (b, "8 33"), (c, "10 33")):
            await x.send(f"POS {woods} {at} 1 3 0")
        status, out = await self.ask(a, "HIDE JOIN 0")
        self.assertEqual(rows(out)[1][:5], [str(online_hide.SEEKER), "0", "0", "30", "120"])
        status, out = await self.ask(b, "HIDE LIST")
        self.assertEqual(rows(out)[1], ["PETALBURG WOODS: JOIN Ann", "0"])
        status, out = await self.ask(b, "HIDE JOIN 0")
        self.assertEqual(rows(out)[1][:3], [str(online_hide.HIDER), "0", "0"])
        await self.ask(c, "HIDE JOIN 0")
        self.now += online_hide.LOBBY
        status, out = await self.ask(a, "HIDE GO")
        self.assertEqual(rows(out)[0][1], "2")
        # Hiding: Ann doesn't see Ben, far away, but Cat sees him.
        await self.drain(a)
        await self.drain(c)
        await b.send(f"POS {woods} 9 33 1 3 0")
        self.assertIn(f"GONE {self.pid('Ben')}", await self.drain(a))
        self.assertTrue(any(l.startswith(f"PLAYER {self.pid('Ben')} ") for l in await self.drain(c)))
        # Not yet seeking.
        self.assertEqual((await self.ask(a, f"HIDE TAG {self.pid('Ben')}"))[0], 1)
        self.now += online_hide.HIDE
        # Ann walks up to Ben: she sees him again, and finds him.
        await self.drain(a)
        await a.send(f"POS {woods} 10 33 1 3 0")
        self.assertTrue(any(l.startswith(f"PLAYER {self.pid('Ben')} ") for l in await self.drain(a)))
        status, out = await self.ask(a, f"HIDE TAG {self.pid('Ben')}")
        self.assertEqual(rows(out)[0][1], "1")
        # Cat runs out of the woods: out.
        await c.send(f"POS {' '.join(map(str, MAPS['ROUTE104']))} 5 5 1 3 0")
        status, out = await self.ask(a, "OSTAT")
        self.assertIn(["S", "HIDE", "0", "0", "1", str(self.server.hide_games[0].id), str(online_hide.SEEK), "0"],
                      rows(out))
        status, out = await self.ask(a, "HIDE END")
        self.assertEqual(rows(out)[1], ["+MONEY", str(online_hide.FIND_MONEY)])
        status, out = await self.ask(b, "HIDE END")
        self.assertIn("found", rows(out)[0][0])
        status, out = await self.ask(c, "HIDE END")
        self.assertIn("out", rows(out)[0][0])
        status, out = await self.ask(a, "HIDEBOARD")
        self.assertEqual(rows(out)[3], ["*1", "Ann", str(online_hide.FIND_POINTS)])

    async def test_hidden_till_the_end(self):
        a, b = await self.connect("Ann", 1), await self.connect("Ben", 2)
        await self.ask(a, "HIDE JOIN 1")
        await self.ask(b, "HIDE JOIN 1")
        self.now += online_hide.LOBBY + online_hide.HIDE + online_hide.SEEK
        status, out = await self.ask(b, "HIDE END")
        self.assertEqual(rows(out)[1], ["+MONEY", str(online_hide.HIDDEN_MONEY)])
        status, out = await self.ask(a, "HIDE END")
        self.assertEqual(rows(out)[0][0], "You found 0 of 1.")
        self.assertEqual(self.server.hide_games[0].paid, {("Ann", 1), ("Ben", 2)})

    async def test_talents(self):
        a, b, c = [await self.connect(n, i) for i, n in enumerate(("Ann", "Ben", "Cat"), 1)]
        woods = " ".join(map(str, MAPS["PETALBURG_WOODS"]))
        for x, at in ((a, "10 20"), (b, "10 25"), (c, "13 20")):
            await x.send(f"POS {woods} {at} 1 3 0")
        # Ann's Kirlia senses hiders; Ben's Gastly helps him blend in.
        status, out = await self.ask(a, f"HIDE JOIN 0 {online_hide.SENSE} 0 {SPECIES['KIRLIA']}")
        self.assertIn("KIRLIA can sense", re.sub("[~^]", " ", rows(out)[0][0]))
        status, out = await self.ask(b, f"HIDE JOIN 0 0 {online_hide.BLEND_IN} {SPECIES['GASTLY']}")
        self.assertIn("GASTLY helps you blend in", re.sub("[~^]", " ", rows(out)[0][0]))
        await self.ask(c, "HIDE JOIN 0")
        self.now += online_hide.LOBBY + online_hide.HIDE
        game = self.server.hide_games[0]
        # Cat is 3 tiles away: seen. Ben, blending in, only from 1.
        ann, ben, cat = (self.server.players[self.pid(n)] for n in ("Ann", "Ben", "Cat"))
        self.assertTrue(self.server.can_see(ann, cat))
        ben.pos = (10, 22, 1, 3, 0)
        self.assertFalse(self.server.can_see(ann, ben))
        ben.pos = (10, 21, 1, 3, 0)
        self.assertTrue(self.server.can_see(ann, ben))
        # Sharp eyes see 2 tiles farther.
        game.seek_talent = online_hide.SHARP_EYES
        ben.pos = (10, 23, 1, 3, 0)
        self.assertTrue(self.server.can_see(ann, ben))
        # Sensing: HOT with Cat 3 tiles away; COLD once she's found and Ben is far.
        game.seek_talent = online_hide.SENSE
        status, out = await self.ask(a, "OSTAT")
        self.assertEqual(next(r for r in rows(out) if r[:2] == ["S", "HIDE"])[7], "3")
        game.found[cat.key] = "found"
        ben.pos = (10, 40, 1, 3, 0)
        status, out = await self.ask(a, "OSTAT")
        self.assertEqual(next(r for r in rows(out) if r[:2] == ["S", "HIDE"])[7], "1")
        # Hiders don't sense.
        status, out = await self.ask(b, "OSTAT")
        self.assertEqual(next(r for r in rows(out) if r[:2] == ["S", "HIDE"])[7], "0")

    async def test_nobody_came(self):
        a = await self.connect("Ann", 1)
        await self.ask(a, "HIDE JOIN 0")
        self.now += online_hide.LOBBY
        status, out = await self.ask(a, "HIDE GO")
        self.assertEqual(rows(out)[0][1], "0")
        self.assertEqual(self.server.hide_games, [])

    def pid(self, name):
        return next(p.id for p in self.server.players.values() if p.name == name)


if __name__ == "__main__":
    unittest.main()
