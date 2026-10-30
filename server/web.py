"""Small HTTP side of the netplay server, on its own port: the league
leaderboard as a page and as JSON, a health check, and the mGBA bridge
script with this server's address filled in, so players download one file
and load it. Standard library only."""

import asyncio
import html
import json
import logging
import os
import re

log = logging.getLogger("netplay.web")

BRIDGE_PATH = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "tools", "netplay_bridge", "netplay_bridge.lua"
)
WORLD_BRIDGE_PATH = os.path.join(os.path.dirname(BRIDGE_PATH), "netplay_world.lua")
GALLERY_FILE_RE = re.compile(r"^/gallery/([\w.-]+\.png)$")

GALLERY_PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pokimon Multi photos</title>
<style>
body {{ font-family: system-ui, sans-serif; margin: 0 auto; max-width: 60rem; padding: 1rem; }}
.grid {{ display: grid; grid-template-columns: repeat(auto-fill, minmax(240px, 1fr)); gap: 1rem; }}
figure {{ margin: 0; }}
img {{ width: 100%; image-rendering: pixelated; border-radius: 4px; }}
figcaption {{ font-size: .9rem; color: #555; }}
</style></head><body>
<h1>Photos</h1>
<p><a href="/">League</a> &middot; taken in game with R &rarr; PHOTO.</p>
<div class="grid">
{photos}
</div>
</body></html>
"""
MAX_REQUEST = 8192

PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pokimon Multi league</title>
<style>
body {{ font-family: system-ui, sans-serif; margin: 0 auto; max-width: 40rem; padding: 1rem; }}
table {{ border-collapse: collapse; width: 100%; }}
th, td {{ padding: .4rem .6rem; text-align: left; border-bottom: 1px solid #ccc; }}
.n {{ text-align: right; font-variant-numeric: tabular-nums; }}
code {{ background: #eee; padding: 0 .2rem; }}
</style></head><body>
<h1>Pokimon Multi league</h1>
<p>{online} online now.</p>
<table>
<tr><th>#</th><th>Trainer</th><th class=n>Rating</th><th class=n>W</th><th class=n>L</th><th class=n>D</th></tr>
{rows}
</table>
<p><a href="/gallery">Photo gallery</a></p>
<h2>Play</h2>
<ol>
<li>Open the game in mGBA 0.10 or newer.</li>
<li>Download <a href="/netplay_bridge.lua">netplay_bridge.lua</a>
(already set to <code>{address}</code>).</li>
<li>In mGBA: Tools &rarr; Scripting&hellip; &rarr; File &rarr; Load script, and pick it.</li>
</ol>
</body></html>
"""


class Web:
    def __init__(self, server, public_host, game_port):
        self.server = server
        self.public_host = public_host
        self.game_port = game_port

    def online(self):
        return sum(1 for p in self.server.players.values() if p.name is not None)

    def board(self):
        return [
            {"rank": i, "name": name, "rating": round(rating), "wins": w, "losses": l, "draws": d}
            for i, (name, rating, w, l, d) in enumerate(self.server.league.top(100), 1)
        ]

    def bridge_script(self, host):
        with open(BRIDGE_PATH, encoding="utf-8") as f:
            script = f.read()
        # One download: the world features part goes first (netplay_world.lua).
        if os.path.exists(WORLD_BRIDGE_PATH):
            with open(WORLD_BRIDGE_PATH, encoding="utf-8") as f:
                script = f.read() + "\n" + script
        header = f"NETPLAY_HOST = NETPLAY_HOST or {json.dumps(host)}\nNETPLAY_PORT = NETPLAY_PORT or {self.game_port}\n"
        return header + script

    def route(self, path, host_header):
        host = self.public_host or re.sub(r":\d+$", "", host_header or "") or "127.0.0.1"
        if path == "/health":
            return 200, "application/json", json.dumps({"ok": True, "online": self.online()})
        if path == "/leaderboard.json":
            return 200, "application/json", json.dumps(self.board())
        if path == "/netplay_bridge.lua":
            return 200, "text/plain; charset=utf-8", self.bridge_script(host)
        gallery = getattr(self.server, "gallery_dir", None)
        if path == "/gallery":
            entries = []
            if gallery and os.path.exists(os.path.join(gallery, "index.json")):
                with open(os.path.join(gallery, "index.json")) as f:
                    entries = json.load(f)
            photos = "\n".join(
                f'<figure><img src="/gallery/{html.escape(e["file"])}" alt="Photo by {html.escape(e["player"])}">'
                f'<figcaption>{html.escape(e["player"])}</figcaption></figure>'
                for e in reversed(entries)
            ) or "<p>No photos yet.</p>"
            return 200, "text/html; charset=utf-8", GALLERY_PAGE.format(photos=photos)
        m = GALLERY_FILE_RE.match(path)
        if m and gallery and os.path.exists(os.path.join(gallery, m.group(1))):
            with open(os.path.join(gallery, m.group(1)), "rb") as f:
                return 200, "image/png", f.read()
        if path == "/":
            rows = "\n".join(
                f"<tr><td>{r['rank']}</td><td>{html.escape(r['name'])}</td><td class=n>{r['rating']}</td>"
                f"<td class=n>{r['wins']}</td><td class=n>{r['losses']}</td><td class=n>{r['draws']}</td></tr>"
                for r in self.board()
            ) or "<tr><td colspan=6>No battles yet.</td></tr>"
            page = PAGE.format(online=self.online(), rows=rows, address=html.escape(f"{host}:{self.game_port}"))
            return 200, "text/html; charset=utf-8", page
        return 404, "text/plain", "not found\n"

    async def handle(self, reader, writer):
        try:
            head = await asyncio.wait_for(reader.readuntil(b"\r\n\r\n"), 10)
            if len(head) > MAX_REQUEST:
                raise ValueError("request too large")
            lines = head.decode("latin-1").split("\r\n")
            method, target, _ = lines[0].split(" ", 2)
            headers = dict(l.split(":", 1) for l in lines[1:] if ":" in l)
            host_header = next((v.strip() for k, v in headers.items() if k.lower() == "host"), "")
            if method not in ("GET", "HEAD"):
                status, ctype, body = 405, "text/plain", "method not allowed\n"
            else:
                status, ctype, body = self.route(target.split("?", 1)[0], host_header)
        except (asyncio.TimeoutError, asyncio.IncompleteReadError, asyncio.LimitOverrunError, ValueError):
            writer.close()
            return
        data = body if isinstance(body, bytes) else body.encode("utf-8")
        reason = {200: "OK", 404: "Not Found", 405: "Method Not Allowed"}[status]
        writer.write(
            f"HTTP/1.1 {status} {reason}\r\nContent-Type: {ctype}\r\nContent-Length: {len(data)}\r\n"
            "Cache-Control: no-store\r\nConnection: close\r\n\r\n".encode("ascii")
        )
        if method != "HEAD":
            writer.write(data)
        try:
            await writer.drain()
        except ConnectionError:
            pass
        writer.close()
