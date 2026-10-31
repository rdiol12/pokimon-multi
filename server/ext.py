"""Request channel for the second batch of online features (guilds, Wonder
Trade, secret bases, Battle Tower, bounties, titles, replays, draft battles,
mail and the Pokédex race).

The game sends one request at a time through its second mailbox (see
docs/netplay_ext.md). The bridge turns it into

    XD <hex>                 (zero or more: binary data for the request)
    X <seq> <word> <args...> (the request itself)

and the server answers with

    XR <seq> <status> <hex>  (status 0: ok, otherwise an error; hex may be empty)

Each feature module has a COMMANDS dict mapping its first word to a handler
`fn(ctx, args, data) -> bytes`. A handler raises Refused(message) to answer
with an error; the message is sent back as the data so the game can show it.
"""

import logging

log = logging.getLogger("netplay.ext")

MAX_DATA = 4096
STATUS_OK = 0
STATUS_REFUSED = 1
STATUS_BAD = 2


class Refused(Exception):
    """A request the game made correctly, but that can't be done."""


class Context:
    def __init__(self, server, player):
        self.server = server
        self.player = player

    @property
    def db(self):
        return self.server.league.db

    @property
    def league(self):
        return self.server.league

    @property
    def key(self):
        return self.player.key

    def online(self):
        return [p for p in self.server.players.values() if p.name is not None]


def text(rows):
    """Rows of fields as the game reads them: fields split by '|', rows by
    newlines, ASCII only."""
    out = []
    for row in rows:
        out.append("|".join(str(f).replace("|", "/").replace("\n", " ") for f in row))
    return ("\n".join(out)).encode("ascii", "replace")


class Ext:
    def __init__(self, server, modules):
        self.server = server
        self.handlers = {}
        for module in modules:
            if hasattr(module, "setup"):
                module.setup(server.league.db)
            for word, fn in module.COMMANDS.items():
                self.handlers[word] = fn
        server.league.db.commit()
        server.extra_commands["XD"] = self.cmd_data
        server.extra_commands["X"] = self.cmd_request

    def cmd_data(self, server, player, payload):
        buf = getattr(player, "ext_data", b"")
        try:
            chunk = bytes.fromhex(payload)
        except ValueError:
            raise ValueError("bad payload") from None
        if len(buf) + len(chunk) > MAX_DATA:
            player.ext_data = b""
            raise ValueError("request too big")
        player.ext_data = buf + chunk

    def cmd_request(self, server, player, seq, word=None, *args):
        data = getattr(player, "ext_data", b"")
        player.ext_data = b""
        status, out = STATUS_OK, b""
        handler = self.handlers.get(word)
        if handler is None:
            status, out = STATUS_BAD, b"Unknown request."
        else:
            try:
                out = handler(Context(server, player), list(args), data) or b""
            except Refused as e:
                status, out = STATUS_REFUSED, str(e).encode("ascii", "replace")
            except (ValueError, IndexError, TypeError) as e:
                log.info("bad %s request from %s: %s", word, player.name, e)
                status, out = STATUS_BAD, b"The server didn't understand."
            server.league.db.commit()
        player.send(f"XR {seq} {status} {out.hex()}")


def install(server):
    import adventure_hof, bases, bounties, dexrace, draft, guilds, mail, replays, titles, tower, wondertrade

    return Ext(server, [guilds, titles, dexrace, wondertrade, bounties, mail, bases, tower, draft, replays, adventure_hof])
