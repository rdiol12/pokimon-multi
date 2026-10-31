"""Draws the field-ability object sprites (graphics/field_ability/*.png),
16x16, indexed in the overworld NPC palettes they use in game."""
import os
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))


# One palette for all of them (graphics/field_ability/field_ability.pal),
# colours taken from the overworld NPC palettes.
PAL = [
    (115, 197, 164),  # 0 transparent
    (255, 255, 255),  # 1 white
    (148, 197, 246),  # 2 light blue
    (90, 139, 189),   # 3 blue
    (16, 49, 82),     # 4 navy
    (213, 106, 123),  # 5 red
    (238, 222, 164),  # 6 light sand
    (255, 213, 32),   # 7 yellow
    (222, 156, 16),   # 8 orange
    (213, 213, 213),  # 9 light grey
    (139, 139, 148),  # 10 grey
    (65, 65, 82),     # 11 dark grey
    (106, 139, 172),  # 12 steel
    (180, 164, 98),   # 13 sand
    (123, 115, 65),   # 14 dark sand
    (57, 57, 24),     # 15 darkest sand
]


def pal(name):
    return PAL


def save(name, palette, rows, key):
    img = Image.new('P', (16, 16))
    flat = [c for rgb in palette for c in rgb]
    img.putpalette(flat + [0] * (768 - len(flat)))
    img.putdata([key[ch] for row in rows for ch in row])
    img.save(f'{ROOT}/graphics/field_ability/{name}.png', transparency=0)


# npc_1: 8 light blue, 9 blue, 10 navy, 6/7 reds, 11-13 tans, 1 pale highlight, 14 white
ICE = [
    "................",
    "...NNNNNNNNNN...",
    "..NWWWLLLLLLLN..",
    ".NWWLLLLLLLLLLN.",
    ".NWLLLLLLLLLLMN.",
    ".NLLLLLLLLLLMMN.",
    ".NNMMMMMMMMMMNN.",
    ".NWMMMMrrrMMMMN.",
    ".NWMMMrrrrrMMMN.",
    ".NLMMMRRWRRMMMN.",
    ".NLMMMLLLLLMMMN.",
    ".NMMMMMLLLMMMWN.",
    ".NMMMMMMMMMMWWN.",
    ".NMMMMMMMMMWWMN.",
    "..NNNNNNNNNNNN..",
    "................",
]
save('ice_block', pal('npc_1'), ICE, {'.': 0, 'N': 4, 'M': 3, 'L': 2, 'W': 1, 'r': 5, 'R': 4})

DIG = [
    "..........L.....",
    "..........W.....",
    "........LWWWL...",
    "..........W.....",
    "...L......L.....",
    "...W............",
    ".LWWWL..........",
    "...W....D.......",
    "...L..DDdDD.....",
    "....DDdShSdDD...",
    "...DdSShhSSSdD..",
    "..DdSSSShSSdSdD.",
    "..DdddSSSSdddDD.",
    "...DDddddddDDD..",
    ".....DDDDDDD....",
    "................",
]
save('dig_spot', pal('npc_1'), DIG, {'.': 0, 'W': 1, 'L': 2, 'h': 6, 'S': 13, 'd': 14, 'D': 15})

# npc_4: 11 light grey, 12 grey, 13 dark grey, 7 near black, 5 steel, 6 dark steel, 1-3 warm glow
GEN_OFF = [
    "................",
    ".....kkk........",
    "...kkkHkkkkkk...",
    "..kGGGGGGGGGGk..",
    "..kGggggggggGk..",
    "..kDDDDDDDDDDk..",
    "..kgppppppppgk..",
    "..kgpppBBpppgk..",
    "..kgppBBppppgk..",
    "..kgpBBBBBppgk..",
    "..kgpppBBpppgk..",
    "..kgppBBppppgk..",
    "..kgppppppppgk..",
    "..kgDDDDDDDDgk..",
    "..kkkkkkkkkkkk..",
    "................",
]
GEN_ON = [
    "..y..........y..",
    ".y...kkk....y...",
    "...kkkHkkkkkk...",
    "..kGGGGGGGGGGk..",
    "..kGggggggggGk..",
    "..kDDDDDDDDDDk..",
    "..kgPPPPPPPPgk..",
    "..kgPPPYYPPPgk..",
    "..kgPPYWPPPPgk..",
    "..kgPYWWWYPPgk..",
    "..kgPPPWYPPPgk..",
    "..kgPPYYPPPPgk..",
    "y.kgPPPPPPPPgk.y",
    "..kgDDDDDDDDgk..",
    "..kkkkkkkkkkkk..",
    "................",
]
key = {'.': 0, 'k': 4, 'G': 9, 'g': 10, 'D': 11, 'H': 10, 'p': 11, 'P': 12, 'B': 10, 'Y': 8, 'W': 7, 'y': 7}
save('generator_off', pal('npc_4'), GEN_OFF, key)
save('generator_on', pal('npc_4'), GEN_ON, key)
with open(f'{ROOT}/graphics/field_ability/field_ability.pal', 'w') as f:
    f.write('JASC-PAL\r\n0100\r\n16\r\n' + ''.join(f'{r} {g} {b}\r\n' for r, g, b in PAL))
print('ok')
