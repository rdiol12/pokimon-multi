"""Mount racing: HUB > MOUNT RACING. Racers who join a course within a few
seconds of each other ride it together (a heat, up to four, each in their own
lane); the game times the ride from GO! to the finish (src/online_racing.c)
and reports it. The first to finish a heat of two or more wins prize money;
every course has a board of the week's best times. A heat's places are the
order racers cross the line (all of them start at the same moment).
"""

import math
import os

from online import Refused, answer, board

COURSES = ["PETALBURG RUN", "SLATEPORT SPRINT", "ASHEN DASH"]  # as sCourses in src/online_racing.c
LANES = 4
HEAT_WAIT = float(os.environ.get("NETPLAY_RACE_WAIT", 20))  # seconds from the first racer to the start
HEAT_CLOSE = 3  # no one joins a heat this close to its start
HEAT_TIMEOUT = 15 * 60  # a heat is over this long after its start, finished or not
WIN_MONEY = 1000  # for each racer beaten
FPS = 60
BOARD_SIZE = 8


class Heat:
    def __init__(self, course, start):
        self.course = course
        self.start = start
        self.racers = []  # keys, in lane order
        self.names = {}
        self.times = {}  # key -> frames, in finishing order
        self.quit = set()

    def over(self, now):
        return now > self.start + HEAT_TIMEOUT or all(k in self.times or k in self.quit for k in self.racers)


def setup(db):
    db.execute(
        """CREATE TABLE IF NOT EXISTS online_race_times (
            week INTEGER NOT NULL, course INTEGER NOT NULL, name TEXT NOT NULL, tid INTEGER NOT NULL,
            frames INTEGER NOT NULL, PRIMARY KEY (week, course, name, tid)
        )"""
    )


def heats(server):
    if not hasattr(server, "race_heats"):
        server.race_heats = []
    return server.race_heats


def heat_of(server, key, now):
    for heat in heats(server):
        if key in heat.racers and key not in heat.times and key not in heat.quit and not heat.over(now):
            return heat
    return None


def time_text(frames):
    cs = frames * 100 // FPS
    return f"{cs // 6000}:{cs // 100 % 60:02}.{cs % 100:02}"


def place_text(n):
    return {1: "1st", 2: "2nd", 3: "3rd"}.get(n, f"{n}th")


def course_arg(args, i):
    try:
        course = int(args[i])
    except (IndexError, ValueError):
        course = -1
    if not 0 <= course < len(COURSES):
        raise Refused("That course is closed.")
    return course


def best_times(db, week, course):
    return db.execute("SELECT name, tid, frames FROM online_race_times WHERE week = ? AND course = ?"
                      " ORDER BY frames, name", (week, course)).fetchall()


def close_heat(ctx, heat):
    """Tells the racers of a heat of two or more how it ended."""
    heats(ctx.server).remove(heat)
    if len(heat.racers) < 2 or not heat.times:
        return
    # In the order they crossed the line.
    text = ", ".join(f"{place_text(i)} {heat.names[k]} {time_text(f)}" for i, (k, f) in enumerate(heat.times.items(), 1))
    for k in heat.racers:
        ctx.online.notify(k, f"{COURSES[heat.course]} results: {text}.")


def cmd_race(ctx, args, data):
    db, key = ctx.db, ctx.key
    now = ctx.online.now()
    week = ctx.online.week()
    word = args[0]
    for heat in [h for h in heats(ctx.server) if h.over(now)]:
        close_heat(ctx, heat)

    if word == "LIST":
        rows = []
        for i, name in enumerate(COURSES):
            best = best_times(db, week, i)
            waiting = [h for h in heats(ctx.server) if h.course == i and h.start > now + HEAT_CLOSE]
            note = f"{len(waiting[0].racers)} waiting" if waiting else (time_text(best[0][2]) if best else "")
            rows.append([f"{name} {note}".strip(), i])
        return answer("Which course will you ride?", rows=rows)

    if word == "JOIN":
        course = course_arg(args, 1)
        old = heat_of(ctx.server, key, now)
        if old:
            old.quit.add(key)
        heat = next((h for h in heats(ctx.server) if h.course == course and h.start > now + HEAT_CLOSE
                     and len(h.racers) < LANES), None)
        if heat is None:
            heat = Heat(course, now + HEAT_WAIT)
            heats(ctx.server).append(heat)
            ctx.online.announce(f"{ctx.player.name} is starting a {COURSES[course]} race! Join in HUB > MOUNT RACING.",
                                exclude=key)
        for k in heat.racers:
            ctx.online.notify(k, f"{ctx.player.name} joined the race!")
        lane = len(heat.racers)
        heat.racers.append(key)
        heat.names[key] = ctx.player.name
        seconds = math.ceil(heat.start - now)
        return answer(f"{COURSES[course]}: the race starts in {seconds} seconds! Off to the start line!",
                      code=seconds, vars=[COURSES[course]], rows=[[lane]])

    if word == "BOARD":
        course = course_arg(args, 1)
        rows = []
        for place, (name, tid, frames) in enumerate(best_times(db, week, course), 1):
            mark = "*" if (name, tid) == tuple(key) else ""
            rows.append([f"{mark}{place}", name, time_text(frames)])
        mine = [r for r in rows if r[0].startswith("*")]
        rows = rows[:BOARD_SIZE] + [r for r in mine if r not in rows[:BOARD_SIZE]]
        message = (f"Your best this week is {mine[0][2]}, number {mine[0][0][1:]}." if mine
                   else "Nobody has raced it this week yet!" if not rows else "Ride it to get on the board!")
        return answer(message, rows=board(f"{COURSES[course]}: BEST TIMES", [(6, "No."), (30, "TRAINER"),
                                                                              (120, "TIME")], rows))

    if word == "FINISH":
        course = course_arg(args, 1)
        try:
            frames = int(args[2])
        except (IndexError, ValueError):
            raise Refused("That didn't work.")
        if frames < FPS * 2:
            raise Refused("That time can't be right.")
        heat = heat_of(ctx.server, key, now)
        if heat is None or heat.course != course:
            raise Refused(f"Your time: {time_text(frames)}. That race was over, though!")
        heat.times[key] = frames
        record = best_times(db, week, course)
        mine = db.execute("SELECT frames FROM online_race_times WHERE week = ? AND course = ? AND name = ? AND tid = ?",
                          (week, course, *key)).fetchone()
        if mine is None or frames < mine[0]:
            db.execute("INSERT OR REPLACE INTO online_race_times VALUES (?, ?, ?, ?, ?)", (week, course, *key, frames))
        if not record or frames < record[0][2]:
            ctx.online.announce(f"{ctx.player.name} set a {COURSES[course]} record: {time_text(frames)}!", exclude=key)
        rows, said = [], f"Your time: {time_text(frames)}."
        if len(heat.racers) > 1 and len(heat.times) == 1:
            prize = WIN_MONEY * (len(heat.racers) - 1)
            rows.append(["+MONEY", prize])
            said += f" You won the race! ${prize}!"
        elif len(heat.racers) > 1:
            said += f" You came in {place_text(len(heat.times))}."
        if not record or frames < record[0][2]:
            said += " A new record this week!"
        if heat.over(now):
            close_heat(ctx, heat)
        return answer(said, rows=rows)

    if word == "QUIT":
        heat = heat_of(ctx.server, key, now)
        if heat:
            heat.quit.add(key)
            for k in heat.racers:
                if k != tuple(key):
                    ctx.online.notify(k, f"{ctx.player.name} gave up the race.")
            if heat.over(now):
                close_heat(ctx, heat)
        return answer()

    raise ValueError("bad RACE")


COMMANDS = {"RACE": cmd_race}
