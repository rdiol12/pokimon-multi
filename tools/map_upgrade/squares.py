"""Town squares: every small Hoenn town grows a new district on a free edge
(south or east, so no old coordinate moves) with a paved square where
players meet. Each square has a guide (online HUB and PLAZA), a matchmaker
(the QUICK MATCH desk's battle/trade finder), a notice board, a local with
something to say, and a hidden item.

    python3 tools/map_upgrade/squares.py [TOWN ...]   # all towns by default
    python3 tools/map_upgrade/squares.py --preview DIR [TOWN ...]

The first run saves each town's original layout and map.json under
tools/map_upgrade/orig/; later runs start from those, so the tool can be
rerun after tweaking a town. A rerun also puts back the saved map.json, so
redo (or move into orig/) any later hand edits to a grown town's map.json.
"""
import json, os, shutil, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'areas'))
from extend import Extension, ROOT, write_json
from synth import Style
from styles import town, jungle, volcanic, TREES, PLAZA, POND
from tiles import layouts

ORIG = f'{HERE}/orig'
SCRIPTS = f'{ROOT}/data/scripts/town_squares.inc'
FLAGS_H = f'{ROOT}/include/constants/map_upgrade.h'
TAG = 'Square_'   # every event the tool adds has a script starting with this

_styles = {}


def style(kind):
    if kind in _styles:
        return _styles[kind]
    if kind == 'town':
        st = town()
    elif kind == 'jungle':
        st = jungle()
    elif kind == 'volcanic':
        st = volcanic()
    elif kind == 'dewford':
        st = Style('gTileset_General', 'gTileset_Dewford', ['LAYOUT_DEWFORD_TOWN', 'LAYOUT_ROUTE105',
                   'LAYOUT_ROUTE106', 'LAYOUT_ROUTE107'], wall_classes={'T': TREES}, colors=True)
    elif kind == 'mauville':
        st = Style('gTileset_General', 'gTileset_Mauville', ['LAYOUT_MAUVILLE_CITY', 'LAYOUT_VERDANTURF_TOWN',
                   'LAYOUT_ROUTE110', 'LAYOUT_ROUTE117', 'LAYOUT_ROUTE118'], wall_classes={'T': TREES}, colors=True)
    elif kind == 'fallarbor':
        st = Style('gTileset_General', 'gTileset_Fallarbor', ['LAYOUT_FALLARBOR_TOWN', 'LAYOUT_ROUTE113',
                   'LAYOUT_ROUTE114', 'LAYOUT_ROUTE115'], wall_classes={'T': TREES}, colors=True)
    st.exclude = set(st.exclude) | EXCLUDE.get(kind, set())
    _styles[kind] = st
    return st


# metatiles the synthesiser may not use in the new ground (ladders, water
# corners and other one-off pieces that look wrong out of place)
EXCLUDE = {
    'volcanic': {0x018, 0x271, 0x2EE, 0x073},
    'mauville': {0x298, 0x299, 0x29A, 0x37D, 0x280, 0x2E5, 0x2FE, 0x2E6, 0x293, 0x294, 0x29B, 0x29C, 0x133, 0x35E, 0x37E, 0x2A3, 0x2A4},
    'fallarbor': {0x28B, 0x20E, 0x060, 0x241, 0x3EB, 0x3AB},
    'jungle': set(),
}


# ------------------------------------------------------------------ originals
def originals(folder):
    """(layout blockdata path, map.json path) restored from the saved originals."""
    m = json.load(open(f'{ROOT}/data/maps/{folder}/map.json'))
    lay = layouts()[m['layout']]
    d = f'{ORIG}/{folder}'
    if not os.path.exists(d):
        os.makedirs(d)
        shutil.copy(f"{ROOT}/{lay['blockdata_filepath']}", f'{d}/map.bin')
        shutil.copy(f'{ROOT}/data/maps/{folder}/map.json', f'{d}/map.json')
        json.dump({'width': lay['width'], 'height': lay['height']}, open(f'{d}/size.json', 'w'))
    return d


def restore(folder):
    d = originals(folder)
    m = json.load(open(f'{ROOT}/data/maps/{folder}/map.json'))
    size = json.load(open(f'{d}/size.json'))
    lay_json = f'{ROOT}/data/layouts/layouts.json'
    data = json.load(open(lay_json))
    for l in data['layouts']:
        if l.get('id') == m['layout']:
            l['width'], l['height'] = size['width'], size['height']
            shutil.copy(f'{d}/map.bin', f"{ROOT}/{l['blockdata_filepath']}")
    write_json(lay_json, data)
    shutil.copy(f'{d}/map.json', f'{ROOT}/data/maps/{folder}/map.json')


# ------------------------------------------------------------------ helpers
def flowers(a, metatile, n, x0, y0, x1, y1, ground='g', seed=1, clump=3):
    """Area.scatter for any ground class."""
    import random
    r = random.Random(seed)
    def ok(x, y):
        return all(0 <= x + dx < a.w and 0 <= y + dy < a.h and a.grid[y + dy][x + dx] == ground
                   and (x + dx, y + dy) not in a.fixed for dx in (-1, 0, 1) for dy in (-1, 0, 1))
    placed = tries = 0
    while placed < n and tries < n * 80:
        tries += 1
        x, y = r.randrange(x0, x1), r.randrange(y0, y1)
        if ok(x, y):
            for _ in range(clump):
                if ok(x, y):
                    a.tile(x, y, metatile, collision=0)
                    placed += 1
                x, y = x + r.choice((-1, 0, 1)), y + r.choice((-1, 0, 1))


class Square:
    """Collects one town's square: its scripts and texts."""
    def __init__(self, key, town_name, ext):
        self.key, self.town, self.ext = key, town_name, ext
        self.texts = []
        self.replace = {}   # bad metatile -> good one, applied to new cells after synthesis

    def finish(self, b):
        e = self.ext
        for y in range(e.h):
            for x in range(e.w):
                ox, oy = x - e.dx, y - e.dy
                if 0 <= ox < e.ow and 0 <= oy < e.oh and (x, y) in e.area.fixed:
                    continue
                m = int(b[y, x]) & 0x3FF
                if m in self.replace:
                    good = self.replace[m]
                    b[y, x] = good if good > 0x3FF else good | (int(e.style.elev[good].most_common(1)[0][0]))
        return b

    def plaza(self, x, y, w, h, c=None):
        """The open square. c: the town's own path class (sand paths in the
        small towns); without it, General's paved plaza."""
        self.ext.area.rect(x, y, w, h, c or 'P')
        self.rect = (x, y, w, h)

    def centre(self, metatile=0x004, ring=True):
        """A flower bed (2x2) in the middle of the square."""
        x, y, w, h = self.rect
        cx, cy = x + w // 2 - 1, y + h // 2 - 1
        for dy in (0, 1):
            for dx in (0, 1):
                self.ext.area.tile(cx + dx, cy + dy, metatile, collision=0, c='g')

    def board(self, x, y):
        self.ext.area.tile(x, y, 0x003, collision=1, c='#')
        self.ext.sign(x, y, f'{TAG}{self.key}_EventScript_Board')

    def guide(self, x, y, gfx='OBJ_EVENT_GFX_WOMAN_2', move='MOVEMENT_TYPE_FACE_DOWN'):
        self.ext.obj(gfx, x, y, f'{TAG}{self.key}_EventScript_Guide', move)

    def matchmaker(self, x, y, gfx='OBJ_EVENT_GFX_COOLTRAINER_M', move='MOVEMENT_TYPE_FACE_DOWN'):
        self.ext.obj(gfx, x, y, f'{TAG}{self.key}_EventScript_Matchmaker', move)

    def local(self, x, y, gfx, text, move='MOVEMENT_TYPE_FACE_DOWN', n=1, rx=0, ry=0):
        self.ext.obj(gfx, x, y, f'{TAG}{self.key}_EventScript_Local{n}', move, rx=rx, ry=ry)
        self.texts.append((n, text))

    def hidden(self, x, y, item):
        self.ext.hidden(x, y, item, f'FLAG_SQUARE_{self.key.upper()}_HIDDEN')

    def script(self, board_text):
        k = self.key
        out = [f'@ {self.town} square (tools/map_upgrade/squares.py)',
               f'{TAG}{k}_EventScript_Board::',
               f'\tmsgbox {TAG}{k}_Text_Board, MSGBOX_SIGN',
               '\tend', '',
               f'{TAG}{k}_EventScript_Guide::',
               '\tgoto Square_EventScript_Guide', '',
               f'{TAG}{k}_EventScript_Matchmaker::',
               '\tgoto HoennMulti_EventScript_QuickMatchDesk', '']
        for n, _ in self.texts:
            out += [f'{TAG}{k}_EventScript_Local{n}::', f'\tmsgbox {TAG}{k}_Text_Local{n}, MSGBOX_NPC', '\tend', '']
        out += [f'{TAG}{k}_Text_Board:', *string(board_text), '']
        for n, t in self.texts:
            out += [f'{TAG}{k}_Text_Local{n}:', *string(t), '']
        return '\n'.join(out)


def check_reachable(e, b):
    """Every new person, sign and hidden item must be reachable on foot from
    the town's first warp (collision, elevation and water respected)."""
    from collections import deque
    water = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x50, 0x51, 0x52, 0x53}
    def beh(x, y):
        return e.style.pair.attr(int(b[y, x]) & 0x3FF) & 0xFF
    def ok(x, y):
        return 0 <= x < e.w and 0 <= y < e.h and not (int(b[y, x]) >> 10) & 3 and beh(x, y) not in water
    w = e.map['warp_events'][0]
    start = (w['x'] + e.dx, w['y'] + e.dy + 1)
    seen, q = {start}, deque([start])
    while q:
        x, y = q.popleft()
        z = int(b[y, x]) >> 12
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in seen or not ok(*n):
                continue
            nz = int(b[n[1], n[0]]) >> 12
            if z and nz and z != 15 and nz != 15 and z != nz:
                continue
            seen.add(n)
            q.append(n)
    bad = []
    for o in e.new_objects:
        if not any((o['x'] + dx, o['y'] + dy) in seen for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0))):
            bad.append(('person', o['x'], o['y']))
    for g in e.new_bgs:
        if g['type'] == 'hidden_item' and (g['x'], g['y']) not in seen:
            bad.append(('hidden item', g['x'], g['y']))
        if g['type'] == 'sign' and (g['x'], g['y'] + 1) not in seen:
            bad.append(('sign', g['x'], g['y']))
    return bad


def string(text):
    """Text with \\n / \\l / \\p already in it -> .string lines."""
    parts, cur = [], ''
    for ch in text:
        cur += ch
    lines = []
    chunk = ''
    i = 0
    while i < len(text):
        if text[i] == '\\' and i + 1 < len(text) and text[i + 1] in 'nlp':
            chunk += text[i:i + 2]
            lines.append(chunk)
            chunk = ''
            i += 2
            continue
        chunk += text[i]
        i += 1
    lines.append(chunk + '$')
    return [f'\t.string "{l}"' for l in lines]


COMMON = r'''@ Shared by every town square.
Square_EventScript_Guide::
	lock
	faceplayer
	callnative HoennMulti_IsOnline
	goto_if_eq VAR_RESULT, FALSE, Square_EventScript_GuideOffline
	message Square_Text_GuideWelcome
	waitmessage
	dynmultipush Square_Text_Hub, 0
	dynmultipush Square_Text_Plaza, 1
	dynmultipush Online_Text_HubClose, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	closemessage
	release
	goto_if_eq VAR_RESULT, 0, Online_EventScript_Hub
	goto_if_eq VAR_RESULT, 1, NetplayExt_EventScript_Plaza
	end

Square_EventScript_GuideOffline:
	msgbox Square_Text_GuideOffline, MSGBOX_DEFAULT
	release
	end

Square_Text_GuideWelcome:
	.string "Welcome to the square! Trainers from\n"
	.string "all over meet up here.\p"
	.string "What would you like to see?$"

Square_Text_GuideOffline:
	.string "Welcome to the square! Trainers from\n"
	.string "all over meet up here.\p"
	.string "Choose JOIN ONLINE on the title screen\n"
	.string "and I can show you the HUB and PLAZA.$"

Square_Text_Hub:
	.string "ONLINE HUB$"

Square_Text_Plaza:
	.string "PLAZA$"
'''


# ------------------------------------------------------------------ towns
def littleroot():
    e = Extension('LittlerootTown', style('town'), down=18, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Littleroot', 'Littleroot Town', e)
    e.reopen(2, 18, 14, 2, 'g')
    a.rect(0, 20, 2, H - 20, 'T')
    a.rect(W - 2, 20, 2, H - 20, 'T')
    a.rect(0, H - 2, W, 2, 'T')
    a.rect(2, 32, 4, 4, 'T')
    a.rect(14, 22, 4, 2, 'T')
    s.plaza(5, 24, 10, 6, 's')
    s.centre()
    a.ellipse(14.5, 33, 3.2, 2.2, 'W')
    a.autotile3('W', POND)
    a.snap_trees(ground='g')
    a.autotile3('P', PLAZA)
    flowers(a, 0x004, 14, 2, 20, 18, 36, seed=4)
    s.board(5, 23)
    s.guide(8, 24)
    s.matchmaker(11, 24, move='MOVEMENT_TYPE_FACE_DOWN')
    s.local(7, 31, 'OBJ_EVENT_GFX_LITTLE_GIRL',
            'Mom says trainers from far away come\\n'
            'all the way here just to battle!\\p'
            'I want to be one of them someday.', move='MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
    s.hidden(17, 35, 'ITEM_POTION')
    return e, s, 'LITTLEROOT GREEN\\nWhere every journey starts.\\p'\
        'Meet trainers from all over Hoenn.\\n'\
        'The guide has the HUB, the matchmaker\\l'\
        'finds you a battle or a trade.'



def border(a, x0, y0, x1, y1, c='T', t=2, sides='lrb'):
    """Tree (or rock) walls t thick on the given sides of the box x0..x1, y0..y1 (exclusive)."""
    if 'l' in sides: a.rect(x0, y0, t, y1 - y0, c)
    if 'r' in sides: a.rect(x1 - t, y0, t, y1 - y0, c)
    if 't' in sides: a.rect(x0, y0, x1 - x0, t, c)
    if 'b' in sides: a.rect(x0, y1 - t, x1 - x0, t, c)


def oldale():
    e = Extension('OldaleTown', style('town'), right=16, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Oldale', 'Oldale Town', e)
    border(a, 20, 0, W, H, sides='trb')
    a.rect(20, 0, 2, 7, 'T')
    a.rect(20, 13, 2, 7, 'T')
    a.rect(28, 2, 4, 3, 'T')
    a.rect(30, 14, 4, 4, 'T')
    s.plaza(23, 6, 10, 7, 's')
    s.centre()
    a.snap_trees(ground='g')
    a.autotile3('P', PLAZA)
    flowers(a, 0x004, 14, 20, 2, 34, 18, seed=5)
    s.board(23, 5)
    s.guide(26, 6, 'OBJ_EVENT_GFX_WOMAN_1')
    s.matchmaker(29, 6, 'OBJ_EVENT_GFX_COOLTRAINER_F')
    s.local(24, 15, 'OBJ_EVENT_GFX_OLD_MAN',
            'When I was young, Oldale was just a\\n'
            'stop on the way to somewhere else.\\p'
            'Now trainers camp out on this square\\n'
            'all night, trading stories.', move='MOVEMENT_TYPE_FACE_UP')
    s.hidden(33, 3, 'ITEM_ANTIDOTE')
    return e, s, 'OLDALE SQUARE\\nWhere all paths cross.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def petalburg():
    e = Extension('PetalburgCity', style('town'), down=16, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Petalburg', 'Petalburg City', e)
    e.reopen(14, 27, 6, 3, 'g')
    a.rect(15, 26, 3, 4, 's')
    border(a, 0, 30, W, H)
    a.rect(2, 30, 3, 4, 'T')
    a.rect(24, 30, 4, 4, 'T')
    a.rect(15, 30, 3, 3, 's')
    s.plaza(9, 33, 14, 7, 's')
    s.centre()
    a.ellipse(5.5, 40, 3.5, 2.5, 'W')
    a.autotile3('W', POND)
    a.snap_trees(ground='g')
    a.autotile3('P', PLAZA)
    flowers(a, 0x004, 14, 2, 30, 28, 44, seed=6)
    s.board(14, 32)
    s.guide(11, 33, 'OBJ_EVENT_GFX_WOMAN_5')
    s.matchmaker(20, 33, 'OBJ_EVENT_GFX_BLACK_BELT')
    s.local(25, 40, 'OBJ_EVENT_GFX_FISHERMAN',
            'Norman runs a tight gym, but out here\\n'
            'anyone can battle anyone.\\p'
            'I come to watch. Best seat in town!', move='MOVEMENT_TYPE_FACE_LEFT')
    s.hidden(26, 43, 'ITEM_SUPER_POTION')
    return e, s, 'PETALBURG PLAZA\\nA city of balance.\\p'\
        'Meet trainers from all over Hoenn.\\n'\
        'The guide has the HUB, the matchmaker\\l'\
        'finds you a battle or a trade.'


def dewford():
    e = Extension('DewfordTown', style('dewford'), down=14, fill='s')
    a, W, H = e.area, e.w, e.h
    s = Square('Dewford', 'Dewford Town', e)
    e.reopen(3, 19, 11, 1, 's')
    border(a, 0, 20, W, H, sides='lr')
    a.rect(0, H - 4, W, 4, '~')
    s.plaza(5, 23, 10, 5, 's')
    a.snap_trees(ground='s')
    a.autotile3('P', PLAZA)
    s.board(5, 22)
    s.guide(8, 23, 'OBJ_EVENT_GFX_FISHERMAN')
    s.matchmaker(11, 23, 'OBJ_EVENT_GFX_SAILOR')
    s.local(14, 28, 'OBJ_EVENT_GFX_TUBER_F',
            'Boats bring trainers from every port\\n'
            'to battle on this beach!\\p'
            'Brawly says it\'s the best training a\\n'
            'surfer could ask for.', move='MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
    s.hidden(3, 28, 'ITEM_SHELL_BELL' if False else 'ITEM_PEARL')
    return e, s, 'DEWFORD BEACH SQUARE\\nWaves, sand and battles.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def verdanturf():
    e = Extension('VerdanturfTown', style('mauville'), down=14, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Verdanturf', 'Verdanturf Town', e)
    e.reopen(3, 18, 4, 2, 'g')
    border(a, 0, 20, W, H)
    a.rect(13, 20, 5, 2, 'T')
    s.plaza(4, 23, 12, 5)
    a.snap_trees(ground='g')
    a.autotile3('P', PLAZA)
    flowers(a, 0x004, 10, 2, 20, 18, 32, seed=7)
    s.board(4, 22)
    s.guide(7, 23, 'OBJ_EVENT_GFX_WOMAN_2')
    s.matchmaker(12, 23, 'OBJ_EVENT_GFX_MAN_3')
    s.local(15, 29, 'OBJ_EVENT_GFX_GIRL_2',
            'The air here is so clean that sick\\n'
            'Pokémon come to rest.\\p'
            'Gentle battles are good for them too,\\n'
            'Wally told me so!', move='MOVEMENT_TYPE_FACE_LEFT')
    s.hidden(2, 29, 'ITEM_HEAL_POWDER')
    return e, s, 'VERDANTURF MEADOW\\nFresh air for every trainer.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def fallarbor():
    e = Extension('FallarborTown', style('fallarbor'), down=14, fill='r')
    a, W, H = e.area, e.w, e.h
    s = Square('Fallarbor', 'Fallarbor Town', e)
    border(a, 0, 20, W, H, c='#')
    a.rect(14, 20, 6, 3, '#')
    a.rect(0, 20, 4, 3, '#')
    s.plaza(4, 23, 12, 5, 's')
    s.centre()
    a.autotile3('P', PLAZA)
    s.board(4, 22)
    s.guide(7, 23, 'OBJ_EVENT_GFX_PICNICKER')
    s.matchmaker(12, 23, 'OBJ_EVENT_GFX_HIKER')
    s.local(14, 29, 'OBJ_EVENT_GFX_SCIENTIST_1',
            'Professor Cozmo says ash falls here\\n'
            'like snow.\\p'
            'Trainers sweep the square every\\n'
            'morning before the battles start!', move='MOVEMENT_TYPE_FACE_LEFT')
    s.replace = {0x36B: 0x279, 0x36E: 0x279, 0x2A0: 0x26B, 0x2A1: 0x26C, 0x050: 0x270, 0x243: 0x270}
    s.hidden(3, 30, 'ITEM_SOFT_SAND')
    return e, s, 'FALLARBOR ASH SQUARE\\nWhere the ash meets the crowd.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def lavaridge():
    e = Extension('LavaridgeTown', style('volcanic'), down=14, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Lavaridge', 'Lavaridge Town', e)
    e.reopen(7, 18, 6, 2, 'g')
    border(a, 0, 20, W, H, c='#')
    e.copy_old(0, 4, 2, 12, 0, 20, c='#')
    e.copy_old(0, 18, 20, 2, 0, H - 2, c='#')
    s.plaza(4, 23, 12, 5, 's')
    s.centre()
    a.autotile3('P', PLAZA)
    s.board(4, 22)
    s.guide(7, 23, 'OBJ_EVENT_GFX_EXPERT_F')
    s.matchmaker(12, 23, 'OBJ_EVENT_GFX_COOLTRAINER_M')
    s.local(15, 29, 'OBJ_EVENT_GFX_OLD_WOMAN',
            'After a soak in the hot sand, there\\n'
            'is nothing like a good battle.\\p'
            'Flannery always says the heat brings\\n'
            'out a trainer\'s fire!', move='MOVEMENT_TYPE_FACE_LEFT')
    s.replace = {0x2AA: 0x001}
    s.hidden(3, 29, 'ITEM_CHARCOAL')
    return e, s, 'LAVARIDGE HOT SQUARE\\nWarm sand, warm hearts.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def fortree():
    e = Extension('FortreeCity', style('jungle'), down=14, fill='.')
    a, W, H = e.area, e.w, e.h
    s = Square('Fortree', 'Fortree City', e)
    border(a, 0, 20, W, H, c='T', t=4)
    a.rect(4, 20, 26, 3, 'T')
    a.rect(37, 20, 3, 3, 'T')
    s.plaza(14, 24, 16, 6, '.')
    a.snap_trees()
    a.autotile3('P', PLAZA)
    s.board(14, 23)
    s.guide(18, 24, 'OBJ_EVENT_GFX_BIRD_KEEPER' if False else 'OBJ_EVENT_GFX_WOMAN_4')
    s.matchmaker(25, 24, 'OBJ_EVENT_GFX_CAMPER')
    s.local(9, 27, 'OBJ_EVENT_GFX_BOY_2',
            'Everyone lives up in the trees, so the\\n'
            'ground is the only place big enough\\l'
            'for a crowd!\\p'
            'Winona flies over to watch sometimes.', move='MOVEMENT_TYPE_FACE_RIGHT')
    s.hidden(33, 29, 'ITEM_SHARP_BEAK')
    return e, s, 'FORTREE CLEARING\\nThe one place on the ground.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


def mauville():
    """Mauville is the crossroads of Hoenn, so its square goes in the middle:
    the city's south edge (and Route 110's road) moves 16 rows down and a
    new district fills the gap."""
    e = Extension('MauvilleCity', style('mauville'), down=16, fill='g')
    a, W, H = e.area, e.w, e.h
    s = Square('Mauville', 'Mauville City', e)
    e.copy_old(0, 18, 40, 2, 0, H - 2, c=None)
    e.move_old_events(0, 18, 40, 2, 0, 16)
    e.reopen(0, 18, 40, 2, 'g')
    a.rect(0, 18, 14, 16, 'T')
    a.rect(14, 18, 1, 16, 'T')
    a.rect(15, 18, 3, 16, '.')
    a.rect(34, 18, 6, 16, '~')
    a.rect(32, 18, 2, 16, 'T')
    a.rect(18, 32, 14, 2, 'T')
    a.rect(2, 22, 8, 6, 'g')
    a.rect(9, 24, 6, 2, 'g')
    s.plaza(19, 21, 12, 9)
    a.rect(18, 24, 1, 3, '.')
    a.snap_trees(ground='g')
    a.autotile3('P', PLAZA)
    flowers(a, 0x004, 8, 2, 20, 12, 30, seed=9)
    s.board(19, 20)
    s.guide(22, 21, 'OBJ_EVENT_GFX_WOMAN_2')
    s.matchmaker(27, 21, 'OBJ_EVENT_GFX_COOLTRAINER_M')
    s.local(30, 28, 'OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M',
            'Cycling Road, Route 111, Route 117...\\n'
            'every road in Hoenn runs into Mauville.\\p'
            'So this is where everyone meets up!', move='MOVEMENT_TYPE_FACE_LEFT')
    s.local(21, 28, 'OBJ_EVENT_GFX_SCHOOL_KID_M',
            'Wattson\'s gym is right up the road.\\n'
            'Trainers warm up here before they go.', move='MOVEMENT_TYPE_WANDER_AROUND', n=2, rx=2, ry=1)
    s.hidden(5, 25, 'ITEM_CELL_BATTERY')
    return e, s, 'MAUVILLE CROSSROADS SQUARE\\nAll of Hoenn passes through.\\p'\
        'The guide has the HUB and PLAZA. The\\n'\
        'matchmaker finds you a battle or trade.'


# name: (map folder, hidden item flag number, builder). Never renumber flags.
TOWNS = {
    'littleroot': ('LittlerootTown', 0, littleroot),
    'oldale': ('OldaleTown', 1, oldale),
    'petalburg': ('PetalburgCity', 2, petalburg),
    'dewford': ('DewfordTown', 3, dewford),
    'verdanturf': ('VerdanturfTown', 4, verdanturf),
    'mauville': {0x298, 0x299, 0x29A, 0x37D, 0x280, 0x2E5, 0x2FE, 0x2E6, 0x293, 0x294, 0x29B, 0x29C, 0x133, 0x35E, 0x37E, 0x2A3, 0x2A4},
    'fallarbor': ('FallarborTown', 5, fallarbor),
    'lavaridge': ('LavaridgeTown', 6, lavaridge),
    'fortree': ('FortreeCity', 7, fortree),
    'mauville': ('MauvilleCity', 8, mauville),
}


def main():
    args = sys.argv[1:]
    preview = None
    if '--preview' in args:
        preview = args[args.index('--preview') + 1]
        del args[args.index('--preview'):args.index('--preview') + 2]
        os.makedirs(preview, exist_ok=True)
    names = args or list(TOWNS)
    scripts, flags = [COMMON], []
    for name, (folder, flag_no, build) in TOWNS.items():
        if name not in names:
            continue
        restore(folder)
        e, s, board = build()
        b = s.finish(e.blocks())
        bad = check_reachable(e, b)
        if bad:
            print(f'{folder}: can\'t reach {bad}')
        if preview:
            e.preview(f'{preview}/{name}.png', b)
            restore_after = True
        else:
            e.write(b)
            print(f'{folder}: {e.ow}x{e.oh} -> {e.w}x{e.h}')
        scripts.append(s.script(board))
        flags.append((s.key, flag_no))
    if preview or names != list(TOWNS):
        return
    open(SCRIPTS, 'w').write('\n'.join(scripts))
    with open(FLAGS_H, 'w') as f:
        f.write('#ifndef GUARD_CONSTANTS_MAP_UPGRADE_H\n#define GUARD_CONSTANTS_MAP_UPGRADE_H\n\n'
                '// Map upgrade flags (tools/map_upgrade, docs/map_upgrade.md). They live in the\n'
                "// extended save, in a block of Solmara's flag space that Solmara doesn't use.\n"
                '#define MAPUP_FLAGS_START  (SOLMARA_FLAGS_START + 1536)\n'
                '#define MAPUP_FLAG(n)      (MAPUP_FLAGS_START + (n))\n\n')
        for k, n in flags:
            f.write(f'#define FLAG_SQUARE_{k.upper()}_HIDDEN MAPUP_FLAG({n})\n')
        f.write('\n#endif // GUARD_CONSTANTS_MAP_UPGRADE_H\n')


if __name__ == '__main__':
    main()
