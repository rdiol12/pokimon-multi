"""Audits every map for things that get in the way when many players share it:
one-tile chokepoints on the main walk between exits, people standing in those
walkways or in front of doors, and story triggers.

    python3 tools/map_upgrade/audit.py [--json out.json] [--top N] [MAP_ID ...]
"""
import json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from mapdata import *

SKIP_MOVES = ('MOVEMENT_TYPE_INVISIBLE',)


def audit(map_id):
    folder, m = maps()[map_id]
    if 'layout' not in m or m['layout'] not in layouts():
        return None
    g = Grid(m['layout'])
    path, groups = main_paths(g, m)
    narrow = sorted(t for t in path if g.walkable(*t) and width_at(g, *t) == 1)
    # merge narrow tiles into runs
    runs, seen = [], set()
    for t in narrow:
        if t in seen:
            continue
        run, q = [], [t]
        seen.add(t)
        while q:
            c = q.pop()
            run.append(c)
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = (c[0] + dx, c[1] + dy)
                if n in narrow and n not in seen:
                    seen.add(n)
                    q.append(n)
        runs.append(sorted(run))
    doors_front = {(w['x'], w['y'] + 1) for w in m.get('warp_events', []) if g.is_door(w['x'], w['y'])}
    blockers = []
    for i, o in enumerate(m.get('object_events', [])):
        if o.get('type') == 'clone' or o.get('movement_type') in SKIP_MOVES:
            continue
        x, y = o['x'], o['y']
        why = None
        if (x, y) in doors_front:
            why = 'door'
        elif (x, y) in path and width_at(g, x, y) <= 2:
            why = 'narrow path'
        elif (x, y) in path and 'WANDER' in o.get('movement_type', ''):
            why = 'wanders on main path'
        if why:
            blockers.append({'index': i, 'x': x, 'y': y, 'gfx': o.get('graphics_id'), 'script': o.get('script'),
                             'flag': o.get('flag'), 'trainer': o.get('trainer_type') not in (None, 'TRAINER_TYPE_NONE', '0'),
                             'why': why})
    return {
        'map': map_id, 'folder': folder, 'type': m.get('map_type'), 'w': g.w, 'h': g.h,
        'exits': len(groups), 'path_tiles': len(path),
        'chokepoints': [{'tiles': len(r), 'at': r[0]} for r in runs],
        'choke_tiles': len(narrow),
        'blockers': blockers,
        'triggers': len([c for c in m.get('coord_events', []) if c.get('type') == 'trigger']),
    }


def main():
    args = sys.argv[1:]
    out_json, top = None, 40
    if '--json' in args:
        out_json = args[args.index('--json') + 1]
        del args[args.index('--json'):args.index('--json') + 2]
    if '--top' in args:
        top = int(args[args.index('--top') + 1])
        del args[args.index('--top'):args.index('--top') + 2]
    ids = args or list(maps())
    res = [r for r in (audit(i) for i in ids) if r]
    if out_json:
        json.dump(res, open(out_json, 'w'), indent=1)
    tot_c = sum(len(r['chokepoints']) for r in res)
    tot_b = sum(len(r['blockers']) for r in res)
    print(f'{len(res)} maps, {tot_c} chokepoint runs, {tot_b} people in walkways/doors')
    for r in sorted(res, key=lambda r: -(r['choke_tiles'] + 3 * len(r['blockers'])))[:top]:
        print(f"{r['map']:45s} {r['type'] or '':18s} {r['w']}x{r['h']} choke={r['choke_tiles']:3d} "
              f"runs={len(r['chokepoints']):2d} blockers={len(r['blockers'])} triggers={r['triggers']}")


if __name__ == '__main__':
    main()
