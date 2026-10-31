#!/usr/bin/env python3
"""In-game screenshots of the trainer look (docs/trainer_look.md).

Usage: shots.py <scenario.lua> <out dir> [frames] [players]

Runs tools/world_shots/run.py with the scenario, first adding LOOK_SYMS:
addresses of the Look screen's own (static) symbols, read with
arm-none-eabi-nm, so a scenario can tell when the screen is up.
"""
import os
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, "..", ".."))
sys.path.insert(0, os.path.join(root, "tools", "world_shots"))
import run as world_run  # noqa: E402

WANTED = ["CB2_Look", "CB2_NamingScreen", "sLook", "CB2_InitLook", "CB2_NewGameBirchSpeech_ReturnFromLookScreen", "CB2_MainMenu", "sStartMenuCursorPos", "sCurrentStartMenuActions"]


def main():
    nm = subprocess.check_output(["arm-none-eabi-nm", os.path.join(root, "pokeemerald.elf")], text=True)
    syms = {}
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] in WANTED:
            syms[parts[2]] = int(parts[0], 16)
    scenario = os.path.abspath(sys.argv[1])
    out = os.path.abspath(sys.argv[2])
    tmp = os.path.join(os.path.dirname(out), "." + os.path.basename(scenario))
    with open(tmp, "w") as f:
        f.write("LOOK_SYMS = {\n" + "".join(f"    {k} = 0x{v:08X},\n" for k, v in syms.items()) + "}\n")
        f.write(f'LOOK_LIB = "{os.path.join(here, "shots", "lib.lua")}"\n')
        f.write(open(scenario).read())
    sys.argv[1] = tmp
    world_run.main()


if __name__ == "__main__":
    main()
