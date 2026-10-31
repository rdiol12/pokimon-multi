#!/usr/bin/env python3
"""End-to-end netplay test: starts the server, runs two headless mGBA
instances of pokeemerald.gba with the bridge and scenario.lua, then checks the
leaderboard. Needs libmgba-dev (mGBA 0.10 with Lua) and a built ROM.

    python3 tools/netplay_bridge/e2e/run_e2e.py [--frames N] [--out DIR]
"""

import argparse
import os
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))


def mgba_flags():
    """Link the harness against our mGBA fork (emulator/build.sh headless): the game needs its
    extended EWRAM and stops on a red screen under a stock libmgba. MGBA_DIR overrides the
    location; MGBA_DIR=system uses the system libmgba-dev."""
    base = os.environ.get("MGBA_DIR", os.path.join(ROOT, "build", "emulator"))
    if base == "system":
        return ["-lmgba"]
    lib = os.path.join(base, "headless")
    if not os.path.exists(os.path.join(lib, "libmgba.so")):
        sys.exit(f"no mGBA fork in {base}: run emulator/build.sh headless first")
    # struct mCore changes shape with the feature defines, so use exactly the ones libmgba was built with
    defines = []
    with open(os.path.join(lib, "CMakeFiles", "mgba.dir", "flags.make")) as f:
        for line in f:
            if line.startswith("C_DEFINES"):
                defines = [d for d in line.split("=", 1)[1].split() if d.startswith("-D") and "MGBA_DLL" not in d]
                break
    return defines + ["-I", os.path.join(base, "src", "include"), "-I", os.path.join(lib, "include"),
                      "-L", lib, "-Wl,-rpath," + lib, "-lmgba"]
SYMBOLS = ["gNetplayMailbox", "gMain", "gParties", "CB2_Overworld", "BattleMainCB2",
           "gLink", "gLinkStatus", "gReceivedRemoteLinkPlayers", "gLinkCallback",
           "gReadyToCloseLink", "gPartiesCount", "gSaveBlock1Ptr", "gSaveBlock2Ptr"]


def server_env():
    """The server's environment for a test run: no fishing tournament, whose
    real-time start notice would otherwise interrupt the scenario depending on
    the hour (server/online_fishing.py)."""
    now = time.time()
    return dict(os.environ, NETPLAY_FISHT_START=str(now + 86400), NETPLAY_FISHT_PERIOD=str(10 * 86400))


def read_symbols(map_path):
    found = {}
    pattern = re.compile(r"^\s+0x([0-9a-f]+)\s+(\w+)\s*$")
    with open(map_path) as f:
        for line in f:
            m = pattern.match(line)
            if m and m.group(2) in SYMBOLS:
                found[m.group(2)] = int(m.group(1), 16)
    missing = set(SYMBOLS) - set(found)
    if missing:
        sys.exit(f"symbols not found in {map_path}: {', '.join(sorted(missing))}")
    return found


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def query_board(port):
    with socket.create_connection(("127.0.0.1", port), timeout=5) as s:
        f = s.makefile("rw")
        f.write("HELLO e2e 1\nBOARD\n")
        f.flush()
        rows = []
        for line in f:
            line = line.strip()
            if line == "BOARDEND":
                return rows
            if line.startswith("RANK"):
                rows.append(line)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--frames", type=int, default=60000)
    parser.add_argument("--out", default=None, help="keep screenshots and saves here")
    args = parser.parse_args()

    rom = os.path.join(ROOT, "pokeemerald.gba")
    out = args.out or tempfile.mkdtemp(prefix="netplay-e2e-")
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)

    harness = os.path.join(out, "harness")
    subprocess.check_call(["gcc", "-O1", "-o", harness, os.path.join(HERE, "harness.c")] + mgba_flags())

    syms = read_symbols(os.path.join(ROOT, "pokeemerald.map"))
    with open(os.path.join(out, "symbols.lua"), "w") as f:
        f.write("return {\n")
        for name, addr in syms.items():
            f.write(f"    {name} = 0x{addr:08X},\n")
        f.write("}\n")
    shutil.copy(os.path.join(HERE, "driver_lib.lua"), out)

    dirs = []
    for n in (1, 2, 3):  # player 3 watches the battle
        d = os.path.join(out, f"p{n}")
        os.makedirs(d)
        shutil.copy(os.path.join(HERE, "scenario.lua"), os.path.join(d, "driver.lua"))
        dirs.append(d)

    # Player 2 (TEST2, trainer id 4098) finished an earlier season in GOLD,
    # so it has a reward to claim from ONLINE > SEASON.
    db_path = os.path.join(out, "league.sqlite3")
    sys.path.insert(0, os.path.join(ROOT, "server"))
    import sqlite3
    from seasons import Seasons
    db = sqlite3.connect(db_path)
    Seasons(db, 1000)
    db.execute("INSERT INTO season_rewards (season, name, tid, tier, rating) VALUES (0, 'TEST2', 4098, 2, 1150)")
    db.commit()
    db.close()

    port = free_port()
    server = subprocess.Popen(
        [sys.executable, os.path.join(ROOT, "server", "netplay_server.py"),
         "--host", "127.0.0.1", "--port", str(port), "--db", db_path],
        stdout=open(os.path.join(out, "server.log"), "w"), stderr=subprocess.STDOUT, env=server_env())
    try:
        time.sleep(0.5)
        env = dict(os.environ, NETPLAY_PORT=str(port))
        bridge = os.path.join(ROOT, "tools", "netplay_bridge", "netplay_bridge.lua")
        with open(os.path.join(out, "harness.log"), "w") as log:
            proc = subprocess.Popen([harness, rom, bridge, str(args.frames), *dirs], env=env,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            for line in proc.stdout:
                log.write(line)
                if not line.startswith("Starting DMA"):
                    print(line, end="")
            proc.wait()
        board = query_board(port)
    finally:
        server.terminate()
        server.wait()

    ok = True
    for n, d in enumerate(dirs, 1):
        path = os.path.join(d, "result.txt")
        result = open(path).read().strip() if os.path.exists(path) else "no result"
        print(f"player {n}: {result}")
        ok &= result == "ok"
    print("leaderboard:", board)
    expected_winner = "TEST2"
    if not board or board[0].split()[2] != expected_winner:
        print(f"expected {expected_winner} on top of the leaderboard")
        ok = False
    print(f"output in {out}")
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
