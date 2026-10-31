"""Builds a new area: sketch the terrain, stamp in pieces of real maps, add
events, and write every file the game needs.

Each area is a script in tools/areas/ (python3 tools/areas/<area>.py) that
uses Area. Running it rewrites the area's layout, map.json and scripts, and
registers the map in layouts.json, map_groups.json, event_scripts.s and the
region map sections (idempotently), and saves a preview to build/areas/.
"""
import json, os, re
import numpy as np
from tiles import ROOT, Pair, layouts, load_blocks, render
from synth import Style

COLL = 0x0C00


def coll(block):
    return block & COLL


class Area:
    def __init__(self, name, w, h, style, fill='.', border=None):
        self.name, self.w, self.h, self.style = name, w, h, style
        self.grid = [[fill] * w for _ in range(h)]
        self.fixed = {}
        self.objects, self.warps, self.coords, self.bgs = [], [], [], []
        self.border = border
        self.connections = None

    # ------------------------------------------------------------ sketching
    def put(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.grid[y][x] = c

    def rect(self, x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.put(xx, yy, c)

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                if ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 <= 1:
                    self.put(x, y, c)

    def line(self, pts, width, c):
        """Thick polyline through pts (cells), e.g. a path or river."""
        for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
            n = max(abs(x1 - x0), abs(y1 - y0), 1)
            for i in range(n + 1):
                x, y = x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n
                self.rect(int(round(x - (width - 1) / 2)), int(round(y - (width - 1) / 2)), width, width, c)

    def sketch(self, x, y, text, legend=None):
        """Paint an ASCII block; spaces leave cells alone."""
        for dy, row in enumerate(text.strip('\n').split('\n')):
            for dx, ch in enumerate(row):
                if ch != ' ':
                    self.put(x + dx, y + dy, (legend or {}).get(ch, ch))

    def snap_trees(self, c='T', ground='.'):
        """Trees are 2x2 metatiles; make every tree cell part of an aligned 2x2 block."""
        for by in range(0, self.h, 2):
            for bx in range(0, self.w, 2):
                cells = [(bx + dx, by + dy) for dy in (0, 1) for dx in (0, 1) if bx + dx < self.w and by + dy < self.h]
                cells = [(x, y) for x, y in cells if (x, y) not in self.fixed]
                n = sum(self.grid[y][x] == c for x, y in cells)
                if n >= 3 or (n and n == len(cells)):
                    for x, y in cells:
                        self.grid[y][x] = c
                elif n:
                    for x, y in cells:
                        if self.grid[y][x] == c:
                            self.grid[y][x] = ground

    def scatter(self, metatile, n, x0=0, y0=0, x1=None, y1=None, seed=1, clump=3):
        """Sprinkles a walkable decoration (e.g. flowers 0x004) over open
        ground in a region, in small clumps, away from edges."""
        import random
        r = random.Random(seed)
        x1, y1 = x1 or self.w, y1 or self.h
        def ok(x, y):
            return all(0 <= x + dx < self.w and 0 <= y + dy < self.h and self.grid[y + dy][x + dx] == '.'
                       and (x + dx, y + dy) not in self.fixed for dx in (-1, 0, 1) for dy in (-1, 0, 1))
        placed = tries = 0
        while placed < n and tries < n * 50:
            tries += 1
            x, y = r.randrange(x0, x1), r.randrange(y0, y1)
            if ok(x, y):
                for _ in range(clump):
                    if ok(x, y):
                        self.tile(x, y, metatile)
                        placed += 1
                    x, y = x + r.choice((-1, 0, 1)), y + r.choice((-1, 0, 1))

    # ------------------------------------------------------------ fixed metatiles
    def tile(self, x, y, metatile, collision=None, elevation=3, c=None):
        if collision is None:
            collision = 1 if c in ('#', 'T') else 0
        self.fixed[(x, y)] = metatile | (collision << 10) | (elevation << 12)
        if c:
            self.put(x, y, c)

    def stamp(self, layout_id, sx, sy, w, h, dx, dy, c=None, skip=None):
        """Copy a w x h block from a real map (with collision and elevation).
        The sketch under it becomes 'o' (a class the real maps never use, so
        neighbours fall back to plain ground) unless c says otherwise. skip: metatile ids left to the synthesiser."""
        l = layouts()[layout_id]
        src = load_blocks(l)
        for y in range(h):
            for x in range(w):
                b = int(src[sy + y, sx + x])
                if skip and (b & 0x3FF) in skip:
                    continue
                self.fixed[(dx + x, dy + y)] = b
                self.put(dx + x, dy + y, c or 'o')

    def autotile3(self, mask_char, base, inner=None, collision=0, elevation=3, out_char='o'):
        """3x3 edge sets laid out 8 metatiles apart (General's pond, sand and
        plaza sets): base is the top-left corner."""
        cells = {(x, y) for y in range(self.h) for x in range(self.w) if self.grid[y][x] == mask_char}
        for (x, y) in cells:
            up, dn = (x, y - 1) in cells, (x, y + 1) in cells
            lf, rt = (x - 1, y) in cells, (x + 1, y) in cells
            row = 0 if not up else (2 if not dn else 1)
            col = 0 if not lf else (2 if not rt else 1)
            mt = base + row * 8 + col
            if inner and row == 1 and col == 1:
                for (ox, oy), m in inner.items():
                    if (x + ox, y + oy) not in cells:
                        mt = m
            self.fixed[(x, y)] = mt | (collision << 10) | (elevation << 12)
            if out_char:
                self.grid[y][x] = out_char

    # ------------------------------------------------------------ events
    def obj(self, gfx, x, y, script='0x0', move='MOVEMENT_TYPE_FACE_DOWN', flag='0', rx=0, ry=0,
            trainer='TRAINER_TYPE_NONE', sight=0, local_id=None, elevation=3):
        o = {'graphics_id': gfx, 'x': x, 'y': y, 'elevation': elevation, 'movement_type': move,
             'movement_range_x': rx, 'movement_range_y': ry, 'trainer_type': trainer,
             'trainer_sight_or_berry_tree_id': str(sight), 'script': script, 'flag': flag}
        if local_id:
            o = {'local_id': local_id, **o}
        self.objects.append(o)
        return o

    def item(self, item, x, y, flag):
        o = self.obj('OBJ_EVENT_GFX_ITEM_BALL', x, y, f'{self.name}_EventScript_Item_{item[5:].title().replace("_", "")}',
                     'MOVEMENT_TYPE_LOOK_AROUND', flag)
        self.item_scripts = getattr(self, 'item_scripts', [])
        self.item_scripts.append((o['script'], item))
        return o

    def trainer(self, gfx, x, y, script, sight=3, move='MOVEMENT_TYPE_FACE_DOWN', **kw):
        return self.obj(gfx, x, y, script, move, trainer='TRAINER_TYPE_NORMAL', sight=sight, **kw)

    def warp(self, x, y, dest_map, dest_warp, elevation=0):
        self.warps.append({'x': x, 'y': y, 'elevation': elevation, 'dest_map': dest_map, 'dest_warp_id': str(dest_warp)})
        return len(self.warps) - 1

    def coord(self, x, y, script, var='VAR_TEMP_0', var_value='0', elevation=3):
        self.coords.append({'type': 'trigger', 'x': x, 'y': y, 'elevation': elevation, 'var': var,
                            'var_value': var_value, 'script': script})

    def sign(self, x, y, script, facing='BG_EVENT_PLAYER_FACING_ANY', elevation=0):
        self.bgs.append({'type': 'sign', 'x': x, 'y': y, 'elevation': elevation, 'player_facing_dir': facing, 'script': script})

    def hidden(self, x, y, item, flag, elevation=3):
        self.bgs.append({'type': 'hidden_item', 'x': x, 'y': y, 'elevation': elevation, 'item': item, 'flag': flag})

    # ------------------------------------------------------------ output
    def blocks(self):
        sketch = [''.join(r) for r in self.grid]
        return self.style.synth(sketch, self.fixed)

    def write(self, *, map_id, layout_id, music, mapsec, mapsec_name, weather='WEATHER_NONE', map_type='MAP_TYPE_ROUTE',
              scripts='', group='gMapGroup_TownsAndRoutes', show_name=True, battle_scene='MAP_BATTLE_SCENE_NORMAL',
              allow_cycling=True, allow_escaping=False, allow_running=True, requires_flash=False, blocks=None,
              border=None, after=None):
        b = self.blocks() if blocks is None else blocks
        self.last_blocks = b
        ldir = f'data/layouts/{self.name}'
        os.makedirs(f'{ROOT}/{ldir}', exist_ok=True)
        b.astype('<u2').tofile(f'{ROOT}/{ldir}/map.bin')
        border = border or self.border or [int(b[0, 0]) & 0x3FF] * 4
        np.array(border, '<u2').tofile(f'{ROOT}/{ldir}/border.bin')
        register_layout({'id': layout_id, 'name': f'{self.name}_Layout', 'width': self.w, 'height': self.h,
                         'primary_tileset': self.style.primary, 'secondary_tileset': self.style.secondary,
                         'border_filepath': f'{ldir}/border.bin', 'blockdata_filepath': f'{ldir}/map.bin'})
        mdir = f'{ROOT}/data/maps/{self.name}'
        os.makedirs(mdir, exist_ok=True)
        m = {'id': map_id, 'name': self.name, 'layout': layout_id, 'music': music, 'region': 'REGION_HOENN',
             'region_map_section': mapsec, 'requires_flash': requires_flash, 'weather': weather, 'map_type': map_type,
             'allow_cycling': allow_cycling, 'allow_escaping': allow_escaping, 'allow_running': allow_running,
             'show_map_name': show_name, 'battle_scene': battle_scene, 'connections': self.connections,
             'object_events': self.objects, 'warp_events': self.warps, 'coord_events': self.coords, 'bg_events': self.bgs}
        with open(f'{mdir}/map.json', 'w') as f:
            json.dump(m, f, indent=2)
            f.write('\n')
        items = ''.join(f'{label}::\n\tfinditem {item}\n\tend\n\n' for label, item in getattr(self, 'item_scripts', []))
        with open(f'{mdir}/scripts.inc', 'w') as f:
            f.write(scripts.strip('\n') + '\n\n' + items)
        register_group(group, self.name, after)
        register_scripts(self.name)
        if mapsec:
            register_mapsec(mapsec, mapsec_name)
        os.makedirs(f'{ROOT}/build/areas', exist_ok=True)
        preview(self, b).save(f'{ROOT}/build/areas/{self.name}.png')
        render(self.style.pair, b, grid=True).save(f'{ROOT}/build/areas/{self.name}-collision.png')
        return b


def preview(area, b):
    return render(area.style.pair, b)


def register_layout(entry):
    p = f'{ROOT}/data/layouts/layouts.json'
    d = json.load(open(p))
    ls = d['layouts']
    for i, l in enumerate(ls):
        if l.get('id') == entry['id']:
            ls[i] = entry
            break
    else:
        ls.append(entry)
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')


def register_group(group, name, after=None):
    p = f'{ROOT}/data/maps/map_groups.json'
    d = json.load(open(p))
    if any(name in v for k, v in d.items() if isinstance(v, list)):
        return
    g = d[group]
    g.insert(g.index(after) + 1 if after else len(g), name)
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')


def register_scripts(name):
    p = f'{ROOT}/data/event_scripts.s'
    s = open(p).read()
    line = f'\t.include "data/maps/{name}/scripts.inc"\n'
    if line in s:
        return
    anchor = '\t.include "data/maps/AmberwoodHollow/scripts.inc"\n'
    s = s.replace(anchor, anchor + line) if anchor in s else s
    open(p, 'w').write(s)


def register_mapsec(mapsec, name):
    p = f'{ROOT}/src/data/region_map/region_map_sections.json'
    d = json.load(open(p))
    if any(s['id'] == mapsec for s in d['map_sections']):
        return
    d['map_sections'].append({'id': mapsec, 'name': name})
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')


def register_block(path, key, text, comment=('/*', '*/')):
    """Puts text between begin/end markers named key in path (replacing an
    earlier copy), so area scripts can be re-run."""
    p = f'{ROOT}/{path}'
    s = open(p).read() if os.path.exists(p) else ''
    begin, end = f'{comment[0]} begin {key} {comment[1]}\n', f'{comment[0]} end {key} {comment[1]}\n'
    block = begin + text.strip('\n') + '\n' + end
    if begin in s:
        a, b = s.index(begin), s.index(end) + len(end)
        s = s[:a] + block + s[b:]
    else:
        anchor = '\n#endif // GUARD_CONSTANTS_AREA_IDS_H'
        s = s.replace(anchor, '\n' + block + anchor) if anchor in s else s + ('\n' if s else '') + block
    open(p, 'w').write(s)


def register_constants(key, area_no, flags=(), trainers=(), extra=''):
    """flags: names (FLAG_ prefix added) given AREA_FLAG(area_no * 24 + i);
    trainers: names given area trainer slots area_no * 8 + i."""
    assert len(flags) <= 24 and len(trainers) <= 8
    lines = [f'#define FLAG_{n:<32} AREA_FLAG({area_no * 24 + i})' for i, n in enumerate(flags)]
    for i, n in enumerate(trainers):
        lines.append(f'#define AREA_TRAINER_{n:<24} {area_no * 8 + i}')
        lines.append(f'#define TRAINER_{n:<29} AREA_TRAINER(AREA_TRAINER_{n})')
    register_block('include/constants/area_ids.h', key, '\n'.join(lines) + ('\n' + extra if extra else ''))
    p = f'{ROOT}/include/constants/area_ids.h'
    s = open(p).read()
    idx = [int(v) for v in re.findall(r'#define AREA_TRAINER_\w+\s+(\d+)\n', s)]
    s = re.sub(r'#define AREA_TRAINERS_COUNT \d+', f'#define AREA_TRAINERS_COUNT {max(idx) + 1 if idx else 0}', s)
    open(p, 'w').write(s)


def register_trainers(key, text):
    register_block('src/data/area_trainers.party', key, text)


def register_wild(map_id, base_label, land=None, water=None, fish=None, rock=None, land_rate=20, water_rate=4,
                  fish_rate=20, rock_rate=20):
    """Each list holds (species, min, max) in encounter-slot order (12 land,
    5 water/rock, 10 fishing)."""
    p = f'{ROOT}/src/data/wild_encounters.json'
    d = json.load(open(p))
    g = d['wild_encounter_groups'][0]
    e = {'map': map_id, 'base_label': base_label}
    for key, mons, rate, n in (('land_mons', land, land_rate, 12), ('water_mons', water, water_rate, 5),
                               ('rock_smash_mons', rock, rock_rate, 5), ('fishing_mons', fish, fish_rate, 10)):
        if mons:
            assert len(mons) == n, (key, len(mons))
            e[key] = {'encounter_rate': rate, 'mons': [{'min_level': a, 'max_level': b, 'species': 'SPECIES_' + s}
                                                       for s, a, b in mons]}
    encs = g['encounters']
    for i, old in enumerate(encs):
        if old['map'] == map_id:
            encs[i] = e
            break
    else:
        encs.append(e)
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')


def add_object(map_name, obj):
    """Adds (or updates, matched by script) an object event on an existing map."""
    p = f'{ROOT}/data/maps/{map_name}/map.json'
    d = json.load(open(p))
    objs = d['object_events']
    for i, o in enumerate(objs):
        if o.get('script') == obj['script']:
            objs[i] = obj
            break
    else:
        objs.append(obj)
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')


def register_script_file(name, text):
    """data/scripts/areas/<name>.inc, included from event_scripts.s."""
    os.makedirs(f'{ROOT}/data/scripts/areas', exist_ok=True)
    open(f'{ROOT}/data/scripts/areas/{name}.inc', 'w').write(text.strip('\n') + '\n')
    p = f'{ROOT}/data/event_scripts.s'
    s = open(p).read()
    line = f'\t.include "data/scripts/areas/{name}.inc"\n'
    if line not in s:
        anchor = '\t.include "data/maps/AmberwoodHollow/scripts.inc"\n'
        s = s.replace(anchor, anchor + line)
        open(p, 'w').write(s)


def npc(gfx, x, y, script, move='MOVEMENT_TYPE_FACE_DOWN', local_id=None, flag='0', elevation=3):
    o = {'graphics_id': gfx, 'x': x, 'y': y, 'elevation': elevation, 'movement_type': move, 'movement_range_x': 1,
         'movement_range_y': 1, 'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
         'script': script, 'flag': flag}
    return {'local_id': local_id, **o} if local_id else o


def interior(name, map_id, layout, doors, outside_map, outside_warp, objects, scripts, mapsec, group,
             music='MUS_NONE', bgs=(), extra_warps=(), requires_flash=False):
    """A room using an existing layout. doors: (x, y) cells of its exit."""
    warps = [{'x': x, 'y': y, 'elevation': 0, 'dest_map': outside_map, 'dest_warp_id': str(outside_warp)} for x, y in doors]
    warps += list(extra_warps)
    m = {'id': map_id, 'name': name, 'layout': layout, 'music': music, 'region': 'REGION_HOENN',
         'region_map_section': mapsec, 'requires_flash': requires_flash, 'weather': 'WEATHER_NONE', 'map_type': 'MAP_TYPE_INDOOR',
         'allow_cycling': False, 'allow_escaping': False, 'allow_running': False, 'show_map_name': False,
         'battle_scene': 'MAP_BATTLE_SCENE_NORMAL', 'connections': None, 'object_events': objects,
         'warp_events': warps, 'coord_events': [], 'bg_events': list(bgs)}
    d = f'{ROOT}/data/maps/{name}'
    os.makedirs(d, exist_ok=True)
    with open(f'{d}/map.json', 'w') as f:
        json.dump(m, f, indent=2, ensure_ascii=False)
        f.write('\n')
    open(f'{d}/scripts.inc', 'w').write(scripts.strip('\n') + '\n')
    register_group(group, name)
    register_scripts(name)


def pokemon_center(prefix, mapsec, group, outside_map, outside_warp, heal_text_npcs=()):
    """A Pokémon Center 1F (Lavaridge's layout) with a nurse and a few NPCs."""
    name = f'{prefix}_PokemonCenter_1F'
    up = re.sub(r'(?<!^)(?=[A-Z])', '_', prefix).upper()
    objs = [npc('OBJ_EVENT_GFX_NURSE', 7, 2, f'{name}_EventScript_Nurse', local_id=f'LOCALID_{up}_NURSE')]
    objs[0]['movement_range_x'] = objs[0]['movement_range_y'] = 0
    lines = [f'{name}_MapScripts::', '\tmap_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume', '\t.byte 0', '',
             f'{name}_EventScript_Nurse::', f'\tsetvar VAR_0x800B, LOCALID_{up}_NURSE',
             '\tcall Common_EventScript_PkmnCenterNurse', '\twaitmessage', '\twaitbuttonpress', '\trelease', '\tend', '']
    for i, (gfx, x, y, text) in enumerate(heal_text_npcs):
        label = f'{name}_EventScript_Npc{i}'
        objs.append(npc(gfx, x, y, label, 'MOVEMENT_TYPE_WANDER_AROUND' if i % 2 else 'MOVEMENT_TYPE_FACE_DOWN'))
        lines += [f'{label}::', f'\tmsgbox {name}_Text_Npc{i}, MSGBOX_NPC', '\tend', '',
                  f'{name}_Text_Npc{i}:'] + [f'\t.string "{l}"' for l in text] + ['']
    interior(name, f'MAP_{up}_POKEMON_CENTER_1F', 'LAYOUT_LAVARIDGE_TOWN_POKEMON_CENTER_1F', [(6, 8), (7, 8)],
             outside_map, outside_warp, objs, '\n'.join(lines), mapsec, group, music='MUS_POKE_CENTER')
    return f'MAP_{up}_POKEMON_CENTER_1F'


def ring(area, inner, ring_char, over=('.',)):
    """Surrounds every inner-class cell with ring_char on cells of the given classes."""
    cells = [(x, y) for y in range(area.h) for x in range(area.w) if area.grid[y][x] == inner]
    for x, y in cells:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                xx, yy = x + dx, y + dy
                if 0 <= xx < area.w and 0 <= yy < area.h and area.grid[yy][xx] in over:
                    area.grid[yy][xx] = ring_char
