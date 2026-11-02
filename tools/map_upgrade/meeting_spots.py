"""Puts a meeting spot in every new-area town and every Solmara town: the
square guide (online HUB and PLAZA) and the matchmaker (QUICK MATCH), side
by side on the most open ground near the middle of town that's reachable
from the town's first door.

    python3 tools/map_upgrade/meeting_spots.py

Data only (the two people are added to each map.json). Idempotent: it drops
its own people first. The area and Solmara generators rewrite map.json, so
run this again after regenerating one of those towns.
"""
import json, os, sys
from collections import deque
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from mapdata import ROOT, Grid, maps
from extend import write_json

TOWNS = ['Cobblerock', 'EmberSprings', 'Hollowmere', 'LaurelCity', 'MirageOasis', 'NimbusCity',
         'SnowdriftVillage', 'SunfairCarnival', 'WonderwildPark',
         'PortSolace', 'Gearholt', 'Ashcombe', 'Silkwood', 'Rosemead', 'Duskmere', 'Frostholm', 'Drakespire']
GUIDE, MATCH = 'Square_EventScript_AreaGuide', 'Square_EventScript_AreaMatchmaker'


def reachable(g, m):
    if not m.get('warp_events'):   # no doors: the biggest open area
        best = set()
        for y in range(g.h):
            for x in range(g.w):
                if g.walkable(x, y) and (x, y) not in best:
                    f = flood(g, (x, y))
                    if len(f) > len(best):
                        best = f
        return best
    w = m['warp_events'][0]
    return flood(g, (w['x'], w['y'] + 1))


def flood(g, start):
    seen, q = {start}, deque([start])
    while q:
        x, y = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            z, nz = g.block(x, y) >> 12, g.block(*n) >> 12 if g.walkable(*n) else -1
            if n not in seen and nz >= 0 and (z == nz or z in (0, 15) or nz in (0, 15)):
                seen.add(n)
                q.append(n)
    return seen


def place(folder):
    path = f'{ROOT}/data/maps/{folder}/map.json'
    m = json.load(open(path))
    m['object_events'] = [o for o in m['object_events'] if o.get('script') not in (GUIDE, MATCH)]
    g = Grid(m['layout'])
    ok = reachable(g, m)
    best = None
    for margin in (2, 1):   # keep clear of everything else; closer only if there's no room
        busy = set()
        for key in ('object_events', 'warp_events', 'bg_events', 'coord_events'):
            for e in m.get(key) or []:
                for dx in range(-margin, margin + 1):
                    for dy in range(-margin, margin + 1):
                        busy.add((e['x'] + dx, e['y'] + dy))
        for y in range(1, g.h - 4):
            for x in range(1, g.w - 6):
                box = [(x + dx, y + dy) for dx in range(6) for dy in range(4)]
                if all(c in ok and c not in busy for c in box):
                    d = abs(x + 3 - g.w / 2) + abs(y + 2 - g.h / 2)
                    if best is None or d < best[0]:
                        best = (d, x, y)
        if best:
            break
    if not best:
        print(f'{folder}: no open spot')
        return
    _, x, y = best
    def person(gfx, px, script):
        return {'graphics_id': gfx, 'x': px, 'y': y + 1, 'elevation': g.block(px, y + 1) >> 12, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN',
                'movement_range_x': 0, 'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                'trainer_sight_or_berry_tree_id': '0', 'script': script, 'flag': '0'}
    m['object_events'] += [person('OBJ_EVENT_GFX_WOMAN_2', x + 1, GUIDE),
                           person('OBJ_EVENT_GFX_COOLTRAINER_M', x + 4, MATCH)]
    write_json(path, m)
    print(f'{folder}: guide ({x + 1},{y + 1}), matchmaker ({x + 4},{y + 1})')


if __name__ == '__main__':
    for t in TOWNS:
        place(t)
