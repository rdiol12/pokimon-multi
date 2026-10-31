"""Sixth batch of online features (docs/online.md): the shared part.

The game sends these over the Online Plaza's request channel (server/ext.py),
with their own first words. Every answer has the same shape:

    row 0      message|code|var1|var2|var3   ("~" new line, "^" new page)
    +ITEM|id|n, -ITEM|id|n, +MONEY|n, -MONEY|n, +MON|species|level|shiny
               things the game applies as soon as the answer comes
    N|text     a notice the game shows once the player is free
    S|key|...  the status of a live feature
    anything else: data rows, for boards and pick lists

Two requests run in the background: `OEV <word> <numbers...>` reports
something that happened in the game (a catch, a race checkpoint...), and
`OSTAT` asks for the status of the live features. Both answers carry the
player's pending notices.

Feature modules (server/online_*.py) have COMMANDS like the plaza modules
(`fn(ctx, args, data) -> bytes`, usually `answer(...)`), and may have
`setup(db)`, `EVENTS = {word: fn(ctx, args)}` and `status(ctx) -> rows`.
`ctx.online` is the Online object below.
"""

import logging
import time

from ext import Refused, text  # noqa: F401  (Refused is re-exported for the modules)

WEEK = 7 * 24 * 3600
NOTE_LENGTH = 118


def game_text(s):
    """Text the game can show: ASCII, with é written as #."""
    return str(s).replace("é", "#").replace("É", "#")


# Pixel widths of the game's normal font (src/fonts.c) where they aren't 6.
NARROW = {" ": 3, "!": 4, "$": 3, ",": 3, ".": 3, ":": 3, ";": 3, "'": 3, "(": 4, ")": 4, "i": 4, "l": 4,
          "j": 5, "r": 5, "&": 7, "=": 8}
LINE_PIXELS = 200


def text_width(s):
    return sum(NARROW.get(c, 6) for c in s)


def wrap(message, width=LINE_PIXELS):
    """Breaks a message for the game's message box: lines of at most `width`
    pixels ("~"), two lines a page ("^"). A "~" already in the message
    forces a line break, a "^" a page break."""
    lines = []
    for page in str(message).split("^"):
        page_lines = []
        for part in page.split("~"):
            line = ""
            for word in part.split(" "):
                if line and text_width(f"{line} {word}") > width:
                    page_lines.append(line)
                    line = word
                else:
                    line = f"{line} {word}" if line else word
            page_lines.append(line)
        lines.append(page_lines)
    out = []
    for page_lines in lines:
        pairs = ["~".join(page_lines[i:i + 2]) for i in range(0, len(page_lines), 2)]
        out.append("^".join(pairs))
    return "^".join(out)


def answer(message="", code=0, vars=(), rows=()):
    first = [game_text(wrap(message)), code, *[game_text(v) for v in vars]]
    return text([first, *[[game_text(f) for f in row] for row in rows]])


def board(title, columns, rows):
    """Data rows for the game's board: title, "x:LABEL" columns, lines.
    A line ["%", percent, text, right text, count] is a progress bar."""
    return [[title], [f"{x}:{label}" for x, label in columns], *rows]


class Online:
    def __init__(self, server, ext_obj, modules, clock=time.time):
        self.server = server
        self.clock = clock
        self.modules = modules
        self.events = {}
        self.notes = []          # (id, text) for everyone online
        self.personal = {}       # key -> [text]
        self._note_id = 0
        server.online = self
        for module in modules:
            if hasattr(module, "setup"):
                module.setup(server.league.db)
            for word, fn in getattr(module, "COMMANDS", {}).items():
                ext_obj.handlers[word] = self._wrap(fn)
            for word, fn in getattr(module, "EVENTS", {}).items():
                self.events.setdefault(word, []).append(fn)
        server.league.db.commit()
        ext_obj.handlers["OEV"] = self._wrap(self.cmd_event)
        ext_obj.handlers["OSTAT"] = self._wrap(self.cmd_status)

    def _wrap(self, fn):
        def handler(ctx, args, data):
            ctx.online = self
            if not hasattr(ctx.player, "online_seen"):
                ctx.player.online_seen = self._note_id
            out = fn(ctx, args, data) or answer()
            # Notices go with the text rows, before any binary part.
            head, nul, tail = out.partition(b"\0")
            return head + self.pending_notes(ctx) + nul + tail
        return handler

    # --- time ----------------------------------------------------------

    def now(self):
        return self.clock()

    def week(self):
        return int(self.now() // WEEK)

    def day(self):
        return int(self.now() // 86400)

    # --- notices -------------------------------------------------------

    def announce(self, message, exclude=None):
        """A notice for every player online now (but `exclude`, a key)."""
        self._note_id += 1
        self.notes.append((self._note_id, game_text(wrap(message))[:NOTE_LENGTH], exclude))
        del self.notes[:-20]

    def notify(self, key, message):
        """A notice for one player, delivered on their next request."""
        self.personal.setdefault(tuple(key), []).append(game_text(wrap(message))[:NOTE_LENGTH])
        del self.personal[tuple(key)][:-5]

    def pending_notes(self, ctx):
        rows = [["N", t] for i, t, exclude in self.notes if i > ctx.player.online_seen and exclude != ctx.key]
        ctx.player.online_seen = self._note_id
        rows += [["N", t] for t in self.personal.pop(ctx.key, [])]
        return b"\n" + text(rows) if rows else b""

    # --- background requests -------------------------------------------

    def cmd_event(self, ctx, args, data):
        word, numbers = args[0], [int(a) for a in args[1:]]
        for fn in self.events.get(word, []):
            try:
                fn(ctx, numbers)
            except Exception:  # one feature's bug mustn't cut the player off
                logging.exception("online event %s", word)
        return answer()

    def cmd_status(self, ctx, args, data):
        rows = []
        for module in self.modules:
            if hasattr(module, "status"):
                try:
                    rows += [["S", *row] for row in module.status(ctx)]
                except Exception:  # as above
                    logging.exception("online status of %s", module.__name__)
        return answer(rows=rows)


def install(server, ext_obj):
    import online_betting
    import online_coopfish
    import online_doubles
    import online_duo
    import online_fishing
    import online_goals
    import online_guilds
    import online_mentors
    import online_pageant
    import online_pquests
    import online_shops
    import online_sightings
    import online_teams
    import online_web

    online_web.install(server)
    online_betting.install(server)
    online_doubles.install(server)
    online_teams.install(server)
    online_guilds.install(server)
    return Online(server, ext_obj, [online_goals, online_sightings, online_web, online_fishing, online_coopfish,
                                       online_shops, online_pageant, online_duo,
                                       online_mentors, online_pquests,
                                       online_betting, online_doubles, online_teams, online_guilds])
