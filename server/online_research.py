"""Research Pokédex leaders: the game keeps each player's research
(src/research.c, offline too); online it reports how many species the
player has completed (`OEV RESEARCH completed species`), and HUB >
RESEARCH LEADERS shows the top researchers."""

from online import answer, board
from online_ids import SPECIES_NAMES

BOARD_SIZE = 8
ANNOUNCE_EVERY = 10  # everyone hears about every 10th completed species


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS research_players (
            name TEXT NOT NULL, tid INTEGER NOT NULL, done INTEGER NOT NULL,
            last_species INTEGER NOT NULL, at REAL NOT NULL,
            PRIMARY KEY (name, tid)
        );
        """
    )


def species_name(species):
    return SPECIES_NAMES.get(species, f"No.{species}").replace("_", " ")


def record(ctx, done, species):
    db = ctx.db
    row = db.execute("SELECT done FROM research_players WHERE name = ? AND tid = ?", ctx.key).fetchone()
    before = row[0] if row else 0
    db.execute("INSERT INTO research_players (name, tid, done, last_species, at) VALUES (?, ?, ?, ?, ?)"
               " ON CONFLICT (name, tid) DO UPDATE SET done = excluded.done,"
               " last_species = CASE WHEN excluded.last_species > 0 THEN excluded.last_species ELSE last_species END,"
               " at = excluded.at",
               (*ctx.key, done, species, ctx.online.now()))
    return before


def on_research(ctx, args):
    """RESEARCH completed species: the player completed `species`."""
    done, species = (args + [0, 0])[:2]
    if done <= 0:
        return
    before = record(ctx, done, species)
    if done > before and done % ANNOUNCE_EVERY == 0:
        ctx.online.announce(f"{ctx.player.name} has completed the research on {done} Pok#mon species!",
                            exclude=ctx.key)


def cmd_research(ctx, args, data):
    """RESEARCH BOARD <completed>"""
    if args[0] != "BOARD":
        raise ValueError("bad RESEARCH")
    done = int(args[1]) if len(args) > 1 else 0
    if done > 0:
        record(ctx, done, 0)
    top = ctx.db.execute("SELECT name, tid, done, last_species FROM research_players"
                         " ORDER BY done DESC, at LIMIT ?", (BOARD_SIZE,)).fetchall()
    rows = []
    for i, (name, tid, n, last) in enumerate(top, 1):
        mark = "*" if (name, tid) == tuple(ctx.key) else ""
        rows.append([f"{mark}{i}. {name}", str(n), species_name(last) if last else "-"])
    return answer(rows=board("RESEARCH LEADERS", [(6, "TRAINER"), (100, "DONE"), (136, "LATEST")], rows))


COMMANDS = {"RESEARCH": cmd_research}
EVENTS = {"RESEARCH": on_research}
