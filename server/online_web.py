"""The website's live map and shiny gallery (on the server's web port, see
server/web.py).

- /map: the Hoenn region map with every online player on it, and the
  legends seen in the last day; it refreshes itself from /map.json.
- /shinies: every shiny Pokémon caught online, newest first, drawn with the
  game's own sprites (/sprite/<species>.png, the front picture with its
  shiny palette).

Shiny catches come from the game's catch reports (OEV CATCH); everyone
online also gets a notice.
"""

import html
import json
import os
import re
import struct
import time
import zlib

from online_ids import MAP_NAMES, MAP_SECTIONS, SECTION_GRID, SPRITES
from online_sightings import ago, place_name, species_name

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
STATIC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "static")
SPRITE_RE = re.compile(r"^/sprite/(\d+)\.png$")
GALLERY_SIZE = 120
LEGEND_HOURS = 24

# Region map cell (x, y) -> pixel of its top left corner in static/hoenn.png.
CELL, MAP_X, MAP_Y = 8, 12, 14

STYLE = """
body { font-family: system-ui, sans-serif; margin: 0 auto; max-width: 64rem; padding: 1rem; }
a { color: #2860b8; }
.map { position: relative; width: 768px; max-width: 100%; aspect-ratio: 256 / 176; }
.map img { width: 100%; image-rendering: pixelated; border-radius: 6px; }
.dot { position: absolute; transform: translate(-50%, -50%); font-size: .8rem; white-space: nowrap; }
.dot b { display: inline-block; width: 12px; height: 12px; border-radius: 50%; border: 2px solid #fff;
         background: #e83030; box-shadow: 0 0 2px #000; vertical-align: middle; }
.dot.legend b { background: #8040d0; }
.dot span { background: rgba(255,255,255,.85); padding: 0 .25rem; border-radius: 3px; margin-left: 2px; }
.grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); gap: 1rem; }
.mon { text-align: center; background: #f4f8ff; border-radius: 8px; padding: .5rem; }
.pic { width: 128px; height: 128px; margin: 0 auto; background-size: 100% 200%; background-repeat: no-repeat;
       image-rendering: pixelated; }
.mon small { color: #555; display: block; }
"""

MAP_PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pokimon Multi live map</title><style>{style}</style></head><body>
<h1>Live map</h1>
<p><a href="/">League</a> &middot; <a href="/shinies">Shiny gallery</a> &middot;
<span id="count"></span> Red: players online. Purple: legends seen in the last day.</p>
<div class="map" id="map"><img src="/static/hoenn.png" alt="The Hoenn region map"></div>
<script>
const W = 256, H = 176;
function place(list, cls) {{
  const seen = {{}};
  return list.map(p => {{
    const k = p.x + "," + p.y, n = seen[k] = (seen[k] || 0) + 1;
    const top = (p.y + (n - 1) * 7) / H * 100, left = p.x / W * 100;
    const d = document.createElement("div");
    d.className = "dot " + cls;
    d.style.left = left + "%"; d.style.top = top + "%";
    d.innerHTML = "<b></b><span></span>";
    d.querySelector("span").textContent = p.label;
    d.title = p.title;
    return d;
  }});
}}
async function refresh() {{
  const r = await fetch("/map.json", {{cache: "no-store"}});
  const data = await r.json();
  const map = document.getElementById("map");
  map.querySelectorAll(".dot").forEach(d => d.remove());
  for (const d of place(data.legends, "legend")) map.appendChild(d);
  for (const d of place(data.players, "")) map.appendChild(d);
  document.getElementById("count").textContent = data.players.length + " online.";
}}
refresh(); setInterval(refresh, 5000);
</script>
</body></html>
"""

SHINY_PAGE = """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pokimon Multi shiny gallery</title><style>{style}</style></head><body>
<h1>Shiny gallery</h1>
<p><a href="/">League</a> &middot; <a href="/map">Live map</a> &middot; {count} shiny Pok&eacute;mon caught online.</p>
<div class="grid">
{mons}
</div>
</body></html>
"""


def setup(db):
    db.executescript(
        """
        CREATE TABLE IF NOT EXISTS online_shinies (
            id INTEGER PRIMARY KEY,
            species INTEGER NOT NULL,
            name TEXT NOT NULL,
            tid INTEGER NOT NULL,
            map_group INTEGER NOT NULL,
            map_num INTEGER NOT NULL,
            at REAL NOT NULL
        );
        """
    )


def on_catch(ctx, args):
    """CATCH species shiny mapGroup mapNum fished"""
    species, shiny, group, num = (args + [0] * 4)[:4]
    if not shiny or species <= 0:
        return
    ctx.db.execute("INSERT INTO online_shinies (species, name, tid, map_group, map_num, at) VALUES (?, ?, ?, ?, ?, ?)",
                   (species, *ctx.key, group, num, ctx.online.now()))
    ctx.online.announce(f"{ctx.player.name} caught a shiny {species_name(species)}!", exclude=ctx.key)


def cell_pixel(group, num):
    """The middle of a map's region map section, in hoenn.png pixels."""
    section = MAP_SECTIONS.get(MAP_NAMES.get((group, num)))
    if section not in SECTION_GRID:
        return None
    x, y, w, h, _ = SECTION_GRID[section]
    if x > 28 or y > 19:
        return None  # not on the Hoenn map (Kanto, Sevii...)
    return MAP_X + x * CELL + w * CELL // 2, MAP_Y + y * CELL + h * CELL // 2


# --- shiny sprites -----------------------------------------------------------

def read_jasc(path):
    lines = open(path).read().split()
    count = int(lines[2])
    return [tuple(int(v) for v in lines[3 + i * 3:6 + i * 3]) for i in range(count)]


def png_chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)


def shiny_png(species):
    """The species' front picture (both frames) with its shiny palette and a
    transparent background, or None. Only the palette chunk changes, so no
    image decoding is needed."""
    if species not in SPRITES:
        return None
    pic, pal = (os.path.join(ROOT, p) for p in SPRITES[species])
    if not (os.path.exists(pic) and os.path.exists(pal)):
        return None
    data = open(pic, "rb").read()
    colors = read_jasc(pal)
    out, pos = [data[:8]], 8
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        kind = data[pos + 4:pos + 8]
        chunk = data[pos:pos + 12 + length]
        pos += 12 + length
        if kind == b"PLTE":
            n = length // 3
            entries = [colors[i] if i < len(colors) else (0, 0, 0) for i in range(n)]
            out.append(png_chunk(b"PLTE", bytes(c for e in entries for c in e)))
            out.append(png_chunk(b"tRNS", b"\x00"))
        elif kind == b"tRNS":
            continue
        else:
            out.append(chunk)
    return b"".join(out)


# --- pages -------------------------------------------------------------------

def map_json(server):
    players = []
    for p in server.players.values():
        if p.name is None or p.map is None:
            continue
        spot = cell_pixel(*p.map)
        if spot:
            place = place_name(*p.map)
            players.append({"x": spot[0], "y": spot[1], "label": p.name, "title": f"{p.name} at {place}"})
    legends = []
    now = server.online.now()
    for species, group, num, name, at in server.league.db.execute(
            "SELECT species, map_group, map_num, name, MAX(at) FROM online_sightings WHERE at > ?"
            " GROUP BY species", (now - LEGEND_HOURS * 3600,)).fetchall():
        spot = cell_pixel(group, num)
        if spot:
            legends.append({"x": spot[0], "y": spot[1], "label": species_name(species),
                            "title": f"{species_name(species)} at {place_name(group, num)}, seen by {name} {ago(now - at)}"
                                     f"{'' if ago(now - at) == 'now' else ' ago'}"})
    return {"players": players, "legends": legends}


def shiny_page(server):
    rows = server.league.db.execute(
        "SELECT species, name, map_group, map_num, at FROM online_shinies ORDER BY id DESC LIMIT ?",
        (GALLERY_SIZE,)).fetchall()
    total = server.league.db.execute("SELECT COUNT(*) FROM online_shinies").fetchone()[0]
    mons = "\n".join(
        f'<div class="mon"><div class="pic" style="background-image:url(/sprite/{species}.png)"></div>'
        f'<strong>{html.escape(species_name(species))}</strong>'
        f'<small>{html.escape(name)} &middot; {html.escape(place_name(g, n))}</small>'
        f'<small>{time.strftime("%Y-%m-%d", time.gmtime(at))}</small></div>'
        for species, name, g, n, at in rows
    ) or "<p>No shiny Pok&eacute;mon yet. Keep looking!</p>"
    return SHINY_PAGE.format(style=STYLE, count=total, mons=mons)


def route(server, path):
    if path == "/map":
        return 200, "text/html; charset=utf-8", MAP_PAGE.format(style=STYLE)
    if path == "/map.json":
        return 200, "application/json", json.dumps(map_json(server))
    if path == "/shinies":
        return 200, "text/html; charset=utf-8", shiny_page(server)
    if path == "/static/hoenn.png":
        return 200, "image/png", open(os.path.join(STATIC, "hoenn.png"), "rb").read()
    m = SPRITE_RE.match(path)
    if m:
        png = shiny_png(int(m.group(1)))
        if png:
            return 200, "image/png", png
    return None


def install(server):
    server.web_routes = getattr(server, "web_routes", []) + [lambda path: route(server, path)]


EVENTS = {"CATCH": on_catch}
