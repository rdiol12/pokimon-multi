#!/usr/bin/env python3
"""Play Pokimon Multi online with one command.

    make play                  host: build, start the server, open the game
    make play JOIN=1.2.3.4     join a friend who is hosting

Hosting starts the netplay server next to the game (if one isn't already
running) and opens mGBA with the bridge script loaded and pointed at it, so
there is nothing to set up by hand. Friends join with your address. The
server stops when you close the game, unless you pass --keep-server.

mGBA 0.10 or newer is needed. It is found on PATH (mgba-qt, mgba, mGBA) or
in the usual install folders; set MGBA=/path/to/mgba-qt to use another one.
Standard library only.
"""

import argparse
import os
import shutil
import socket
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SERVER = os.path.join(ROOT, "server", "netplay_server.py")
BRIDGE = os.path.join(ROOT, "tools", "netplay_bridge", "netplay_bridge.lua")
OUT_DIR = os.path.join(ROOT, "build", "play")

MGBA_NAMES = ["mgba-qt", "mgba", "mGBA"]
MGBA_PATHS = [
    r"C:\Program Files\mGBA\mGBA.exe",
    r"C:\Program Files (x86)\mGBA\mGBA.exe",
    "/Applications/mGBA.app/Contents/MacOS/mGBA",
    "/usr/games/mgba-qt",
]


def server_running(host, port):
    try:
        with socket.create_connection((host, port), timeout=1):
            return True
    except OSError:
        return False


def lan_address():
    """The address friends on the same network use to join."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("10.255.255.255", 1))
        return s.getsockname()[0]
    except OSError:
        return "127.0.0.1"
    finally:
        s.close()


def find_mgba():
    found = os.environ.get("MGBA")
    if found:
        return found
    for name in MGBA_NAMES:
        found = shutil.which(name)
        if found:
            return found
    for path in MGBA_PATHS:
        if os.path.exists(path):
            return path
    return None


def bridge_script(host, port):
    """A copy of the bridge with the server address filled in, because mGBA
    can't pass settings to a script given on its command line."""
    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, "netplay_bridge.lua")
    with open(BRIDGE, encoding="utf-8") as f:
        script = f.read()
    with open(path, "w", encoding="utf-8") as f:
        f.write(f'NETPLAY_HOST = "{host}"\nNETPLAY_PORT = {port}\n' + script)
    return path


def start_server(args):
    os.makedirs(OUT_DIR, exist_ok=True)
    log = open(os.path.join(OUT_DIR, "server.log"), "a")
    cmd = [sys.executable, SERVER, "--host", "0.0.0.0", "--port", str(args.port),
           "--http-port", str(args.http_port), "--db", args.db]
    proc = subprocess.Popen(cmd, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    for _ in range(50):
        if server_running("127.0.0.1", args.port):
            return proc
        if proc.poll() is not None:
            break
        time.sleep(0.1)
    sys.exit(f"The server didn't start; see {log.name}")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--join", metavar="HOST", help="join a friend's game instead of hosting")
    p.add_argument("--port", type=int, default=7777, help="game port (default 7777)")
    p.add_argument("--http-port", type=int, default=8080, help="leaderboard page port when hosting (default 8080)")
    p.add_argument("--db", default=os.path.join(OUT_DIR, "league.sqlite3"), help="league database when hosting")
    p.add_argument("--rom", default=os.path.join(ROOT, "pokeemerald.gba"))
    p.add_argument("--no-build", action="store_true", help="don't run make first")
    p.add_argument("--keep-server", action="store_true", help="leave the server running after the game closes")
    p.add_argument("--server-only", action="store_true", help="host without opening the game")
    args = p.parse_args()

    if not args.no_build and not args.join:
        subprocess.run(["make", f"-j{os.cpu_count() or 1}"], cwd=ROOT, check=True)

    server = None
    host = args.join or "127.0.0.1"
    if not args.join:
        if server_running("127.0.0.1", args.port):
            print(f"Using the server already running on port {args.port}.")
        else:
            server = start_server(args)
            print(f"Server started on port {args.port}.")
        print(f"Friends join with:  make play JOIN={lan_address()}")
        if args.http_port:
            print(f"Leaderboard:        http://{lan_address()}:{args.http_port}/")
    elif not server_running(host, args.port):
        print(f"Can't reach {host}:{args.port} yet; the game will keep trying.")

    try:
        if args.server_only:
            if server is None:
                return
            print("Press Ctrl+C to stop the server.")
            server.wait()
            return
        if not os.path.exists(args.rom):
            sys.exit(f"No ROM at {args.rom}. Build it with make, or pass --rom.")
        mgba = find_mgba()
        if mgba is None:
            sys.exit("mGBA 0.10+ wasn't found. Install it from https://mgba.io or set MGBA=/path/to/mgba-qt.")
        subprocess.run([mgba, "--script", bridge_script(host, args.port), args.rom])
    except KeyboardInterrupt:
        pass
    finally:
        if server is not None and not args.keep_server:
            server.terminate()
            server.wait()
            print("Server stopped.")


if __name__ == "__main__":
    main()
