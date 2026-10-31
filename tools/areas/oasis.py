"""The Mirage Dunes and the Mirage Oasis, for after the eighth badge: a sea
of dunes north of Route 111 where a caravan has lost its way, and the
hidden oasis village beyond the haze.

The dunes shimmer with three oases that aren't there: walk into one and it
melts away, leaving something glinting in the sand. See through all three
and the heat haze across the north pass lifts, showing the real oasis
gate. The oasis has gone dry: its elder says the Spring Stone was carried
down to the sunken court below the dunes, where three stones mark the
sinking sand. Sink through that pit, take the stone from its altar and
bring it to the well: the spring flows, the pond fills, the elder gives
you a Larvesta, and Landorus comes to rest on the mesa above the village.

New art (palms and their mirage twins, mirage ponds, the haze, tents,
market stalls, the well dry and full, the dry pond bed, quicksand pits,
jars, rugs, pillars, the altar, stairs and the oasis gate) is drawn in
tools/areas/oasis_art.py (gTileset_Oasis: Route 111's desert tiles plus
the art).

    python3 tools/areas/oasis_art.py   # the tileset, when the art changes
    python3 tools/areas/oasis.py
"""
import collections, copy, json, sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import desert, POND as POND_SET

AREA_NO = 19
DUNES = 'MirageDunes'
OASIS = 'MirageOasis'
COURT = 'MirageOasis_SunkenCourt'
INN = 'MirageOasis_Inn'
MAPSEC = 'MAPSEC_MIRAGE_OASIS'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'oasis_tiles.json')))
MT = ART['objects']
WATER_BEHAVIORS = {0x10, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17, 0x1A}

register_constants('Mirage', AREA_NO, flags=[
    'MIRAGE_1', 'MIRAGE_2', 'MIRAGE_3', 'MIRAGE_SPRING_STONE', 'MIRAGE_SPRING_DONE', 'MIRAGE_LARVESTA',
    'HIDE_MIRAGE_LANDORUS', 'MIRAGE_LANDORUS_DONE', 'MIRAGE_ELDER_TALKED',
    'ITEM_MIRAGE_ULTRA_BALL', 'ITEM_MIRAGE_MAX_ELIXIR', 'ITEM_MIRAGE_PP_MAX', 'ITEM_MIRAGE_TM_EARTHQUAKE',
    'ITEM_MIRAGE_BIG_NUGGET', 'ITEM_MIRAGE_MAX_REVIVE', 'ITEM_MIRAGE_FULL_RESTORE',
    'HIDDEN_ITEM_MIRAGE_STAR_PIECE', 'HIDDEN_ITEM_MIRAGE_RARE_CANDY', 'HIDDEN_ITEM_MIRAGE_BIG_PEARL',
    'HIDDEN_ITEM_MIRAGE_RARE_BONE', 'HIDDEN_ITEM_MIRAGE_STARDUST',
], trainers=['MIRAGE_RASHID', 'MIRAGE_ZAHRA', 'MIRAGE_DUNSTAN', 'MIRAGE_NEFERA', 'MIRAGE_OMAR'])

register_trainers('Mirage', '''
=== AREA_TRAINER_MIRAGE_RASHID ===
Name: RASHID
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Hippowdon
Level: 49

Krookodile
Level: 50

=== AREA_TRAINER_MIRAGE_ZAHRA ===
Name: ZAHRA
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Cacturne
Level: 49

Flygon
Level: 50

Maractus
Level: 49

=== AREA_TRAINER_MIRAGE_DUNSTAN ===
Name: DUNSTAN
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Sandaconda
Level: 50

Garchomp
Level: 51

=== AREA_TRAINER_MIRAGE_NEFERA ===
Name: NEFERA
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Cofagrigus
Level: 50

Runerigus
Level: 50

Sigilyph
Level: 51

=== AREA_TRAINER_MIRAGE_OMAR ===
Name: OMAR
Class: Expert
Pic: Expert M
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Claydol
Level: 51

Golurk
Level: 52
''')


def place(a, name, x, y, blocked=()):
    """An art object at x, y; cells in blocked stay solid even if the art lets you walk there."""
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    cells = []
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                open_ = (r, c) in walk and (r, c) not in blocked
                a.tile(x + c, y + r, mt, collision=0 if open_ else 1, c='o')
                cells.append((x + c, y + r))
    return cells


def one(name):
    return MT[name][0][0]


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


def trainer_script(m, key, intro, defeat, post):
    label = key.title()
    return rf'''{m}_EventScript_{label}::
	trainerbattle_single TRAINER_MIRAGE_{key}, {m}_Text_{label}Intro, {m}_Text_{label}Defeat
	msgbox {m}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{m}_Text_{label}Intro:
{intro}
{m}_Text_{label}Defeat:
{defeat}
{m}_Text_{label}Post:
{post}
'''


def plain(b, cells):
    """The commonest metatile among cells: the plain sand to put back."""
    return collections.Counter(int(b[y, x]) & 0x3FF for x, y in cells).most_common(1)[0][0]


st = copy.copy(desert())
st.secondary = 'gTileset_Oasis'          # Route 111's tiles, same metatile ids, plus the art

ENC_DUNES = [('KROKOROK', 46, 48), ('SANDSLASH', 46, 48), ('CACTURNE', 46, 48), ('VIBRAVA', 46, 48),
             ('MARACTUS', 47, 48), ('DUGTRIO', 47, 48), ('HIPPOWDON', 47, 49), ('SANDACONDA', 47, 49),
             ('CRUSTLE', 48, 49), ('SIGILYPH', 48, 50), ('KROOKODILE', 49, 50), ('FLYGON', 49, 50)]
ENC_COURT = [('COFAGRIGUS', 47, 49), ('SIGILYPH', 47, 49), ('CLAYDOL', 47, 49), ('KROKOROK', 46, 48),
             ('SANDSLASH', 46, 48), ('CRUSTLE', 47, 49), ('BRONZONG', 48, 49), ('DUSCLOPS', 48, 49),
             ('GOLURK', 48, 50), ('RUNERIGUS', 48, 50), ('HIPPOWDON', 49, 50), ('KROOKODILE', 49, 50)]

# ---------------------------------------------------------------- the dunes
W, H = 56, 52
d = Area(DUNES, W, H, st, fill='#')
d.ellipse(28, 27, 26, 24, 's')
d.rect(24, 44, 8, 8, 's')                        # the caravan trail in from Route 111
d.rect(25, 0, 6, 7, 's')                         # the north pass
for x, y, rx, ry in ((13, 14, 5, 3.5), (43, 40, 5, 3), (15, 34, 3.5, 2.5), (40, 19, 4, 2.5), (31, 29, 3, 2),
                     (21, 22, 3, 2), (8, 40, 3, 2.5), (48, 15, 3, 3)):
    d.ellipse(x, y, rx, ry, '#')
for x, y in ((9, 30), (35, 36), (19, 44), (46, 24), (24, 32), (34, 22)):
    d.stamp('LAYOUT_ROUTE111', 25, 82, 2, 2, x, y)   # boulders

GATE = (25, 0)
gate_cells = place(d, 'gate', 26, 0)
HAZE = [(x, 5) for x in range(25, 31)]
for x, y in HAZE:
    d.tile(x, y, one('haze'), collision=1, c='o')
d.connections = [{'map': 'MAP_MIRAGE_OASIS', 'offset': 4, 'direction': 'up'}]

MIRAGES = [  # pond (3x2) and its two palms
    ((7, 27), (4, 24), (10, 24)),
    ((46, 30), (43, 27), (49, 27)),
    ((36, 11), (33, 8), (39, 8)),
]
mirage_cells = []
for i, (pond, p1, p2) in enumerate(MIRAGES, 1):
    cells = place(d, 'pond_mirage', *pond) + place(d, 'palm_mirage', *p1) + place(d, 'palm_mirage', *p2)
    mirage_cells.append(cells)
    trigger = [(pond[0] + dx, pond[1] + dy) for dx in range(3) for dy in range(2)]
    trigger += [(p[0] + dx, p[1] + 2) for p in (p1, p2) for dx in range(2)]
    for x, y in trigger:
        d.coord(x, y, f'{DUNES}_EventScript_Mirage{i}')

PITS = {'A': (12, 41), 'B': (45, 20), 'C': (22, 10)}
for x, y in PITS.values():
    d.tile(x, y, one('pit'), collision=0, c='o')
for x, y in ((21, 9), (23, 9), (22, 12)):            # the three stones around pit C
    place(d, 'rubble', x, y)

# a court's worth of ruins, half buried in the dunes
for x, y in ((17, 18), (36, 25), (8, 36), (49, 37), (33, 4)):
    place(d, 'pillar_broken', x, y - 1)
for x, y in ((18, 19), (37, 26), (25, 38), (50, 22), (6, 18)):
    place(d, 'rubble', x, y)

# the caravan's camp
place(d, 'tent_red', 30, 39, blocked={(2, 1)})
place(d, 'rug', 27, 42)
place(d, 'jar', 33, 42)
place(d, 'jar', 34, 41)
place(d, 'palm', 20, 37)
d.obj('OBJ_EVENT_GFX_SPECIES(CAMERUPT)', 26, 40, f'{DUNES}_EventScript_Camerupt', 'MOVEMENT_TYPE_FACE_DOWN')
d.obj('OBJ_EVENT_GFX_MAN_4', 29, 43, f'{DUNES}_EventScript_Trader', 'MOVEMENT_TYPE_FACE_LEFT')
ARRIVE = (28, 50)
d.obj('OBJ_EVENT_GFX_HIKER', 27, 50, f'{DUNES}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_RIGHT')
d.sign(29, 47, f'{DUNES}_EventScript_Sign')

d.trainer('OBJ_EVENT_GFX_HIKER', 17, 27, f'{DUNES}_EventScript_Rashid', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
d.trainer('OBJ_EVENT_GFX_COOLTRAINER_F', 40, 34, f'{DUNES}_EventScript_Zahra', 3, 'MOVEMENT_TYPE_FACE_LEFT')
d.trainer('OBJ_EVENT_GFX_MANIAC', 30, 15, f'{DUNES}_EventScript_Dunstan', 3, 'MOVEMENT_TYPE_FACE_DOWN')
d.item('ITEM_ULTRA_BALL', 5, 33, 'FLAG_ITEM_MIRAGE_ULTRA_BALL')
d.item('ITEM_MAX_ELIXIR', 50, 34, 'FLAG_ITEM_MIRAGE_MAX_ELIXIR')
for (pond, _, _), (item, flag) in zip(MIRAGES, (('ITEM_STAR_PIECE', 'STAR_PIECE'), ('ITEM_RARE_CANDY', 'RARE_CANDY'),
                                             ('ITEM_BIG_PEARL', 'BIG_PEARL'))):
    d.hidden(pond[0] + 1, pond[1] + 1, item, f'FLAG_HIDDEN_ITEM_MIRAGE_{flag}')

# the pits' warps: A and B to their courts, C to the altar
d.warp(*PITS['A'], 'MAP_MIRAGE_OASIS_SUNKEN_COURT', 0)
d.warp(*PITS['B'], 'MAP_MIRAGE_OASIS_SUNKEN_COURT', 1)
d.warp(*PITS['C'], 'MAP_MIRAGE_OASIS_SUNKEN_COURT', 2)
# arrival points for the court's stairs (plain sand: stepping on them does nothing)
STAIRS_OUT = [(13, 43), (44, 22), (24, 11)]
for x, y in STAIRS_OUT:
    d.warp(x, y, 'MAP_MIRAGE_OASIS_SUNKEN_COURT', 3)

b = d.blocks()
sand = plain(b, [(x, y) for x in range(20, 36) for y in range(44, 50)])
mirage_restore = ''
for i, cells in enumerate(mirage_cells, 1):
    mirage_restore += f'{DUNES}_EventScript_Gone{i}::\n'
    mirage_restore += ''.join(f'\tsetmetatile {x}, {y}, {sand}, FALSE\n' for x, y in cells)
    mirage_restore += '\treturn\n\n'
haze_lift = f'{DUNES}_EventScript_HazeGone::\n' + ''.join(f'\tsetmetatile {x}, {y}, {sand}, FALSE\n' for x, y in HAZE) + '\treturn\n\n'


def mirage_script(i):
    return rf'''{DUNES}_EventScript_Mirage{i}::
	goto_if_set FLAG_MIRAGE_{i}, {DUNES}_EventScript_MirageNone
	lockall
	playse SE_M_MIST
	call {DUNES}_EventScript_Gone{i}
	special DrawWholeMapView
	waitse
	setflag FLAG_MIRAGE_{i}
	msgbox {DUNES}_Text_Mirage, MSGBOX_DEFAULT
	goto_if_unset FLAG_MIRAGE_1, {DUNES}_EventScript_MirageEnd{i}
	goto_if_unset FLAG_MIRAGE_2, {DUNES}_EventScript_MirageEnd{i}
	goto_if_unset FLAG_MIRAGE_3, {DUNES}_EventScript_MirageEnd{i}
	delay 30
	playse SE_M_GUST
	call {DUNES}_EventScript_HazeGone
	special DrawWholeMapView
	delay 40
	msgbox {DUNES}_Text_HazeLifts, MSGBOX_DEFAULT
{DUNES}_EventScript_MirageEnd{i}::
	releaseall
	end

'''


d.write(blocks=b, map_id='MAP_MIRAGE_DUNES', layout_id='LAYOUT_MIRAGE_DUNES', music='MUS_DESERT', mapsec=MAPSEC,
        mapsec_name='MIRAGE OASIS', weather='WEATHER_SUNNY', after='PrismCaverns_Depths', scripts=rf'''
{DUNES}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {DUNES}_OnLoad
	.byte 0

{DUNES}_OnLoad:
	call_if_set FLAG_MIRAGE_1, {DUNES}_EventScript_Gone1
	call_if_set FLAG_MIRAGE_2, {DUNES}_EventScript_Gone2
	call_if_set FLAG_MIRAGE_3, {DUNES}_EventScript_Gone3
	goto_if_unset FLAG_MIRAGE_1, {DUNES}_EventScript_OnLoadEnd
	goto_if_unset FLAG_MIRAGE_2, {DUNES}_EventScript_OnLoadEnd
	goto_if_unset FLAG_MIRAGE_3, {DUNES}_EventScript_OnLoadEnd
	call {DUNES}_EventScript_HazeGone
{DUNES}_EventScript_OnLoadEnd:
	end

{mirage_restore}{haze_lift}{mirage_script(1)}{mirage_script(2)}{mirage_script(3)}{DUNES}_EventScript_MirageNone::
	end

{DUNES}_EventScript_Guide::
	lock
	faceplayer
	msgbox {DUNES}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {DUNES}_EventScript_GuideStay
	msgbox {DUNES}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE111, 20, 106
	waitstate
	release
	end

{DUNES}_EventScript_GuideStay::
	msgbox {DUNES}_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

{DUNES}_EventScript_Sign::
	msgbox {DUNES}_Text_Sign, MSGBOX_SIGN
	end

{DUNES}_EventScript_Camerupt::
	lock
	faceplayer
	playmoncry SPECIES_CAMERUPT, CRY_MODE_NORMAL
	msgbox {DUNES}_Text_Camerupt, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{DUNES}_EventScript_Trader::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_DONE, {DUNES}_EventScript_TraderDone
	msgbox {DUNES}_Text_Trader, MSGBOX_DEFAULT
	release
	end

{DUNES}_EventScript_TraderDone::
	msgbox {DUNES}_Text_TraderDone, MSGBOX_DEFAULT
	release
	end

{trainer_script(DUNES, 'RASHID', text("Lost in the dunes? Me too! Might", "as well have a battle while we're here."),
                text("Sand in my boots…"), text("Those pools out there? Don't trust", "them. Walk right in and see."))}
{trainer_script(DUNES, 'ZAHRA', text("The heat plays tricks on the eyes.", "My POKéMON don't play tricks!"),
                text("Seeing things…"), text("Once you see through every mirage,", "the haze in the north pass lifts."))}
{trainer_script(DUNES, 'DUNSTAN', text("A real oasis lies beyond the haze.", "I've come to find its ruins!"),
                text("Buried, like the old court."), text("They say a whole court sank beneath", "these dunes. The sand swallows you", "where three stones stand."))}
{DUNES}_Text_Mirage:
{text("The oasis wavered in the heat…", "and melted away into the sand!", "", "It was only a mirage.")}
{DUNES}_Text_HazeLifts:
{text("A hot wind swept across the dunes.", "", "To the north, the haze has lifted.", "A gate stands in the pass!")}
{DUNES}_Text_GuideBack:
{text("I'm the caravan's scout. The dunes", "are no place to get lost.", "", "Head back to ROUTE 111?")}
{DUNES}_Text_GuideLead:
{text("Follow my footprints!")}
{DUNES}_Text_GuideStay:
{text("Keep the sun on your left and you'll", "find your way. Mostly.")}
{DUNES}_Text_Sign:
{text("MIRAGE DUNES", "", "Beware the sinking sand.", "Trust no water you cannot touch.")}
{DUNES}_Text_Camerupt:
{text("CAMERUPT: Grooomph…", "", "It's carrying the caravan's packs.", "It looks very thirsty.")}
{DUNES}_Text_Trader:
{text("We came to trade at the oasis, but", "every pool we find vanishes when", "we get close!", "", "The real one must be north, past", "that shimmering haze.")}
{DUNES}_Text_TraderDone:
{text("The spring is flowing again! Our", "CAMERUPT finally had a long drink.")}
''')
print('ok', DUNES)
register_wild('MAP_MIRAGE_DUNES', 'gMirageDunes', land=ENC_DUNES, land_rate=12)

# ---------------------------------------------------------------- the oasis
W, H = 48, 40
o = Area(OASIS, W, H, st, fill='#')
o.ellipse(24, 21, 22.5, 17.5, 's')
o.rect(21, 33, 6, 7, 's')                         # the way south to the gate
o.rect(18, 1, 12, 6, 's')                         # the mesa top
o.rect(22, 6, 4, 2, 's')
POND = (24, 17, 6.5, 3.8)
o.ellipse(*POND, 'P')
o.autotile3('P', POND_SET)               # General's pond, its grassy rim the oasis' green
PALMS = [(14, 11), (31, 10), (13, 18), (34, 18), (17, 22), (29, 23), (8, 26), (39, 26), (36, 6), (10, 6)]
for x, y in PALMS:
    place(o, 'palm', x, y)
WELL = (25, 25)
place(o, 'well_dry', *WELL)
place(o, 'tent_red', 4, 10, blocked={(2, 1)})
INN_TENT = (39, 11)
place(o, 'tent_blue', *INN_TENT)
place(o, 'tent_red', 41, 18, blocked={(2, 1)})
STALLS = [(6, 30), (11, 30), (16, 30)]
for x, y in STALLS:
    place(o, 'stall', x, y)
place(o, 'rug', 8, 15)
place(o, 'rug', 35, 31)
for x, y in ((3, 14), (8, 23), (37, 30), (44, 22), (20, 30)):
    place(o, 'jar', x, y)

o.connections = [{'map': 'MAP_MIRAGE_DUNES', 'offset': -4, 'direction': 'down'}]
o.warp(INN_TENT[0] + 1, INN_TENT[1] + 2, 'MAP_MIRAGE_OASIS_INN', 0)
o.obj('OBJ_EVENT_GFX_OLD_MAN', 27, 26, f'{OASIS}_EventScript_Elder', 'MOVEMENT_TYPE_FACE_DOWN')
o.obj('OBJ_EVENT_GFX_MAN_4', STALLS[0][0] + 1, STALLS[0][1], f'{OASIS}_EventScript_Merchant', 'MOVEMENT_TYPE_FACE_DOWN')
o.obj('OBJ_EVENT_GFX_WOMAN_2', STALLS[1][0] + 1, STALLS[1][1], f'{OASIS}_EventScript_FruitSeller', 'MOVEMENT_TYPE_FACE_DOWN')
o.obj('OBJ_EVENT_GFX_HEX_MANIAC', STALLS[2][0] + 1, STALLS[2][1], f'{OASIS}_EventScript_Fortune', 'MOVEMENT_TYPE_FACE_DOWN')
o.obj('OBJ_EVENT_GFX_LITTLE_GIRL', 20, 23, f'{OASIS}_EventScript_Girl', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
o.obj('OBJ_EVENT_GFX_FAT_MAN', 33, 14, f'{OASIS}_EventScript_Traveller', 'MOVEMENT_TYPE_FACE_LEFT')
o.obj('OBJ_EVENT_GFX_SPECIES(LANDORUS)', 24, 3, f'{OASIS}_EventScript_Landorus', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_MIRAGE_LANDORUS')
o.trainer('OBJ_EVENT_GFX_EXPERT_M', 22, 8, f'{OASIS}_EventScript_Omar', 2, 'MOVEMENT_TYPE_FACE_DOWN')
o.item('ITEM_PP_MAX', 44, 26, 'FLAG_ITEM_MIRAGE_PP_MAX')
o.sign(23, 34, f'{OASIS}_EventScript_Sign')
o.sign(WELL[0], WELL[1] + 1, f'{OASIS}_EventScript_Well', 'BG_EVENT_PLAYER_FACING_NORTH')
o.sign(WELL[0] + 1, WELL[1] + 1, f'{OASIS}_EventScript_Well', 'BG_EVENT_PLAYER_FACING_NORTH')

ob = o.blocks()
osand = plain(ob, [(x, y) for x in range(20, 28) for y in range(33, 38)])
cx, cy, rx, ry = POND
pond = []
for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
    for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
        if (ob[y, x] & 0x3FF) != osand and ((x + 0.5 - cx - 0.5) / (rx + 1.5)) ** 2 + ((y + 0.5 - cy - 0.5) / (ry + 1.5)) ** 2 <= 1:
            pond.append((x, y, int(ob[y, x])))
dry = one('dry_bed')
for x, y, v in pond:
    ob[y, x] = (3 << 12) | dry             # dry and walkable
refill = f'{OASIS}_EventScript_Refill::\n' + ''.join(
    f'\tsetmetatile {x}, {y}, {v & 0x3FF}, {"TRUE" if (v >> 10) & 3 else "FALSE"}\n' for x, y, v in pond) + '\treturn\n\n'
well_full = MT['well_full']
fill_well = f'{OASIS}_EventScript_WellFull::\n' + ''.join(
    f'\tsetmetatile {WELL[0] + c}, {WELL[1] + r}, {mt}, TRUE\n' for r, row in enumerate(well_full) for c, mt in enumerate(row)) + '\treturn\n\n'

o.write(blocks=ob, map_id='MAP_MIRAGE_OASIS', layout_id='LAYOUT_MIRAGE_OASIS', music='MUS_ROUTE119', mapsec=MAPSEC,
        mapsec_name='MIRAGE OASIS', weather='WEATHER_SUNNY', map_type='MAP_TYPE_TOWN', after=DUNES, scripts=rf'''
{OASIS}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {OASIS}_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, {OASIS}_OnTransition
	.byte 0

{OASIS}_OnLoad:
	call_if_set FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_Refill
	call_if_set FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_WellFull
	end

{OASIS}_OnTransition:
	setflag FLAG_HIDE_MIRAGE_LANDORUS
	goto_if_unset FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_TransitionEnd
	goto_if_set FLAG_MIRAGE_LANDORUS_DONE, {OASIS}_EventScript_TransitionEnd
	clearflag FLAG_HIDE_MIRAGE_LANDORUS
{OASIS}_EventScript_TransitionEnd:
	end

{refill}{fill_well}{OASIS}_EventScript_Sign::
	msgbox {OASIS}_Text_Sign, MSGBOX_SIGN
	end

{OASIS}_EventScript_Well::
	lockall
	goto_if_set FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_WellFlowing
	goto_if_set FLAG_MIRAGE_SPRING_STONE, {OASIS}_EventScript_PlaceStone
	msgbox {OASIS}_Text_WellDry, MSGBOX_DEFAULT
	releaseall
	end

{OASIS}_EventScript_WellFlowing::
	msgbox {OASIS}_Text_WellFlowing, MSGBOX_DEFAULT
	releaseall
	end

{OASIS}_EventScript_PlaceStone::
	msgbox {OASIS}_Text_PlaceStoneAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {OASIS}_EventScript_WellLeave
	msgbox {OASIS}_Text_PlaceStone, MSGBOX_DEFAULT
	closemessage
	playse SE_M_WATERFALL
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	call {OASIS}_EventScript_WellFull
	special DrawWholeMapView
	delay 30
	call {OASIS}_EventScript_Refill
	special DrawWholeMapView
	delay 40
	setflag FLAG_MIRAGE_SPRING_DONE
	msgbox {OASIS}_Text_SpringFlows, MSGBOX_DEFAULT
	releaseall
	end

{OASIS}_EventScript_WellLeave::
	releaseall
	end

{OASIS}_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_LARVESTA, {OASIS}_EventScript_ElderDone
	goto_if_set FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_ElderReward
	goto_if_set FLAG_MIRAGE_SPRING_STONE, {OASIS}_EventScript_ElderStone
	goto_if_set FLAG_MIRAGE_ELDER_TALKED, {OASIS}_EventScript_ElderAgain
	setflag FLAG_MIRAGE_ELDER_TALKED
	msgbox {OASIS}_Text_Elder, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_ElderAgain::
	msgbox {OASIS}_Text_ElderAgain, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_ElderStone::
	msgbox {OASIS}_Text_ElderStone, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_ElderReward::
	msgbox {OASIS}_Text_ElderThanks, MSGBOX_DEFAULT
	givemon SPECIES_LARVESTA, 45
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {OASIS}_EventScript_ElderFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {OASIS}_Text_GotLarvesta, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_MIRAGE_LARVESTA
	msgbox {OASIS}_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_ElderFull::
	msgbox {OASIS}_Text_ElderFull, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_ElderDone::
	msgbox {OASIS}_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_Merchant::
	lock
	faceplayer
	message {OASIS}_Text_Merchant
	waitmessage
	pokemart {OASIS}_Mart
	msgbox {OASIS}_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
{OASIS}_Mart:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_MAX_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_SOFT_SAND
	.2byte ITEM_SMOOTH_ROCK
	.2byte ITEM_HEAT_ROCK
	.2byte ITEM_NONE
	release
	end

{OASIS}_EventScript_FruitSeller::
	lock
	faceplayer
	message {OASIS}_Text_Fruit
	waitmessage
	pokemart {OASIS}_FruitMart
	msgbox {OASIS}_Text_MerchantBye, MSGBOX_DEFAULT
	release
	end

	.align 2
{OASIS}_FruitMart:
	.2byte ITEM_ORAN_BERRY
	.2byte ITEM_SITRUS_BERRY
	.2byte ITEM_LUM_BERRY
	.2byte ITEM_LEPPA_BERRY
	.2byte ITEM_NONE
	release
	end

{OASIS}_EventScript_Fortune::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_STONE, {OASIS}_EventScript_FortuneDone
	msgbox {OASIS}_Text_Fortune, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_FortuneDone::
	msgbox {OASIS}_Text_FortuneDone, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_Girl::
	lock
	faceplayer
	goto_if_set FLAG_MIRAGE_SPRING_DONE, {OASIS}_EventScript_GirlHappy
	msgbox {OASIS}_Text_Girl, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_GirlHappy::
	msgbox {OASIS}_Text_GirlHappy, MSGBOX_DEFAULT
	release
	end

{OASIS}_EventScript_Traveller::
	msgbox {OASIS}_Text_Traveller, MSGBOX_NPC
	end

{OASIS}_EventScript_Landorus::
	lock
	faceplayer
	msgbox {OASIS}_Text_LandorusSees, MSGBOX_DEFAULT
	setwildbattle SPECIES_LANDORUS, 55
	playmoncry SPECIES_LANDORUS, CRY_MODE_ENCOUNTER
	msgbox {OASIS}_Text_Landorus, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {OASIS}_EventScript_LandorusStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {OASIS}_EventScript_LandorusStays
	setflag FLAG_MIRAGE_LANDORUS_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {OASIS}_EventScript_LandorusEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {OASIS}_Text_LandorusGone, MSGBOX_DEFAULT
{OASIS}_EventScript_LandorusEnd::
	release
	end

{OASIS}_EventScript_LandorusStays::
	msgbox {OASIS}_Text_LandorusStays, MSGBOX_DEFAULT
	release
	end

{trainer_script(OASIS, 'OMAR', text("I guard the path to the mesa. Only", "the strong may look upon the sands", "from up here!"),
                text("You are strong indeed."), text("From the mesa you can see the whole", "desert. And sometimes… a guardian."))}
{OASIS}_Text_Sign:
{text("MIRAGE OASIS", "The true water of the dunes.")}
{OASIS}_Text_WellDry:
{text("The well is bone dry. A round hollow", "is carved in the stone at the bottom,", "as if something should sit there.")}
{OASIS}_Text_WellFlowing:
{text("Cool, clear spring water fills the", "well to the brim.")}
{OASIS}_Text_PlaceStoneAsk:
{text("A round hollow is carved at the", "bottom of the dry well.", "", "Put the SPRING STONE in it?")}
{OASIS}_Text_PlaceStone:
{text("{PLAYER} lowered the SPRING STONE", "into the well…")}
{OASIS}_Text_SpringFlows:
{text("Water gushed up from the stone!", "The spring is flowing again!")}
{OASIS}_Text_Elder:
{text("Welcome, traveller. You saw through", "the mirages to find us. Few can.", "", "But our oasis is dying. The spring", "has dried and the pond is cracked mud.", "", "Long ago the SPRING STONE fed our", "well. Thieves took it, and the sand", "swallowed them and their court.", "", "Old tales say the sand swallows you", "where three stones stand. Will you", "bring the stone home?")}
{OASIS}_Text_ElderAgain:
{text("Where three stones stand in the dunes,", "the sand will swallow you. Below lies", "the court, and the SPRING STONE.")}
{OASIS}_Text_ElderStone:
{text("The SPRING STONE! You found it!", "Quickly, place it in the well!")}
{OASIS}_Text_ElderThanks:
{text("Listen to it… the spring sings again!", "The pond is full for the first time", "in years.", "", "Take this LARVESTA, child of the sun.", "It hatched by the well as the water", "rose.")}
{OASIS}_Text_GotLarvesta:
{text("{PLAYER} received a LARVESTA!")}
{OASIS}_Text_ElderFull:
{text("Your party is full. Come back for", "LARVESTA when there's room.")}
{OASIS}_Text_ElderDone:
{text("Something came down to rest on the", "mesa when the water rose. The", "guardian of the land, they say.")}
{OASIS}_Text_Merchant:
{text("Welcome to the bazaar! Goods from", "every corner of the dunes!")}
{OASIS}_Text_Fruit:
{text("Fresh berries from the oasis", "gardens!")}
{OASIS}_Text_MerchantBye:
{text("Come again, traveller!")}
{OASIS}_Text_Fortune:
{text("I see… sand. Falling sand. And three", "stones in a ring, near the north", "pass, west of the mirage palms.", "", "That will be one POKé DOLLAR.", "…Just kidding.")}
{OASIS}_Text_FortuneDone:
{text("The stars told me you'd find it.")}
{OASIS}_Text_Girl:
{text("The pond used to be full of water.", "Now it's just cracks. I miss", "splashing!")}
{OASIS}_Text_GirlHappy:
{text("The pond is back! Splash splash!")}
{OASIS}_Text_Traveller:
{text("I followed the caravan here. The", "inn in the blue tent lets you rest", "for free!")}
{OASIS}_Text_LandorusSees:
{text("A great POKéMON rests on the mesa,", "gazing over the restored oasis.")}
{OASIS}_Text_Landorus:
{text("Graaoooh!")}
{OASIS}_Text_LandorusGone:
{text("LANDORUS soared away over the dunes.")}
{OASIS}_Text_LandorusStays:
{text("LANDORUS is still watching over the", "oasis from the mesa.")}
''')
print('ok', OASIS)

# ---------------------------------------------------------------- the inn
interior(INN, 'MAP_MIRAGE_OASIS_INN', 'LAYOUT_HOUSE3', [(3, 7), (4, 7)], 'MAP_MIRAGE_OASIS', 0,
         mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes', music='MUS_ROUTE119',
         objects=[npc('OBJ_EVENT_GFX_WOMAN_5', 6, 3, f'{INN}_EventScript_Keeper', 'MOVEMENT_TYPE_FACE_DOWN')],
         scripts=rf'''
{INN}_MapScripts::
	.byte 0

{INN}_EventScript_Keeper::
	lock
	faceplayer
	msgbox {INN}_Text_Keeper, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {INN}_EventScript_KeeperNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox {INN}_Text_KeeperDone, MSGBOX_DEFAULT
	release
	end

{INN}_EventScript_KeeperNo::
	msgbox {INN}_Text_KeeperNo, MSGBOX_DEFAULT
	release
	end

{INN}_Text_Keeper:
{text("Welcome to the oasis inn! Rest in", "the shade a while?")}
{INN}_Text_KeeperDone:
{text("Refreshed? The desert is kinder to", "those who rest.")}
{INN}_Text_KeeperNo:
{text("The shade is always here.")}
''')
print('ok', INN)

# ---------------------------------------------------------------- the sunken court
W, H = 40, 28
c = Area(COURT, W, H, st, fill='#')
c.ellipse(8, 15, 6, 10, 's')                      # the west court (pit A)
c.ellipse(32, 15, 6, 10, 's')                     # the east court (pit B)
c.ellipse(20, 14, 4.5, 10, 's')                   # the altar court (pit C)
ARRIVALS = [(8, 22), (32, 22), (20, 22)]
for x, y in ARRIVALS:
    c.warp(x, y, 'MAP_MIRAGE_DUNES', 0)           # where you land: plain sand, so stepping here does nothing
EXITS = [(8, 5), (32, 5), (20, 5)]
for i, (x, y) in enumerate(EXITS):
    c.rect(x, y, 1, 2, 's')
    place(c, 'stairs', x, y)
    c.warp(x, y, 'MAP_MIRAGE_DUNES', 3 + i)        # the dunes' arrival points, after its three pits
ALTAR = (19, 8)
place(c, 'altar', *ALTAR)
c.sign(ALTAR[0], ALTAR[1] + 1, f'{COURT}_EventScript_Altar', 'BG_EVENT_PLAYER_FACING_NORTH')
c.sign(ALTAR[0] + 1, ALTAR[1] + 1, f'{COURT}_EventScript_Altar', 'BG_EVENT_PLAYER_FACING_NORTH')
for x, y in ((17, 11), (23, 11), (17, 16), (23, 16), (5, 9), (11, 9), (29, 9), (35, 9)):
    place(c, 'pillar', x, y)
for x, y in ((5, 15), (11, 17), (29, 17), (35, 14), (18, 19)):
    place(c, 'pillar_broken', x, y - 1)
for x, y in ((6, 19), (10, 12), (30, 12), (34, 19), (22, 20)):
    place(c, 'rubble', x, y)
for x, y in ((4, 12), (36, 12), (21, 9)):
    place(c, 'jar', x, y)
place(c, 'rug', 19, 13)
c.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 31, 13, f'{COURT}_EventScript_Nefera', 3, 'MOVEMENT_TYPE_FACE_DOWN')
c.item('ITEM_TM_EARTHQUAKE', 35, 22, 'FLAG_ITEM_MIRAGE_TM_EARTHQUAKE')
c.item('ITEM_BIG_NUGGET', 4, 20, 'FLAG_ITEM_MIRAGE_BIG_NUGGET')
c.item('ITEM_MAX_REVIVE', 11, 7, 'FLAG_ITEM_MIRAGE_MAX_REVIVE')
c.item('ITEM_FULL_RESTORE', 23, 7, 'FLAG_ITEM_MIRAGE_FULL_RESTORE')
c.hidden(29, 21, 'ITEM_RARE_BONE', 'FLAG_HIDDEN_ITEM_MIRAGE_RARE_BONE')
c.hidden(17, 21, 'ITEM_STARDUST', 'FLAG_HIDDEN_ITEM_MIRAGE_STARDUST')
empty = MT['altar_empty']
take = ''.join(f'\tsetmetatile {ALTAR[0] + cc}, {ALTAR[1] + r}, {mt}, TRUE\n' for r, row in enumerate(empty) for cc, mt in enumerate(row))
c.write(map_id='MAP_MIRAGE_OASIS_SUNKEN_COURT', layout_id='LAYOUT_MIRAGE_OASIS_SUNKEN_COURT', music='MUS_DESERT',
        mapsec=MAPSEC, mapsec_name='MIRAGE OASIS', map_type='MAP_TYPE_UNDERGROUND', after=INN, show_name=False,
        allow_escaping=True, scripts=rf'''
{COURT}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {COURT}_OnLoad
	.byte 0

{COURT}_OnLoad:
	call_if_set FLAG_MIRAGE_SPRING_STONE, {COURT}_EventScript_AltarEmpty
	end

{COURT}_EventScript_AltarEmpty::
{take}	return

{COURT}_EventScript_Altar::
	lockall
	goto_if_set FLAG_MIRAGE_SPRING_STONE, {COURT}_EventScript_AltarBare
	msgbox {COURT}_Text_Altar, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {COURT}_EventScript_AltarLeave
	call {COURT}_EventScript_AltarEmpty
	special DrawWholeMapView
	setflag FLAG_MIRAGE_SPRING_STONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox {COURT}_Text_GotStone, MSGBOX_DEFAULT
	waitfanfare
	msgbox {COURT}_Text_StoneCool, MSGBOX_DEFAULT
	releaseall
	end

{COURT}_EventScript_AltarBare::
	msgbox {COURT}_Text_AltarBare, MSGBOX_DEFAULT
{COURT}_EventScript_AltarLeave::
	releaseall
	end

{trainer_script(COURT, 'NEFERA', text("You fell in too? Ha! This court is", "cursed. Let me show you!"),
                text("The curse is broken…"), text("The stairs lead back up to the", "dunes. Every court has its own."))}
{COURT}_Text_Altar:
{text("A blue stone rests on the altar,", "cool and damp to the touch.", "", "Take the stone?")}
{COURT}_Text_GotStone:
{text("{PLAYER} found the SPRING STONE!")}
{COURT}_Text_StoneCool:
{text("Water beads on its surface, though", "the court is dry as bone.")}
{COURT}_Text_AltarBare:
{text("The altar is bare. Sand trickles", "where the stone used to sit.")}
''')
print('ok', COURT)
register_wild('MAP_MIRAGE_OASIS_SUNKEN_COURT', 'gMirageOasisSunkenCourt', land=ENC_COURT, land_rate=8)

# ---------------------------------------------------------------- the way there, from Route 111
add_object('Route111', {'graphics_id': 'OBJ_EVENT_GFX_MAN_4', 'x': 20, 'y': 105, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': 'Route111_EventScript_MirageGuide', 'flag': '0'})
register_script_file('oasis', rf'''
@ Mirage Dunes caravan master on Route 111 (tools/areas/oasis.py).
Route111_EventScript_MirageGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE08_GET, Route111_EventScript_MirageNoBadge
	msgbox Route111_Text_MirageAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route111_EventScript_MirageNo
	msgbox Route111_Text_MirageGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MIRAGE_DUNES, {ARRIVE[0]}, {ARRIVE[1]}
	waitstate
	release
	end

Route111_EventScript_MirageNoBadge::
	msgbox Route111_Text_MirageNoBadge, MSGBOX_DEFAULT
	release
	end

Route111_EventScript_MirageNo::
	msgbox Route111_Text_MirageNo, MSGBOX_DEFAULT
	release
	end

Route111_Text_MirageNoBadge:
{text("I lead caravans across the MIRAGE", "DUNES, beyond this desert.", "", "The deep dunes are for the strongest", "trainers only. Come back with all", "eight badges.")}
Route111_Text_MirageAsk:
{text("Eight badges! Our caravan lost its", "way to the MIRAGE OASIS. The dunes", "are full of false water.", "", "Will you ride out with me and help", "find the real oasis?")}
Route111_Text_MirageGo:
{text("Hold on to your hat!")}
Route111_Text_MirageNo:
{text("The caravan waits for no one… but", "I'll wait for you.")}
''')
print('ok entrance')
