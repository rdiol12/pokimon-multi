"""Player shops: every player can run a shop with up to six shelves of items
from their bag, each at a price they pick. Other players browse the shops
(HUB > PLAYER SHOPS) and buy from them, even while the owner is offline; the
money waits on the server until the owner collects it, and an owner who is
online hears of each sale.

The market board (server/market.py) and the auction house sell Pokémon;
shops sell items.
"""

from online import Refused, answer, board
from online_ids import ITEMS

SHELVES = 6
MAX_PRICE = 99999
MAX_COUNT = 99
LIST_SIZE = 20
ITEM_NAMES = {v: k for k, v in ITEMS.items()}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_shops (
            id INTEGER PRIMARY KEY, name TEXT NOT NULL, tid INTEGER NOT NULL,
            earnings INTEGER NOT NULL DEFAULT 0, sold INTEGER NOT NULL DEFAULT 0,
            UNIQUE (name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_shop_shelves (
            id INTEGER PRIMARY KEY, shop INTEGER NOT NULL, item INTEGER NOT NULL,
            count INTEGER NOT NULL, price INTEGER NOT NULL
        );
        """
    )


def item_name(item):
    """As the game writes it: "Super Potion", "Poké Ball"."""
    name = ITEM_NAMES.get(item, "?").replace("_", " ").title()
    return "Poké" + name[4:] if name.startswith("Poke ") else name


def shop_of(db, key, create=False):
    row = db.execute("SELECT id, earnings, sold FROM online_shops WHERE name = ? AND tid = ?", (*key,)).fetchone()
    if row is None and create:
        db.execute("INSERT INTO online_shops (name, tid) VALUES (?, ?)", (*key,))
        return shop_of(db, key)
    return row


def shelves(db, shop):
    return db.execute("SELECT id, item, count, price FROM online_shop_shelves WHERE shop = ? ORDER BY id",
                      (shop,)).fetchall()


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def shelf_row(shelf):
    """A pick-list row: label, shelf id, item, count, price."""
    sid, item, count, price = shelf
    return [f"{item_name(item)} x{count} ${price}", sid, item, count, price]


def cmd_shop(ctx, args, data):
    db, key = ctx.db, ctx.key
    word = args[0]

    if word == "LIST":
        shops = db.execute(
            "SELECT s.id, s.name, s.sold, COUNT(h.id) FROM online_shops s JOIN online_shop_shelves h ON h.shop = s.id"
            " WHERE NOT (s.name = ? AND s.tid = ?) GROUP BY s.id ORDER BY s.sold DESC, s.name LIMIT ?",
            (*key, LIST_SIZE)).fetchall()
        if not shops:
            raise Refused("No other trainer has a shop open yet. Why not open one of your own?")
        rows = [[f"{name}'s Shop", sid] for sid, name, sold, n in shops]
        return answer("Whose shop would you like to visit?", code=len(rows), rows=rows)

    if word in ("OPEN", "ITEMS"):
        # ITEMS without a shop is the one opened last (after a purchase the
        # game's picked row is the shelf, not the shop).
        sid = number(args, 1) if len(args) > 1 else getattr(ctx.player, "online_shop", 0)
        ctx.player.online_shop = sid
        owner = db.execute("SELECT name, sold FROM online_shops WHERE id = ?", (sid,)).fetchone()
        stock = shelves(db, sid)
        if owner is None or not stock:
            raise Refused("This shop has sold out!")
        if word == "ITEMS":
            return answer("What would you like?", code=len(stock), rows=[shelf_row(s) for s in stock])
        lines = [[item_name(item), count, f"${price}"] for _, item, count, price in stock]
        sold = f" {owner[1]} sold so far!" if owner[1] else ""
        return answer(f"Welcome to {owner[0]}'s shop!{sold}",
                      rows=board(f"{owner[0]}'s Shop", [(6, "ITEM"), (120, "LEFT"), (156, "PRICE")], lines))

    if word == "BUY":
        # item count shelf money
        item, count, shelf_id, money = (number(args, i) for i in range(1, 5))
        shelf = db.execute("SELECT h.shop, h.item, h.count, h.price, s.name, s.tid FROM online_shop_shelves h"
                           " JOIN online_shops s ON s.id = h.shop WHERE h.id = ?", (shelf_id,)).fetchone()
        if shelf is None or shelf[1] != item:
            raise Refused("Sorry, somebody just bought the last of those!")
        shop, item, left, price, seller, seller_tid = shelf
        if (seller, seller_tid) == key:
            raise Refused("That's your own shop!")
        count = max(1, min(count, left))
        cost = price * count
        if cost > money:
            raise Refused(f"You don't have enough money. {count} costs ${cost}.")
        if count == left:
            db.execute("DELETE FROM online_shop_shelves WHERE id = ?", (shelf_id,))
        else:
            db.execute("UPDATE online_shop_shelves SET count = count - ? WHERE id = ?", (count, shelf_id))
        db.execute("UPDATE online_shops SET earnings = earnings + ?, sold = sold + ? WHERE id = ?",
                   (cost, count, shop))
        what = f"{item_name(item)} x{count}"
        ctx.online.notify((seller, seller_tid), f"{ctx.player.name} bought {what} at your shop for ${cost}!"
                                                " Collect it in HUB > PLAYER SHOPS.")
        return answer(f"{ctx.player.name} bought {what} for ${cost}. Thank you!",
                      rows=[["-MONEY", cost], ["+ITEM", item, count]])

    if word == "MINE":
        shop = shop_of(db, key)
        stock = shelves(db, shop[0]) if shop else []
        earnings = shop[1] if shop else 0
        message = f"Shelves stocked: {len(stock)} of {SHELVES}."
        if earnings:
            message += f"~${earnings} is waiting for you!"
        return answer(message, code=earnings, vars=[earnings], rows=[shelf_row(s) for s in stock])

    if word == "PRICES":
        # item count storePrice: four prices to pick from, around the store's.
        base = number(args, 3) or 1000
        prices = sorted({max(1, min(MAX_PRICE, round(base * f / 10) * 10 or 10)) for f in (0.5, 1, 1.5, 2)})
        return answer(f"How much for each {item_name(number(args, 1))}?",
                      rows=[[f"${p} each", p] for p in prices])

    if word == "STOCK":
        # item count price
        item, count, price = (number(args, i) for i in range(1, 4))
        if item not in ITEM_NAMES or not 1 <= count <= MAX_COUNT or not 1 <= price <= MAX_PRICE:
            raise Refused("That can't go on the shelf.")
        shop = shop_of(db, key, create=True)
        stock = shelves(db, shop[0])
        same = [s for s in stock if s[1] == item and s[3] == price]
        if same:
            db.execute("UPDATE online_shop_shelves SET count = MIN(count + ?, ?) WHERE id = ?",
                       (count, MAX_COUNT, same[0][0]))
        elif len(stock) >= SHELVES:
            raise Refused(f"Your shop has only {SHELVES} shelves. Take something back first.")
        else:
            db.execute("INSERT INTO online_shop_shelves (shop, item, count, price) VALUES (?, ?, ?, ?)",
                       (shop[0], item, count, price))
        return answer(f"{item_name(item)} x{count} is on your shelf for ${price} each.",
                      rows=[["-ITEM", item, count]])

    if word == "TAKE":
        shelf_id = number(args, 1)
        shop = shop_of(db, key)
        shelf = db.execute("SELECT item, count FROM online_shop_shelves WHERE id = ? AND shop = ?",
                           (shelf_id, shop[0] if shop else -1)).fetchone()
        if shelf is None:
            raise Refused("That shelf is already empty.")
        db.execute("DELETE FROM online_shop_shelves WHERE id = ?", (shelf_id,))
        return answer(f"You took back {item_name(shelf[0])} x{shelf[1]}.", rows=[["+ITEM", shelf[0], shelf[1]]])

    if word == "COLLECT":
        shop = shop_of(db, key)
        if not shop or not shop[1]:
            raise Refused("Nothing has sold since you last collected.")
        db.execute("UPDATE online_shops SET earnings = 0 WHERE id = ?", (shop[0],))
        return answer(f"{ctx.player.name} collected ${shop[1]} from the shop!", rows=[["+MONEY", shop[1]]])

    raise ValueError("bad SHOP")


COMMANDS = {"SHOP": cmd_shop}
