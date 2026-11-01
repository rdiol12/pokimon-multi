#!/usr/bin/env python3
"""Builds the Haven Reserve (data/maps/HavenReserve, docs/reserve.md): a
guarded meadow west of Verdanturf where trainers leave Pokémon they don't
want. The left Pokémon roam inside, and anyone can walk in and battle or
catch them.

    python3 tools/reserve/make_reserve.py

Layout, 36x30, Hoenn's small-town look (tools/areas/styles.py town()):
a forest ring around a meadow with tall grass, a pond and tree clumps; a
tree line across the south with one gate in it, watched by two guards;
below it the gate yard with the warden's lodge (a Petalburg house), the
warden who takes Pokémon in, and the notice board. The way back to
Verdanturf is at the bottom middle.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'areas'))
import copy  # noqa: E402
from build import Area, interior, npc, ROOT  # noqa: E402
from styles import town, POND  # noqa: E402

NAME = 'HavenReserve'
W, H = 36, 30
GATE_X = 17          # the gate in the south tree line: x 17-18
FENCE_Y = 20         # the tree line: rows 20-21

# Where the roaming Pokémon start, and how far each wanders (all on open
# meadow, clear of the pond and the trees).
ROAMERS = [(6, 6, 2, 2), (12, 4, 2, 1), (17, 7, 3, 2), (9, 12, 2, 2),
           (15, 14, 3, 2), (26, 13, 2, 2), (30, 9, 1, 3), (22, 17, 3, 1)]
# 12 more since the extended GBA (64 objects): objects 14-25.
MORE_ROAMERS = [(5, 10, 2, 2), (8, 16, 2, 1), (11, 14, 1, 2), (14, 11, 2, 1),
                (20, 13, 2, 2), (21, 5, 1, 1), (27, 16, 2, 1), (32, 9, 1, 3),
                (4, 14, 1, 2), (28, 11, 1, 1), (9, 3, 2, 1), (14, 18, 2, 1)]

st = copy.copy(town())
st.exclude = set(st.exclude) | {0x129, 0x087, 0x0C6, 0x1D4, 0x1E7, 0x1D9, 0x1D1, 0x1E0, 0x1E1, 0x1E2}  # worn sand, fences, signs, worn grass
a = Area(NAME, W, H, st, fill='T')

# The meadow inside the ring of trees.
a.rect(2, 3, 32, 17, 'g')
a.rect(4, 2, 28, 1, 'g')
# The tree line with the gate, and the gate yard below it.
a.rect(0, FENCE_Y, W, 2, 'T')
a.rect(4, 22, 28, 6, 'g')
a.rect(GATE_X, FENCE_Y, 2, 2, 'g')
a.rect(GATE_X, 27, 2, 3, 'g')
# Tall grass where the Pokémon like to hide (no wild encounters: the map
# has none, the grass is for looks).
a.ellipse(7, 15.5, 4, 2.5, '"')
a.ellipse(13, 3.5, 4, 1.6, '"')
a.ellipse(28, 17.5, 3.5, 1.6, '"')
# Tree clumps, 2x2 like everywhere in Hoenn.
for x, y in [(20, 2), (2, 8), (12, 8), (32, 16), (2, 18), (24, 12), (32, 2)]:
    a.rect(x, y, 2, 2, 'T')
a.snap_trees(ground='g')
# Sand paths: from the way out up through the gate into the meadow, and
# across the yard to the lodge door.
a.line([(GATE_X + 0.5, 29), (GATE_X + 0.5, 10)], 2, 's')
a.rect(7, 26, GATE_X - 7, 2, 's')
a.line([(GATE_X, 10), (27, 10)], 2, 's')
# The pond.
a.rect(23, 3, 8, 5, 'P')
a.autotile3('P', POND)
# The warden's lodge: Petalburg's small house (door at its bottom left).
a.stamp('LAYOUT_PETALBURG_CITY', 9, 16, 4, 4, 6, 22)
LODGE_DOOR = (7, 25)
FLOWERS = [(4, 4), (10, 6), (21, 6), (30, 12), (6, 18), (14, 17), (24, 18), (22, 25), (27, 24), (11, 26)]
for i, (x, y) in enumerate(FLOWERS):
    for dx, dy in ((0, 0), (1, 0), (0, 1), (1, 1), (2, 0))[:2 + i % 3]:
        if a.grid[y + dy][x + dx] == 'g' and (x + dx, y + dy) not in a.fixed:
            a.tile(x + dx, y + dy, 0x004, collision=0)

# --- events -------------------------------------------------------------------
# Object order = local ids 1.. (the roamers are 1-8 and 14-25, see include/constants/reserve.h).
for i, (x, y, rx, ry) in enumerate(ROAMERS):
    a.obj(f'OBJ_EVENT_GFX_VAR_{i:X}', x, y, f'HavenReserve_EventScript_Roamer{i + 1}',
          'MOVEMENT_TYPE_WANDER_AROUND', f'FLAG_TEMP_{0x11 + i:X}', rx=rx, ry=ry)
a.obj('OBJ_EVENT_GFX_POLICEMAN', GATE_X - 1, 22, 'HavenReserve_EventScript_GuardWest', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_POLICEMAN', GATE_X + 2, 22, 'HavenReserve_EventScript_GuardEast', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WOMAN_5', 13, 24, 'HavenReserve_EventScript_Warden', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_POLICEMAN', 5, 4, 'HavenReserve_EventScript_Patrol', 'MOVEMENT_TYPE_WALK_LEFT_AND_RIGHT', rx=4)
a.obj('OBJ_EVENT_GFX_PICNICKER', 21, 15, 'HavenReserve_EventScript_Visitor', 'MOVEMENT_TYPE_FACE_LEFT')
# Roamers 9-20 (local ids 14-25, after the people above).
for i, (x, y, rx, ry) in enumerate(MORE_ROAMERS):
    a.obj('OBJ_EVENT_GFX_VAR_0', x, y, f'HavenReserve_EventScript_Roamer{i + 9}',
          'MOVEMENT_TYPE_WANDER_AROUND', f'FLAG_TEMP_{0x1 + i:X}', rx=rx, ry=ry)
a.sign(21, 23, 'HavenReserve_EventScript_Board')
a.sign(15, 28, 'HavenReserve_EventScript_Sign')
a.warp(LODGE_DOOR[0], LODGE_DOOR[1], 'MAP_HAVEN_RESERVE_LODGE', 0)
for x in (GATE_X, GATE_X + 1):
    a.coord(x, 29, 'HavenReserve_EventScript_Leave')

MAP_SCRIPTS = r'''
@ The Haven Reserve (src/reserve.c, data/scripts/reserve.inc,
@ tools/reserve/make_reserve.py).

HavenReserve_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HavenReserve_OnTransition
	.byte 0

HavenReserve_OnTransition:
	callnative Reserve_SetupMap
	end
'''

a.write(map_id='MAP_HAVEN_RESERVE', layout_id='LAYOUT_HAVEN_RESERVE', music='MUS_ROUTE104',
        mapsec='MAPSEC_HAVEN_RESERVE', mapsec_name='HAVEN RESERVE', weather='WEATHER_SUNNY',
        scripts=MAP_SCRIPTS, allow_cycling=False, show_name=True)

# The lodge: a house with the reserve's nurse and the head guard.
interior('HavenReserve_Lodge', 'MAP_HAVEN_RESERVE_LODGE', 'LAYOUT_HOUSE1', [(3, 8), (4, 8)],
         'MAP_HAVEN_RESERVE', 0,
         [npc('OBJ_EVENT_GFX_NURSE', 7, 4, 'HavenReserve_EventScript_Nurse'),
          npc('OBJ_EVENT_GFX_POLICEMAN', 2, 4, 'HavenReserve_EventScript_HeadGuard', 'MOVEMENT_TYPE_FACE_RIGHT')],
         'HavenReserve_Lodge_MapScripts::\n\t.byte 0\n',
         'MAPSEC_HAVEN_RESERVE', 'gMapGroup_TownsAndRoutes', music='MUS_ROUTE104')
print('wrote', NAME)
