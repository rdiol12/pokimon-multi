"""Pokémon fashion online: TEST1 and TEST2 meet in Oldale Town, each with an
accessory on their follower (in Slateport, its people take every object
slot, so other players' followers don't show). Writes the addresses the
scenario dresses the lead Pokémon with (fashion_addr.lua in the output
folder)."""
import os
import subprocess

ENV = {"NETPLAY_FISHT_LENGTH": 0}
HERE = os.path.dirname(os.path.abspath(__file__))


def seed(port, out):
    nm = subprocess.run(["arm-none-eabi-nm", os.path.join(HERE, "..", "..", "pokeemerald.elf")],
                        capture_output=True, text=True).stdout
    sym = {line.split()[2]: int(line.split()[0], 16) for line in nm.splitlines() if len(line.split()) == 3}
    with open(os.path.join(out, "fashion_addr.lua"), "w") as f:
        f.write(f"PASTIME_SAVE_PTR = 0x{sym['gPastimeSavePtr']:08x}\n")
        f.write(f"PLAYER_PARTY = 0x{sym['gParties']:08x}\n")  # gParties[B_TRAINER_PLAYER]
