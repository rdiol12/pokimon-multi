"""The living world on the server (docs/alive.md): the news the town criers,
newspapers and TVs report, and the town projects every player builds
together.

ANEWS: the latest headlines, newest first. One row each: age in minutes|text.
The news is everything the server announces to players online (shiny
catches, sightings, records, ladder results...), the rovers' news (catches,
the Rover League) and the town projects. Players' good deeds are reported
with AEV.

APROJ INFO: one row per town project: project|given|goal|done (1) and the
name of the biggest donor.
APROJ GIVE <project> <amount>: the player gives money (the game takes it
off them once the answer is ok). Answer: one row: given|goal|done|1 if
this gift finished it.
AEV <kind> <map group> <map num>: a good deed for the news (1 lost child
walked home, 2 Pokémon rescued, 3 stampede seen).
"""

import time

from ext import Refused, text

now = time.time  # tests replace this

MAX_NEWS = 40
MAX_GIFT = 100000

# Index = ALIVE_PROJECT_* (include/constants/alive.h).
PROJECTS = [
    ("OLDALE TOWN", "flower garden", 150000),
    ("PETALBURG CITY", "pond park", 200000),
    ("RUSTBORO CITY", "founder's statue", 250000),
    ("SLATEPORT CITY", "market stalls", 300000),
    ("MAUVILLE CITY", "city plaza", 300000),
    ("FORTREE CITY", "rope bridge", 350000),
    ("LILYCOVE CITY", "seaside promenade", 400000),
    ("DEWFORD TOWN", "fishing pier", 200000),
]

DEEDS = {
    1: "{name} walked a lost child home safely near {place}.",
    2: "{name} rescued a Pokemon in trouble near {place}.",
    3: "A stampede of wild Pokemon thundered across {place}!",
    4: "{name} found a washed-up crate on the shore near {place}.",
}


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS alive_news (
            id INTEGER PRIMARY KEY, at REAL NOT NULL, line TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS alive_projects (
            project INTEGER PRIMARY KEY, given INTEGER NOT NULL DEFAULT 0, done_at REAL
        );
        CREATE TABLE IF NOT EXISTS alive_donors (
            project INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL, given INTEGER NOT NULL,
            PRIMARY KEY (project, name, tid)
        );
        """
    )


def plain(line):
    """News lines in the game's ASCII: no accents, no line breaks."""
    line = str(line).replace("é", "e").replace("É", "E").replace("\n", " ").replace("~", " ").replace("^", " ")
    return " ".join(line.split())


def add_news(db, line):
    db.execute("INSERT INTO alive_news (at, line) VALUES (?, ?)", (now(), plain(line)))
    db.execute("DELETE FROM alive_news WHERE id <= (SELECT MAX(id) FROM alive_news) - ?", (MAX_NEWS,))


def headlines(db, most=12):
    rows = list(db.execute("SELECT at, line FROM alive_news ORDER BY id DESC LIMIT ?", (most,)))
    try:
        rows += list(db.execute("SELECT at, line FROM rover_news ORDER BY id DESC LIMIT ?", (most // 2,)))
    except Exception:  # the rovers' table isn't there (tests without rovers)
        pass
    rows.sort(key=lambda r: -r[0])
    return rows[:most]


def place_name(group, num):
    try:
        import online_sightings
        return online_sightings.place_name(group, num)
    except Exception:
        return "a route"


def cmd_news(ctx, args, data):
    t = now()
    return text([[max(0, int((t - at) // 60)), line] for at, line in headlines(ctx.db)])


def project_row(db, p):
    row = db.execute("SELECT given, done_at FROM alive_projects WHERE project = ?", (p,)).fetchone()
    given, done_at = row if row else (0, None)
    top = db.execute("SELECT name FROM alive_donors WHERE project = ? ORDER BY given DESC LIMIT 1", (p,)).fetchone()
    return [p, given, PROJECTS[p][2], 1 if done_at else 0, top[0] if top else ""]


def number(args, i):
    try:
        return int(args[i])
    except (IndexError, ValueError):
        raise Refused("That didn't work. Please try again.")


def cmd_project(ctx, args, data):
    db = ctx.db
    word = args[0] if args else "INFO"
    if word == "INFO":
        return text([project_row(db, p) for p in range(len(PROJECTS))])
    if word == "GIVE":
        p, amount = number(args, 1), number(args, 2)
        if not 0 <= p < len(PROJECTS) or not 0 < amount <= MAX_GIFT:
            raise Refused("That didn't work. Please try again.")
        row = project_row(db, p)
        if row[3]:
            raise Refused("It's already built. Thank you all the same!")
        town, what, goal = PROJECTS[p]
        given = min(goal, row[1] + amount)
        finished = given >= goal
        db.execute("INSERT INTO alive_projects (project, given) VALUES (?, 0) ON CONFLICT(project) DO NOTHING", (p,))
        db.execute("UPDATE alive_projects SET given = ?, done_at = ? WHERE project = ?",
                   (given, now() if finished else None, p))
        name, tid = ctx.key
        db.execute("INSERT INTO alive_donors (project, name, tid, given) VALUES (?, ?, ?, ?)"
                   " ON CONFLICT(project, name, tid) DO UPDATE SET given = given + excluded.given",
                   (p, name, tid, amount))
        if finished:
            line = f"{town}'s {what} is finished! Thank you, everyone who gave."
            add_news(db, line)
            online = getattr(ctx.server, "online", None)
            if online is not None:
                online.announce(line)
        elif row[1] * 4 // goal != given * 4 // goal:
            add_news(db, f"{town}'s {what} is {given * 100 // goal}% funded. Every bit helps!")
        return text([[given, goal, 1 if finished else 0, 1 if finished else 0]])
    raise Refused("That didn't work. Please try again.")


def cmd_event(ctx, args, data):
    kind, group, num = number(args, 0), number(args, 1), number(args, 2)
    fmt = DEEDS.get(kind)
    if fmt:
        add_news(ctx.db, fmt.format(name=ctx.player.name, place=place_name(group, num)))
    return text([])


COMMANDS = {"ANEWS": cmd_news, "APROJ": cmd_project, "AEV": cmd_event}


def install(online):
    """Everything the server announces to players also goes in the news."""
    announce = online.announce
    db = online.server.league.db

    def announce_and_report(message, exclude=None):
        add_news(db, message)
        return announce(message, exclude)

    online.announce = announce_and_report
