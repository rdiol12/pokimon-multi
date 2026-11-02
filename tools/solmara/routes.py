"""Routes 201-208: the roads round the Solmara ring. Each one has a main
path at least 4 tiles wide (room for a crowd going both ways), wide tall
grass fields to hunt in together, a pond or lake, four trainers standing
beside the path (never across it), items, hidden items and one secret
clearing with a rare Pokémon's tracks and a prize.

Trainers and wild Pokémon are postgame strength. Trainers also grow with
the player's Solmara badges (constants/solmara.h), so every road is a fair
fight whichever way round the ring you go.
"""
import copy, random
from sol import *
from styles import TREES, town, desert, volcanic, marsh, canyon, woods

H_LEN, H_H = 64, 26     # left-right routes
V_W, V_LEN = 26, 64     # up-down routes
EXITS = {}
for n in (201, 204, 205, 208):
    EXITS[f'Route{n}'] = {'left': 11, 'right': 11}
for n in (202, 203, 206, 207):
    EXITS[f'Route{n}'] = {'up': 11, 'down': 11}


def dewford():
    return Style('gTileset_General', 'gTileset_Dewford', ['LAYOUT_DEWFORD_TOWN', 'LAYOUT_ROUTE105', 'LAYOUT_ROUTE106',
                                                        'LAYOUT_ROUTE107'], wall_classes={'T': TREES})


def dunes():
    """Route 111's desert with its deep sand (wild Pokémon live in it) as the
    grass class."""
    return Style('gTileset_General', 'gTileset_Mauville', ['LAYOUT_ROUTE111'], wall_classes={'T': TREES}, colors=True,
                 classes={'"': {0x248, 0x249, 0x24A, 0x250, 0x251, 0x252, 0x258, 0x259, 0x25A}})


def frost():
    st = copy.copy(canyon())
    st.secondary = 'gTileset_Frost'
    return st


def slots(common, uncommon, rare, lv):
    """12 land slots (20,20,10,10,10,10,5,5,4,4,1,1 %)."""
    a, b = common
    c, d, e, f = uncommon
    g, h, i, j = rare
    lo, hi = lv
    names = [a, b, c, d, e, f, a, b, g, h, i, j]
    return [(n, lo + (k % 3), hi + (1 if k >= 10 else 0)) for k, n in enumerate(names)]


# name, style, ground char, music, wild land (common x2, uncommon x4, rare x4), level range, water, fish,
# trainers: (key, gfx, class, pic, gender, music, name, [(species, level)], intro, defeat, post)
ROUTES = {
    201: dict(style=town, ground='g', music='MUS_ROUTE101', weather='WEATHER_NONE', title='ROUTE 201',
              land=slots(('LECHONK', 'SKWOVET'), ('WOOLOO', 'ROOKIDEE', 'YUNGOOS', 'FLETCHLING'),
                         ('PAWMI', 'NICKIT', 'DEERLING', 'MAREEP'), (44, 46)),
              water=[('PSYDUCK', 40, 45), ('WOOPER', 40, 45), ('LOTAD', 40, 45), ('PSYDUCK', 42, 46), ('GOLDEEN', 42, 46)],
              fish=[('MAGIKARP', 30, 40), ('GOLDEEN', 40, 44), ('MAGIKARP', 30, 40), ('POLIWAG', 40, 44), ('GOLDEEN', 40, 44),
                    ('POLIWAG', 42, 46), ('ARROKUDA', 42, 46), ('ARROKUDA', 42, 46), ('ARROKUDA', 44, 48), ('BASCULIN', 44, 48)],
              secret=('MAREEP', 'ITEM_RARE_CANDY'),
              trainers=[
                  ('ELLA', 'OBJ_EVENT_GFX_LASS', 'Lass', 'Lass', 'Female', 'Female', 'ELLA', [('Lechonk', 46), ('Pawmo', 47)],
                   ['You came on the ship too? Then', 'let\'s see what HOENN taught you!'], ['HOENN taught you well!'],
                   ['There are trainers from all over', 'on these roads now. It\'s great!']),
                  ('TOBY', 'OBJ_EVENT_GFX_YOUNGSTER', 'Youngster', 'Youngster', 'Male', 'Male', 'TOBY', [('Yungoos', 46), ('Nickit', 46), ('Skwovet', 47)],
                   ['I\'m training for the PORT SOLACE', 'gym! Battle me!'], ['Aww, splash!'],
                   ['MARINA\'s WATER types are strong.', 'GRASS and ELECTRIC moves help.']),
                  ('NADIA', 'OBJ_EVENT_GFX_PICNICKER', 'Picnicker', 'Picnicker', 'Female', 'Girl', 'NADIA', [('Dubwool', 48)],
                   ['A picnic is better after a', 'battle!'], ['Time for a snack.'],
                   ['GEARHOLT is east. You can hear', 'the factories from here!']),
                  ('RICO', 'OBJ_EVENT_GFX_CAMPER', 'Camper', 'Camper', 'Male', 'Male', 'RICO', [('Corvisquire', 47), ('Fletchinder', 47)],
                   ['Birds of a feather battle', 'together!'], ['Flown away...'],
                   ['The clearing north of the lake', 'has strange tracks in it.']),
              ]),
    202: dict(style=dunes, ground='s', music='MUS_ROUTE110', weather='WEATHER_SANDSTORM', title='ROUTE 202',
              land=slots(('SANDILE', 'SILICOBRA'), ('HIPPOPOTAS', 'CACNEA', 'TRAPINCH', 'HELIOPTILE'),
                         ('DWEBBLE', 'SANDYGAST', 'MARACTUS', 'SIGILYPH'), (45, 47)),
              secret=('GABITE', 'ITEM_SOFT_SAND'),
              trainers=[
                  ('DUSTY', 'OBJ_EVENT_GFX_HIKER', 'Hiker', 'Hiker', 'Male', 'Hiker', 'DUSTY', [('Krokorok', 47), ('Sandaconda', 48)],
                   ['Sand in my boots, sand in my', 'teeth. Sand in my battles!'], ['Buried!'],
                   ['The sandstorm never stops here.', 'Stick close to your friends!']),
                  ('MIRA', 'OBJ_EVENT_GFX_PICNICKER', 'Picnicker', 'Picnicker', 'Female', 'Girl', 'MIRA', [('Maractus', 47), ('Cacturne', 48)],
                   ['Desert flowers are tough, just', 'like me!'], ['Wilted...'],
                   ['ASHCOMBE is north. Hot springs', 'after a hot desert. Lovely.']),
                  ('OSKAR', 'OBJ_EVENT_GFX_MANIAC', 'Ruin Maniac', 'Ruin Maniac', 'Male', 'Hiker', 'OSKAR', [('Sigilyph', 48), ('Hippowdon', 48)],
                   ['There are ruins under these', 'dunes! Probably!'], ['Ruined.'],
                   ['A GABITE was seen digging near', 'the rocks in the east.']),
                  ('SANDRA', 'OBJ_EVENT_GFX_EXPERT_F', 'Expert', 'Expert F', 'Female', 'Intense', 'SANDRA', [('Flygon', 49)],
                   ['Only the strong cross the dunes.', 'Prove it!'], ['You are strong.'],
                   ['Go on. The springs are waiting.']),
              ]),
    203: dict(style=volcanic, ground='s', music='MUS_ROUTE113', weather='WEATHER_VOLCANIC_ASH', title='ROUTE 203',
              land=slots(('SALANDIT', 'LITLEO'), ('SIZZLIPEDE', 'TORKOAL', 'ROLYCOLY', 'HEATMOR'),
                         ('CHARCADET', 'MAGBY', 'LARVESTA', 'SALANDIT'), (45, 47)),
              secret=('LARVESTA', 'ITEM_CHARCOAL'),
              trainers=[
                  ('BLAZE', 'OBJ_EVENT_GFX_MAN_5', 'Kindler', 'Kindler', 'Male', 'Male', 'BRAND', [('Pyroar', 48), ('Centiskorch', 48)],
                   ['My fire burns hotter than the', 'volcano!'], ['Doused!'],
                   ['The ash makes the grass grow', 'thick around here.']),
                  ('ROSA', 'OBJ_EVENT_GFX_WOMAN_5', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'ROSA', [('Salazzle', 48), ('Coalossal', 48)],
                   ['A good trainer keeps cool in the', 'heat!'], ['Too hot to handle!'],
                   ['SILKWOOD is north, in the woods.']),
                  ('GORDO', 'OBJ_EVENT_GFX_HIKER', 'Hiker', 'Hiker', 'Male', 'Hiker', 'GORDO', [('Torkoal', 48), ('Heatmor', 48)],
                   ['Smoke? That\'s just my TORKOAL', 'saying hi!'], ['Puff...'],
                   ['A LARVESTA lives in a hollow by', 'the lava rocks. Shy little thing.']),
                  ('JUNO', 'OBJ_EVENT_GFX_BLACK_BELT', 'Black Belt', 'Black Belt', 'Male', 'Intense', 'JUNO', [('Hariyama', 49), ('Infernape', 49)],
                   ['Hot iron, hard fists!'], ['My fists are smoking!'],
                   ['Train hard. The gyms here grow', 'with you.']),
              ]),
    204: dict(style=marsh, ground='.', music='MUS_ROUTE119', weather='WEATHER_RAIN', title='ROUTE 204',
              land=slots(('SEWADDLE', 'VENIPEDE'), ('KARRABLAST', 'SHELMET', 'CROAGUNK', 'YANMA'),
                         ('FOMANTIS', 'DEWPIDER', 'NYMBLE', 'GOOMY'), (46, 48)),
              water=[('WOOPER', 44, 48), ('BARBOACH', 44, 48), ('PALPITOAD', 46, 50), ('DEWPIDER', 46, 50), ('STUNFISK', 46, 50)],
              fish=[('MAGIKARP', 30, 40), ('BARBOACH', 42, 46), ('MAGIKARP', 30, 40), ('TYMPOLE', 42, 46), ('BARBOACH', 42, 46),
                    ('STUNFISK', 44, 48), ('BASCULIN', 44, 48), ('BASCULIN', 44, 48), ('BASCULIN', 46, 50), ('GOOMY', 46, 50)],
              secret=('GOOMY', 'ITEM_BIG_ROOT'),
              trainers=[
                  ('PIP', 'OBJ_EVENT_GFX_BUG_CATCHER', 'Bug Catcher', 'Bug Catcher', 'Male', 'Male', 'PIP', [('Leavanny', 48), ('Scolipede', 48)],
                   ['The marsh is full of BUGS. I', 'love it here!'], ['Squashed!'],
                   ['ROSEMEAD is west. It smells of', 'flowers, not swamp!']),
                  ('MAE', 'OBJ_EVENT_GFX_WOMAN_4', 'Pkmn Breeder', 'Pokemon Breeder F', 'Female', 'Female', 'MAE', [('Toxicroak', 48), ('Araquanid', 49)],
                   ['My POKéMON love the rain. Do', 'yours?'], ['Rained out!'],
                   ['A GOOMY hides in the reeds by', 'the far pond. Too cute!']),
                  ('WADE', 'OBJ_EVENT_GFX_FISHERMAN', 'Fisherman', 'Fisherman', 'Male', 'Hiker', 'WADE', [('Whiscash', 48), ('Basculin', 48), ('Seismitoad', 49)],
                   ['Shh! You\'ll scare the fish!', 'Battle me quietly!'], ['They got away...'],
                   ['Fish here with a friend. It\'s', 'more fun and you can trade', 'what you catch.']),
                  ('IVY', 'OBJ_EVENT_GFX_WOMAN_2', 'Aroma Lady', 'Aroma Lady', 'Female', 'Female', 'IVY', [('Lurantis', 49), ('Accelgor', 49)],
                   ['Even a swamp has a sweet smell', 'if you look for it.'], ['How refreshing.'],
                   ['SILKWOOD\'s gym is just east.']),
              ]),
    205: dict(style=canyon, ground='s', music='MUS_ROUTE113', weather='WEATHER_NONE', title='ROUTE 205',
              land=slots(('MUDBRAY', 'ROGGENROLA'), ('DRILBUR', 'RUFFLET', 'VULLABY', 'GLIGAR'),
                         ('NACLI', 'CUFANT', 'BRONZOR', 'TINKATINK'), (46, 48)),
              secret=('SKARMORY', 'ITEM_HARD_STONE'),
              trainers=[
                  ('CLIFF', 'OBJ_EVENT_GFX_HIKER', 'Hiker', 'Hiker', 'Male', 'Hiker', 'CLIFF', [('Mudsdale', 49), ('Boldore', 48)],
                   ['These canyon walls are older', 'than POKéMON battles!'], ['Rockslide!'],
                   ['DUSKMERE is west, past the', 'canyon. Bring a light!']),
                  ('KESTREL', 'OBJ_EVENT_GFX_MAN_3', 'Bird Keeper', 'Bird Keeper', 'Male', 'Male', 'KESTREL', [('Braviary', 49), ('Mandibuzz', 49)],
                   ['My birds rule the canyon skies!'], ['Grounded!'],
                   ['A SKARMORY nests up on the', 'ledge to the north.']),
                  ('TINA', 'OBJ_EVENT_GFX_GIRL_3', 'Battle Girl', 'Battle Girl', 'Female', 'Intense', 'TINA', [('Tinkatuff', 48), ('Garganacl', 49)],
                   ['Hammer time!'], ['Flattened!'],
                   ['ROSEMEAD\'s gym is east.']),
                  ('GRAHAM', 'OBJ_EVENT_GFX_MANIAC', 'Ruin Maniac', 'Ruin Maniac', 'Male', 'Hiker', 'GRAHAM', [('Bronzong', 49), ('Excadrill', 49)],
                   ['I dig and dig, and find only', 'trainers to battle!'], ['Dug too deep.'],
                   ['Trainers meet up at the overlook', 'to trade. Wide enough for all.']),
              ]),
    206: dict(style=frost, ground='s', music='MUS_ROUTE120', weather='WEATHER_SNOW', title='ROUTE 206',
              land=slots(('SNOM', 'CUBCHOO'), ('BERGMITE', 'SNOVER', 'SWINUB', 'SNORUNT'),
                         ('SNEASEL', 'CETODDLE', 'VANILLITE', 'FRIGIBAX'), (47, 49)),
              secret=('FRIGIBAX', 'ITEM_NEVER_MELT_ICE'),
              trainers=[
                  ('BJORN', 'OBJ_EVENT_GFX_HIKER', 'Hiker', 'Hiker', 'Male', 'Hiker', 'BJORN', [('Beartic', 50), ('Avalugg', 49)],
                   ['Brr! Battle me to warm up!'], ['Frozen solid...'],
                   ['FROSTHOLM is south. Their bread', 'is the best in SOLMARA!']),
                  ('ELSA', 'OBJ_EVENT_GFX_WOMAN_5', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'ELSA', [('Froslass', 50), ('Frosmoth', 50)],
                   ['The cold never bothered me.'], ['Melted!'],
                   ['A FRIGIBAX plays in the snow', 'past the frozen pond.']),
                  ('RUNE', 'OBJ_EVENT_GFX_BOY_2', 'School Kid', 'School Kid M', 'Male', 'Male', 'RUNE', [('Mamoswine', 50)],
                   ['Snow day! No school! Battle!'], ['Snow fair...'],
                   ['DUSKMERE is north, in the fog.']),
                  ('FREYA', 'OBJ_EVENT_GFX_EXPERT_F', 'Expert', 'Expert F', 'Female', 'Intense', 'FREYA', [('Weavile', 50), ('Glalie', 50)],
                   ['Only the toughest train in the', 'snow. Are you tough?'], ['You are tough.'],
                   ['SIGRID\'s gym floor is ice. Plan', 'every step!']),
              ]),
    207: dict(style=woods, ground='.', music='MUS_ROUTE120', weather='WEATHER_NONE', title='ROUTE 207',
              land=slots(('NOIBAT', 'AXEW'), ('DEINO', 'DRUDDIGON', 'BAGON', 'TURTONATOR'),
                         ('JANGMO_O', 'DRAMPA', 'GIBLE', 'SKARMORY'), (48, 50)),
              water=[('SKRELP', 44, 48), ('PSYDUCK', 44, 48), ('SKRELP', 46, 50), ('LAPRAS', 48, 52), ('SKRELP', 46, 50)],
              fish=[('MAGIKARP', 30, 40), ('SKRELP', 44, 48), ('MAGIKARP', 30, 40), ('GOLDEEN', 44, 48), ('SKRELP', 44, 48),
                    ('GOLDEEN', 46, 50), ('ARROKUDA', 46, 50), ('ARROKUDA', 46, 50), ('ARROKUDA', 48, 52), ('LAPRAS', 48, 52)],
              secret=('JANGMO_O', 'ITEM_DRAGON_FANG'),
              trainers=[
                  ('DRACO', 'OBJ_EVENT_GFX_MAN_3', 'Dragon Tamer', 'Dragon Tamer', 'Male', 'Intense', 'DRACO', [('Fraxure', 50), ('Druddigon', 50)],
                   ['DRAGONS answer only to the', 'worthy!'], ['You are worthy.'],
                   ['DRAKESPIRE is just south.']),
                  ('LUNA', 'OBJ_EVENT_GFX_WOMAN_5', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'LUNA', [('Noivern', 50), ('Drampa', 50)],
                   ['Can your team handle dragons?'], ['Clipped wings!'],
                   ['A JANGMO-O trains alone in the', 'clearing past the pines.']),
                  ('BRICK', 'OBJ_EVENT_GFX_BLACK_BELT', 'Black Belt', 'Black Belt', 'Male', 'Intense', 'BRICK', [('Hakamo-o', 50), ('Hawlucha', 50)],
                   ['Scales and fists!'], ['My scales are cracked!'],
                   ['VALEN trains up on the cliffs', 'every morning.']),
                  ('GWEN', 'OBJ_EVENT_GFX_WOMAN_2', 'Pkmn Breeder', 'Pokemon Breeder F', 'Female', 'Female', 'GWEN', [('Shelgon', 50), ('Gabite', 50)],
                   ['I raised these from eggs!'], ['They\'ll grow from this.'],
                   ['FROSTHOLM is north, up in the', 'snow.']),
              ]),
    208: dict(style=town, ground='s', coast=True, music='MUS_ROUTE104', weather='WEATHER_NONE', title='ROUTE 208',
              land=slots(('WINGULL', 'CRABRAWLER'), ('WATTREL', 'PIKIPEK', 'SANDYGAST', 'ORICORIO'),
                         ('BINACLE', 'PYUKUMUKU', 'MAREANIE', 'WIMPOD'), (46, 48)),
              water=[('TENTACOOL', 44, 48), ('FINIZEN', 44, 48), ('WAILMER', 46, 50), ('MANTYKE', 46, 50), ('LAPRAS', 48, 52)],
              fish=[('MAGIKARP', 30, 40), ('KRABBY', 44, 48), ('MAGIKARP', 30, 40), ('CLAUNCHER', 44, 48), ('KRABBY', 44, 48),
                    ('BRUXISH', 46, 50), ('CLAUNCHER', 46, 50), ('ARROKUDA', 46, 50), ('BRUXISH', 48, 52), ('WAILMER', 48, 52)],
              secret=('WIMPOD', 'ITEM_PEARL_STRING'),
              trainers=[
                  ('CORAL', 'OBJ_EVENT_GFX_SWIMMER_F', 'Swimmer F', 'Swimmer F', 'Female', 'Swimmer', 'CORAL', [('Palafin', 50), ('Clawitzer', 50)],
                   ['The sea here is perfect for', 'training!'], ['Washed up!'],
                   ['PORT SOLACE is east, along the', 'beach.']),
                  ('SKIP', 'OBJ_EVENT_GFX_TUBER_M', 'Tuber M', 'Tuber M', 'Male', 'Male', 'SKIP', [('Pyukumuku', 49), ('Crabominable', 50)],
                   ['Splash battle!'], ['Glub...'],
                   ['A WIMPOD hides under the rocks', 'at the end of the beach!']),
                  ('MARLIN', 'OBJ_EVENT_GFX_FISHERMAN', 'Fisherman', 'Fisherman', 'Male', 'Hiker', 'MARLIN', [('Bruxish', 50), ('Barraskewda', 50)],
                   ['Reeled you in!'], ['The one that got away...'],
                   ['DRAKESPIRE is west. Watch the', 'sky for dragons!']),
                  ('SUNNY', 'OBJ_EVENT_GFX_WOMAN_4', 'Beauty', 'Beauty', 'Female', 'Female', 'SUNNY', [('Oricorio', 50), ('Kilowattrel', 50)],
                   ['Sun, sand and battles!'], ['What a day!'],
                   ['Everyone meets at HARBOR', 'SQUARE. Want to come?']),
              ]),
}


def carve_route(a, horizontal, rnd, ground, grass='"'):
    """Path, grass fields, a pond and the secret clearing; returns the
    path cells and the clearing's top-left corner."""
    L = a.w if horizontal else a.h
    E = 11  # exit row/column, 4 wide
    pts = [(0, E)]
    for k in range(1, 5):
        pts.append((L * k // 5, rnd.randint(7, 15)))
    pts.append((L - 1, E))
    if not horizontal:
        pts = [(y, x) for x, y in pts]
    path = [(x + 1.5, y + 1.5) if horizontal else (x + 1.5, y + 1.5) for x, y in pts]
    a.line([(int(x), int(y)) for x, y in path], 4, ground)
    # Grass fields beside the path, wide enough for several people.
    for k in range(1, 5):
        x, y = pts[k]
        if horizontal:
            gx, gy = x + rnd.randint(-4, 4), (y - 6 if y > 12 else y + 8)
            a.ellipse(gx, gy, rnd.randint(5, 7), 3, grass)
        else:
            gx, gy = (x - 6 if x > 12 else x + 8), y + rnd.randint(-4, 4)
            a.ellipse(gx, gy, 3, rnd.randint(5, 7), grass)
        a.line([(int(x) + 1, int(y) + 1), (int(gx), int(gy))], 3, ground)
    return pts


def build(ctx):
    for n, R in ROUTES.items():
        build_route(ctx, n, R)


def build_route(ctx, n, R):
    name = f'Route{n}'
    horizontal = 'left' in EXITS[name]
    w, h = (H_LEN, H_H) if horizontal else (V_W, V_LEN)
    st = R['style']()
    a = Area(name, w, h, st, fill='T')
    rnd = random.Random(n)
    g = R['ground']
    pts = carve_route(a, horizontal, rnd, g, R.get('grass', '"'))
    # Pond or lake (routes with water encounters), on the far side of a field.
    if 'water' in R:
        if horizontal:
            px, py = w * 0.55, (5 if pts[3][1] > 12 else h - 7)
            a.ellipse(px, py, 5, 2.5, '~')
            a.line([(int(px), int(py)), (int(px), int(pts[3][1]) + 1)], 3, g)
        else:
            px, py = (5 if pts[3][0] > 12 else w - 7), h * 0.55
            a.ellipse(px, py, 2.5, 5, '~')
            a.line([(int(px), int(py)), (int(pts[3][0]) + 1, int(py))], 3, g)
        a.ellipse(px, py, 5, 2.5, '~') if horizontal else a.ellipse(px, py, 2.5, 5, '~')
        ring(a, '~', g, over=('T',))
    if R.get('coast'):
        a.rect(0, h - 5, w, 5, '~')
    # The secret clearing: off the beaten track, reached through a gap.
    if horizontal:
        cx, cy = (w * 3 // 10, 2) if pts[1][1] > 11 else (w * 3 // 10, h - 8)
        a.rect(cx, cy, 6, 6, g)
        a.rect(cx + 2, min(cy, pts[1][1]), 3, abs(pts[1][1] - cy) + 2, g)
    else:
        cx, cy = (2, h * 3 // 10) if pts[1][0] > 11 else (w - 8, h * 3 // 10)
        a.rect(cx, cy, 6, 6, g)
        a.rect(min(cx, pts[1][0]), cy + 2, abs(pts[1][0] - cx) + 2, 3, g)
    a.snap_trees(ground=g)
    for x, y in [(0, 0)]:
        pass
    # Exit mouths must stay open after snapping.
    if horizontal:
        a.rect(0, 11, 3, 4, g)
        a.rect(w - 3, 11, 3, 4, g)
    else:
        a.rect(11, 0, 4, 3, g)
        a.rect(11, h - 3, 4, 3, g)
    a.snap_trees(ground=g)

    C = f'{name}_EventScript'
    sc = f'{name}_MapScripts::\n\t.byte 0\n\n'
    # Trainers beside the path, facing it.
    path_cells = [(x, y) for y in range(h) for x in range(w) if a.grid[y][x] == g]
    for i, (key, gfx, cls, pic, gender, music, tname, mons, intro, defeat, post) in enumerate(R['trainers']):
        k = pts[i + 1]
        x, y = int(k[0]) + 1, int(k[1]) + 1
        if horizontal:
            # stand on the path's edge, above or below it, looking across
            y = y - 2 if i % 2 == 0 else y + 3
            move, sight = ('MOVEMENT_TYPE_FACE_DOWN', 4) if i % 2 == 0 else ('MOVEMENT_TYPE_FACE_UP', 4)
        else:
            x = x - 2 if i % 2 == 0 else x + 3
            move, sight = ('MOVEMENT_TYPE_FACE_RIGHT', 4) if i % 2 == 0 else ('MOVEMENT_TYPE_FACE_LEFT', 4)
        x, y = nearest_open(a, x, y, g, edge=True)
        tid = REG.trainer(f'R{n}_{key}', party(tname, cls, pic, gender, music, [(s, l) for s, l in mons]))
        a.trainer(gfx, x, y, f'{C}_{key.title()}', sight, move)
        sc += trainer_script(f'{C}_{key.title()}', tid, intro, defeat, post)
    # Items along the way, and in the clearing.
    picks = ['ITEM_ULTRA_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_REVIVE' if n % 2 else 'ITEM_PP_UP', 'ITEM_FULL_RESTORE']
    for i, it in enumerate(picks[:3]):
        k = pts[min(i + 2, 4)]
        x, y = nearest_open(a, int(k[0]) + (2 if horizontal else 5), int(k[1]) + (5 if horizontal else 2), g)
        a.item(it, x, y, REG.flag(f'ITEM_R{n}_{i}'))
    species, prize = R['secret']
    a.item(prize, cx + 3, cy + 2, REG.flag(f'ITEM_R{n}_SECRET'))
    a.hidden(cx + 1, cy + 4, 'ITEM_RARE_CANDY', REG.flag(f'HIDDEN_R{n}_SECRET'))
    hx, hy = nearest_open(a, w // 2, h // 2, g)
    a.hidden(hx, hy, 'ITEM_BIG_NUGGET' if n % 2 else 'ITEM_PP_MAX', REG.flag(f'HIDDEN_R{n}_0'))
    # The secret: a rare Pokémon you can see in the clearing; talk to it to battle.
    fl = REG.flag(f'R{n}_SECRET_MON')
    a.obj(f'OBJ_EVENT_GFX_SPECIES({species})', cx + 2, cy + 3, f'{C}_Secret', 'MOVEMENT_TYPE_WANDER_AROUND', fl, rx=1, ry=1)
    secret_text = text("It's a wild " + species.replace('_', '-') + '! It looks like', 'it has been waiting for a', 'challenger.')
    sc += f"""{C}_Secret::
\tlock
\tfaceplayer
\tplaymoncry SPECIES_{species}, CRY_MODE_ENCOUNTER
\tmsgbox {C}_Text_Secret, MSGBOX_DEFAULT
\twaitmoncry
\tsetwildbattle SPECIES_{species}, 52
\tdowildbattle
\tsetflag {fl}
\tremoveobject VAR_LAST_TALKED
\trelease
\tend

{C}_Text_Secret:
{secret_text}
"""
    # A signpost at each end.
    a.sign(*nearest_open(a, 4 if horizontal else 9, 9 if horizontal else 4, g), f'{C}_Sign')
    sc += signpost(f'{C}_Sign', [R['title'], ROUTE_ENDS[n]])
    register_wild(map_id(name), f'g{name}', land=[(s, lo, hi) for s, lo, hi in R['land']],
                  water=R.get('water'), fish=R.get('fish'))
    a.connections = connections(name)
    a.write(map_id=map_id(name), layout_id='LAYOUT_' + snake(name), music=R['music'], mapsec=f'MAPSEC_SOLMARA_ROUTE_{n}',
            mapsec_name=R['title'], weather=R['weather'], group=GROUP, scripts=sc)


ROUTE_ENDS = {201: 'PORT SOLACE - GEARHOLT', 202: 'GEARHOLT - ASHCOMBE', 203: 'ASHCOMBE - SILKWOOD',
              204: 'SILKWOOD - ROSEMEAD', 205: 'ROSEMEAD - DUSKMERE', 206: 'DUSKMERE - FROSTHOLM',
              207: 'FROSTHOLM - DRAKESPIRE', 208: 'DRAKESPIRE - PORT SOLACE'}


def nearest_open(a, x, y, g, edge=False, taken=set()):
    """The nearest cell of ground class g (not fixed) to (x, y). edge: only
    cells at the side of the path (touching trees or grass), so a trainer
    never stands in the middle of the walkway."""
    best = None
    for yy in range(a.h):
        for xx in range(a.w):
            if a.grid[yy][xx] == g and (xx, yy) not in a.fixed and 2 <= xx < a.w - 2 and 2 <= yy < a.h - 2 \
                    and (xx, yy) not in taken:
                if edge and all(a.grid[yy + dy][xx + dx] == g for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                    continue
                d = abs(xx - x) + abs(yy - y)
                if best is None or d < best[0]:
                    best = (d, xx, yy)
    taken.add((best[1], best[2]))
    return best[1], best[2]
