"""The Stormwild: the extraction zone (docs/stormwild.md). A wild, overgrown
basin with no road out: players arrive at one of eight drop points around
the edge, hunt for supply caches and rare Pokémon, and leave only through an
extraction beacon (src/stormwild.c, server/online_xrun.py).

The beacons, caches and drop points are objects/coordinates that
src/stormwild.c also lists (sBeacons, sCaches, sDrops); keep them in step.

    python3 tools/areas/stormwild.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import woods, POND

NAME = 'Stormwild'
W, H = 64, 64

st = woods()
st.exclude |= {0x118, 0x119, 0x11A, 0x120, 0x121, 0x122, 0x128, 0x129, 0x12A}  # no sand patches
a = Area(NAME, W, H, st, fill='T')

# --- terrain ------------------------------------------------------------
# A ring trail around the basin, with spokes to the middle.
ring_pts = [(8, 8), (32, 5), (56, 8), (59, 32), (56, 56), (32, 59), (8, 56), (5, 32), (8, 8)]
a.line(ring_pts, 3, '.')
for p in [(32, 5), (59, 32), (32, 59), (5, 32)]:
    a.line([p, (32, 32)], 2, '.')
a.line([(8, 8), (20, 20)], 2, '.')
a.line([(56, 56), (44, 44)], 2, '.')

# Clearings: the three beacon sites and a few meadows.
a.ellipse(11, 11, 7, 6, '.')      # NW: the old watchtower ruins (beacon A)
a.ellipse(53, 30, 6, 7, '.')      # E: lakeshore (beacon B)
a.ellipse(30, 54, 8, 5, '.')      # S: the hollow (beacon C)
a.ellipse(32, 32, 10, 8, '.')     # the middle: the storm lake
a.ellipse(50, 12, 6, 5, '.')      # NE meadow
a.ellipse(13, 47, 6, 6, '.')      # SW thicket clearing
a.ellipse(47, 49, 5, 4, '.')
a.ellipse(20, 26, 5, 4, '.')

# Tall grass where the rare ones hide.
for cx, cy, rx, ry in [(50, 12, 4.5, 3.5), (13, 47, 4.5, 4), (20, 26, 4, 3), (40, 20, 3, 2.5),
                       (25, 40, 3.5, 2.5), (46, 49, 3.5, 2.5), (30, 14, 4, 2), (57, 44, 2, 3)]:
    a.ellipse(cx, cy, rx, ry, '"')
a.ellipse(40, 20, 3.5, 3, '.')
a.ellipse(40, 20, 3, 2.5, '"')
a.ellipse(25, 40, 4, 3, '.')
a.ellipse(25, 40, 3.5, 2.5, '"')
a.ellipse(30, 14, 4.5, 2.5, '.')
a.ellipse(30, 14, 4, 2, '"')

# The storm lake, with a little island.
a.ellipse(32, 32, 6, 4.5, 'P')
a.ellipse(32, 32, 1.6, 1.2, '.')
a.ellipse(53, 34, 3, 2, 'P')

# Scattered big trees and copses, so the open ground doesn't feel empty.
for x, y in [(14, 14), (8, 12), (26, 54), (34, 55), (48, 14), (16, 50), (36, 28), (27, 35),
             (14, 30), (10, 36), (44, 30), (46, 40), (40, 56), (20, 58), (42, 8), (24, 7),
             (58, 26), (58, 48), (6, 44), (20, 34), (38, 46), (50, 56), (12, 22), (46, 18)]:
    a.rect(x & ~1, y & ~1, 2, 2, 'T')
a.snap_trees()
# Keep the drop points, beacons and caches clear of trees after snapping.
DROPS = [(8, 8), (32, 5), (56, 8), (59, 32), (56, 56), (32, 59), (8, 56), (5, 32)]
BEACONS = [(11, 10), (54, 28), (30, 53)]
CACHES = [
    (17, 9), (6, 14), (44, 7), (53, 12), (58, 20), (49, 24), (57, 38), (51, 46),
    (44, 52), (36, 58), (24, 56), (12, 51), (17, 44), (4, 40), (6, 24), (18, 24),
    (24, 29), (38, 25), (41, 37), (28, 42), (32, 31), (36, 16), (26, 13), (60, 52),
]
for x, y in DROPS + BEACONS + CACHES:
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            if a.grid[y + dy][x + dx] == 'T':
                a.put(x + dx, y + dy, '.')
a.snap_trees()
for x, y in DROPS + BEACONS + CACHES:
    a.put(x, y, '.')
    a.put(x, y + 1, '.')

a.autotile3('P', POND)
a.scatter(0x004, 36, 2, 2, 62, 62, seed=11)

# --- events -------------------------------------------------------------
# Beacons: local ids 1-3. Caches: 4-27 (src/stormwild.c hides the ones not
# in this run, or already looted).
for i, (x, y) in enumerate(BEACONS):
    a.obj('OBJ_EVENT_GFX_STORM_BEACON', x, y, f'{NAME}_EventScript_Beacon', 'MOVEMENT_TYPE_FACE_DOWN',
          local_id=f'LOCALID_STORMWILD_BEACON_{i + 1}')
for i, (x, y) in enumerate(CACHES):
    a.obj('OBJ_EVENT_GFX_STORM_CACHE', x, y, f'{NAME}_EventScript_Cache', 'MOVEMENT_TYPE_FACE_DOWN',
          f'FLAG_TEMP_{i + 1:X}')

SCRIPTS = r'''
@ The Stormwild (src/stormwild.c, data/scripts/stormwild.inc).
Stormwild_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Stormwild_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, Stormwild_OnResume
	.byte 0

Stormwild_OnTransition:
	callnative Stormwild_SetupObjects
	end

Stormwild_OnResume:
	callnative Stormwild_HideTakenCaches
	end
'''

a.write(map_id='MAP_STORMWILD', layout_id='LAYOUT_STORMWILD', music='MUS_RG_VICTORY_ROAD',
        mapsec='MAPSEC_STORMWILD', mapsec_name='STORMWILD', weather='WEATHER_RAIN',
        scripts=SCRIPTS, allow_escaping=False)

register_wild('MAP_STORMWILD', 'gStormwild',
              land=[('ABSOL', 30, 34), ('ZORUA', 30, 34), ('RIOLU', 30, 34), ('AXEW', 30, 34),
                    ('LARVITAR', 30, 34), ('DEINO', 30, 34), ('GOOMY', 32, 35), ('JANGMO_O', 32, 35),
                    ('BAGON', 33, 36), ('BELDUM', 33, 36), ('GIBLE', 34, 36), ('DREEPY', 35, 38)],
              water=[('DRATINI', 30, 34), ('WOOPER', 28, 32), ('LAPRAS', 34, 38), ('DRATINI', 32, 36), ('DRAGONAIR', 36, 40)],
              fish=[('MAGIKARP', 20, 30), ('FEEBAS', 20, 30), ('DRATINI', 28, 32), ('BARBOACH', 28, 32), ('DRATINI', 28, 34),
                    ('GYARADOS', 32, 36), ('DRATINI', 32, 36), ('DRAGONAIR', 34, 38), ('MILOTIC', 36, 40), ('DRAGONAIR', 38, 40)])

# The Storm Warden, by Mauville's Pokémon Center (runners come back to the
# tile below; GATE_X/GATE_Y in src/stormwild.c).
add_object('MauvilleCity', npc('OBJ_EVENT_GFX_HIKER', 20, 6, 'Stormwild_EventScript_Warden'))
