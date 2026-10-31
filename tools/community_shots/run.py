#!/usr/bin/env python3
"""Headless in-game screenshots of the community features.

Usage: run.py <scenario.lua> <out dir> [frames] [players] [VAR=value...]

The world screenshot runner (tools/world_shots/run.py) with the community
mailbox's symbol added and community_lib.lua loaded before the scenario.
VAR=value pairs go into the server's environment (NETPLAY_TOUR_SIZE=4).
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gCommunityMailbox", "gCommunityShowcase", "gStringVar1", "gSpecialVar_0x8004"]
world_run.HELPERS += open(os.path.join(here, "community_lib.lua")).read()

if __name__ == "__main__":
    env = [a for a in sys.argv[1:] if "=" in a and not a.startswith("/")]
    for pair in env:
        k, v = pair.split("=", 1)
        os.environ[k] = v
    sys.argv = [sys.argv[0]] + [a for a in sys.argv[1:] if a not in env]
    world_run.main()
