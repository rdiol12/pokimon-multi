#!/usr/bin/env python3
"""Screenshot of the tournament bracket page (/tournament) mid-cup.

Usage: tournament_page.py <out.png>

Runs an 8-player cup on an in-memory server (the quarterfinals and one
semifinal played) and renders the page with Playwright's Chromium (node).
"""
import json
import os
import subprocess
import sys
import tempfile

root = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
sys.path.insert(0, os.path.join(root, "server"))
os.environ.setdefault("NETPLAY_TOUR_SIZE", "8")

import netplay_server as ns  # noqa: E402
import community_tournament as tour  # noqa: E402

PLAYERS = ["RON", "AIKO", "MATEO", "PRIYA", "JONAS", "LENA", "KOFI", "SUN"]


def main():
    league = ns.League(":memory:")
    server = ns.Server(league)
    tours = tour._tours(server)
    keys = [(name, 1000 + i) for i, name in enumerate(PLAYERS)]
    for key in keys:
        tours.join(key)
    cup = tours.running_cup()
    for _, _, an, at, bn, bt, _, _ in tours.matches(cup, 1):
        tours.record((an, at), (bn, bt), (an, at))
    _, _, an, at, bn, bt, _, _ = tours.matches(cup, 2)[0]
    tours.record((an, at), (bn, bt), (bn, bt))
    for key in [("NOAH", 2001), ("ELIF", 2002)]:
        tours.join(key)
    status, _, page = tour.page(server)
    assert status == 200

    with tempfile.NamedTemporaryFile("w", suffix=".html", delete=False) as f:
        f.write(page)
    shoot = (
        "const { chromium } = require('playwright');"
        "(async () => { const b = await chromium.launch();"
        " const t = await b.newPage({ viewport: { width: 760, height: 420 } });"
        f" await t.goto('file://' + {json.dumps(f.name)});"
        f" await t.screenshot({{ path: {json.dumps(os.path.abspath(sys.argv[1]))}, fullPage: true }});"
        " await b.close(); })();"
    )
    subprocess.run(["node", "-e", shoot], check=True)
    os.unlink(f.name)


if __name__ == "__main__":
    main()
