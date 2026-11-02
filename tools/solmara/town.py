"""A Solmara town from a short spec: trees round the edge with 4-wide exits,
a paved square in the middle (room for a crowd), the gym at the top, the
Pokémon Center (with its shop counter and flight desk), houses, people,
signs and a hidden item or two. Towns with something special pass `extra`,
a function that gets the Area (and returns extra script text)."""
from sol import *
from styles import TREES, PLAZA
import flight


class Town:
    def __init__(self, **kw):
        self.__dict__.update(kw)


def build_town(ctx, T):
    g = getattr(T, 'ground', '.')
    a = Area(T.name, T.w, T.h, T.style, fill=g)
    W, H = T.w, T.h
    edge = getattr(T, 'edge', 'T')
    a.rect(0, 0, W, 3, edge)
    a.rect(0, H - 3, W, 3, edge)
    a.rect(0, 0, 3, H, edge)
    a.rect(W - 3, 0, 3, H, edge)
    for side, first in T.exits.items():
        if side == 'left':
            a.rect(0, first, 4, 4, g)
        elif side == 'right':
            a.rect(W - 4, first, 4, 4, g)
        elif side == 'up':
            a.rect(first, 0, 4, 4, g)
        else:
            a.rect(first, H - 4, 4, 4, g)
    if getattr(T, 'sketch', None):
        T.sketch(a)
    a.snap_trees()
    # The square.
    if getattr(T, 'plaza', None):
        # A compact square, not a sea of paving: trimmed to at most 12x6.
        x, y, w, h = T.plaza
        nw, nh = min(w, 12), min(h, 6)
        x, y, w, h = x + (w - nw) // 2, y + (h - nh) // 2, nw, nh
        a.rect(x, y, w, h, 'P')
        a.autotile3('P', PLAZA)
    gym = put_general(a, 'gym', *T.gym)
    pc = put_general(a, 'pc', *T.pc)
    homes = HOUSES[{'gTileset_Frost': 'gTileset_Fallarbor'}.get(T.style.secondary, T.style.secondary)]
    doors = [put_building(a, homes[i % len(homes)], x, y) for i, (x, y) in enumerate(T.houses)]
    flight.LANDING[T.name] = (pc[0], pc[1] + 1)
    a.warp(*gym, map_id(f'{T.name}_Gym'), 0)
    a.warp(*pc, map_id(f'{T.name}_PokemonCenter_1F'), 0)
    for i, d in enumerate(doors):
        a.warp(*d, map_id(f'{T.name}_House{i + 1}'), 0)

    C = f'{T.name}_EventScript'
    sc = f"""{T.name}_MapScripts::
\tmap_script MAP_SCRIPT_ON_TRANSITION, {T.name}_OnTransition
\t.byte 0

{T.name}_OnTransition:
\tsetflag {ctx.visited(T.name)}
\tend

"""
    for i, (gfx, x, y, move, lines) in enumerate(T.people):
        key = f'Person{i}'
        a.obj(gfx, x, y, f'{C}_{key}', move, rx=1, ry=1)
        sc += talker(f'{C}_{key}', lines)
    a.sign(gym[0] + 2, gym[1] + 1, f'{C}_GymSign')
    sc += signpost(f'{C}_GymSign', [f'{T.title} POKéMON GYM', f'LEADER: {T.leader}', T.motto])
    if getattr(T, 'town_sign', None):
        x, y, lines = T.town_sign
        a.sign(x, y, f'{C}_TownSign')
        sc += signpost(f'{C}_TownSign', lines)
    for i, (x, y, item) in enumerate(getattr(T, 'hidden', ())):
        a.hidden(x, y, item, REG.flag(f'HIDDEN_{snake(T.name)}_{i}'))
    for i, (x, y, item) in enumerate(getattr(T, 'items', ())):
        a.item(item, x, y, REG.flag(f'ITEM_{snake(T.name)}_{i}'))
    if getattr(T, 'flowers', 0):
        a.scatter(0x004, T.flowers, 3, 3, W - 3, H - 3, seed=len(T.name))
    if getattr(T, 'extra', None):
        sc += T.extra(a, ctx) or ''
    groves(a, T, g, edge)
    a.connections = connections(T.name)
    a.write(map_id=map_id(T.name), layout_id='LAYOUT_' + snake(T.name), music=T.music, mapsec=T.mapsec,
            mapsec_name=T.title, map_type='MAP_TYPE_CITY' if getattr(T, 'city', False) else 'MAP_TYPE_TOWN',
            group=GROUP, scripts=sc, weather=getattr(T, 'weather', 'WEATHER_NONE'))

    pokecenter(T.name, map_id(T.name), 1, T.mapsec, ctx.town_index(T.name), T.shop, npcs=T.pc_people)
    for i, people in enumerate(T.house_people):
        house(T.name, i + 1, map_id(T.name), 2 + i, T.mapsec, people,
              layout=['LAYOUT_HOUSE1', 'LAYOUT_HOUSE2', 'LAYOUT_HOUSE3', 'LAYOUT_HOUSE4'][i % 4])
    return a


def groves(a, T, g, edge, n=8):
    """Small stands of trees in the open ground near the town's edge, so the
    corners aren't empty. Each 2x2 tree keeps two clear tiles to anything
    placed (buildings, people, warps, signs) and stays off the exit lanes,
    so no path gets blocked."""
    import random
    r = random.Random(len(T.name) * 7)
    W, H = a.w, a.h
    busy = set(a.fixed)
    for o in a.objects:
        busy.add((o['x'], o['y']))
    for w in a.warps:
        busy.add((w['x'], w['y']))
    for b in a.bgs:
        busy.add((b['x'], b['y']))
    lanes = []
    for side, first in T.exits.items():
        if side in ('left', 'right'):
            lanes.append(lambda x, y, f=first: f - 3 <= y <= f + 6)
        else:
            lanes.append(lambda x, y, f=first: f - 3 <= x <= f + 6)
    spots = []
    for by in range(4, H - 5, 2):
        for bx in range(4, W - 5, 2):
            if not (bx < 10 or bx > W - 12 or by < 9 or by > H - 11):
                continue
            ok = all(0 <= x < W and 0 <= y < H and a.grid[y][x] in (g, edge) and (x, y) not in busy
                     for y in range(by - 2, by + 4) for x in range(bx - 2, bx + 4))
            if ok and not any(l(bx, by) or l(bx + 1, by + 1) for l in lanes):
                spots.append((bx, by))
    r.shuffle(spots)
    used = []
    for bx, by in spots:
        if len(used) >= n:
            break
        if any(abs(bx - ux) < 6 and abs(by - uy) < 6 for ux, uy in used):
            continue
        a.rect(bx, by, 2, 2, edge)
        used.append((bx, by))
