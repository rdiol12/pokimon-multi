#!/usr/bin/env python3
"""Headless two-player screenshots of the sixth-batch online features.

Usage: run.py <scenario.lua> <out dir> [frames] [players] [seed.py]

Like tools/world_shots/run.py (whose driver helpers it reuses), plus:
- the games run with tools/netplay_bridge as the working directory, so the
  bridge loads its extensions (the Online Plaza channel these features use);
- seed.py, if given, has `seed(port, out)`, run once the server is up and
  before the games start: it can play other players with raw protocol lines
  (see `Fake` below) to fill boards. `seed` may return a function, which is
  then run in a thread alongside the games (`after(port, out)`). The server
  also serves its web pages, on port `Fake.http`.
"""
import importlib.util
import os
import shutil
import socket
import subprocess
import sys
import threading
import time

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))
sys.path.insert(0, os.path.join(root, "tools", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gOnlineShowcase", "gNetplayExtMailbox", "gBattleResults", "gSpecialVar_0x800A", "gSpecialVar_0x800B"]


class Fake:
    """Another player on the server, speaking the protocol directly."""

    def __init__(self, port, name, tid):
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=5)
        self.f = self.sock.makefile("rw")
        self.seq = 0
        self.say(f"HELLO {name} {tid}")
        self.f.readline()

    def say(self, line):
        self.f.write(line + "\n")
        self.f.flush()

    def ask(self, request):
        self.seq += 1
        self.say(f"X {self.seq} {request}")
        for line in self.f:
            if line.startswith(f"XR {self.seq} "):
                parts = line.split()
                return int(parts[2]), bytes.fromhex(parts[3]).decode("ascii", "replace") if len(parts) > 3 else ""
        return None

    def close(self):
        self.sock.close()


def main():
    args = sys.argv[1:]
    seed_path = args.pop() if args and args[-1].endswith(".py") else None
    scenario, out = args[0], args[1]
    frames = args[2] if len(args) > 2 else "200000"
    players = int(args[3]) if len(args) > 3 else 2
    sys.argv = [sys.argv[0], scenario, out, frames, str(players)]

    # world_run.main starts the server and then the harness; hook both to
    # run the seed in between and to set the harness's working directory.
    real_popen, real_call = subprocess.Popen, subprocess.call
    state = {}

    def popen(cmd, *a, **kw):
        if "netplay_server.py" in " ".join(map(str, cmd)):
            state["port"] = int(cmd[cmd.index("--port") + 1])
            with socket.socket() as s:
                s.bind(("127.0.0.1", 0))
                state["http"] = s.getsockname()[1]
            cmd = [*cmd, "--http-port", str(state["http"])]
            Fake.http = state["http"]
        return real_popen(cmd, *a, **kw)

    def call(cmd, *a, **kw):
        if not str(cmd[0]).endswith("harness"):
            return real_call(cmd, *a, **kw)
        kw["cwd"] = os.path.join(root, "tools", "netplay_bridge")
        shutil.copy(os.path.join(here, "lib.lua"), os.path.join(os.path.abspath(out), "lib.lua"))
        thread = None
        if seed_path and "port" in state:
            time.sleep(0.5)
            spec = importlib.util.spec_from_file_location("seed", seed_path)
            mod = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(mod)
            Fake.port = state["port"]
            mod.Fake = Fake
            after = mod.seed(state["port"], os.path.abspath(out))
            if callable(after):
                thread = threading.Thread(target=after, args=(state["port"], os.path.abspath(out)), daemon=True)
                thread.start()
        return real_call(cmd, *a, **kw)

    subprocess.Popen, subprocess.call = popen, call
    world_run.main()


if __name__ == "__main__":
    main()
