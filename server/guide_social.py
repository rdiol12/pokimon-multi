"""The guide's social side (docs/guide.md).

- GUIDE INVITE: tells everyone on your friend list (and everyone who has you
  on theirs) that you're online. Friends online get a notice now, the rest
  the next time they play. The title menu's INVITE FRIENDS sends it as soon
  as the game is online.
- GUIDE NEARBY: the players online, nearest first by the region map, with
  where they are. Picking one (GUIDE FRIEND <id>) adds them as a friend.
- In the background (status polls): a notice when another player comes
  near you, and new trainers (fewer than six badges) are paired with a free
  mentor automatically. Saying goodbye to that mentor stops it for good.
  Trainers who reach six badges are told once that they can mentor.
"""

from online import Refused, answer
from online_ids import MAP_NAMES, MAP_SECTIONS, SECTION_GRID
import online_mentors as mentors

NEAR_CELLS = 2          # region map cells that count as "nearby"
NEAR_NOTICE_GAP = 1800  # seconds before the same player is announced again
NEARBY_ROWS = 10


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS guide_auto_mentor (
            name TEXT NOT NULL, tid INTEGER NOT NULL, mentor_name TEXT, mentor_tid INTEGER,
            declined INTEGER NOT NULL DEFAULT 0, PRIMARY KEY (name, tid)
        );
        CREATE TABLE IF NOT EXISTS guide_mentor_told (
            name TEXT NOT NULL, tid INTEGER NOT NULL, PRIMARY KEY (name, tid)
        );
        """
    )


# --- where players are --------------------------------------------------

def cell(map_):
    """The middle of the map's region map section, in cells, or None."""
    if map_ is None:
        return None
    section = MAP_SECTIONS.get(MAP_NAMES.get(tuple(map_)))
    if section not in SECTION_GRID:
        return None
    x, y, w, h, _ = SECTION_GRID[section]
    if w == 0 or x > 28 or y > 19:
        return None
    return x + w / 2, y + h / 2


def place(map_):
    name = MAP_NAMES.get(tuple(map_)) if map_ else None
    if name is None:
        return "somewhere"
    section = MAP_SECTIONS.get(name)
    if section in SECTION_GRID and SECTION_GRID[section][4]:
        return SECTION_GRID[section][4]
    return name.replace("_", " ")


def distance(a, b):
    if a is None or b is None:
        return None
    return max(abs(a[0] - b[0]), abs(a[1] - b[1]))


def direction(src, dst):
    dx, dy = dst[0] - src[0], dst[1] - src[1]
    ns = "north" if dy < -0.5 else "south" if dy > 0.5 else ""
    ew = "west" if dx < -0.5 else "east" if dx > 0.5 else ""
    return f"{ns}-{ew}" if ns and ew else ns or ew or "here"


def others(ctx):
    return [p for p in ctx.server.players.values()
            if p.name is not None and p is not ctx.player and p.map is not None]


def nearest(ctx):
    """[(distance or 99, player)] nearest first."""
    me = cell(ctx.player.map)
    out = []
    for p in others(ctx):
        d = distance(me, cell(p.map))
        out.append((99 if d is None else d, p))
    out.sort(key=lambda dp: (dp[0], dp[1].name))
    return out


# --- requests -------------------------------------------------------------

def invite(ctx):
    db, key = ctx.db, ctx.key
    friends = {tuple(f) for f in ctx.server.friends.list(key)}
    friends |= {tuple(r) for r in db.execute("SELECT owner_name, owner_tid FROM friends"
                                             " WHERE friend_name = ? AND friend_tid = ?", key)}
    friends.discard(tuple(key))
    if not friends:
        raise Refused("You have no friends on your list yet.~Talk to a player and pick ADD FRIEND,"
                      " or try PLAYERS NEARBY.")
    online_now = {p.key: p for p in others(ctx)}
    name = ctx.player.name
    where = place(ctx.player.map)
    here = []
    for friend in sorted(friends):
        if friend in online_now:
            here.append(friend[0])
            ctx.online.notify(friend, f"{name} is online at {where} and invites you to play! "
                                      "ONLINE > FRIENDS takes you there.")
        else:
            ctx.online.notify(friend, f"{name} invited you to play. Check ONLINE > FRIENDS!")
    n = len(friends)
    message = f"You invited {n} friend{'' if n == 1 else 's'}."
    if here:
        message += f" Online now: {', '.join(here[:4])}{'...' if len(here) > 4 else ''}."
    else:
        message += " They'll hear of it next time they play."
    return answer(message, code=len(here))


def nearby(ctx):
    found = nearest(ctx)
    if not found:
        return answer("Nobody else is online right now.~Invite your friends to play!", code=0)
    me = cell(ctx.player.map)
    rows = []
    for d, p in found[:NEARBY_ROWS]:
        busy = " (busy)" if p.session is not None else ""
        rows.append([f"{p.name}: {place(p.map)}{busy}", p.id])
    d, p = found[0]
    if d < 1:
        where = f"{p.name} is right here in {place(p.map)}!"
    elif d < 99:
        how = "just " if d <= NEAR_CELLS else "far " if d > 6 else ""
        where = f"The nearest is {p.name} at {place(p.map)}, {how}{direction(me, cell(p.map))} of you."
    else:
        where = f"{p.name} is at {place(p.map)}."
    n = len(found)
    return answer(f"{n} trainer{'' if n == 1 else 's'} online. {where}~Add one as a friend?",
                  code=n, rows=rows)


def add_friend(ctx, args):
    try:
        other = ctx.server.players.get(int(args[1]))
    except (IndexError, ValueError):
        other = None
    if other is None or other.name is None or other is ctx.player:
        raise Refused("That trainer has gone offline.")
    if not ctx.server.friends.add(ctx.key, other.key):
        raise Refused("Your friend list is full (10).")
    ctx.online.notify(other.key, f"{ctx.player.name} added you as a friend! Add them back with ONLINE > FRIENDS.")
    return answer(f"{other.name} is on your friend list now!")


def cmd_guide(ctx, args, data):
    word = args[0] if args else ""
    if word == "INVITE":
        return invite(ctx)
    if word == "NEARBY":
        return nearby(ctx)
    if word == "FRIEND":
        return add_friend(ctx, args)
    raise ValueError("bad GUIDE")


COMMANDS = {"GUIDE": cmd_guide}


# --- background -----------------------------------------------------------

def announce_nearby(ctx):
    """A notice when another player comes within NEAR_CELLS of you."""
    player, now = ctx.player, ctx.online.now()
    near_before = getattr(player, "guide_near", None)
    told = getattr(player, "guide_near_told", {})
    near = {p.key: p for d, p in nearest(ctx) if d <= NEAR_CELLS}
    player.guide_near, player.guide_near_told = set(near), told
    if near_before is None:
        return  # first look after joining: nobody "arrived"
    for key, p in near.items():
        if key in near_before or now - told.get(key, -NEAR_NOTICE_GAP) < NEAR_NOTICE_GAP:
            continue
        told[key] = now
        ctx.online.notify(ctx.key, f"{p.name} is nearby at {place(p.map)}! Wave hello with L.")


def free_mentor(ctx, key):
    db = ctx.db
    online_keys = {p.key for p in ctx.server.players.values() if p.name is not None}
    best = None
    for n, t, grads in db.execute("SELECT name, tid, graduates FROM online_mentors WHERE volunteer = 1"):
        mentor = (n, t)
        if mentor == tuple(key):
            continue
        load = len(mentors.apprentices(db, mentor))
        if load >= mentors.MAX_APPRENTICES:
            continue
        # Online mentors first, then the least busy, then the most experienced.
        rank = (mentor not in online_keys, load, -grads, n)
        if best is None or rank < best[0]:
            best = (rank, mentor)
    return best[1] if best else None


def auto_mentor(ctx):
    db, key = ctx.db, tuple(ctx.key)
    reported = db.execute("SELECT badges FROM online_badges WHERE name = ? AND tid = ?", key).fetchone()
    if reported is None:
        return  # the game hasn't told us its badges yet
    badges = reported[0]
    row = db.execute("SELECT mentor_name, mentor_tid, declined FROM guide_auto_mentor WHERE name = ? AND tid = ?",
                     key).fetchone()
    current = mentors.mentor_of(db, key)

    if badges >= mentors.MENTOR_BADGES:
        told = db.execute("SELECT 1 FROM guide_mentor_told WHERE name = ? AND tid = ?", key).fetchone()
        volunteered = db.execute("SELECT 1 FROM online_mentors WHERE name = ? AND tid = ?", key).fetchone()
        if not told and not volunteered:
            db.execute("INSERT INTO guide_mentor_told VALUES (?, ?)", key)
            ctx.online.notify(key, f"With {badges} badges you can mentor new trainers! "
                                   "HUB > MENTORS > JOIN THE LIST.")
        return

    if row is not None and row[2]:
        return  # said goodbye to an automatic mentor once: leave them be
    if row is not None and row[0] is not None and current is None:
        # Their automatic mentor is gone: they said goodbye.
        db.execute("UPDATE guide_auto_mentor SET declined = 1 WHERE name = ? AND tid = ?", key)
        return
    if current is not None:
        return
    mentor = free_mentor(ctx, key)
    if mentor is None:
        return
    db.execute("INSERT INTO online_apprentices (name, tid, mentor_name, mentor_tid) VALUES (?, ?, ?, ?)",
               (*key, *mentor))
    db.execute("INSERT OR REPLACE INTO guide_auto_mentor (name, tid, mentor_name, mentor_tid, declined)"
               " VALUES (?, ?, ?, ?, 0)", (*key, *mentor))
    mb = mentors.badges_of(db, mentor)
    ctx.online.notify(key, f"Meet your mentor, {mentor[0]} ({mb} badges)! You'll both get gifts as you "
                           "win badges. See HUB > MENTORS.")
    ctx.online.notify(mentor, f"{ctx.player.name} is new here, so they're your apprentice now! "
                              "Cheer them on in HUB > MENTORS.")


def status(ctx):
    announce_nearby(ctx)
    auto_mentor(ctx)
    return []
