"""Shared pieces for Solmara, the second region (docs/solmara.md).

Every Solmara map is written by tools/solmara/region.py, which runs the
towns, routes, gyms, the League and the ship in order. This module holds
what they share: the flag and trainer registry (Solmara keeps its own, in
the extended save), General-tileset buildings, house stamps per tileset,
interiors, map connections and the text helpers.

Multiplayer rules every map follows (ron, 2026-11-01): paths and doors at
least 3 tiles wide where the tiles allow, NPCs off the walkways, no story
blockers or one-time cutscenes, and room for many players at once.
"""
import copy, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'areas'))
from build import *  # noqa: F401,F403  Area, npc, interior, register_*, ...
from build import register_group as _area_register_group
from synth import Style
import build as _build
import numpy as np

GROUP = 'gMapGroup_Solmara'
SECONDARY = set(range(0x200, 0x400))


# ---------------------------------------------------------------- map group
def ensure_group():
    p = f'{ROOT}/data/maps/map_groups.json'
    d = json.load(open(p))
    if GROUP not in d:
        d['group_order'].append(GROUP)
        d[GROUP] = []
        with open(p, 'w') as f:
            json.dump(d, f, indent=2, ensure_ascii=False)
            f.write('\n')


def _register_group(group, name, after=None):
    _area_register_group(GROUP if group in (None, GROUP) else group, name, after)


_build.register_group = _register_group


# ---------------------------------------------------------------- flags and trainers
class Registry:
    def __init__(self):
        self.flags, self.trainers = [], []

    def flag(self, name):
        n = f'FLAG_SOL_{name}'
        if n not in self.flags:
            self.flags.append(n)
        return n

    def trainer(self, key, party):
        """key: e.g. 'PORT_MARINA'. party: the .party body after the === line."""
        assert key not in [k for k, _ in self.trainers], key
        self.trainers.append((key, party.strip('\n')))
        return f'TRAINER_SOL_{key}'

    def write(self, extra_defines=''):
        assert len(self.flags) <= 1792 - 0, len(self.flags)
        assert len(self.trainers) <= 256
        lines = ['#ifndef GUARD_CONSTANTS_SOLMARA_IDS_H', '#define GUARD_CONSTANTS_SOLMARA_IDS_H', '',
                 "// Written by tools/solmara/region.py: Solmara's flags and trainers.", '']
        lines += [f'#define {n:<40} SOLMARA_FLAG({i})' for i, n in enumerate(self.flags)]
        lines.append('')
        for i, (k, _) in enumerate(self.trainers):
            lines.append(f'#define SOL_T_{k:<34} {256 + i}')
            lines.append(f'#define TRAINER_SOL_{k:<28} SOLMARA_TRAINER({i})')
        lines += ['', f'#define SOLMARA_TRAINERS_COUNT {len(self.trainers)}', extra_defines, '',
                  '#endif // GUARD_CONSTANTS_SOLMARA_IDS_H', '']
        open(f'{ROOT}/include/constants/solmara_ids.h', 'w').write('\n'.join(lines))
        party = ''.join(f'=== SOL_T_{k} ===\n{body}\n\n' for k, body in self.trainers)
        open(f'{ROOT}/src/data/solmara_trainers.party', 'w').write(party)


REG = Registry()


def party(name, cls, pic, gender, music, mons, double=False, items=()):
    """mons: (species, level) or (species, level, [moves]) or dicts with extras."""
    out = [f'Name: {name}', f'Class: {cls}', f'Pic: {pic}', f'Gender: {gender}', f'Music: {music}',
           f'Double Battle: {"Yes" if double else "No"}', 'AI: Basic Trainer' if len(mons) < 5 else 'AI: Smart Trainer']
    if items:
        out.append('Items: ' + ' / '.join(items))
    out.append('')
    for m in mons:
        species, level = m[0], m[1]
        head = species
        extra = []
        if len(m) > 2 and m[2]:
            for k, v in m[2].items():
                if k == 'item':
                    head = f'{species} @ {v}'
                elif k == 'moves':
                    extra += [f'- {mv}' for mv in v]
                else:
                    extra.append(f'{k}: {v}')
        out += [head, f'Level: {level}', 'IVs: 31 HP / 31 Atk / 31 Def / 31 SpA / 31 SpD / 31 Spe' if level >= 60 else 'IVs: 20 HP / 20 Atk / 20 Def / 20 SpA / 20 SpD / 20 Spe'] + extra + ['']
    return '\n'.join(out)


# ---------------------------------------------------------------- text
def text(*lines):
    """Lines of one message: \\n and \\l alternate, \\p where a line is ''."""
    out, n = [], 0
    for i, l in enumerate(lines):
        if l == '':
            out[-1] = out[-1][:-3] + '\\p"'
            n = 0
            continue
        end = '$' if i == len(lines) - 1 else ('\\n' if n == 0 else '\\l')
        out.append(f'\t.string "{l}{end}"')
        n += 1
    return '\n'.join(out) + '\n'


def talker(label, lines):
    return f"""{label}::
\tmsgbox {label}_Text, MSGBOX_NPC
\tend

{label}_Text:
{text(*lines)}
"""


def signpost(label, lines):
    return f"""{label}::
\tmsgbox {label}_Text, MSGBOX_SIGN
\tend

{label}_Text:
{text(*lines)}
"""


def trainer_script(label, trainer, intro, defeat, post):
    return f"""{label}::
\ttrainerbattle_single {trainer}, {label}_Intro, {label}_Defeat
\tmsgbox {label}_Post, MSGBOX_AUTOCLOSE
\tend

{label}_Intro:
{text(*intro)}
{label}_Defeat:
{text(*defeat)}
{label}_Post:
{text(*post)}
"""


def snake(name):
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name).upper().replace('__', '_')


def map_id(name):
    return 'MAP_' + snake(name)


# ---------------------------------------------------------------- buildings
# General-tileset buildings (the same in every town): layout, x, y, w, h, door (dx, dy).
GYM = ('LAYOUT_MAUVILLE_CITY', 5, 1, 6, 5, (3, 4))
PC = ('LAYOUT_MAUVILLE_CITY', 21, 2, 4, 4, (1, 3))
MART = ('LAYOUT_MAUVILLE_CITY', 22, 11, 4, 4, (1, 3))
# Houses per secondary tileset (stamped from that tileset's own town).
HOUSES = {
    'gTileset_Dewford': [('LAYOUT_DEWFORD_TOWN', 7, 5, 4, 4, (1, 3)), ('LAYOUT_DEWFORD_TOWN', 1, 0, 5, 4, (2, 3))],
    'gTileset_Mauville': [('LAYOUT_MAUVILLE_CITY', 18, 11, 3, 4, (1, 3)), ('LAYOUT_MAUVILLE_CITY', 31, 11, 5, 4, (1, 3))],
    'gTileset_Lavaridge': [('LAYOUT_LAVARIDGE_TOWN', 11, 12, 4, 4, (1, 3))],
    'gTileset_Fallarbor': [('LAYOUT_FALLARBOR_TOWN', 0, 3, 4, 4, (1, 3)), ('LAYOUT_FALLARBOR_TOWN', 5, 13, 4, 5, (1, 4))],
    'gTileset_Frost': [('LAYOUT_FALLARBOR_TOWN', 0, 3, 4, 4, (1, 3)), ('LAYOUT_FALLARBOR_TOWN', 5, 13, 4, 5, (1, 4))],
    'gTileset_Petalburg': [('LAYOUT_PETALBURG_CITY', 9, 16, 4, 4, (1, 3)), ('LAYOUT_PETALBURG_CITY', 5, 2, 5, 4, (2, 3))],
    'gTileset_Fortree': [('LAYOUT_FORTREE_CITY', 14, 0, 6, 5, (3, 3)), ('LAYOUT_FORTREE_CITY', 10, 10, 5, 4, (2, 3))],
    'gTileset_Mossdeep': [('LAYOUT_MOSSDEEP_CITY', 26, 6, 4, 4, (2, 3)), ('LAYOUT_MOSSDEEP_CITY', 17, 7, 4, 4, (2, 3))],
    'gTileset_Lilycove': [('LAYOUT_LILYCOVE_CITY', 35, 3, 4, 4, (1, 3)), ('LAYOUT_LILYCOVE_CITY', 54, 12, 4, 4, (1, 3))],
    'gTileset_Rustboro': [('LAYOUT_RUSTBORO_CITY', 31, 16, 4, 4, (2, 3)), ('LAYOUT_RUSTBORO_CITY', 24, 43, 4, 4, (2, 3))],
}


def general_building(kind, secondary):
    """The Pokémon Center ('pc'), Mart ('mart') or gym ('gym') as drawn in a
    Hoenn town with the same secondary tileset (their corners carry that
    town's ground), else Mauville's with the ground left to the synthesiser."""
    rel = {'pc': (1, 3, 4, 4, '_POKEMON_CENTER_1F'), 'mart': (1, 3, 4, 4, '_MART'), 'gym': (3, 4, 6, 5, '_GYM')}[kind]
    dx, dy, w, h, suffix = rel
    L = layouts()
    for d in sorted(os.listdir(f'{ROOT}/data/maps')):
        p = f'{ROOT}/data/maps/{d}/map.json'
        if not os.path.exists(p) or not re.match(r'^[A-Z][a-z]+(Town|City)$', d) or d in ('LaurelCity',):
            continue
        m = json.load(open(p))
        l = L.get(m.get('layout'))
        if not l or l.get('secondary_tileset') != secondary or l.get('primary_tileset') != 'gTileset_General':
            continue
        for wp in m['warp_events']:
            dest = wp['dest_map']
            if dest.endswith(suffix) or (kind == 'gym' and dest.endswith('_GYM_1F')):
                return (m['layout'], wp['x'] - dx, wp['y'] - dy, w, h, (dx, dy)), False
    return {'pc': PC, 'mart': MART, 'gym': GYM}[kind], True


def put_general(a, kind, x, y):
    secondary = {'gTileset_Frost': 'gTileset_Fallarbor', 'gTileset_Bamboo': 'gTileset_Rustboro'}.get(a.style.secondary, a.style.secondary)
    spec, general = general_building(kind, secondary)
    return put_building(a, spec, x, y, general)


def put_building(a, spec, x, y, general=False):
    """Stamps a building with its top-left at (x, y); returns the door cell."""
    lid, sx, sy, w, h, (dx, dy) = spec
    a.stamp(lid, sx, sy, w, h, x, y, skip=SECONDARY if general else None)
    return (x + dx, y + dy)


# ---------------------------------------------------------------- interiors
def house(prefix, idx, outside_map, outside_warp, mapsec, npcs, layout='LAYOUT_HOUSE1', scripts_extra=''):
    """A house using one of Hoenn's generic house layouts. npcs: (gfx, x, y, label_suffix, lines or None, move)."""
    name = f'{prefix}_House{idx}'
    S = f'{name}_EventScript'
    objs, sc = [], f'{name}_MapScripts::\n\t.byte 0\n\n'
    for gfx, x, y, key, lines, move in npcs:
        o = npc(gfx, x, y, f'{S}_{key}', move)
        if move != 'MOVEMENT_TYPE_WANDER_AROUND':
            o['movement_range_x'] = o['movement_range_y'] = 0
        objs.append(o)
        if lines is not None:
            sc += talker(f'{S}_{key}', lines)
    sc += scripts_extra
    doors = {'LAYOUT_HOUSE1': [(3, 8), (4, 8)], 'LAYOUT_HOUSE2': [(3, 7), (4, 7)], 'LAYOUT_HOUSE3': [(3, 7), (4, 7)],
             'LAYOUT_HOUSE4': [(3, 8), (4, 8)]}
    interior(name, map_id(name), layout, doors.get(layout, [(3, 8)]), outside_map, outside_warp, objs, sc, mapsec, GROUP,
             music='MUS_NONE')
    return map_id(name)


def mart(prefix, outside_map, outside_warp, mapsec, items, npcs=()):
    name = f'{prefix}_Mart'
    S = f'{name}_EventScript'
    clerk = npc('OBJ_EVENT_GFX_MART_EMPLOYEE', 1, 3, f'{S}_Clerk', 'MOVEMENT_TYPE_FACE_RIGHT')
    clerk['movement_range_x'] = clerk['movement_range_y'] = 0
    objs = [clerk]
    sc = f"""{name}_MapScripts::
\t.byte 0

{S}_Clerk::
\tlock
\tfaceplayer
\tmessage gText_HowMayIServeYou
\twaitmessage
\tpokemart {S}_Items
\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
\trelease
\tend

\t.align 2
{S}_Items:
""" + ''.join(f'\t.2byte {i}\n' for i in items) + '\tpokemartlistend\n\n'
    for i, (gfx, x, y, lines) in enumerate(npcs):
        objs.append(npc(gfx, x, y, f'{S}_Npc{i}', 'MOVEMENT_TYPE_WANDER_AROUND'))
        sc += talker(f'{S}_Npc{i}', lines)
    # The online market board (other players' Pokémon for sale) on the back wall.
    bgs = [{'type': 'sign', 'x': 6, 'y': 1, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
            'script': 'Netplay_EventScript_MarketBoard'}]
    interior(name, map_id(name), 'LAYOUT_MART', [(3, 7), (4, 7)], outside_map, outside_warp, objs, sc, mapsec, GROUP,
             music='MUS_POKE_MART', bgs=bgs)
    return map_id(name)


def pokecenter(prefix, outside_map, outside_warp, mapsec, town_key, items, npcs=()):
    """Pokémon Center 1F (Lavaridge's layout) with the nurse, a shop clerk at
    the same counter (Solmara has no separate marts), the Solmara flight desk (to any Solmara town the player has visited) and a few
    people. The 2F (Union Room/link desks) is left out: everything online
    goes through JOIN ONLINE instead."""
    name = f'{prefix}_PokemonCenter_1F'
    up = snake(prefix)
    objs = [npc('OBJ_EVENT_GFX_NURSE', 7, 2, f'{name}_EventScript_Nurse', local_id=f'LOCALID_{up}_NURSE')]
    objs[0]['movement_range_x'] = objs[0]['movement_range_y'] = 0
    pilot = npc('OBJ_EVENT_GFX_PILOT' if False else 'OBJ_EVENT_GFX_SAILOR', 1, 3, f'{name}_EventScript_Flight',
                'MOVEMENT_TYPE_FACE_RIGHT')
    pilot['graphics_id'] = 'OBJ_EVENT_GFX_GENTLEMAN'
    pilot['movement_range_x'] = pilot['movement_range_y'] = 0
    objs.append(pilot)
    clerk = npc('OBJ_EVENT_GFX_MART_EMPLOYEE', 5, 2, f'{name}_EventScript_Clerk')
    clerk['movement_range_x'] = clerk['movement_range_y'] = 0
    objs.append(clerk)
    shop = f"""{name}_EventScript_Clerk::
\tlock
\tfaceplayer
\tmessage gText_HowMayIServeYou
\twaitmessage
\tpokemart {name}_Items
\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
\trelease
\tend

\t.align 2
{name}_Items:
""" + ''.join(f'\t.2byte {i}\n' for i in items) + '\tpokemartlistend\n\n'
    sc = shop + f"""{name}_MapScripts::
\tmap_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
\t.byte 0

{name}_EventScript_Nurse::
\tsetvar VAR_0x800B, LOCALID_{up}_NURSE
\tcall Common_EventScript_PkmnCenterNurse
\twaitmessage
\twaitbuttonpress
\trelease
\tend

{name}_EventScript_Flight::
\tsetvar VAR_0x8008, {town_key}
\tgoto Solmara_EventScript_FlightDesk

"""
    for i, (gfx, x, y, lines) in enumerate(npcs):
        objs.append(npc(gfx, x, y, f'{name}_EventScript_Npc{i}', 'MOVEMENT_TYPE_WANDER_AROUND' if i % 2 else 'MOVEMENT_TYPE_FACE_DOWN'))
        sc += talker(f'{name}_EventScript_Npc{i}', lines)
    # The online market board (other players' Pokémon for sale) on the back wall.
    bgs = [{'type': 'sign', 'x': 11, 'y': 1, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
            'script': 'Netplay_EventScript_MarketBoard'}]
    interior(name, map_id(name), 'LAYOUT_LAVARIDGE_TOWN_POKEMON_CENTER_1F', [(6, 8), (7, 8)],
             outside_map, outside_warp, objs, sc, mapsec, GROUP, music='MUS_POKE_CENTER', bgs=bgs)
    return map_id(name)


# ---------------------------------------------------------------- connections
_links = {}


def link(a, side, b, a_cell, b_cell):
    """Joins map a's `side` edge to map b. a_cell/b_cell: the row (for
    left/right) or column (up/down) of the same exit cell in each map.
    Both maps' connection lists are filled; region.py writes them."""
    opp = {'left': 'right', 'right': 'left', 'up': 'down', 'down': 'up'}[side]
    _links.setdefault(a, []).append({'map': map_id(b), 'offset': a_cell - b_cell, 'direction': side})
    _links.setdefault(b, []).append({'map': map_id(a), 'offset': b_cell - a_cell, 'direction': opp})


def connections(name):
    return _links.get(name) or None


# ---------------------------------------------------------------- gyms
GYM_STYLES = {
    # tileset, layout learned from, (leader stamp: x, y, w, h) or None
    'water': ('gTileset_SootopolisGym', 'LAYOUT_SOOTOPOLIS_CITY_GYM_1F', None),
    'steel': ('gTileset_MauvilleGym', 'LAYOUT_MAUVILLE_CITY_GYM', None),
    'fire': ('gTileset_LavaridgeGym', 'LAYOUT_LAVARIDGE_TOWN_GYM_1F', None),
    'bug': ('gTileset_FortreeGym', 'LAYOUT_FORTREE_CITY_GYM', (12, 0, 7, 4)),
    'fairy': ('gTileset_RustboroGym', 'LAYOUT_RUSTBORO_CITY_GYM', (2, 0, 7, 4)),
    'ghost': ('gTileset_DewfordGym', 'LAYOUT_DEWFORD_TOWN_GYM', None),
    'dragon': ('gTileset_MossdeepGym', 'LAYOUT_MOSSDEEP_CITY_GYM', None),
}


def gym_style(kind):
    ts, lid, _ = GYM_STYLES[kind]
    st = Style('gTileset_Building', ts, [lid])
    st.min_count = 1
    # Only plain floors and walls: no warps, ledges, thin ice, slides or the
    # Mossdeep gym's arrow panels.
    l = layouts()[lid]
    b = _build.load_blocks(l)
    floors = {}
    for v in b.flat:
        m, coll = int(v) & 0x3FF, (int(v) >> 10) & 3
        beh = st.pair.attr(m) & 0xFF
        if beh != 0:
            st.exclude.add(m)
        elif not coll:
            floors[m] = floors.get(m, 0) + 1
    keep = sorted(floors, key=floors.get, reverse=True)[:3]
    st.exclude |= {m for m in floors if m not in keep}
    return st


def entrance_cells(kind):
    """The original gym's door mats (MB_SOUTH_ARROW_WARP) with the 7x3 block
    around them, to stamp at the bottom of a new gym."""
    _, lid, _ = GYM_STYLES[kind]
    l = layouts()[lid]
    b = _build.load_blocks(l)
    p = Pair(l['primary_tileset'], l['secondary_tileset'])
    mats = [(x, y) for y in range(b.shape[0]) for x in range(b.shape[1]) if p.attr(int(b[y, x]) & 0x3FF) & 0xFF == 0x65]
    mats.sort(key=lambda c: (-c[1], c[0]))
    return lid, mats[0][0], mats[0][1]


def build_gym(name, kind, sketch, mapsec, outside_map, outside_warp, objects, scripts, music='MUS_GYM'):
    """A gym hall synthesised from a Hoenn gym's tiles: '#' wall, '.' floor.
    The sketch's bottom row must be wall; the door goes in the middle of it,
    with the original gym's statues and mat stamped around it. Returns the
    Area (objects/warps already added) after writing it."""
    rows = sketch.strip('\n').split('\n')
    w, h = len(rows[0]), len(rows)
    st = gym_style(kind)
    a = Area(name, w, h, st, fill='#')
    a.sketch(0, 0, '\n'.join(rows))
    lid, mx, my = entrance_cells(kind)
    cx = w // 2 - 1
    # 7 wide, from two rows above the mats down to the bottom wall.
    l = layouts()[lid]
    src = _build.load_blocks(l)
    sh = min(src.shape[0] - (my - 2), 3)
    a.stamp(lid, mx - 3, my - 2, 7, sh, cx - 3, h - sh)
    door_y = h - sh + 2
    stamp = GYM_STYLES[kind][2]
    if stamp:
        sx, sy, sw, shh = stamp
        a.stamp(lid, sx, sy, sw, shh, w // 2 - sw // 2, 0)
    a.objects.extend(objects)
    a.warp(cx + 0, door_y, outside_map, outside_warp)
    a.warp(cx + 1, door_y, outside_map, outside_warp)
    a.write(map_id=map_id(name), layout_id='LAYOUT_' + snake(name), music=music, mapsec=mapsec, mapsec_name=None,
            map_type='MAP_TYPE_INDOOR', group=GROUP, scripts=scripts, show_name=False, allow_cycling=False,
            allow_running=True)
    return a
