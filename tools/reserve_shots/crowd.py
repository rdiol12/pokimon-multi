#!/usr/bin/env python3
"""Runs crowd.lua against a local server whose reserve already holds 14
residents (so the game gets the 11 it can show).

    python3 tools/reserve_shots/crowd.py <out dir>
"""
import os
import socket
import sqlite3
import subprocess
import sys
import time

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))
sys.path.insert(0, os.path.join(root, "server"))
from community_reserve import Reserve  # noqa: E402

RESIDENTS = [(263, "ZIGZAGOON"), (270, "LOTAD"), (273, "SEEDOT"), (285, "SHROOMISH"), (183, "MARILL"),
             (300, "SKITTY"), (309, "ELECTRIKE"), (311, "PLUSLE"), (312, "MINUN"), (333, "SWABLU"),
             (316, "GULPIN"), (261, "POOCHYENA"), (278, "WINGULL"), (280, "RALTS")]


def species_id(name):
    import re
    text = open(os.path.join(root, "include", "constants", "species.h")).read()
    return int(re.search(rf"^\s+SPECIES_{name} = (\d+),", text, re.M).group(1))


def main():
    out = os.path.abspath(sys.argv[1])
    os.makedirs(out, exist_ok=True)
    db_path = os.path.join(os.path.dirname(out), os.path.basename(out) + "-league.sqlite3")
    if os.path.exists(db_path):
        os.remove(db_path)
    db = sqlite3.connect(db_path)
    reserve = Reserve(db)
    for i, (_, name) in enumerate(RESIDENTS):
        reserve.add(f"TRAINER{i}", 100 + i, species_id(name), 10 + i, 0x5EED0000 + i)
    db.close()
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        port = s.getsockname()[1]
    server = subprocess.Popen(
        [sys.executable, os.path.join(root, "server", "netplay_server.py"), "--host", "127.0.0.1",
         "--port", str(port), "--db", db_path],
        stdout=open(db_path + ".log", "w"), stderr=subprocess.STDOUT)
    try:
        time.sleep(0.5)
        env = dict(os.environ, NETPLAY_PORT=str(port))
        subprocess.call([sys.executable, os.path.join(here, "run.py"), os.path.join(here, "crowd.lua"), out, "60000"],
                        env=env, cwd=os.path.join(root, "tools", "netplay_bridge"))
    finally:
        server.terminate()
        server.wait()


if __name__ == "__main__":
    main()
