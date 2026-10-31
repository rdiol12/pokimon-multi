"""Cobblerock: a mining town in the hills near Rustboro, for after the first
badge. Mine-cart rails run from the mine into the ore yard. The assayer
wants three ore samples chipped from veins deep in Cobblerock Mine (1F and
B1F); bring them and the foreman clears the boulder from the old shaft,
where a Carbink sleeps among the crystals.

New art (rails, carts, crates, ore, timber supports, lamps) is drawn in
tools/areas/mine_art.py; the town uses it through the bamboo tileset
(Rustboro's tiles), the mine through gTileset_Mine (the cave's tiles).

    python3 tools/areas/bamboo_art.py && python3 tools/areas/mine_art.py   # when the art changes
    python3 tools/areas/cobblerock.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import woods, mine, TREES, MINE_LADDER_DOWN, MINE_LADDER_UP

AREA_NO = 12
TOWN, M1, M2, SHAFT = 'Cobblerock', 'CobblerockMine_1F', 'CobblerockMine_B1F', 'CobblerockMine_OldShaft'
ASSAY, REST = 'Cobblerock_AssayOffice', 'Cobblerock_MinersRest'
MAPSEC = 'MAPSEC_COBBLEROCK'
HERE = os.path.dirname(__file__)
TOWN_ART = json.load(open(os.path.join(HERE, 'bamboo_tiles.json')))
MINE_ART = json.load(open(os.path.join(HERE, 'mine_tiles.json')))

register_constants(TOWN, AREA_NO, flags=[
    'COBBLE_ORE_1', 'COBBLE_ORE_2', 'COBBLE_ORE_3', 'COBBLE_ASSAY_DONE', 'COBBLE_CARBINK_DONE',
    'ITEM_COBBLE_REPEL', 'ITEM_COBBLE_ESCAPE_ROPE', 'ITEM_COBBLE_HARD_STONE', 'ITEM_COBBLE_EVERSTONE',
    'ITEM_COBBLE_SUPER_POTION', 'ITEM_COBBLE_TM_DIG', 'ITEM_COBBLE_NUGGET', 'ITEM_COBBLE_GREAT_BALL',
    'HIDDEN_ITEM_COBBLE_HEART_SCALE', 'HIDDEN_ITEM_COBBLE_STAR_PIECE', 'HIDDEN_ITEM_COBBLE_PEARL',
    'COBBLE_FOREMAN_TALKED',
], trainers=['COBBLE_GRANT', 'COBBLE_RHODA', 'COBBLE_DOUG', 'COBBLE_TODD'])

register_trainers(TOWN, '''
=== AREA_TRAINER_COBBLE_GRANT ===
Name: GRANT
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Geodude
Level: 13

Roggenrola
Level: 14

=== AREA_TRAINER_COBBLE_RHODA ===
Name: RHODA
Class: Picnicker
Pic: Picnicker
Gender: Female
Music: Girl
Double Battle: No
AI: Basic Trainer

Drilbur
Level: 14

Whismur
Level: 13

=== AREA_TRAINER_COBBLE_DOUG ===
Name: DOUG
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Nosepass
Level: 15

Rolycoly
Level: 15

=== AREA_TRAINER_COBBLE_TODD ===
Name: TODD
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Baltoy
Level: 15

Onix
Level: 16
''')


def placer(art):
    mt = art['objects']

    def place(a, name, x, y, ground_char='o', elevation=3):
        walk = {tuple(rc) for rc in art['walkable'][name]}
        for r, row in enumerate(mt[name]):
            for c, m in enumerate(row):
                if m >= 0:
                    a.tile(x + c, y + r, m, collision=0 if (r, c) in walk else 1, elevation=elevation, c=ground_char)
    return place, lambda name: mt[name][0][0]


place_t, one_t = placer(TOWN_ART)
place_m, one_m = placer(MINE_ART)


def rails(a, place, pts):
    """Track along a polyline of (x, y) cells joined by straight runs, with
    curves at the corners."""
    cells = []
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        n = max(abs(x1 - x0), abs(y1 - y0))
        cells += [(x0 + (x1 - x0) * i // n, y0 + (y1 - y0) * i // n) for i in range(n)]
    cells.append(pts[-1])
    for i, (x, y) in enumerate(cells):
        sides = set()
        nb = [c for c in (cells[i - 1] if i else None, cells[i + 1] if i + 1 < len(cells) else None) if c]
        for nx, ny in nb:
            sides.add('n' if ny < y else 's' if ny > y else 'w' if nx < x else 'e')
        if len(sides) == 1:
            sides.add({'n': 's', 's': 'n', 'e': 'w', 'w': 'e'}[next(iter(sides))])
        if sides == {'e', 'w'}:
            name = 'rail_h'
        elif sides == {'n', 's'}:
            name = 'rail_v'
        else:
            name = 'rail_' + ''.join(d for d in 'ns' if d in sides) + ''.join(d for d in 'ew' if d in sides)
        place(a, name, x, y, ground_char='.')


def ore_veins(a, place, veins):
    for i, x, y in veins:
        place(a, 'ore_vein', x, y)
        a.sign(x, y, f'{a.name}_EventScript_Ore{i}')


def ore_scripts(name, veins):
    out = f'{name}_OnLoad:\n' + ''.join(f'\tcall_if_set FLAG_COBBLE_ORE_{i}, {name}_EventScript_Mined{i}\n' for i, _, _ in veins)
    out += '\tend\n\n'
    for i, x, y in veins:
        out += rf'''{name}_EventScript_Mined{i}::
	setmetatile {x}, {y}, {MINE_ART['objects']['ore_vein_mined'][0][0]}, TRUE
	return

{name}_EventScript_Ore{i}::
	lockall
	goto_if_set FLAG_COBBLE_ORE_{i}, {name}_EventScript_OreDone
	msgbox Cobblerock_Text_OreAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {name}_EventScript_OreLeave
	playse SE_M_ROCK_THROW
	setmetatile {x}, {y}, {MINE_ART['objects']['ore_vein_mined'][0][0]}, TRUE
	special DrawWholeMapView
	setflag FLAG_COBBLE_ORE_{i}
	msgbox Cobblerock_Text_OreGot, MSGBOX_DEFAULT
	releaseall
	end

'''
    out += rf'''{name}_EventScript_OreDone::
	msgbox Cobblerock_Text_OreDone, MSGBOX_DEFAULT
	releaseall
	end

{name}_EventScript_OreLeave::
	releaseall
	end

'''
    return out


def south_exit(a, x, y):
    """Granite Cave's way out: an arrow on the floor at the bottom edge."""
    a.tile(x, y, 0x207, collision=0, elevation=3, c='o')
    for dx, m in ((-1, 0x258), (0, 0x259), (1, 0x25A)):
        a.tile(x + dx, y + 1, m, collision=1, elevation=0, c='o')


def trainer_script(name, key, intro, defeat, post):
    return rf'''{name}_EventScript_{key.title()}::
	trainerbattle_single TRAINER_COBBLE_{key}, {name}_Text_{key.title()}Intro, {name}_Text_{key.title()}Defeat
	msgbox {name}_Text_{key.title()}Post, MSGBOX_AUTOCLOSE
	end

{name}_Text_{key.title()}Intro:
{intro}
{name}_Text_{key.title()}Defeat:
{defeat}
{name}_Text_{key.title()}Post:
{post}
'''


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


ENC_MINE = dict(land=[('ZUBAT', 12, 14), ('GEODUDE', 12, 14), ('ROGGENROLA', 12, 14), ('ROLYCOLY', 12, 14),
                      ('WHISMUR', 12, 14), ('DIGLETT', 13, 15), ('DRILBUR', 13, 15), ('NOSEPASS', 13, 15),
                      ('MAKUHITA', 13, 15), ('ARON', 14, 15), ('ONIX', 14, 16), ('SABLEYE', 15, 16)])

# ---------------------------------------------------------------- the town
st = copy.copy(woods())
st.secondary = 'gTileset_Bamboo'
st.exclude = set(st.exclude) | {0x122, 0x333, 0x334, 0x344, 0x346, 0x347, 0x34C, 0x34D, 0x34E, 0x34F, 0x350, 0x351}
W, H = 48, 40
a = Area(TOWN, W, H, st, fill='.')
a.rect(0, 0, 8, H, 'T')
a.rect(40, 0, 8, H, 'T')
a.rect(0, 36, 22, 4, 'T')
a.rect(28, 36, 20, 4, 'T')
a.ellipse(36, 31, 4, 3, '"')
a.ellipse(10, 30, 3, 3, '"')
a.snap_trees()
MTN = (7, 0)                                     # Route 116's mountain, 34x16
a.stamp('LAYOUT_ROUTE116', 42, 0, 34, 16, *MTN, skip=TREES | {0x068, 0x069, 0x33C, 0x1F0})
MINE_DOOR = (MTN[0] + 5, MTN[1] + 8)             # Rusturf's west entrance, now the mine
SHAFT_DOOR = (MTN[0] + 23, MTN[1] + 10)          # the east one, the old shaft
place_t(a, 'ore_pile', 15, 20)
place_t(a, 'ore_pile', 18, 22)
for x, y in ((14, 23), (15, 23), (21, 19), (20, 25)):
    place_t(a, 'crate', x, y)
place_t(a, 'mine_lamp', 11, 9)
place_t(a, 'mine_lamp', 21, 22)
rails(a, place_t, [(12, 9), (12, 17), (17, 17), (17, 19), (24, 19)])
place_t(a, 'cart', 24, 19)
place_t(a, 'cart_empty', 12, 13)
HOUSES = [(26, 22), (32, 27)]                    # the assay office and the miners' rest
for x, y in HOUSES:
    a.stamp('LAYOUT_ROUTE116', 36, 5, 5, 4, x, y)
place_t(a, 'lantern', 25, 24)
place_t(a, 'lantern', 31, 29)
ARRIVE = (24, 38)
a.rect(22, 36, 6, 4, '.')

a.warp(*MINE_DOOR, 'MAP_COBBLEROCK_MINE_1F', 0)
a.warp(*SHAFT_DOOR, 'MAP_COBBLEROCK_MINE_OLD_SHAFT', 0)
a.warp(HOUSES[0][0] + 2, HOUSES[0][1] + 3, 'MAP_COBBLEROCK_ASSAY_OFFICE', 0)
a.warp(HOUSES[1][0] + 2, HOUSES[1][1] + 3, 'MAP_COBBLEROCK_MINERS_REST', 0)
a.obj('OBJ_EVENT_GFX_HIKER', 25, 38, f'{TOWN}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(23, 34, f'{TOWN}_EventScript_Sign')
a.sign(MINE_DOOR[0] + 1, MINE_DOOR[1] + 1, f'{TOWN}_EventScript_MineSign')
a.obj('OBJ_EVENT_GFX_PUSHABLE_BOULDER', *SHAFT_DOOR, f'{TOWN}_EventScript_Boulder', 'MOVEMENT_TYPE_LOOK_AROUND',
      'FLAG_COBBLE_ASSAY_DONE')
a.obj('OBJ_EVENT_GFX_BLACK_BELT', 14, 10, f'{TOWN}_EventScript_Foreman', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_MAN_4', 20, 21, f'{TOWN}_EventScript_Merchant', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_LITTLE_BOY', 28, 31, f'{TOWN}_EventScript_Boy', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_OLD_MAN', 33, 20, f'{TOWN}_EventScript_OldMiner', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SPECIES(ROLYCOLY)', 16, 25, f'{TOWN}_EventScript_Rolycoly', 'MOVEMENT_TYPE_WANDER_AROUND',
      rx=1, ry=1)
a.trainer('OBJ_EVENT_GFX_PICNICKER', 10, 26, f'{TOWN}_EventScript_Rhoda', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.item('ITEM_REPEL', 37, 34, 'FLAG_ITEM_COBBLE_REPEL')
a.item('ITEM_GREAT_BALL', 9, 20, 'FLAG_ITEM_COBBLE_GREAT_BALL')
a.hidden(19, 26, 'ITEM_HEART_SCALE', 'FLAG_HIDDEN_ITEM_COBBLE_HEART_SCALE')

a.write(map_id='MAP_COBBLEROCK', layout_id='LAYOUT_COBBLEROCK', music='MUS_ROUTE104', mapsec=MAPSEC,
        mapsec_name='COBBLEROCK', map_type='MAP_TYPE_TOWN', after='WhisperstemValley_Teahouse', scripts=rf'''
{TOWN}_MapScripts::
	.byte 0

{TOWN}_EventScript_Guide::
	lock
	faceplayer
	msgbox {TOWN}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {TOWN}_EventScript_GuideStay
	msgbox {TOWN}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_RUSTBORO_CITY, 13, 41
	waitstate
	release
	end

{TOWN}_EventScript_GuideStay::
	msgbox {TOWN}_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

{TOWN}_EventScript_Sign::
	msgbox {TOWN}_Text_Sign, MSGBOX_SIGN
	end

{TOWN}_EventScript_MineSign::
	msgbox {TOWN}_Text_MineSign, MSGBOX_SIGN
	end

{TOWN}_EventScript_Boulder::
	msgbox {TOWN}_Text_Boulder, MSGBOX_SIGN
	end

{TOWN}_EventScript_Foreman::
	lock
	faceplayer
	goto_if_set FLAG_COBBLE_ASSAY_DONE, {TOWN}_EventScript_ForemanDone
	msgbox {TOWN}_Text_Foreman, MSGBOX_DEFAULT
	release
	end

{TOWN}_EventScript_ForemanDone::
	msgbox {TOWN}_Text_ForemanDone, MSGBOX_DEFAULT
	release
	end

{TOWN}_EventScript_Merchant::
	lock
	faceplayer
	message {TOWN}_Text_Merchant
	waitmessage
	pokemart {TOWN}_Mart
	msgbox {TOWN}_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
{TOWN}_Mart:
	.2byte ITEM_ESCAPE_ROPE
	.2byte ITEM_REPEL
	.2byte ITEM_POTION
	.2byte ITEM_POKE_BALL
	.2byte ITEM_HARD_STONE
	pokemartlistend

{TOWN}_EventScript_Boy::
	msgbox {TOWN}_Text_Boy, MSGBOX_NPC
	end

{TOWN}_EventScript_OldMiner::
	msgbox {TOWN}_Text_OldMiner, MSGBOX_NPC
	end

{TOWN}_EventScript_Rolycoly::
	lock
	faceplayer
	playmoncry SPECIES_ROLYCOLY, CRY_MODE_NORMAL
	msgbox {TOWN}_Text_Rolycoly, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{trainer_script(TOWN, 'RHODA', text("Cobblerock's hills are perfect for", "a picnic. And a battle!"),
                text("Oof! I picked the wrong hill."), text("My DRILBUR dug a hole right", "through our picnic blanket."))}
{TOWN}_Text_GuideBack:
{text("Heading back down to RUSTBORO?")}
{TOWN}_Text_GuideLead:
{text("Watch your step on the way down!")}
{TOWN}_Text_GuideStay:
{text("Take your time. Mind the carts!")}
{TOWN}_Text_Sign:
{text("COBBLEROCK", "The town that digs deep.", "", "COBBLEROCK MINE is to the north.", "Visitors, please keep off the rails.")}
Cobblerock_Text_OreAsk:
{text("A vein of ore sparkles red and", "blue in the rock.", "", "Chip off a sample for the", "ASSAYER?")}
Cobblerock_Text_OreGot:
{text("{PLAYER} chipped off a sample of", "the ore!")}
Cobblerock_Text_OreDone:
{text("The vein has been chipped. Only", "plain rock is left.")}
{TOWN}_Text_MineSign:
{text("COBBLEROCK MINE", "Miners and trainers only. Mind", "the carts!")}
{TOWN}_Text_Boulder:
{text("A huge boulder blocks the old shaft.", "It won't budge an inch.")}
{TOWN}_Text_Foreman:
{text("I'm the FOREMAN here. The old shaft", "over there caved in years ago.", "", "Nobody goes in now… but the", "ASSAYER says the best ore in", "Cobblerock is down there.")}
{TOWN}_Text_ForemanDone:
{text("We moved the boulder off the old", "shaft for you. Something sparkles", "in there… be careful!")}
{TOWN}_Text_Merchant:
{text("Supplies for the mine! Can't go", "down without a few ESCAPE ROPES.")}
{TOWN}_Text_MerchantBye:
{text("Come back up safe!")}
{TOWN}_Text_Boy:
{text("My dad drives the ore carts! The", "rails go all the way into the mine.")}
{TOWN}_Text_OldMiner:
{text("I dug in that mine for fifty years.", "", "The veins with the red and blue", "sparkles are the good ones. Chip", "a little off, and the ASSAYER can", "tell you what's in it.")}
{TOWN}_Text_Rolycoly:
{text("ROLYCOLY is rolling around the ore", "yard, looking for coal to munch.")}
''')
print('ok', TOWN)

register_wild('MAP_COBBLEROCK', 'gCobblerock',
              land=[('ZIGZAGOON', 11, 13), ('TAILLOW', 11, 13), ('WHISMUR', 11, 13), ('SKWOVET', 11, 13),
                    ('ROOKIDEE', 11, 13), ('NINCADA', 11, 13), ('GEODUDE', 12, 13), ('ROGGENROLA', 12, 13),
                    ('ARON', 12, 13), ('ROLYCOLY', 12, 13), ('DRILBUR', 13, 14), ('NOSEPASS', 13, 14)])

# ---------------------------------------------------------------- the mine, 1F
mst = mine()
m = Area(M1, 40, 30, mst, fill='#')
m.rect(19, 20, 3, 9, '.')                        # the way in
m.ellipse(20, 14, 10, 6.5, '.')                  # the main gallery
m.line([(11, 14), (6, 14), (6, 20)], 3, '.')     # west drift
m.ellipse(6, 21, 4, 4, '.')
m.line([(29, 15), (34, 15), (34, 21)], 3, '.')   # east drift
m.ellipse(34, 22, 4.5, 4, '.')
m.rect(19, 4, 3, 5, '.')                         # north drift
m.rect(14, 1, 13, 4, '.')                        # the ladder room
south_exit(m, 20, 28)
place_m(m, 'support', 19, 23)
place_m(m, 'support', 19, 5)
rails(m, place_m, [(20, 19), (20, 10), (27, 10)])
place_m(m, 'cart', 27, 10)
rails(m, place_m, [(4, 23), (8, 23)])
place_m(m, 'cart_empty', 3, 23)
for x, y in ((14, 2), (15, 2), (37, 20), (25, 18)):
    place_m(m, 'crate', x, y)
place_m(m, 'mine_lamp', 13, 11)
place_m(m, 'mine_lamp', 27, 13)
place_m(m, 'ore_pile', 30, 24)
m.tile(24, 2, MINE_LADDER_DOWN, c='o')
VEINS_1 = [(1, 3, 19)]
ore_veins(m, place_m, VEINS_1)
m.warp(20, 28, 'MAP_COBBLEROCK', 0)
m.warp(24, 2, 'MAP_COBBLEROCK_MINE_B1F', 0)
m.trainer('OBJ_EVENT_GFX_HIKER', 16, 12, f'{M1}_EventScript_Grant', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
m.obj('OBJ_EVENT_GFX_MAN_4', 33, 19, f'{M1}_EventScript_Miner', 'MOVEMENT_TYPE_FACE_DOWN')
m.item('ITEM_ESCAPE_ROPE', 8, 20, 'FLAG_ITEM_COBBLE_ESCAPE_ROPE')
m.item('ITEM_HARD_STONE', 36, 24, 'FLAG_ITEM_COBBLE_HARD_STONE')
m.hidden(25, 1, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_COBBLE_PEARL')
m.write(map_id='MAP_COBBLEROCK_MINE_1F', layout_id='LAYOUT_COBBLEROCK_MINE_1F', music='MUS_PETALBURG_WOODS', mapsec=MAPSEC,
        mapsec_name='COBBLEROCK', map_type='MAP_TYPE_UNDERGROUND', after=TOWN, allow_cycling=False,
        allow_escaping=True, battle_scene='MAP_BATTLE_SCENE_NORMAL', scripts=rf'''
{M1}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {M1}_OnLoad
	.byte 0

{ore_scripts(M1, VEINS_1)}
{M1}_EventScript_Miner::
	msgbox {M1}_Text_Miner, MSGBOX_NPC
	end

{trainer_script(M1, 'GRANT', text("You're no miner! But you look like", "you could dig up a battle."),
                text("Rock bottom…"), text("The ladder down is in the room", "at the end of the north drift."))}
{M1}_Text_Miner:
{text("Most of the good veins on this floor", "are dug out. Try the west drift,", "past the old cart.")}
''')
print('ok', M1)
register_wild('MAP_COBBLEROCK_MINE_1F', 'gCobblerockMine1F', **ENC_MINE, land_rate=10)

# ---------------------------------------------------------------- the mine, B1F
b = Area(M2, 40, 30, mst, fill='#')
b.rect(20, 2, 6, 5, '.')                          # under the ladder
b.line([(22, 6), (22, 12)], 3, '.')
b.ellipse(20, 16, 12, 6, '.')                     # the deep gallery
b.line([(9, 16), (4, 16), (4, 26)], 3, '.')
b.ellipse(7, 25, 5, 3.5, '.')
b.line([(31, 17), (35, 17), (35, 26)], 3, '.')
b.ellipse(32, 26, 5, 3, '.')
b.ellipse(10, 6, 5, 3, '.')                       # a dead-end pocket
b.line([(14, 6), (20, 6)], 2, '.')
b.tile(23, 3, MINE_LADDER_UP, c='o')
place_m(b, 'support', 21, 8)
rails(b, place_m, [(10, 14), (30, 14), (30, 19)])
place_m(b, 'cart', 10, 13)
place_m(b, 'cart_empty', 30, 20)
for x, y in ((14, 18), (15, 18), (26, 19), (11, 27)):
    place_m(b, 'crate', x, y)
place_m(b, 'mine_lamp', 17, 11)
place_m(b, 'mine_lamp', 24, 11)
place_m(b, 'ore_pile', 3, 24)
VEINS_2 = [(2, 8, 4), (3, 36, 27)]
ore_veins(b, place_m, VEINS_2)
b.warp(23, 3, 'MAP_COBBLEROCK_MINE_1F', 1)
b.trainer('OBJ_EVENT_GFX_HIKER', 20, 17, f'{M2}_EventScript_Doug', 4, 'MOVEMENT_TYPE_FACE_UP')
b.trainer('OBJ_EVENT_GFX_HIKER', 33, 23, f'{M2}_EventScript_Todd', 3, 'MOVEMENT_TYPE_FACE_LEFT')
b.item('ITEM_EVERSTONE', 12, 4, 'FLAG_ITEM_COBBLE_EVERSTONE')
b.item('ITEM_TM_DIG', 4, 27, 'FLAG_ITEM_COBBLE_TM_DIG')
b.item('ITEM_SUPER_POTION', 28, 27, 'FLAG_ITEM_COBBLE_SUPER_POTION')
b.write(map_id='MAP_COBBLEROCK_MINE_B1F', layout_id='LAYOUT_COBBLEROCK_MINE_B1F', music='MUS_PETALBURG_WOODS', mapsec=MAPSEC,
        mapsec_name='COBBLEROCK', map_type='MAP_TYPE_UNDERGROUND', after=M1, allow_cycling=False, allow_escaping=True,
        show_name=False, scripts=rf'''
{M2}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {M2}_OnLoad
	.byte 0

{ore_scripts(M2, VEINS_2)}
{trainer_script(M2, 'DOUG', text("Deep down here, only the tough", "survive! Show me you're tough!"),
                text("You're tougher than granite!"), text("I heard there's a vein in the far", "west pocket. Mind the dead end."))}
{trainer_script(M2, 'TODD', text("These tunnels are older than the", "town! Who knows what's buried here?"),
                text("I dug up a loss."), text("The ancients dug here first. Their", "old shaft is sealed up, outside."))}
''')
print('ok', M2)
register_wild('MAP_COBBLEROCK_MINE_B1F', 'gCobblerockMineB1F', **ENC_MINE, land_rate=10)

# ---------------------------------------------------------------- the old shaft (behind the boulder)
s = Area(SHAFT, 26, 20, mst, fill='#')
s.rect(11, 12, 3, 7, '.')
s.ellipse(13, 8, 9, 5, '.')
s.ellipse(5, 13, 3, 3, '.')
s.line([(6, 12), (10, 10)], 2, '.')
south_exit(s, 12, 18)
place_m(s, 'support', 11, 14)
rails(s, place_m, [(18, 12), (18, 6)])
place_m(s, 'cart_empty', 18, 5)
place_m(s, 'ore_pile', 6, 6)
place_m(s, 'ore_pile', 19, 9)
place_m(s, 'mine_lamp', 8, 9)
s.warp(12, 18, 'MAP_COBBLEROCK', 1)
s.obj('OBJ_EVENT_GFX_SPECIES(CARBINK)', 13, 5, f'{SHAFT}_EventScript_Carbink', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_COBBLE_CARBINK_DONE')
s.item('ITEM_NUGGET', 4, 13, 'FLAG_ITEM_COBBLE_NUGGET')
s.hidden(21, 7, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_COBBLE_STAR_PIECE')
s.write(map_id='MAP_COBBLEROCK_MINE_OLD_SHAFT', layout_id='LAYOUT_COBBLEROCK_MINE_OLD_SHAFT', music='MUS_PETALBURG_WOODS',
        mapsec=MAPSEC, mapsec_name='COBBLEROCK', map_type='MAP_TYPE_UNDERGROUND', after=M2, allow_cycling=False,
        allow_escaping=True, show_name=False, scripts=rf'''
{SHAFT}_MapScripts::
	.byte 0

{SHAFT}_EventScript_Carbink::
	lock
	faceplayer
	msgbox {SHAFT}_Text_Glitter, MSGBOX_DEFAULT
	setwildbattle SPECIES_CARBINK, 18
	playmoncry SPECIES_CARBINK, CRY_MODE_ENCOUNTER
	msgbox {SHAFT}_Text_Carbink, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {SHAFT}_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {SHAFT}_EventScript_Stays
	setflag FLAG_COBBLE_CARBINK_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {SHAFT}_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {SHAFT}_Text_Gone, MSGBOX_DEFAULT
{SHAFT}_EventScript_End::
	release
	end

{SHAFT}_EventScript_Stays::
	msgbox {SHAFT}_Text_Stays, MSGBOX_DEFAULT
	release
	end

{SHAFT}_Text_Glitter:
{text("Something glitters among the rocks…", "", "It's a POKéMON made of jewels!")}
{SHAFT}_Text_Carbink:
{text("Kiii-iin!")}
{SHAFT}_Text_Gone:
{text("CARBINK burrowed back into the rock.")}
{SHAFT}_Text_Stays:
{text("CARBINK is still glittering quietly", "in the dark.")}
''')
print('ok', SHAFT)

# ---------------------------------------------------------------- the assay office and the miners' rest
interior(ASSAY, 'MAP_COBBLEROCK_ASSAY_OFFICE', 'LAYOUT_ROUTE116_TUNNELERS_REST_HOUSE', [(4, 8), (5, 8)],
         'MAP_COBBLEROCK', 2, mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes',
         objects=[npc('OBJ_EVENT_GFX_SCIENTIST_1', 6, 4, f'{ASSAY}_EventScript_Assayer', 'MOVEMENT_TYPE_FACE_DOWN'),
                  npc('OBJ_EVENT_GFX_WOMAN_5', 2, 6, f'{ASSAY}_EventScript_Clerk', 'MOVEMENT_TYPE_FACE_RIGHT')],
         scripts=rf'''
{ASSAY}_MapScripts::
	.byte 0

{ASSAY}_EventScript_Assayer::
	lock
	faceplayer
	goto_if_set FLAG_COBBLE_ASSAY_DONE, {ASSAY}_EventScript_Done
	goto_if_unset FLAG_COBBLE_ORE_1, {ASSAY}_EventScript_Ask
	goto_if_unset FLAG_COBBLE_ORE_2, {ASSAY}_EventScript_Ask
	goto_if_unset FLAG_COBBLE_ORE_3, {ASSAY}_EventScript_Ask
	msgbox {ASSAY}_Text_Thanks, MSGBOX_DEFAULT
	givemon SPECIES_ROLYCOLY, 15
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {ASSAY}_EventScript_Full
	setflag FLAG_COBBLE_ASSAY_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox {ASSAY}_Text_GotRolycoly, MSGBOX_DEFAULT
	waitfanfare
	msgbox {ASSAY}_Text_Shaft, MSGBOX_DEFAULT
	release
	end

{ASSAY}_EventScript_Ask::
	msgbox {ASSAY}_Text_Ask, MSGBOX_DEFAULT
	release
	end

{ASSAY}_EventScript_Full::
	msgbox {ASSAY}_Text_Full, MSGBOX_DEFAULT
	release
	end

{ASSAY}_EventScript_Done::
	msgbox {ASSAY}_Text_Done, MSGBOX_DEFAULT
	release
	end

{ASSAY}_EventScript_Clerk::
	msgbox {ASSAY}_Text_Clerk, MSGBOX_NPC
	end

{ASSAY}_Text_Ask:
{text("I'm the ASSAYER. I test ore to see", "what the mine is worth.", "", "I need three fresh samples from", "the veins in COBBLEROCK MINE. One", "on 1F, two on B1F. Look for red", "and blue sparkles in the rock!")}
{ASSAY}_Text_Thanks:
{text("Three samples! Let me see…", "", "Iron, copper… and traces of", "crystal! Just like the old shaft.", "", "Here, take this ROLYCOLY. It sniffs", "out coal better than any miner.")}
{ASSAY}_Text_GotRolycoly:
{text("{PLAYER} received a ROLYCOLY!")}
{ASSAY}_Text_Shaft:
{text("I'll tell the FOREMAN to move the", "boulder off the old shaft. With", "crystal like this, it must be worth", "a look!")}
{ASSAY}_Text_Full:
{text("Your party's full. Make some room,", "then come back for your reward.")}
{ASSAY}_Text_Done:
{text("The old shaft is open now. Crystal", "like that only grows where a", "CARBINK lives, you know.")}
{ASSAY}_Text_Clerk:
{text("Every cart of ore that leaves", "COBBLEROCK gets weighed in here.")}
''')
interior(REST, 'MAP_COBBLEROCK_MINERS_REST', 'LAYOUT_ROUTE116_TUNNELERS_REST_HOUSE', [(4, 8), (5, 8)],
         'MAP_COBBLEROCK', 3, mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes',
         objects=[npc('OBJ_EVENT_GFX_HIKER', 6, 5, f'{REST}_EventScript_Miner1', 'MOVEMENT_TYPE_FACE_LEFT'),
                  npc('OBJ_EVENT_GFX_MAN_4', 3, 6, f'{REST}_EventScript_Miner2', 'MOVEMENT_TYPE_FACE_RIGHT'),
                  npc('OBJ_EVENT_GFX_WOMAN_2', 7, 2, f'{REST}_EventScript_Cook', 'MOVEMENT_TYPE_FACE_DOWN')],
         scripts=rf'''
{REST}_MapScripts::
	.byte 0

{REST}_EventScript_Miner1::
	msgbox {REST}_Text_Miner1, MSGBOX_NPC
	end

{REST}_EventScript_Miner2::
	msgbox {REST}_Text_Miner2, MSGBOX_NPC
	end

{REST}_EventScript_Cook::
	lock
	faceplayer
	msgbox {REST}_Text_Cook, MSGBOX_DEFAULT
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox {REST}_Text_CookAfter, MSGBOX_DEFAULT
	release
	end

{REST}_Text_Miner1:
{text("A long day in the mine and a hot", "meal after. That's the miner's life.")}
{REST}_Text_Miner2:
{text("When I was a kid, I swore I saw a", "POKéMON made of jewels in the old", "shaft. Nobody believed me.")}
{REST}_Text_Cook:
{text("You look worn out! Sit down and", "rest a while.")}
{REST}_Text_CookAfter:
{text("There! Good as new. Off you go.")}
''')
print('ok interiors')

# ---------------------------------------------------------------- the way there, from Rustboro City
add_object('RustboroCity', {'graphics_id': 'OBJ_EVENT_GFX_HIKER', 'x': 12, 'y': 41, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'RustboroCity_EventScript_CobblerockGuide', 'flag': '0'})
register_script_file('cobblerock', r'''
@ Cobblerock guide in Rustboro City (tools/areas/cobblerock.py).
RustboroCity_EventScript_CobblerockGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE01_GET, RustboroCity_EventScript_CobblerockNoBadge
	msgbox RustboroCity_Text_CobblerockAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_EventScript_CobblerockNo
	msgbox RustboroCity_Text_CobblerockGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_COBBLEROCK, 24, 38
	waitstate
	release
	end

RustboroCity_EventScript_CobblerockNoBadge::
	msgbox RustboroCity_Text_CobblerockNoBadge, MSGBOX_DEFAULT
	release
	end

RustboroCity_EventScript_CobblerockNo::
	msgbox RustboroCity_Text_CobblerockNo, MSGBOX_DEFAULT
	release
	end

RustboroCity_Text_CobblerockNoBadge:
	.string "I'm a miner from COBBLEROCK, up in\n"
	.string "the hills. The mine's no place for\l"
	.string "beginners, though.\p"
	.string "Beat ROXANNE first, then I'll take\n"
	.string "you up.$"

RustboroCity_Text_CobblerockAsk:
	.string "ROXANNE's badge! You can handle\n"
	.string "the mine.\p"
	.string "Want to come up to COBBLEROCK?$"

RustboroCity_Text_CobblerockGo:
	.string "Up we go!$"

RustboroCity_Text_CobblerockNo:
	.string "Suit yourself. I'll be here.$"
''')
print('ok entrance')
