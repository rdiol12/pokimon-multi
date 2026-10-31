#!/usr/bin/env python3
"""End-to-end test of the second online batch (docs/netplay_ext.md): two
headless games run scenario_ext.lua against a fresh server and go through
the Online Plaza's ten features. Screenshots go to --out.

    python3 tools/netplay_bridge/e2e/run_ext_e2e.py [--frames N] [--out DIR]
"""

import argparse
import os
import shutil
import socket
import sqlite3
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import run_e2e  # noqa: E402

ROOT = run_e2e.ROOT
run_e2e.SYMBOLS += ["gNetplayExtMailbox", "CB2_BagMenuRun", "gActionSelectionCursor", "gMoveSelectionCursor",
                    "gBattleMons"]


def seed_server(port):
    """A third player, Gary, posts a bounty for a POOCHYENA and shares a
    tower team is not possible without a game, so he only posts the bounty."""
    with socket.create_connection(("127.0.0.1", port), timeout=5) as s:
        f = s.makefile("rw")
        f.write("HELLO Gary 4242\nX 1 BOUNTY POST 261 1500\n")
        f.flush()
        for line in f:
            if line.startswith("XR"):
                return line.strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--frames", type=int, default=150000)
    parser.add_argument("--out", default=None)
    args = parser.parse_args()

    rom = os.path.join(ROOT, "pokeemerald.gba")
    out = args.out or tempfile.mkdtemp(prefix="netplay-ext-e2e-")
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    harness = os.path.join(out, "harness")
    subprocess.check_call(["gcc", "-O1", "-o", harness, os.path.join(HERE, "harness.c"), "-lmgba"])
    syms = run_e2e.read_symbols(os.path.join(ROOT, "pokeemerald.map"))
    with open(os.path.join(out, "symbols.lua"), "w") as f:
        f.write("return {\n")
        for name, addr in syms.items():
            f.write(f"    {name} = 0x{addr:08X},\n")
        f.write("}\n")
    shutil.copy(os.path.join(HERE, "driver_lib.lua"), out)
    dirs = []
    for n in (1, 2):
        d = os.path.join(out, f"p{n}")
        os.makedirs(d)
        shutil.copy(os.path.join(HERE, "scenario_ext.lua"), os.path.join(d, "driver.lua"))
        dirs.append(d)

    port = run_e2e.free_port()
    db = os.path.join(out, "league.sqlite3")
    server = subprocess.Popen(
        [sys.executable, os.path.join(ROOT, "server", "netplay_server.py"),
         "--host", "127.0.0.1", "--port", str(port), "--db", db],
        stdout=open(os.path.join(out, "server.log"), "w"), stderr=subprocess.STDOUT, env=run_e2e.server_env())
    try:
        time.sleep(0.7)
        print("seed:", seed_server(port))
        env = dict(os.environ, NETPLAY_PORT=str(port))
        bridge = os.path.join(ROOT, "tools", "netplay_bridge", "netplay_bridge.lua")
        with open(os.path.join(out, "harness.log"), "w") as log:
            # The harness has no `script.dir`, so the bridge finds
            # extensions.txt (and netplay_ext.lua) in the working directory.
            proc = subprocess.Popen([harness, rom, bridge, str(args.frames), *dirs], env=env,
                                    cwd=os.path.dirname(bridge),
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            for line in proc.stdout:
                log.write(line)
                if not line.startswith("Starting DMA"):
                    print(line, end="")
            proc.wait()
    finally:
        server.terminate()
        server.wait()

    ok = True
    for n, d in enumerate(dirs, 1):
        path = os.path.join(d, "result.txt")
        result = open(path).read().strip() if os.path.exists(path) else "no result"
        print(f"player {n}: {result}")
        ok &= result == "ok"
    con = sqlite3.connect(db)
    for table in ("guilds", "guild_members", "titles", "dex", "wonder_stats", "bounties", "mail", "bases",
                  "tower_runs", "replays"):
        rows = con.execute(f"SELECT * FROM {table}").fetchall()
        rows = [tuple(r if not isinstance(r, bytes) else f"<{len(r)} bytes>" for r in row) for row in rows]
        print(f"{table}: {rows}")
    print(f"output in {out}")
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
