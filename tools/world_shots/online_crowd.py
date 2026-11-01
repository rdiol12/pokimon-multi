#!/usr/bin/env python3
"""Screenshot of a crowded map online: starts the netplay server, connects
23 stand-in players standing around Petalburg City, and runs
online_crowd.lua through run.py.

    python3 tools/world_shots/online_crowd.py <out dir>
"""
import os
import socket
import subprocess
import sys
import threading
import time

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))

PETALBURG = (0, 0)
# Event object graphics: Brendan, May and a few trainer classes.
LOOKS = [0, 89, 0, 89, 35, 47, 31, 32, 0, 89, 55, 36, 49, 6, 0, 89, 27, 50, 0, 89, 35, 47, 31]
SPOTS = [(x, y) for y in (9, 11, 13, 15, 17) for x in (8, 10, 12, 14, 16, 18, 20, 22)
         if (x, y) != (15, 12)]


def stand_in(port, i, stop):
    x, y = SPOTS[i * 3 % len(SPOTS)]
    with socket.create_connection(("127.0.0.1", port)) as s:
        s.sendall(f"HELLO CROWD{i:02d} {5000 + i}\n".encode())
        s.sendall(f"POS {PETALBURG[0]} {PETALBURG[1]} {x} {y} {1 + i % 4} 3 {LOOKS[i % len(LOOKS)]}\n".encode())
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
    server = subprocess.Popen(
        [sys.executable, os.path.join(root, "server", "netplay_server.py"), "--host", "127.0.0.1",
         "--port", str(port), "--db", os.path.join(out, "..", os.path.basename(out) + "-league.sqlite3")],
        stdout=open(os.path.join(out, "..", os.path.basename(out) + "-server.log"), "w"), stderr=subprocess.STDOUT)
    stop = threading.Event()
    try:
        time.sleep(0.5)
        threads = [threading.Thread(target=stand_in, args=(port, i, stop), daemon=True) for i in range(23)]
        for t in threads:
            t.start()
        env = dict(os.environ, NETPLAY_PORT=str(port))
        subprocess.call([sys.executable, os.path.join(here, "run.py"), os.path.join(here, "online_crowd.lua"), out],
                        env=env, cwd=os.path.join(root, "tools", "netplay_bridge"))
    finally:
        stop.set()
        server.terminate()
        server.wait()


if __name__ == "__main__":
    main()
