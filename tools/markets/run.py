#!/usr/bin/env python3
"""Headless in-game screenshots of the Market Hall and the Auction House.

Usage (from tools/netplay_bridge, so the bridge loads its extensions):
    python3 ../markets/run.py ../markets/shots.lua <out dir> 60000 2

The areas' runner (tools/areas/shots/run.py: Driver.scene, Driver.season),
with the online request mailbox added for the auction.
"""
import importlib.util, os, sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, '..', 'world_shots'))
import run as world  # noqa: E402  (tools/world_shots/run.py; the areas' runner imports it too)

world.SYMBOLS += ['gNetplayExtMailbox']
spec = importlib.util.spec_from_file_location('areas_run', os.path.join(here, '..', 'areas', 'shots', 'run.py'))
areas = importlib.util.module_from_spec(spec)
spec.loader.exec_module(areas)

if __name__ == '__main__':
    areas.main()
