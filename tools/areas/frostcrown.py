"""Mt. Frostcrown and Snowdrift Village, for after the sixth badge: a
switchback trail up a snowy mountain, a village of log cabins on the
plateau at the top, and a sled run that whisks you back down.

A blizzard stranded three climbers on the trail. Find them and send them
down, and the Mountain Rescue chief in the lodge digs out the path to the
summit, where Articuno roosts by the cairn.

The rock is Routes 114 and 115's (styles.canyon()) with its palette turned
to snow; the sled run, cabins, lodge and props are drawn in
tools/areas/frost_art.py (gTileset_Frost). General's trees and grass show
their winter colours on these maps all year (src/life_seasons.c).

    python3 tools/areas/frost_art.py   # the tileset, when the art changes
    python3 tools/areas/frostcrown.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import canyon

AREA_NO = 15
TRAIL, VILLAGE = 'MtFrostcrown', 'SnowdriftVillage'
LODGE, CABIN_W, CABIN_E = 'SnowdriftVillage_Lodge', 'SnowdriftVillage_WestCabin', 'SnowdriftVillage_EastCabin'
MAPSEC_T, MAPSEC_V = 'MAPSEC_MT_FROSTCROWN', 'MAPSEC_SNOWDRIFT_VILLAGE'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'frost_tiles.json')))
MT = ART['objects']
FLOOR = 0x241

register_constants(TRAIL, AREA_NO, flags=[
    'FROST_CLIMBER_1', 'FROST_CLIMBER_2', 'FROST_CLIMBER_3', 'FROST_RESCUE_DONE', 'FROST_ARTICUNO_DONE',
    'FROST_SCULPTOR_GIFT', 'ITEM_FROST_NEVER_MELT_ICE', 'ITEM_FROST_MAX_REPEL', 'ITEM_FROST_ULTRA_BALL',
    'ITEM_FROST_TM_HAIL', 'ITEM_FROST_MAX_POTION', 'ITEM_FROST_ICE_STONE', 'HIDDEN_ITEM_FROST_NUGGET',
    'HIDDEN_ITEM_FROST_RARE_CANDY', 'HIDDEN_ITEM_FROST_HEART_SCALE', 'HIDDEN_ITEM_FROST_PP_UP',
], trainers=['FROST_BRAM', 'FROST_MAREN', 'FROST_ROALD', 'FROST_ELSA'])

register_trainers(TRAIL, '''
=== AREA_TRAINER_FROST_BRAM ===
Name: BRAM
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Smart Trainer

Piloswine
Level: 38

Golem
Level: 39

=== AREA_TRAINER_FROST_MAREN ===
Name: MAREN
Class: Expert
Pic: Expert F
Gender: Female
Music: Intense
Double Battle: No
AI: Smart Trainer

Froslass
Level: 39

Glalie
Level: 39

Absol
Level: 40

=== AREA_TRAINER_FROST_ROALD ===
Name: ROALD
Class: Cooltrainer
Pic: Cooltrainer M
Gender: Male
Music: Cool
Double Battle: No
AI: Smart Trainer

Weavile
Level: 40

Abomasnow
Level: 41

=== AREA_TRAINER_FROST_ELSA ===
Name: ELSA
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Smart Trainer

Vanilluxe
Level: 41

Avalugg
Level: 41

Beartic
Level: 42
''')


def place(a, name, x, y):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


def one(name):
    return MT[name][0][0]


def sled_run(a, path):
    """Lays the run along path, a list of cells: each piece slides you on
    to the next cell (turns where the direction changes)."""
    for (x, y), (nx, ny) in zip(path, path[1:]):
        d = {(1, 0): 'e', (-1, 0): 'w', (0, 1): 's', (0, -1): 'n'}[(nx - x, ny - y)]
        a.tile(x, y, one(f'sled_{d}'), collision=0, c='o')
    # turns: where the way in and the way out differ
    for (px, py), (x, y), (nx, ny) in zip(path, path[1:], path[2:]):
        din = {(1, 0): 'e', (-1, 0): 'w', (0, 1): 's', (0, -1): 'n'}[(x - px, y - py)]
        dout = {(1, 0): 'e', (-1, 0): 'w', (0, 1): 's', (0, -1): 'n'}[(nx - x, ny - y)]
        if din != dout:
            a.tile(x, y, one(f'sled_{din}{dout}'), collision=0, c='o')


def flat(a):
    """The synthesised blocks with every floor at elevation 3: the mountain
    is one walking level (Route 114 puts its mesa tops at 5)."""
    b = a.blocks()
    floor = (b >> 12) == 5
    b[floor] = (b[floor] & 0x0FFF) | (3 << 12)
    return b


def run_cells(*legs):
    """Cells of a run from straight legs: each leg is (x0, y0, x1, y1)."""
    cells = []
    for x0, y0, x1, y1 in legs:
        n = max(abs(x1 - x0), abs(y1 - y0))
        for i in range(n + 1):
            c = (x0 + (x1 - x0) * i // n, y0 + (y1 - y0) * i // n)
            if not cells or cells[-1] != c:
                cells.append(c)
    return cells


st = copy.copy(canyon())
st.secondary = 'gTileset_Frost'               # Fallarbor's tiles (snowy palette) plus the art, same metatile ids

# ---------------------------------------------------------------- the trail
W, H = 48, 88
t = Area(TRAIL, W, H, st, fill='#')
t.ellipse(22, 80, 17, 5.5, '.')                 # base camp
t.rect(20, 84, 6, 4, '.')                       # the way in from below
t.line([(11, 76), (11, 71)], 4, '.')
t.rect(4, 66, 30, 7, '.')                       # shelf 1
t.rect(35, 67, 4, 3, '.')                       # the east alcove, behind a rock
t.rect(34, 68, 1, 1, '.')
t.line([(30, 66), (30, 57)], 4, '.')
t.rect(6, 50, 30, 8, '.')                       # shelf 2
t.ellipse(10, 53, 5, 3, '"')
t.line([(8, 50), (8, 41)], 4, '.')
t.rect(4, 29, 34, 13, '.')                      # shelf 3: the snowy pines
for x, y in ((14, 30), (16, 30), (18, 30), (20, 32), (12, 34), (14, 34), (24, 30), (26, 30), (10, 30),
             (4, 36), (6, 36), (34, 30), (36, 32), (34, 38), (20, 38), (22, 38)):
    t.rect(x, y, 2, 2, 'T')
t.rect(4, 29, 6, 1, '#')
t.ellipse(28, 36, 5, 3, '"')
t.line([(30, 29), (30, 20)], 4, '.')
t.rect(8, 12, 30, 9, '.')                       # shelf 4
t.ellipse(14, 15, 4, 2.5, '"')
t.line([(22, 12), (22, 0)], 6, '.')             # up to the village
t.rect(4, 6, 7, 5, '.')                         # the hermit's ledge, from shelf 4
t.line([(9, 12), (8, 9)], 3, '.')
t.snap_trees('T', '.')
RUN_T = run_cells((43, 0, 43, 30), (43, 30, 39, 30), (39, 30, 39, 56), (39, 56, 44, 56), (44, 56, 44, 78))
t.rect(38, 78, 8, 4, '.')                       # where the run comes out
t.line([(38, 80), (34, 80)], 3, '.')
sled_run(t, RUN_T)
place(t, 'sled_stop', 44, 78)
for x, y in ((14, 79), (30, 79)):
    place(t, 'campfire', x, y)
place(t, 'sled', 16, 80)
place(t, 'lantern', 24, 76)
place(t, 'ice_rock', 36, 51)
place(t, 'ice_rock', 7, 14)
place(t, 'snowman', 6, 6)

t.connections = [{'map': 'MAP_SNOWDRIFT_VILLAGE', 'offset': 0, 'direction': 'up'}]
t.obj('OBJ_EVENT_GFX_HIKER', 26, 82, f'{TRAIL}_EventScript_Ranger',
      'MOVEMENT_TYPE_FACE_LEFT')
t.obj('OBJ_EVENT_GFX_BREAKABLE_ROCK', 34, 68, 'EventScript_RockSmash', 'MOVEMENT_TYPE_LOOK_AROUND')
CLIMBERS = [((8, 52), 'OBJ_EVENT_GFX_HIKER'), ((37, 68), 'OBJ_EVENT_GFX_CAMPER'), ((5, 31), 'OBJ_EVENT_GFX_PICNICKER')]
for i, ((x, y), gfx) in enumerate(CLIMBERS, 1):
    t.obj(gfx, x, y, f'{TRAIL}_EventScript_Climber{i}', 'MOVEMENT_TYPE_FACE_DOWN', f'FLAG_FROST_CLIMBER_{i}')
t.trainer('OBJ_EVENT_GFX_HIKER', 20, 68, f'{TRAIL}_EventScript_Bram', 3, 'MOVEMENT_TYPE_FACE_DOWN')
t.trainer('OBJ_EVENT_GFX_EXPERT_F', 24, 51, f'{TRAIL}_EventScript_Maren', 3, 'MOVEMENT_TYPE_FACE_DOWN')
t.trainer('OBJ_EVENT_GFX_COOLTRAINER_M', 18, 35, f'{TRAIL}_EventScript_Roald', 4, 'MOVEMENT_TYPE_FACE_LEFT')
t.trainer('OBJ_EVENT_GFX_COOLTRAINER_F', 30, 15, f'{TRAIL}_EventScript_Elsa', 3, 'MOVEMENT_TYPE_FACE_LEFT')
t.obj('OBJ_EVENT_GFX_OLD_MAN', 7, 8, f'{TRAIL}_EventScript_Hermit', 'MOVEMENT_TYPE_FACE_DOWN')
t.obj('OBJ_EVENT_GFX_SPECIES(SNORUNT)', 16, 70, f'{TRAIL}_EventScript_Snorunt', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
t.item('ITEM_MAX_REPEL', 5, 70, 'FLAG_ITEM_FROST_MAX_REPEL')
t.item('ITEM_ULTRA_BALL', 33, 54, 'FLAG_ITEM_FROST_ULTRA_BALL')
t.item('ITEM_TM_HAIL', 36, 40, 'FLAG_ITEM_FROST_TM_HAIL')
t.item('ITEM_NEVER_MELT_ICE', 9, 7, 'FLAG_ITEM_FROST_NEVER_MELT_ICE')
t.item('ITEM_MAX_POTION', 36, 68, 'FLAG_ITEM_FROST_MAX_POTION')
t.hidden(6, 39, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_FROST_NUGGET')
t.hidden(37, 19, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_FROST_RARE_CANDY')
t.hidden(41, 80, 'ITEM_HEART_SCALE', 'FLAG_HIDDEN_ITEM_FROST_HEART_SCALE')
t.sign(19, 84, f'{TRAIL}_EventScript_Sign')


def climber(i, text):
    return rf'''{TRAIL}_EventScript_Climber{i}::
	lock
	faceplayer
	msgbox {TRAIL}_Text_Climber{i}, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_FROST_CLIMBER_{i}
	fadescreen FADE_FROM_BLACK
	msgbox {TRAIL}_Text_ClimberGone, MSGBOX_DEFAULT
	release
	end

{TRAIL}_Text_Climber{i}:
{text}
'''


def trainer(name, label, intro, defeat, post):
    return rf'''{TRAIL}_EventScript_{label}::
	trainerbattle_single TRAINER_FROST_{name}, {TRAIL}_Text_{label}Intro, {TRAIL}_Text_{label}Defeat
	msgbox {TRAIL}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{TRAIL}_Text_{label}Intro:
	.string "{intro}$"

{TRAIL}_Text_{label}Defeat:
	.string "{defeat}$"

{TRAIL}_Text_{label}Post:
	.string "{post}$"

'''


t.write(blocks=flat(t), map_id='MAP_MT_FROSTCROWN', layout_id='LAYOUT_MT_FROSTCROWN', music='MUS_ROUTE113', mapsec=MAPSEC_T,
        mapsec_name='MT. FROSTCROWN', weather='WEATHER_SNOW', allow_cycling=False, scripts=rf'''
{TRAIL}_MapScripts::
	.byte 0

{TRAIL}_EventScript_Ranger::
	lock
	faceplayer
	msgbox {TRAIL}_Text_RangerBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {TRAIL}_EventScript_RangerStay
	msgbox {TRAIL}_Text_RangerGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 20, 17
	waitstate
	release
	end

{TRAIL}_EventScript_RangerStay::
	msgbox {TRAIL}_Text_RangerStay, MSGBOX_DEFAULT
	release
	end

{TRAIL}_EventScript_Snorunt::
	lock
	faceplayer
	playmoncry SPECIES_SNORUNT, CRY_MODE_NORMAL
	msgbox {TRAIL}_Text_Snorunt, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{TRAIL}_EventScript_Hermit::
	msgbox {TRAIL}_Text_Hermit, MSGBOX_NPC
	end

{TRAIL}_EventScript_Sign::
	msgbox {TRAIL}_Text_Sign, MSGBOX_SIGN
	end

{climber(1, '''	.string "Brr… I've been stuck in this grass\\n"
	.string "since the blizzard. My legs are\\l"
	.string "frozen stiff!\\p"
	.string "Thanks for finding me. I'll make my\\n"
	.string "way down to the ranger's camp.$"''')}{climber(2, '''	.string "That rock rolled down and shut me\\n"
	.string "in! I thought I'd never get out.\\p"
	.string "Phew… I'm heading back down while\\n"
	.string "the weather's clear.$"''')}{climber(3, '''	.string "I took shelter under the pines and\\n"
	.string "lost the trail in the snow.\\p"
	.string "You found me! I'll follow your\\n"
	.string "tracks back down.$"''')}{TRAIL}_Text_ClimberGone:
	.string "The climber headed down the mountain.\n"
	.string "Better tell the MOUNTAIN RESCUE.$"

{trainer('BRAM', 'Bram', 'This mountain is my backyard! Show\\nme your climbing legs!', 'My legs gave out first…', 'The trail zigzags all the way up.\\nTake it one shelf at a time.')}{trainer('MAREN', 'Maren', 'The cold sharpens the mind. And my\\nPOKéMON.', 'Your mind is sharper still.', 'Some climbers never came back from\\nthe blizzard. Keep your eyes open.')}{trainer('ROALD', 'Roald', "You made it to the pines? Let's see\\nif you can make it past me!", 'Snowed under!', 'The pines hide all sorts of things.\\nEven people, sometimes.')}{trainer('ELSA', 'Elsa', 'Nearly at the top! But the last\\nstretch is the coldest.', 'I\'m iced!', 'Snowdrift Village is just above.\\nTry the sled run on the way down!')}{TRAIL}_Text_RangerBack:
	.string "I run the ranger's camp. Want a\n"
	.string "lift back to LILYCOVE CITY?$"

{TRAIL}_Text_RangerGo:
	.string "Hold on, it's a bumpy ride!$"

{TRAIL}_Text_RangerStay:
	.string "Stay warm up there. And if you see\n"
	.string "any lost climbers, send them down!$"

{TRAIL}_Text_Snorunt:
	.string "SNORUNT is rolling in the snow.$"

{TRAIL}_Text_Hermit:
	.string "I've lived on this ledge for forty\n"
	.string "winters.\p"
	.string "They say a bird of ice roosts by the\n"
	.string "summit cairn. Its wings make the\l"
	.string "snow fall.$"

{TRAIL}_Text_Sign:
	.string "MT. FROSTCROWN TRAIL\n"
	.string "SNOWDRIFT VILLAGE: straight up!$"
''')
print('ok', TRAIL)

# ---------------------------------------------------------------- the village
W, H = 48, 32
v = Area(VILLAGE, W, H, st, fill='#')
v.ellipse(23, 19, 19, 8, '.')
v.rect(19, 24, 8, 8, '.')                       # down to the trail
v.ellipse(23, 4, 9, 3.5, '.')                   # the summit
v.rect(26, 6, 2, 6, '.')                        # the path up, under the snowdrift
v.rect(36, 16, 8, 4, '.')                       # out to the top of the sled run
v.ellipse(9, 23, 4, 2, '"')
for x, y in ((4, 16), (4, 20), (40, 22), (36, 24), (6, 26)):
    v.rect(x, y, 2, 2, 'T')
v.snap_trees('T', '.')
LODGE_AT, CABIN_W_AT, CABIN_E_AT = (15, 9), (5, 13), (31, 12)
place(v, 'lodge', *LODGE_AT)                     # 7x5, door at (+3, +4)
place(v, 'cabin', *CABIN_W_AT)                   # 5x4, door at (+2, +3)
place(v, 'cabin', *CABIN_E_AT)
place(v, 'snowman', 13, 18)
place(v, 'snowman', 30, 21)
place(v, 'campfire', 23, 19)
for x, y in ((14, 13), (24, 13), (19, 23), (27, 23)):
    place(v, 'lantern', x, y)
place(v, 'sled', 40, 17)
place(v, 'sled', 41, 17)
place(v, 'cairn', 23, 1)
DRIFT = [(26, 8), (27, 8)]
for x, y in DRIFT:
    v.tile(x, y, one('snowdrift'), collision=1, c='o')
RUN_V = run_cells((43, 18, 43, 31))
sled_run(v, RUN_V + [(43, 32)])
v.connections = [{'map': 'MAP_MT_FROSTCROWN', 'offset': 0, 'direction': 'down'}]
v.warp(LODGE_AT[0] + 3, LODGE_AT[1] + 4, 'MAP_SNOWDRIFT_VILLAGE_LODGE', 0)
v.warp(CABIN_W_AT[0] + 2, CABIN_W_AT[1] + 3, 'MAP_SNOWDRIFT_VILLAGE_WEST_CABIN', 0)
v.warp(CABIN_E_AT[0] + 2, CABIN_E_AT[1] + 3, 'MAP_SNOWDRIFT_VILLAGE_EAST_CABIN', 0)
v.obj('OBJ_EVENT_GFX_SPECIES(ARTICUNO)', 21, 3, f'{VILLAGE}_EventScript_Articuno', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_FROST_ARTICUNO_DONE')
v.obj('OBJ_EVENT_GFX_BOY_1', 41, 16, f'{VILLAGE}_EventScript_SledKid', 'MOVEMENT_TYPE_FACE_LEFT')
v.obj('OBJ_EVENT_GFX_WOMAN_3', 21, 20, f'{VILLAGE}_EventScript_Woman', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
v.obj('OBJ_EVENT_GFX_SPECIES(CUBCHOO)', 12, 21, f'{VILLAGE}_EventScript_Cubchoo', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
v.obj('OBJ_EVENT_GFX_SPECIES(DELIBIRD)', 33, 19, f'{VILLAGE}_EventScript_Delibird', 'MOVEMENT_TYPE_FACE_DOWN')
v.obj('OBJ_EVENT_GFX_OLD_MAN', 28, 9, f'{VILLAGE}_EventScript_Digger', 'MOVEMENT_TYPE_FACE_LEFT')
v.item('ITEM_ICE_STONE', 31, 3, 'FLAG_ITEM_FROST_ICE_STONE')
v.hidden(6, 24, 'ITEM_PP_UP', 'FLAG_HIDDEN_ITEM_FROST_PP_UP')
v.sign(22, 24, f'{VILLAGE}_EventScript_Sign')
v.sign(39, 18, f'{VILLAGE}_EventScript_SledSign')
v.write(blocks=flat(v), map_id='MAP_SNOWDRIFT_VILLAGE', layout_id='LAYOUT_SNOWDRIFT_VILLAGE', music='MUS_FALLARBOR', mapsec=MAPSEC_V,
        mapsec_name='SNOWDRIFT VILLAGE', map_type='MAP_TYPE_TOWN', weather='WEATHER_SNOW', allow_cycling=False,
        after=TRAIL, scripts=rf'''
{VILLAGE}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {VILLAGE}_OnLoad
	.byte 0

{VILLAGE}_OnLoad:
	call_if_set FLAG_FROST_RESCUE_DONE, {VILLAGE}_EventScript_ClearDrift
	end

{VILLAGE}_EventScript_ClearDrift::
''' + ''.join(f'\tsetmetatile {x}, {y}, {FLOOR}, FALSE\n' for x, y in DRIFT) + rf'''	return

{VILLAGE}_EventScript_Digger::
	lock
	faceplayer
	goto_if_set FLAG_FROST_RESCUE_DONE, {VILLAGE}_EventScript_DiggerDone
	msgbox {VILLAGE}_Text_Digger, MSGBOX_DEFAULT
	release
	end

{VILLAGE}_EventScript_DiggerDone::
	msgbox {VILLAGE}_Text_DiggerDone, MSGBOX_DEFAULT
	release
	end

{VILLAGE}_EventScript_Articuno::
	lock
	faceplayer
	msgbox {VILLAGE}_Text_Chill, MSGBOX_DEFAULT
	setwildbattle SPECIES_ARTICUNO, 45
	playmoncry SPECIES_ARTICUNO, CRY_MODE_ENCOUNTER
	msgbox {VILLAGE}_Text_Articuno, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {VILLAGE}_EventScript_ArticunoStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {VILLAGE}_EventScript_ArticunoStays
	setflag FLAG_FROST_ARTICUNO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {VILLAGE}_EventScript_ArticunoEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {VILLAGE}_Text_ArticunoGone, MSGBOX_DEFAULT
{VILLAGE}_EventScript_ArticunoEnd::
	release
	end

{VILLAGE}_EventScript_ArticunoStays::
	msgbox {VILLAGE}_Text_ArticunoStays, MSGBOX_DEFAULT
	release
	end

{VILLAGE}_EventScript_SledKid::
	msgbox {VILLAGE}_Text_SledKid, MSGBOX_NPC
	end

{VILLAGE}_EventScript_Woman::
	msgbox {VILLAGE}_Text_Woman, MSGBOX_NPC
	end

{VILLAGE}_EventScript_Cubchoo::
	lock
	faceplayer
	playmoncry SPECIES_CUBCHOO, CRY_MODE_NORMAL
	msgbox {VILLAGE}_Text_Cubchoo, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{VILLAGE}_EventScript_Delibird::
	lock
	faceplayer
	playmoncry SPECIES_DELIBIRD, CRY_MODE_NORMAL
	msgbox {VILLAGE}_Text_Delibird, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{VILLAGE}_EventScript_Sign::
	msgbox {VILLAGE}_Text_Sign, MSGBOX_SIGN
	end

{VILLAGE}_EventScript_SledSign::
	msgbox {VILLAGE}_Text_SledSign, MSGBOX_SIGN
	end

{VILLAGE}_Text_Digger:
	.string "The blizzard buried the summit path\n"
	.string "under a mountain of snow.\p"
	.string "MOUNTAIN RESCUE has every shovel\n"
	.string "out looking for the lost climbers.\l"
	.string "Once they're safe, we'll dig.$"

{VILLAGE}_Text_DiggerDone:
	.string "All dug out! The summit path is\n"
	.string "open. Mind the wind up there.$"

{VILLAGE}_Text_Chill:
	.string "The air turns bitterly cold…$"

{VILLAGE}_Text_Articuno:
	.string "Kyaaaah!$"

{VILLAGE}_Text_ArticunoGone:
	.string "ARTICUNO flew off into the snow.$"

{VILLAGE}_Text_ArticunoStays:
	.string "ARTICUNO is still perched by the\n"
	.string "cairn.$"

{VILLAGE}_Text_SledKid:
	.string "Wanna race? Step on the run and\n"
	.string "whoosh! All the way to the bottom\l"
	.string "of the mountain!$"

{VILLAGE}_Text_Woman:
	.string "Welcome to SNOWDRIFT VILLAGE!\n"
	.string "Warm up by the fire.$"

{VILLAGE}_Text_Cubchoo:
	.string "CUBCHOO sniffled. Its nose is\n"
	.string "always running!$"

{VILLAGE}_Text_Delibird:
	.string "DELIBIRD is guarding a sack of\n"
	.string "something. Presents, maybe?$"

{VILLAGE}_Text_Sign:
	.string "SNOWDRIFT VILLAGE\n"
	.string "Warm hearts on a cold mountain.$"

{VILLAGE}_Text_SledSign:
	.string "SLED RUN\n"
	.string "To the ranger's camp. One way only!$"
''')
print('ok', VILLAGE)

register_wild('MAP_MT_FROSTCROWN', 'gMtFrostcrown',
              land=[('SNORUNT', 36, 38), ('SNOVER', 36, 38), ('SWINUB', 36, 38), ('CUBCHOO', 36, 38),
                    ('VANILLITE', 37, 39), ('BERGMITE', 37, 39), ('SNEASEL', 37, 39), ('DELIBIRD', 37, 39),
                    ('PILOSWINE', 38, 40), ('ABSOL', 38, 40), ('GLALIE', 39, 40), ('ABOMASNOW', 39, 40)])

# ---------------------------------------------------------------- inside
interior(LODGE, 'MAP_SNOWDRIFT_VILLAGE_LODGE', 'LAYOUT_HOUSE3', [(3, 7), (4, 7)], 'MAP_SNOWDRIFT_VILLAGE', 0,
         mapsec=MAPSEC_V, group='gMapGroup_TownsAndRoutes', music='MUS_FALLARBOR',
         objects=[npc('OBJ_EVENT_GFX_MAN_4', 6, 3, f'{LODGE}_EventScript_Chief', 'MOVEMENT_TYPE_FACE_DOWN'),
                  npc('OBJ_EVENT_GFX_WOMAN_2', 2, 3, f'{LODGE}_EventScript_Keeper', 'MOVEMENT_TYPE_FACE_DOWN')],
         scripts=rf'''
{LODGE}_MapScripts::
	.byte 0

{LODGE}_EventScript_Keeper::
	lock
	faceplayer
	msgbox {LODGE}_Text_Keeper, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {LODGE}_EventScript_KeeperNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox {LODGE}_Text_KeeperDone, MSGBOX_DEFAULT
	release
	end

{LODGE}_EventScript_KeeperNo::
	msgbox {LODGE}_Text_KeeperNo, MSGBOX_DEFAULT
	release
	end

{LODGE}_EventScript_Chief::
	lock
	faceplayer
	goto_if_set FLAG_FROST_RESCUE_DONE, {LODGE}_EventScript_ChiefDone
	goto_if_unset FLAG_FROST_CLIMBER_1, {LODGE}_EventScript_ChiefAsk
	goto_if_unset FLAG_FROST_CLIMBER_2, {LODGE}_EventScript_ChiefAsk
	goto_if_unset FLAG_FROST_CLIMBER_3, {LODGE}_EventScript_ChiefAsk
	msgbox {LODGE}_Text_ChiefThanks, MSGBOX_DEFAULT
	givemon SPECIES_FRIGIBAX, 35
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {LODGE}_EventScript_ChiefFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {LODGE}_Text_GotFrigibax, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_FROST_RESCUE_DONE
	msgbox {LODGE}_Text_ChiefDig, MSGBOX_DEFAULT
	release
	end

{LODGE}_EventScript_ChiefFull::
	msgbox {LODGE}_Text_ChiefFull, MSGBOX_DEFAULT
	release
	end

{LODGE}_EventScript_ChiefAsk::
	msgbox {LODGE}_Text_ChiefAsk, MSGBOX_DEFAULT
	release
	end

{LODGE}_EventScript_ChiefDone::
	msgbox {LODGE}_Text_ChiefDig, MSGBOX_DEFAULT
	release
	end

{LODGE}_Text_Keeper:
	.string "Welcome to the lodge! Would you\n"
	.string "like to rest by the stove?$"

{LODGE}_Text_KeeperDone:
	.string "All warmed up! Your POKéMON look\n"
	.string "ready for anything.$"

{LODGE}_Text_KeeperNo:
	.string "The stove's always lit.$"

{LODGE}_Text_ChiefAsk:
	.string "I'm the chief of MOUNTAIN RESCUE.\p"
	.string "Three climbers went missing on the\n"
	.string "trail in the blizzard: one near the\l"
	.string "tall grass, one by a fallen rock,\l"
	.string "one somewhere in the pines.\p"
	.string "If you find them, send them down to\n"
	.string "the ranger's camp. Please hurry!$"

{LODGE}_Text_ChiefThanks:
	.string "The ranger radioed up: all three\n"
	.string "climbers are safe at the camp!\p"
	.string "You've done MOUNTAIN RESCUE proud.\n"
	.string "Take this FRIGIBAX. We raised it\l"
	.string "here at the lodge.$"

{LODGE}_Text_GotFrigibax:
	.string "{{PLAYER}} received a FRIGIBAX!$"

{LODGE}_Text_ChiefFull:
	.string "Your party's full! Make some room\n"
	.string "and come back for FRIGIBAX.$"

{LODGE}_Text_ChiefDig:
	.string "Now my crew can dig out the summit\n"
	.string "path. Go and see the view!$"
''')
interior(CABIN_W, 'MAP_SNOWDRIFT_VILLAGE_WEST_CABIN', 'LAYOUT_HOUSE1', [(3, 8), (4, 8)], 'MAP_SNOWDRIFT_VILLAGE', 1,
         mapsec=MAPSEC_V, group='gMapGroup_TownsAndRoutes',
         objects=[npc('OBJ_EVENT_GFX_OLD_WOMAN', 4, 4, f'{CABIN_W}_EventScript_Granny', 'MOVEMENT_TYPE_FACE_DOWN'),
                  npc('OBJ_EVENT_GFX_SPECIES(SNOM)', 6, 5, f'{CABIN_W}_EventScript_Snom', 'MOVEMENT_TYPE_WANDER_AROUND')],
         scripts=rf'''
{CABIN_W}_MapScripts::
	.byte 0

{CABIN_W}_EventScript_Granny::
	msgbox {CABIN_W}_Text_Granny, MSGBOX_NPC
	end

{CABIN_W}_EventScript_Snom::
	lock
	faceplayer
	playmoncry SPECIES_SNOM, CRY_MODE_NORMAL
	msgbox {CABIN_W}_Text_Snom, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CABIN_W}_Text_Granny:
	.string "When I was a girl, we sledded to\n"
	.string "the bottom of the mountain and\l"
	.string "walked all the way back up!\p"
	.string "Young folk today have it easy.$"

{CABIN_W}_Text_Snom:
	.string "SNOM is nibbling on an icicle.$"
''')
interior(CABIN_E, 'MAP_SNOWDRIFT_VILLAGE_EAST_CABIN', 'LAYOUT_HOUSE1', [(3, 8), (4, 8)], 'MAP_SNOWDRIFT_VILLAGE', 2,
         mapsec=MAPSEC_V, group='gMapGroup_TownsAndRoutes',
         objects=[npc('OBJ_EVENT_GFX_MAN_2', 4, 4, f'{CABIN_E}_EventScript_Sculptor', 'MOVEMENT_TYPE_FACE_DOWN')],
         scripts=rf'''
{CABIN_E}_MapScripts::
	.byte 0

{CABIN_E}_EventScript_Sculptor::
	lock
	faceplayer
	goto_if_set FLAG_FROST_SCULPTOR_GIFT, {CABIN_E}_EventScript_SculptorDone
	goto_if_unset FLAG_FROST_ARTICUNO_DONE, {CABIN_E}_EventScript_SculptorAsk
	msgbox {CABIN_E}_Text_SculptorSaw, MSGBOX_DEFAULT
	giveitem ITEM_ICY_ROCK
	setflag FLAG_FROST_SCULPTOR_GIFT
	release
	end

{CABIN_E}_EventScript_SculptorAsk::
	msgbox {CABIN_E}_Text_SculptorAsk, MSGBOX_DEFAULT
	release
	end

{CABIN_E}_EventScript_SculptorDone::
	msgbox {CABIN_E}_Text_SculptorDone, MSGBOX_DEFAULT
	release
	end

{CABIN_E}_Text_SculptorAsk:
	.string "I carve ice sculptures. My dream is\n"
	.string "to carve the bird that roosts by\l"
	.string "the summit cairn.\p"
	.string "If you ever meet it, tell me all\n"
	.string "about it!$"

{CABIN_E}_Text_SculptorSaw:
	.string "You met ARTICUNO? Tell me… wow!\n"
	.string "Wings like that!\p"
	.string "Here, take this ICY ROCK. It keeps\n"
	.string "the cold in. I carve with it.$"

{CABIN_E}_Text_SculptorDone:
	.string "My ARTICUNO sculpture is coming\n"
	.string "along nicely. Just the beak left.$"
''')
print('ok interiors')

# ---------------------------------------------------------------- the way there, from Lilycove City
add_object('LilycoveCity', {'graphics_id': 'OBJ_EVENT_GFX_HIKER', 'x': 20, 'y': 16, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'LilycoveCity_EventScript_FrostcrownGuide', 'flag': '0'})
register_script_file('frostcrown', r'''
@ Mt. Frostcrown guide in Lilycove City (tools/areas/frostcrown.py).
LilycoveCity_EventScript_FrostcrownGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, LilycoveCity_EventScript_FrostcrownNoBadge
	msgbox LilycoveCity_Text_FrostcrownAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LilycoveCity_EventScript_FrostcrownNo
	msgbox LilycoveCity_Text_FrostcrownGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MT_FROSTCROWN, 22, 85
	waitstate
	release
	end

LilycoveCity_EventScript_FrostcrownNoBadge::
	msgbox LilycoveCity_Text_FrostcrownNoBadge, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_FrostcrownNo::
	msgbox LilycoveCity_Text_FrostcrownNo, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Text_FrostcrownNoBadge:
	.string "I'm a ranger on MT. FROSTCROWN, the\n"
	.string "snowy peak north of here.\p"
	.string "The trail's dangerous after the\n"
	.string "blizzard. I'll only take trainers\l"
	.string "with six badges.$"

LilycoveCity_Text_FrostcrownAsk:
	.string "Six badges! Just the trainer we\n"
	.string "need. Climbers went missing on\l"
	.string "MT. FROSTCROWN in the blizzard.\p"
	.string "Will you come up and help look?$"

LilycoveCity_Text_FrostcrownGo:
	.string "Wrap up warm!$"

LilycoveCity_Text_FrostcrownNo:
	.string "If you change your mind, I'll be\n"
	.string "right here.$"
''')
print('ok entrance')
