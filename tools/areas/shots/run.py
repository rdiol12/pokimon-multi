#!/usr/bin/env python3
"""Headless in-game screenshots of the new areas.

Usage: run.py <scenario.lua> <out dir> [frames]

Same as tools/world_shots/run.py (Driver.scene warps anywhere), plus
Driver.flag(name, on) to set any flag (FLAG_* names, including the area
flags that live in SaveBlock3) before warping, Driver.where() for the
player's position and Driver.season(s) to force a season.
"""
import os, re, subprocess, sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.abspath(os.path.join(here, '..', '..', '..'))
sys.path.insert(0, os.path.join(root, 'tools', 'world_shots'))
import run as world  # noqa: E402

CFLAGS = ['-mthumb', '-mabi=apcs-gnu', '-O2', '-std=gnu17', '-iquote', os.path.join(root, 'include'),
          '-DMODERN=1', '-DTESTING=0', '-DEMERALD']


def constants(names):
    """Values of C constants (flags etc.), learned by compiling a snippet."""
    src = '#include "global.h"\n#include "constants/flags.h"\n' + open(os.path.join(here, 'offsets.c')).read().split('\n', 1)[1]
    src += ''.join(f'const int kC_{n} = {n};\n' for n in names)
    path = os.path.join('/tmp', 'area_constants.c')
    open(path, 'w').write(src)
    asm = subprocess.check_output(['arm-none-eabi-gcc', *CFLAGS, '-S', '-o', '-', path], cwd=root).decode()
    return {m.group(1): int(m.group(2)) for m in re.finditer(r'^k(?:C_)?(\w+):\n\t\.word\t(-?\d+)', asm, re.M)}


def main():
    scenario = sys.argv[1]
    names = sorted(set(re.findall(r'\b(FLAG_\w+)', open(scenario).read())))
    c = constants(names)
    world.SYMBOLS += ['gSaveBlock3Ptr', 'gObjectEvents', 'gPlayerAvatar', 'gLifeShowcase']
    world.HELPERS += '\nFLAG = {\n' + ''.join(f'    {n} = {c[n]},\n' for n in names) + '}\n' + f'''
-- Sets or clears a flag (area flags live in SaveBlock3).
function Driver.flag(name, on)
    local id = FLAG[name] or name
    local addr
    if id >= 0x6000 then
        addr = E.read32(Driver.SYM.gSaveBlock3Ptr) + {c['SaveBlock3Areas']} + (id - 0x6000) // 8
    else
        addr = E.read32(Driver.SYM.gSaveBlock1Ptr) + {c['SaveBlock1Flags']} + id // 8
    end
    local v = E.read8(addr)
    local bit = 1 << (id % 8)
    if on == false then v = v & ~bit else v = v | bit end
    E.write8(addr, v)
end

-- Forces a season (SEASON_SPRING .. SEASON_WINTER = 0 .. 3; nil uses the clock).
-- Takes effect on the next warp.
function Driver.season(s)
    E.write8(Driver.SYM.gLifeShowcase + {c['LifeShowcaseSeason']}, s and s + 1 or 0)
end

-- The player's map position (x, y).
function Driver.where()
    local id = E.read8(Driver.SYM.gPlayerAvatar + {c['PlayerAvatarObjectId']})
    local o = Driver.SYM.gObjectEvents + id * {c['ObjectEventSize']} + {c['ObjectEventCoords']}
    local x, y = E.read16(o), E.read16(o + 2)
    return x - 7, y - 7
end
'''
    world.main()


if __name__ == '__main__':
    main()
