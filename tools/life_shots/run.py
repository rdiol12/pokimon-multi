#!/usr/bin/env python3
"""Headless in-game screenshots of the fifth batch's features (docs/life.md).

Usage: run.py <scenario.lua> <out dir> [frames] [players]

The same runner as tools/world_shots/run.py (the scenarios use its Driver,
including Driver.scene), with this batch's symbols added.
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "world_shots"))
import run  # noqa: E402

run.SYMBOLS += ["gLifeShowcase", "gLifeMailbox", "gSaveBlock3Ptr"]

if __name__ == "__main__":
    run.main()
