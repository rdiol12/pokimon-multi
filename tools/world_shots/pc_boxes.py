#!/usr/bin/env python3
"""Screenshots of the 32 PC boxes: python3 tools/world_shots/pc_boxes.py <out dir>"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, here)
import run as world_run  # noqa: E402

world_run.SYMBOLS += ["gPokemonStoragePtr", "CB2_ReturnToPokeStorage"]
sys.argv = [sys.argv[0], os.path.join(here, "pc_boxes.lua"), sys.argv[1]]
world_run.main()
