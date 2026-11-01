#!/usr/bin/env python3
"""Screenshots of the Hoenn multiplayer fixes (docs/hoenn_multi.md): starts the
netplay server with stand-in players placed around Hoenn, then runs
hoenn_multi.lua through run.py.

    python3 tools/world_shots/hoenn_multi.py <out dir>
"""
import json
import os
import socket
import sqlite3
import subprocess
import sys
import threading
import time

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))


def map_id(name):
    groups = json.load(open(os.path.join(root, "data", "maps", "map_groups.json")))
    for g, group in enumerate(groups["group_order"]):
        if name in groups[group]:
            return g, groups[group].index(name)
    raise KeyError(name)


# name, trainer id, map, x, y, facing, graphics id (0 Brendan, 89 May, others trainer classes)
STAND_INS = [
    ("ASH", 5001, "OldaleTown_PokemonCenter_1F", 7, 5, 1, 89),      # in the aisle
    ("MISTY", 5002, "OldaleTown_PokemonCenter_1F", 7, 4, 2, 35),    # at the nurse
    ("BROCK", 5003, "OldaleTown_PokemonCenter_1F", 7, 7, 2, 0),     # by the door
    ("GARY", 5004, "LittlerootTown_BrendansHouse_2F", 6, 5, 3, 0),  # in "your" bedroom
    ("DAWN", 5005, "PetalburgCity_Gym", 4, 108, 2, 89),             # in the gym
    ("IRIS", 5006, "Route104", 10, 30, 1, 47),                      # a friend somewhere new
]


def stand_in(port, i, stop):
    name, tid, mapname, x, y, facing, gfx = STAND_INS[i]
    group, num = map_id(mapname)
    for _ in range(100):  # the server takes a moment to start
        try:
            conn = socket.create_connection(("127.0.0.1", port))
            break
        except OSError:
            time.sleep(0.1)
    with conn as s:
        s.sendall(f"HELLO {name} {tid}\n".encode())
        s.sendall(f"POS {group} {num} {x} {y} {facing} 3 {gfx}\n".encode())
        s.settimeout(1)
        while not stop.is_set():
            try:
                if not s.recv(4096):
                    break
            except socket.timeout:
                pass


def main():
    out = os.path.abspath(sys.argv[1])
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        port = s.getsockname()[1]
    os.makedirs(out, exist_ok=True)
    db = os.path.join(out, "..", os.path.basename(out) + "-league.sqlite3")
    if os.path.exists(db):
        os.remove(db)
    # TEST1 (trainer id 0x1001) already has IRIS on their friend list.
    con = sqlite3.connect(db)
    con.execute("CREATE TABLE friends (owner_name TEXT NOT NULL, owner_tid INTEGER NOT NULL, friend_name TEXT NOT NULL,"
                " friend_tid INTEGER NOT NULL, added_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
                " PRIMARY KEY (owner_name, owner_tid, friend_name, friend_tid))")
    con.execute("INSERT INTO friends (owner_name, owner_tid, friend_name, friend_tid) VALUES ('TEST1', 4097, 'IRIS', 5006)")
    con.commit()
    con.close()
    server = subprocess.Popen(
        [sys.executable, os.path.join(root, "server", "netplay_server.py"), "--host", "127.0.0.1",
         "--port", str(port), "--db", db],
        stdout=open(os.path.join(out, "..", os.path.basename(out) + "-server.log"), "w"), stderr=subprocess.STDOUT)
    stop = threading.Event()
    try:
        time.sleep(0.5)
        threads = [threading.Thread(target=stand_in, args=(port, i, stop), daemon=True) for i in range(len(STAND_INS))]
        for t in threads:
            t.start()
        env = dict(os.environ, NETPLAY_PORT=str(port))
        subprocess.call([sys.executable, os.path.join(here, "run.py"), os.path.join(here, "hoenn_multi.lua"), out],
                        env=env, cwd=os.path.join(root, "tools", "netplay_bridge"))
    finally:
        stop.set()
        server.terminate()


if __name__ == "__main__":
    main()
