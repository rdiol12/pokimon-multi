"""Spectator betting with play money: when a PvP battle starts, everyone
else online hears of it and can bet Game Corner COINS on either side for
the first two minutes (HUB > BETTING). When the battle ends the pool goes
to those who backed the winner, in proportion to their bets; after a draw,
a dispute or a battle nobody backed the winner of, every bet is returned.
Winnings wait in HUB > BETTING. A player with no coins gets 100 from the
house once a day.

Battles are the plaza's PvP sessions (server/netplay_server.py): install()
wraps the server's accept and finish so this module hears of both.
"""

from online import Refused, answer, board

BET_SECONDS = 120
AMOUNTS = (10, 50, 100, 500)
FREE_COINS = 100
MAX_COINS = 9999
BOARD_SIZE = 8


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_bets (
            battle INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, side TEXT NOT NULL,
            coins INTEGER NOT NULL, PRIMARY KEY (battle, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_bet_payouts (
            name TEXT NOT NULL, tid INTEGER NOT NULL, coins INTEGER NOT NULL, why TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS online_bettors (
            name TEXT NOT NULL, tid INTEGER NOT NULL, won INTEGER NOT NULL DEFAULT 0,
            bets INTEGER NOT NULL DEFAULT 0, free_day INTEGER NOT NULL DEFAULT -1, PRIMARY KEY (name, tid)
        );
        """
    )


def battles(server):
    """Battles open for betting: session id -> (host name, guest name, started at)."""
    if not hasattr(server, "bet_battles"):
        server.bet_battles = {}
    return server.bet_battles


def pools(db, sid):
    out = {"h": 0, "g": 0}
    for side, coins in db.execute("SELECT side, SUM(coins) FROM online_bets WHERE battle = ? GROUP BY side", (sid,)):
        out[side] = coins
    return out


def odds(pool, side):
    """What one coin on `side` brings back, as text ("x2.5")."""
    total = pool["h"] + pool["g"]
    if not pool[side]:
        return "-"
    return f"x{total / pool[side]:.1f}".replace(".0", "")


def settle(server, session):
    """The battle is over: pays out its bets."""
    online = server.online
    db = server.league.db
    names = battles(server).pop(session.id, None)
    bets = db.execute("SELECT name, tid, side, coins FROM online_bets WHERE battle = ?", (session.id,)).fetchall()
    if names is None or not bets:
        return
    h, g = session.reports.get(session.host.key), session.reports.get(session.guest.key)
    winner = None
    if (h, g) == ("win", "loss") or (h == "win" and g is None) or (h is None and g == "loss"):
        winner = "h"
    elif (h, g) == ("loss", "win") or (h == "loss" and g is None) or (h is None and g == "win"):
        winner = "g"
    pool = pools(db, session.id)
    title = f"{names[0]} vs {names[1]}"
    if winner is None or not pool[winner]:
        for name, tid, side, coins in bets:
            db.execute("INSERT INTO online_bet_payouts VALUES (?, ?, ?, ?)", (name, tid, coins, f"{title}: refund"))
            online.notify((name, tid), f"{title} had no winner to pay. Your {coins} COINS are back"
                                       " in HUB > BETTING.")
    else:
        total = pool["h"] + pool["g"]
        champ = names[0] if winner == "h" else names[1]
        for name, tid, side, coins in bets:
            if side != winner:
                db.execute("UPDATE online_bettors SET won = won - ? WHERE name = ? AND tid = ?", (coins, name, tid))
                online.notify((name, tid), f"{champ} won {title}. Your {coins} COINS are lost!")
                continue
            won = min(MAX_COINS, coins * total // pool[winner])
            db.execute("INSERT INTO online_bet_payouts VALUES (?, ?, ?, ?)", (name, tid, won, title))
            db.execute("UPDATE online_bettors SET won = won + ? WHERE name = ? AND tid = ?", (won - coins, name, tid))
            online.notify((name, tid), f"{champ} won {title}! You won {won} COINS. Collect them in HUB > BETTING.")
    db.execute("DELETE FROM online_bets WHERE battle = ?", (session.id,))
    db.commit()


def install(server):
    """Hooks the plaza's battles: their start opens betting, their end settles it."""
    accept, finish = server.cmd_accept, server.finish_battle

    def cmd_accept(player, sid):
        accept(player, sid)
        session = server.sessions.get(int(sid))
        if session and session.kind == "battle" and session.started and hasattr(server, "online"):
            battles(server)[session.id] = (session.host.name, session.guest.name, server.online.now())
            for p in server.players.values():
                if p not in (session.host, session.guest) and p.name is not None:
                    server.online.notify(p.key, f"{session.host.name} vs {session.guest.name} is on! Bet"
                                                " in HUB > BETTING, or watch it in ONLINE > WATCH.")

    def finish_battle(session):
        finish(session)
        settle(server, session)

    server.cmd_accept = cmd_accept
    server.finish_battle = finish_battle


def bettor(db, key):
    db.execute("INSERT OR IGNORE INTO online_bettors (name, tid) VALUES (?, ?)", (*key,))
    return db.execute("SELECT won, bets, free_day FROM online_bettors WHERE name = ? AND tid = ?", (*key,)).fetchone()


def payouts(db, key):
    return db.execute("SELECT rowid, coins FROM online_bet_payouts WHERE name = ? AND tid = ?", (*key,)).fetchall()


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def cmd_bet(ctx, args, data):
    db, key = ctx.db, ctx.key
    now = ctx.online.now()
    word = args[0]
    open_battles = battles(ctx.server)

    if word == "LIST":
        # coins
        coins = number(args, 1)
        won, bets, free_day = bettor(db, key)
        rows = []
        if coins == 0 and free_day != ctx.online.day():
            db.execute("UPDATE online_bettors SET free_day = ? WHERE name = ? AND tid = ?", (ctx.online.day(), *key))
            rows.append(["+COINS", FREE_COINS])
            ctx.online.notify(key, f"The house gave you {FREE_COINS} COINS to play with!")
        for sid, (hn, gn, at) in sorted(open_battles.items()):
            session = ctx.server.sessions.get(sid)
            if session is None or now - at > BET_SECONDS or ctx.player in (session.host, session.guest):
                continue
            left = int(BET_SECONDS - (now - at))
            rows.append([f"{hn} vs {gn}", sid, left])
        n = len([r for r in rows if r[0] != "+COINS"])
        waiting = sum(c for _, c in payouts(db, key))
        message = "Which battle will you bet on?" if n else "No battles are open for bets right now."
        if waiting:
            message += f"~{waiting} COINS are waiting for you!"
        return answer(message, code=n, vars=[waiting], rows=rows)

    if word == "SHOW":
        sid = number(args, 1)
        if sid not in open_battles or now - open_battles[sid][2] > BET_SECONDS:
            raise Refused("Bets on that battle are closed.")
        ctx.player.bet_battle = sid
        hn, gn, at = open_battles[sid]
        pool = pools(db, sid)
        mine = db.execute("SELECT side, coins FROM online_bets WHERE battle = ? AND name = ? AND tid = ?",
                          (sid, *key)).fetchone()
        message = f"{hn} vs {gn}. Bets: {pool['h']} on {hn}, {pool['g']} on {gn}."
        if mine:
            message += f"~You bet {mine[1]} on {hn if mine[0] == 'h' else gn}."
        rows = [[f"{hn} ({odds(pool, 'h')})", "h"], [f"{gn} ({odds(pool, 'g')})", "g"]]
        return answer(message, rows=rows)

    if word == "PLACE":
        # amount side coins
        amount, side, coins = number(args, 1), args[2] if len(args) > 2 else "", number(args, 3)
        sid = getattr(ctx.player, "bet_battle", None)
        if sid not in open_battles or now - open_battles[sid][2] > BET_SECONDS:
            raise Refused("Bets on that battle are closed.")
        if side not in ("h", "g") or amount not in AMOUNTS:
            raise Refused("That bet can't be placed.")
        if amount > coins:
            raise Refused(f"You only have {coins} COINS.")
        mine = db.execute("SELECT side FROM online_bets WHERE battle = ? AND name = ? AND tid = ?",
                          (sid, *key)).fetchone()
        if mine and mine[0] != side:
            raise Refused("You already backed the other side!")
        db.execute("INSERT INTO online_bets VALUES (?, ?, ?, ?, ?) ON CONFLICT (battle, name, tid)"
                   " DO UPDATE SET coins = coins + excluded.coins", (sid, *key, side, amount))
        bettor(db, key)
        db.execute("UPDATE online_bettors SET bets = bets + 1 WHERE name = ? AND tid = ?", (*key,))
        hn, gn, at = open_battles[sid]
        who = hn if side == "h" else gn
        return answer(f"You bet {amount} COINS on {who}! Good luck!", rows=[["-COINS", amount]])

    if word == "COLLECT":
        got = payouts(db, key)
        if not got:
            raise Refused("No winnings are waiting.")
        total = sum(c for _, c in got)
        db.execute("DELETE FROM online_bet_payouts WHERE name = ? AND tid = ?", (*key,))
        return answer(f"{ctx.player.name} collected {total} COINS!", rows=[["+COINS", total]])

    if word == "BOARD":
        rows = []
        top = db.execute("SELECT name, tid, won, bets FROM online_bettors WHERE bets > 0 ORDER BY won DESC, name"
                         " LIMIT ?", (BOARD_SIZE,)).fetchall()
        for place, (n, t, won, bets) in enumerate(top, 1):
            rows.append([f"{'*' if (n, t) == tuple(key) else ''}{place}", n, bets, f"{won:+d}"])
        return answer(rows=board("BEST BETTORS", [(6, "No."), (24, "TRAINER"), (110, "BETS"), (150, "COINS WON")],
                                 rows))

    raise ValueError("bad BET")


COMMANDS = {"BET": cmd_bet}
