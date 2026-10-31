import asyncio
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))
import ext  # noqa: E402
import netplay_server as ns  # noqa: E402
import online  # noqa: E402
import online_betting  # noqa: E402
import online_coopfish  # noqa: E402
import online_duo  # noqa: E402
import online_fishing  # noqa: E402
import online_goals  # noqa: E402
import online_mentors  # noqa: E402
import online_pageant  # noqa: E402
import online_pquests  # noqa: E402
import online_shops  # noqa: E402
import online_sightings  # noqa: E402
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
                                     online_pquests, online_betting], clock=lambda: self.now)
        online_betting.install(self.server)
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
    async def battle(self, host, guest):
        guest_id = next(p.id for p in self.server.players.values() if p.name == guest.name)
        await host.send(f"REQ battle {guest_id}")
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


if __name__ == "__main__":
    unittest.main()
