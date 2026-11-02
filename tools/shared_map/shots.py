#!/usr/bin/env python3
"""In-game screenshots of shared wild spawns and map channels
(docs/shared_map.md). Needs what tools/world_shots/run.py needs.

    python3 tools/shared_map/shots.py <out dir>

<out>/spawns: two players on Route 101 (tools/shared_map/spawns.lua).
<out>/channels: channels of 2 players, a stand-in on Route 101 and two
players (tools/shared_map/channels.lua).
"""
import os
import socket
import subprocess
import sys
import threading
import time

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))
run_py = os.path.join(root, "tools", "world_shots", "run.py")
bridge_dir = os.path.join(root, "tools", "netplay_bridge")
ROUTE101 = (0, 16)


def free_port():
    with socket.socket() as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def stand_in(port, out, stop):
    """A player standing on Route 101 until the scenario writes standin-leave."""
    with socket.create_connection(("127.0.0.1", port)) as s:
        s.sendall(b"HELLO CROWD 4242\n")
        s.sendall(f"POS {ROUTE101[0]} {ROUTE101[1]} 9 14 1 3 0\n".encode())
        s.settimeout(0.5)
        while not stop.is_set() and not os.path.exists(os.path.join(out, "standin-leave")):
            try:
                if not s.recv(4096):
                    break
            except socket.timeout:
                pass


def main():
    out = os.path.abspath(sys.argv[1])
    os.makedirs(out, exist_ok=True)
    only = sys.argv[2] if len(sys.argv) > 2 else None  # "spawns" or "channels"
    if only != "channels":
        subprocess.call([sys.executable, run_py, os.path.join(here, "spawns.lua"), os.path.join(out, "spawns"), "60000", "2"],
                        cwd=bridge_dir)
    if only == "spawns":
        return

    port = free_port()
    channels = os.path.join(out, "channels")
    if os.path.exists(os.path.join(channels, "standin-leave")):
        os.remove(os.path.join(channels, "standin-leave"))
    env = dict(os.environ, NETPLAY_CHANNEL_SIZE="2")
    server = subprocess.Popen(
        [sys.executable, os.path.join(root, "server", "netplay_server.py"), "--host", "127.0.0.1",
         "--port", str(port), "--db", os.path.join(out, "channels-league.sqlite3")],
        env=env, stdout=open(os.path.join(out, "channels-server.log"), "w"), stderr=subprocess.STDOUT)
    stop = threading.Event()
    try:
        time.sleep(0.5)
        threading.Thread(target=stand_in, args=(port, channels, stop), daemon=True).start()
        subprocess.call([sys.executable, run_py, os.path.join(here, "channels.lua"), channels, "60000", "2"],
                        env=dict(os.environ, SHOTS_SERVER_PORT=str(port)), cwd=bridge_dir)
    finally:
        stop.set()
        server.terminate()
        server.wait()


if __name__ == "__main__":
    main()
