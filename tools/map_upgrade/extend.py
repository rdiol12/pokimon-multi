"""Grows an existing map: adds rows or columns on any side, keeps every old
metatile, and lets the area synthesiser (tools/areas/synth.py) draw the new
ground and the seam where old meets new, learning from Game Freak's own maps.

    ext = Extension('LittlerootTown', style, down=16)
    ext.reopen(0, 16, 20, 4)          # let the synthesiser redraw old rows 16-19
    ext.area.rect(...)                 # sketch the new ground (new coordinates)
    ext.write()

Growing left or up shifts the map's own events and its connections (and the
neighbours' matching connections) so everything stays where it was.
"""
import json, os, sys
import numpy as np
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'areas'))
from build import Area, ROOT, render
from tiles import layouts as _layouts, load_blocks

LAYOUTS_JSON = f'{ROOT}/data/layouts/layouts.json'


def read_map(folder):
    return json.load(open(f'{ROOT}/data/maps/{folder}/map.json'))


def write_json(path, data):
    with open(path, 'w') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write('\n')


def map_folder(map_id):
    for d in os.listdir(f'{ROOT}/data/maps'):
        p = f'{ROOT}/data/maps/{d}/map.json'
        if os.path.exists(p) and json.load(open(p)).get('id') == map_id:
            return d
    raise KeyError(map_id)


class Extension:
    def __init__(self, folder, style, left=0, right=0, up=0, down=0, fill='T'):
        self.folder = folder
        self.map = read_map(folder)
        self.layout = _layouts()[self.map['layout']]
        if (self.layout['primary_tileset'], self.layout['secondary_tileset']) != (style.primary, style.secondary):
            raise ValueError('style uses another tileset pair')
        self.old = load_blocks(self.layout)
        self.oh, self.ow = self.old.shape
        self.dx, self.dy = left, up
        self.grow = dict(left=left, right=right, up=up, down=down)
        self.w, self.h = self.ow + left + right, self.oh + up + down
        if (self.w + 15) * (self.h + 14) > 10240:  # MAX_MAP_DATA_SIZE (include/fieldmap.h)
            raise ValueError('map too big for the map buffer')
        self.area = Area(folder, self.w, self.h, style, fill=fill)
        self.style = style
        for y in range(self.oh):
            for x in range(self.ow):
                b = int(self.old[y, x])
                self.area.fixed[(x + left, y + up)] = b
                self.area.grid[y + up][x + left] = style.cls(b)
        self.new_objects, self.new_bgs, self.new_warps, self.new_coords = [], [], [], []

    # coordinates: everything below takes NEW map coordinates
    def old_to_new(self, x, y):
        return x + self.dx, y + self.dy

    def reopen(self, x, y, w, h, c=None):
        """Let the synthesiser redraw these cells (old or new); c repaints them."""
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.area.fixed.pop((xx, yy), None)
                if c:
                    self.area.put(xx, yy, c)

    def keep(self, x, y, w, h):
        """Cells (new coords) copied unchanged from wherever they sit in the old map."""
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                ox, oy = xx - self.dx, yy - self.dy
                if 0 <= ox < self.ow and 0 <= oy < self.oh:
                    b = int(self.old[oy, ox])
                    self.area.fixed[(xx, yy)] = b
                    self.area.grid[yy][xx] = self.style.cls(b)

    def copy_old(self, sx, sy, w, h, dx, dy, c='o'):
        """Copy a block of the OLD map (old coords) to new coords dx, dy.
        c=None keeps each cell's own terrain class."""
        for yy in range(h):
            for xx in range(w):
                b = int(self.old[sy + yy, sx + xx])
                self.area.fixed[(dx + xx, dy + yy)] = b
                self.area.put(dx + xx, dy + yy, c or self.style.cls(b))

    def move_old_events(self, x, y, w, h, dx, dy):
        """Old events inside the box (old coords) move by dx, dy (used when a
        strip of the old map is moved to make room for a new district)."""
        for key in ('object_events', 'warp_events', 'coord_events', 'bg_events'):
            for ev in self.map.get(key) or []:
                if x <= ev['x'] < x + w and y <= ev['y'] < y + h:
                    ev['x'] += dx
                    ev['y'] += dy

    # ------------------------------------------------------------ events
    def obj(self, *a, **kw):
        o = self.area.obj(*a, **kw)
        self.area.objects.remove(o)
        self.new_objects.append(o)
        return o

    def sign(self, x, y, script, facing='BG_EVENT_PLAYER_FACING_ANY', elevation=0):
        self.new_bgs.append({'type': 'sign', 'x': x, 'y': y, 'elevation': elevation,
                             'player_facing_dir': facing, 'script': script})

    def hidden(self, x, y, item, flag, elevation=3):
        self.new_bgs.append({'type': 'hidden_item', 'x': x, 'y': y, 'elevation': elevation, 'item': item, 'flag': flag})

    def warp(self, x, y, dest_map, dest_warp, elevation=0):
        self.new_warps.append({'x': x, 'y': y, 'elevation': elevation, 'dest_map': dest_map,
                               'dest_warp_id': str(dest_warp)})
        return len(self.map.get('warp_events', [])) + len(self.new_warps) - 1

    # ------------------------------------------------------------ output
    def blocks(self):
        return self.area.blocks()

    def preview(self, path, blocks=None):
        b = self.blocks() if blocks is None else blocks
        render(self.style.pair, b).save(path)
        return b

    def write(self, blocks=None):
        b = self.blocks() if blocks is None else blocks
        b.astype('<u2').tofile(f"{ROOT}/{self.layout['blockdata_filepath']}")
        data = json.load(open(LAYOUTS_JSON))
        for l in data['layouts']:
            if l.get('id') == self.layout['id']:
                l['width'], l['height'] = self.w, self.h
        write_json(LAYOUTS_JSON, data)
        m = self.map
        if self.dx or self.dy:
            for key in ('object_events', 'warp_events', 'coord_events', 'bg_events'):
                for e in m.get(key) or []:
                    e['x'] += self.dx
                    e['y'] += self.dy
            self._shift_connections()
        m.setdefault('object_events', []).extend(self.new_objects)
        m.setdefault('bg_events', []).extend(self.new_bgs)
        m.setdefault('warp_events', []).extend(self.new_warps)
        m.setdefault('coord_events', []).extend(self.new_coords)
        write_json(f'{ROOT}/data/maps/{self.folder}/map.json', m)
        _layouts.cache_clear() if hasattr(_layouts, 'cache_clear') else None
        return b

    def _shift_connections(self):
        """Growing left moves the map's origin: up/down connections (offset
        along x) shift by dx, left/right ones (offset along y) by dy, and each
        neighbour's matching connection back to us shifts the other way."""
        me = self.map['id']
        for c in self.map.get('connections') or []:
            d = c['direction']
            shift = self.dx if d in ('up', 'down') else self.dy if d in ('left', 'right') else 0
            if not shift:
                continue
            c['offset'] += shift
            folder = map_folder(c['map'])
            other = read_map(folder)
            for oc in other.get('connections') or []:
                if oc['map'] == me:
                    oc['offset'] -= shift
            write_json(f'{ROOT}/data/maps/{folder}/map.json', other)
