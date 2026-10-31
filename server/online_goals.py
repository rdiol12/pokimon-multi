"""Server-wide catch goals: every catch online counts towards the week's
goals. When a goal is reached, everyone who caught at least one Pokémon
that week can collect its reward from HUB > CATCH GOALS."""

from online import Refused, answer, board
from online_ids import ITEMS, SPECIES

# Featured species, one per week: common enough to find anywhere in Hoenn.
FEATURED = ["ZIGZAGOON", "WINGULL", "TAILLOW", "MARILL", "ODDISH", "GEODUDE", "ZUBAT", "TENTACOOL",
            "WURMPLE", "LOTAD", "SEEDOT", "MAKUHITA", "ELECTRIKE", "SWABLU", "NUMEL", "SPINDA"]

# (id, label, target, reward item, count)
GOALS = [
    ("catch", "Catch {n} Pok#mon", 300, "ULTRA_BALL", 10),
    ("fish", "Fish up {n} Pok#mon", 100, "DIVE_BALL", 10),
    ("featured", "Catch {n} {species}", 40, "RARE_CANDY", 3),
    ("shiny", "Find {n} shiny Pok#mon", 3, "MASTER_BALL", 1),
]


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_goal_counts (
            week INTEGER NOT NULL, goal TEXT NOT NULL, count INTEGER NOT NULL,
            PRIMARY KEY (week, goal)
        );
        CREATE TABLE IF NOT EXISTS online_goal_helpers (
            week INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, catches INTEGER NOT NULL,
            PRIMARY KEY (week, name, tid)
        );
        CREATE TABLE IF NOT EXISTS online_goal_claims (
            week INTEGER NOT NULL, goal TEXT NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            PRIMARY KEY (week, goal, name, tid)
        );
        """
    )


def featured(week):
    return FEATURED[week % len(FEATURED)]


def label(goal, week):
    gid, text, target, *_ = goal
    return text.format(n=target, species=featured(week))


def count(db, week, gid):
    row = db.execute("SELECT count FROM online_goal_counts WHERE week = ? AND goal = ?", (week, gid)).fetchone()
    return row[0] if row else 0


def helped(db, week, key):
    row = db.execute("SELECT catches FROM online_goal_helpers WHERE week = ? AND name = ? AND tid = ?",
                     (week, *key)).fetchone()
    return row[0] if row else 0


def claimable(db, week, key):
    if not helped(db, week, key):
        return []
    out = []
    for goal in GOALS:
        if count(db, week, goal[0]) >= goal[2] and not db.execute(
                "SELECT 1 FROM online_goal_claims WHERE week = ? AND goal = ? AND name = ? AND tid = ?",
                (week, goal[0], *key)).fetchone():
            out.append(goal)
    return out


def bump(ctx, week, goal):
    db = ctx.db
    before = count(db, week, goal[0])
    db.execute("INSERT INTO online_goal_counts (week, goal, count) VALUES (?, ?, 1)"
               " ON CONFLICT (week, goal) DO UPDATE SET count = count + 1", (week, goal[0]))
    if before + 1 == goal[2]:
        ctx.online.announce(f"The server reached a goal: {label(goal, week)}!~"
                            f"Everyone who helped can collect it in HUB > CATCH GOALS.")


def on_catch(ctx, args):
    """CATCH species shiny mapGroup mapNum fished"""
    species, shiny, _, _, fished = (args + [0] * 5)[:5]
    week, db = ctx.online.week(), ctx.db
    db.execute("INSERT INTO online_goal_helpers (week, name, tid, catches) VALUES (?, ?, ?, 1)"
               " ON CONFLICT (week, name, tid) DO UPDATE SET catches = catches + 1", (week, *ctx.key))
    goals = {g[0]: g for g in GOALS}
    bump(ctx, week, goals["catch"])
    if fished:
        bump(ctx, week, goals["fish"])
    if species == SPECIES.get(featured(week)):
        bump(ctx, week, goals["featured"])
    if shiny:
        bump(ctx, week, goals["shiny"])


def reward_name(goal):
    item, n = goal[3].replace("_", " "), goal[4]
    if n > 1:
        item = item[:-1] + "IES" if item.endswith("Y") else item + "S"
    return f"{n} {item}"


def cmd_goal(ctx, args, data):
    db, key, week = ctx.db, ctx.key, ctx.online.week()
    if args[0] == "VIEW":
        rows = []
        for goal in GOALS:
            n = min(count(db, week, goal[0]), goal[2])
            rows.append(["%", n * 100 // goal[2], label(goal, week), reward_name(goal), f"{n}/{goal[2]}"])
        mine = helped(db, week, key)
        lines = board("CATCH GOALS", [(10, f"You've caught {mine} this week. Rewards:"), (150, "")], rows)
        return answer(code=len(claimable(db, week, key)), rows=lines)
    if args[0] == "CLAIM":
        goals = claimable(db, week, key)
        if not goals:
            raise Refused("There's nothing to collect yet.")
        rows = []
        for goal in goals:
            db.execute("INSERT INTO online_goal_claims (week, goal, name, tid) VALUES (?, ?, ?, ?)",
                       (week, goal[0], *key))
            rows.append(["+ITEM", ITEMS[goal[3]], goal[4]])
        names = " and ".join(reward_name(g) for g in goals)
        return answer(f"{ctx.player.name} got {names}!", rows=rows)
    raise ValueError("bad GOAL")


COMMANDS = {"GOAL": cmd_goal}
EVENTS = {"CATCH": on_catch}
