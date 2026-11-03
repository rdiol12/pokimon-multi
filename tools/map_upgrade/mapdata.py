"""Shared map reading for the map upgrade tools: layouts, collision, behaviours,
map.json events and walkable-tile analysis.
"""
import json, os, re, struct
from functools import lru_cache
from collections import deque

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
NUM_PRIMARY = 512


@lru_cache(None)
def behaviours():
    """MB_* name -> value, from the enum."""
    text = open(f'{ROOT}/include/constants/metatile_behaviors.h').read()
    body = text[text.index('enum'):]
    out, n = {}, 0
    for line in body.split('\n'):
        m = re.match(r'\s*(MB_\w+)\s*(?:=\s*(\w+))?\s*,', line)
        if m:
            if m.group(2):
                n = int(m.group(2), 0)
            out[m.group(1)] = n
            n += 1
    return out


@lru_cache(None)
def tileset_dirs():
    hdr = open(f'{ROOT}/src/data/tilesets/headers.h').read()
    mt = open(f'{ROOT}/src/data/tilesets/metatiles.h').read()
    out = {}
    for name, body in re.findall(r'const struct Tileset (gTileset_\w+) =\s*\{(.*?)\};', hdr, re.S):
        attrs = re.search(r'\.metatileAttributes = (\w+)', body)
        if not attrs:
            continue
        mm = re.search(attrs.group(1) + r'\[\] = INCBIN_U16\("([^"]+)"', mt)
        if mm:
            out[name] = f'{ROOT}/' + mm.group(1)
    return out


@lru_cache(None)
def tileset_attrs(name):
    raw = open(tileset_dirs()[name], 'rb').read()
    return struct.unpack(f'<{len(raw)//2}H', raw)


@lru_cache(None)
def layouts():
    return {l['id']: l for l in json.load(open(f'{ROOT}/data/layouts/layouts.json'))['layouts'] if 'id' in l}


@lru_cache(None)
def maps():
    """MAP_ID -> (folder, map.json dict) for every map."""
    out = {}
    for d in sorted(os.listdir(f'{ROOT}/data/maps')):
        p = f'{ROOT}/data/maps/{d}/map.json'
        if os.path.exists(p):
            m = json.load(open(p))
            out[m['id']] = (d, m)
    return out


WATER = {'MB_POND_WATER', 'MB_INTERIOR_DEEP_WATER', 'MB_DEEP_WATER', 'MB_WATERFALL', 'MB_OCEAN_WATER',
         'MB_SOOTOPOLIS_DEEP_WATER', 'MB_NO_SURFACING', 'MB_EASTWARD_CURRENT', 'MB_WESTWARD_CURRENT',
         'MB_NORTHWARD_CURRENT', 'MB_SOUTHWARD_CURRENT'}
DOORS = {'MB_ANIMATED_DOOR', 'MB_NON_ANIMATED_DOOR', 'MB_PETALBURG_GYM_DOOR', 'MB_WATER_DOOR'}


class Grid:
    """A layout's blocks with collision/behaviour lookups."""
    def __init__(self, layout_id):
        lay = layouts()[layout_id]
        self.layout = lay
        self.w, self.h = lay['width'], lay['height']
        raw = open(f"{ROOT}/{lay['blockdata_filepath']}", 'rb').read()
        self.blocks = list(struct.unpack(f'<{len(raw)//2}H', raw))
        self.prim = tileset_attrs(lay['primary_tileset'])
        self.sec = tileset_attrs(lay['secondary_tileset'])
        inv = {v: k for k, v in behaviours().items()}
        self.inv = inv
        self.water = {behaviours()[k] for k in WATER if k in behaviours()}
        self.doors = {behaviours()[k] for k in DOORS if k in behaviours()}

    def block(self, x, y):
        return self.blocks[y * self.w + x]

    def behaviour(self, x, y):
        mt = self.block(x, y) & 0x3FF
        a = self.prim[mt] if mt < NUM_PRIMARY else self.sec[mt - NUM_PRIMARY] if mt - NUM_PRIMARY < len(self.sec) else 0
        return a & 0xFF

    def walkable(self, x, y):
        if not (0 <= x < self.w and 0 <= y < self.h):
            return False
        b = self.block(x, y)
        if (b >> 10) & 3:
            return False
        if (b >> 12) == 1:  # surf-only height
            return False
        return self.behaviour(x, y) not in self.water

    def is_door(self, x, y):
        if not (0 <= x < self.w and 0 <= y < self.h):
            return False
        return self.behaviour(x, y) in self.doors


def entrances(grid, m):
    """Tiles a player arrives on or leaves from: warps and connected edges."""
    out = set()
    for w in m.get('warp_events', []):
        out.add((w['x'], w['y']))
    for c in m.get('connections') or []:
        d = c['direction']
        if d in ('up', 'down'):
            y = 0 if d == 'up' else grid.h - 1
            for x in range(grid.w):
                if grid.walkable(x, y):
                    out.add((x, y))
        elif d in ('left', 'right'):
            x = 0 if d == 'left' else grid.w - 1
            for y in range(grid.h):
                if grid.walkable(x, y):
                    out.add((x, y))
    return out


def bfs(grid, starts, blocked=frozenset()):
    dist, prev = {}, {}
    q = deque()
    for s in starts:
        if s in dist:
            continue
        dist[s] = 0
        q.append(s)
    while q:
        x, y = q.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in dist or n in blocked:
                continue
            if grid.walkable(*n) or n in starts:
                dist[n] = dist[(x, y)] + 1
                prev[n] = (x, y)
                q.append(n)
    return dist, prev


def main_paths(grid, m):
    """Tiles on a shortest walk between any two entrance groups."""
    ents = sorted(entrances(grid, m))
    # group neighbouring entrance tiles (a 3-wide edge is one entrance)
    groups, seen = [], set()
    for e in ents:
        if e in seen:
            continue
        g, q = [], [e]
        seen.add(e)
        while q:
            c = q.pop()
            g.append(c)
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = (c[0] + dx, c[1] + dy)
                if n in ents and n not in seen:
                    seen.add(n)
                    q.append(n)
        groups.append(g)
    path = set()
    for i, g in enumerate(groups):
        dist, prev = bfs(grid, g)
        for h in groups[i + 1:]:
            reach = [t for t in h if t in dist]
            if not reach:
                continue
            c = min(reach, key=dist.get)
            while c in prev:
                path.add(c)
                c = prev[c]
            path.add(c)
    return path, groups


def width_at(grid, x, y):
    """Narrowest open run through (x, y): horizontal and vertical."""
    def run(dx, dy):
        n = 1
        for s in (1, -1):
            cx, cy = x + dx * s, y + dy * s
            while grid.walkable(cx, cy) and n < 9:
                n += 1
                cx += dx * s
                cy += dy * s
        return n
    return min(run(1, 0), run(0, 1))
