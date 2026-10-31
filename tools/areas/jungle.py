"""The Verdant River, for after the fifth badge: a jungle river reached from
Route 119, with a stilt village at its mouth, rapids, a hidden lagoon and
the river temple on an island in the lake above the gorge.

The village elder says the temple has been cut off since the old sluice
stuck open: rapids fill the gorge, and a whirling current rings the
temple island. Pull the sluice lever on the lookout above the village and
the gorge calms, so you can surf up to the lake. Pull the second lever on
the lake's west beach and the whirl opens a way in to the island. Keldeo
waits inside the temple, and the elder gives you a Froakie.

New art (rapids in four directions, sluice levers, dock planks, canoes,
stilt huts, totems and the temple) is drawn in tools/areas/jungle_art.py
(gTileset_Jungle: Fortree's tiles plus the art).

    python3 tools/areas/jungle_art.py   # the tileset, when the art changes
    python3 tools/areas/jungle.py
"""
import collections, copy, json, sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import marsh, ruins
from tiles import Pair

AREA_NO = 20
NAME = 'VerdantRiver'
TEMPLE = 'VerdantRiver_Temple'
HUT = 'VerdantRiver_Hut'
MAPSEC = 'MAPSEC_VERDANT_RIVER'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'jungle_tiles.json')))
MT = ART['objects']
EXIT = 0x207

register_constants('Verdant', AREA_NO, flags=[
    'VERDANT_SLUICE', 'VERDANT_WHIRL', 'VERDANT_TEMPLE_REACHED', 'VERDANT_KELDEO_DONE', 'VERDANT_FROAKIE',
    'VERDANT_ELDER_TALKED',
    'ITEM_VERDANT_MYSTIC_WATER', 'ITEM_VERDANT_NET_BALL', 'ITEM_VERDANT_DIVE_BALL', 'ITEM_VERDANT_TM_CALM_MIND',
    'ITEM_VERDANT_HYPER_POTION', 'ITEM_VERDANT_WATER_STONE', 'ITEM_VERDANT_LEAF_STONE', 'ITEM_VERDANT_PP_UP',
    'ITEM_VERDANT_MAX_REVIVE',
    'HIDDEN_ITEM_VERDANT_BIG_PEARL', 'HIDDEN_ITEM_VERDANT_HEART_SCALE', 'HIDDEN_ITEM_VERDANT_NUGGET',
    'HIDDEN_ITEM_VERDANT_STAR_PIECE',
], trainers=['VERDANT_KOBE', 'VERDANT_MAYA', 'VERDANT_REED', 'VERDANT_LIANA', 'VERDANT_ROWAN'])

register_trainers('Verdant', '''
=== AREA_TRAINER_VERDANT_KOBE ===
Name: KOBE
Class: Bug Maniac
Pic: Bug Maniac
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Heracross
Level: 33

Vikavolt
Level: 34

=== AREA_TRAINER_VERDANT_MAYA ===
Name: MAYA
Class: Swimmer F
Pic: Swimmer F
Gender: Female
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Lombre
Level: 33

Azumarill
Level: 34

=== AREA_TRAINER_VERDANT_REED ===
Name: REED
Class: Fisherman
Pic: Fisherman
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Basculin
Level: 33

Whiscash
Level: 34

Barraskewda
Level: 34

=== AREA_TRAINER_VERDANT_LIANA ===
Name: LIANA
Class: Bird Keeper
Pic: Bird Keeper
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Trumbeak
Level: 34

Tropius
Level: 35

=== AREA_TRAINER_VERDANT_ROWAN ===
Name: ROWAN
Class: Swimmer M
Pic: Swimmer M
Gender: Male
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Golduck
Level: 35

Drednaw
Level: 36
''')


def place(a, name, x, y, blocked=()):
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
	trainerbattle_single TRAINER_VERDANT_{key}, {m}_Text_{label}Intro, {m}_Text_{label}Defeat
	msgbox {m}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{m}_Text_{label}Intro:
{intro}
{m}_Text_{label}Defeat:
{defeat}
{m}_Text_{label}Post:
{post}
'''


def current(a, cells, d):
    for x, y in cells:
        a.tile(x, y, one(f'rapids_{d}'), collision=0, elevation=1, c='o')


ENC_LAND = [('AIPOM', 31, 33), ('BRELOOM', 31, 33), ('VIGOROTH', 31, 33), ('TRUMBEAK', 31, 33),
            ('PASSIMIAN', 32, 34), ('ORANGURU', 32, 34), ('YANMA', 32, 34), ('LOMBRE', 32, 34),
            ('TROPIUS', 33, 35), ('HERACROSS', 33, 35), ('PINSIR', 33, 35), ('SEVIPER', 34, 35)]
ENC_WATER = [('LOMBRE', 30, 35), ('POLIWHIRL', 30, 35), ('AZUMARILL', 32, 35), ('BASCULIN', 32, 35),
             ('DREDNAW', 34, 36)]
ENC_FISH = [('MAGIKARP', 10, 20), ('BASCULIN', 15, 25), ('MAGIKARP', 20, 30), ('ARROKUDA', 25, 30),
            ('BARBOACH', 25, 30), ('BASCULIN', 30, 35), ('WHISCASH', 32, 36), ('BARRASKEWDA', 32, 36),
            ('GYARADOS', 33, 37), ('FEEBAS', 33, 36)]

# ---------------------------------------------------------------- the river
W, H = 56, 64
st = copy.copy(marsh())                          # the jungle, with one clean colour of water
_pair = Pair('gTileset_General', 'gTileset_Fortree')
st.exclude = set(st.exclude) | {m for m in range(0x200, 0x200 + 280) if 0x70 <= _pair.attr(m) & 0xFF <= 0x7F} | {0x171}  # no bridges, no deep shade
st.secondary = 'gTileset_Jungle'                 # Fortree's tiles, same metatile ids, plus the art
a = Area(NAME, W, H, st, fill='T')

# the lake and the temple island
a.ellipse(28, 18, 17, 11, '~')
a.ellipse(10, 18, 4, 5, '.')                    # the west beach, only by water
a.rect(19, 10, 4, 14, 'T')                      # the jungle ridge west of the island
a.rect(23, 13, 11, 10, '.')                     # the island
a.ellipse(36, 6, 3, 2, '.')                     # a nook on the north shore
# the gorge, the lagoon and the river mouth
a.rect(26, 28, 5, 18, '~')
a.ellipse(28, 52, 8.5, 7, '~')
a.rect(25, 58, 7, 6, '~')
# the village banks, the lookout path and the hidden lagoon
a.rect(5, 44, 15, 20, '.')
a.rect(37, 52, 14, 12, '.')
a.line([(16, 44), (16, 33)], 3, '.')
a.ellipse(15, 31, 4, 2.5, '.')
a.ellipse(45, 47, 4.5, 2.6, '~')
a.rect(34, 47, 7, 1, '~')
a.rect(34, 49, 7, 1, '~')
a.rect(46, 47, 1, 1, '.')
# tall grass
a.ellipse(9, 52, 3, 2, '"')
a.ellipse(48, 60, 3, 1.5, '"')
a.ellipse(15, 37, 1.5, 3, '"')
a.ellipse(9, 16, 2, 2, '"')
a.snap_trees()

# the whirl round the island: east along the top, south down the east
# side, west along the bottom and out past the ridge
RING_TOP = [(x, 11) for x in range(23, 35)]
RING_EAST = [(35, y) for y in range(11, 24)]
RING_BOTTOM = [(x, 24) for x in range(23, 36)]
current(a, RING_TOP, 'e')
current(a, RING_EAST, 's')
current(a, RING_BOTTOM, 'w')
WHIRL_GATE = (28, 24)                             # turned north by the beach lever
GORGE = [(x, y) for x in range(26, 31) for y in range(31, 43)]
current(a, GORGE, 's')
current(a, [(x, 47) for x in range(35, 41)], 'e')  # into the hidden lagoon
current(a, [(x, 49) for x in range(35, 41)], 'w')  # and back out

TEMPLE_AT = (26, 13)
place(a, 'temple', *TEMPLE_AT)
place(a, 'dock', 28, 22)
place(a, 'totem', 24, 17)
place(a, 'totem', 32, 17)
LEVER_A = (8, 15)                                 # on the west beach
LEVER_B = (15, 29)                                # on the lookout
place(a, 'lever', *LEVER_A)
place(a, 'lever', *LEVER_B)
a.sign(LEVER_A[0], LEVER_A[1] + 1, f'{NAME}_EventScript_LeverWhirl', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(LEVER_B[0], LEVER_B[1] + 1, f'{NAME}_EventScript_LeverSluice', 'BG_EVENT_PLAYER_FACING_NORTH')

# the village
HUTS = [(7, 45), (12, 49), (39, 53), (44, 55)]
for i, (x, y) in enumerate(HUTS):
    place(a, 'hut', x, y, blocked=() if i == 1 else {(2, 1)})
place(a, 'totem', 18, 46)
place(a, 'totem', 38, 59)
place(a, 'canoe', 21, 47)
place(a, 'canoe', 34, 57)
for x in range(20, 37):                           # the footbridge over the river mouth
    a.tile(x, 61, one('dock'), collision=0, c='o')
for x in (18, 19):
    a.tile(x, 56, one('dock'), collision=0, c='o')  # the fishing pier
a.tile(20, 56, one('dock'), collision=0, c='o')

a.warp(HUTS[1][0] + 1, HUTS[1][1] + 2, 'MAP_VERDANT_RIVER_HUT', 0)
a.warp(TEMPLE_AT[0] + 2, TEMPLE_AT[1] + 3, 'MAP_VERDANT_RIVER_TEMPLE', 0)
ARRIVE = (10, 60)
a.obj('OBJ_EVENT_GFX_MAN_4', 9, 60, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_OLD_WOMAN', 10, 48, f'{NAME}_EventScript_Elder', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_NINJA_BOY', 15, 55, f'{NAME}_EventScript_Kid', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_MAN_3', 41, 57, f'{NAME}_EventScript_Builder', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_SPECIES(LOTAD)', 47, 47, f'{NAME}_EventScript_Lotad', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(17, 60, f'{NAME}_EventScript_Sign')
a.trainer('OBJ_EVENT_GFX_FISHERMAN', 19, 56, f'{NAME}_EventScript_Reed', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_BUG_CATCHER', 50, 57, f'{NAME}_EventScript_Kobe', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_PICNICKER', 16, 38, f'{NAME}_EventScript_Liana', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_SWIMMER_F', 31, 52, f'{NAME}_EventScript_Maya', 3, 'MOVEMENT_TYPE_FACE_LEFT', elevation=1)
a.trainer('OBJ_EVENT_GFX_SWIMMER_M', 40, 20, f'{NAME}_EventScript_Rowan', 3, 'MOVEMENT_TYPE_FACE_LEFT', elevation=1)
a.item('ITEM_NET_BALL', 6, 56, 'FLAG_ITEM_VERDANT_NET_BALL')
a.item('ITEM_HYPER_POTION', 49, 61, 'FLAG_ITEM_VERDANT_HYPER_POTION')
a.item('ITEM_LEAF_STONE', 13, 31, 'FLAG_ITEM_VERDANT_LEAF_STONE')
a.item('ITEM_MYSTIC_WATER', 46, 47, 'FLAG_ITEM_VERDANT_MYSTIC_WATER')
a.item('ITEM_WATER_STONE', 37, 6, 'FLAG_ITEM_VERDANT_WATER_STONE')
a.item('ITEM_DIVE_BALL', 11, 20, 'FLAG_ITEM_VERDANT_DIVE_BALL')
a.item('ITEM_PP_UP', 31, 20, 'FLAG_ITEM_VERDANT_PP_UP')
a.hidden(8, 21, 'ITEM_BIG_PEARL', 'FLAG_HIDDEN_ITEM_VERDANT_BIG_PEARL')
a.hidden(24, 21, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_VERDANT_STAR_PIECE')
a.hidden(36, 63 - 4, 'ITEM_HEART_SCALE', 'FLAG_HIDDEN_ITEM_VERDANT_HEART_SCALE')
a.hidden(18, 31, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_VERDANT_NUGGET')

b = a.blocks()
water = collections.Counter(int(b[y, x]) & 0x3FF for x in range(14, 20) for y in range(13, 22)).most_common(1)[0][0]
calm_gorge = f'{NAME}_EventScript_GorgeCalm::\n' + ''.join(f'\tsetmetatile {x}, {y}, {water}, FALSE\n' for x, y in GORGE) + '\treturn\n\n'
open_whirl = f'{NAME}_EventScript_WhirlOpen::\n\tsetmetatile {WHIRL_GATE[0]}, {WHIRL_GATE[1]}, {one("rapids_n")}, FALSE\n\treturn\n\n'
lever_on = MT['lever_on']


def lever_script(label, flag, at, effect, ask, done, already):
    x, y = at
    return rf'''{NAME}_EventScript_LeverOn{label}::
	setmetatile {x}, {y}, {lever_on[0][0]}, TRUE
	setmetatile {x}, {y + 1}, {lever_on[1][0]}, TRUE
	return

{NAME}_EventScript_Lever{label}::
	lockall
	goto_if_set {flag}, {NAME}_EventScript_Lever{label}Done
	msgbox {NAME}_Text_{ask}, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_LeverLeave
	playse SE_PIN
	call {NAME}_EventScript_LeverOn{label}
	special DrawWholeMapView
	delay 30
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_WATERFALL
	call {NAME}_EventScript_{effect}
	special DrawWholeMapView
	delay 40
	setflag {flag}
	msgbox {NAME}_Text_{done}, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_Lever{label}Done::
	msgbox {NAME}_Text_{already}, MSGBOX_DEFAULT
	releaseall
	end

'''


a.write(blocks=b, map_id='MAP_VERDANT_RIVER', layout_id='LAYOUT_VERDANT_RIVER', music='MUS_ROUTE119', mapsec=MAPSEC,
        mapsec_name='VERDANT RIVER', weather='WEATHER_NONE', after='MirageOasis_SunkenCourt', scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	.byte 0

{NAME}_OnLoad:
	call_if_set FLAG_VERDANT_SLUICE, {NAME}_EventScript_LeverOnSluice
	call_if_set FLAG_VERDANT_SLUICE, {NAME}_EventScript_GorgeCalm
	call_if_set FLAG_VERDANT_WHIRL, {NAME}_EventScript_LeverOnWhirl
	call_if_set FLAG_VERDANT_WHIRL, {NAME}_EventScript_WhirlOpen
	end

{calm_gorge}{open_whirl}{lever_script('Sluice', 'FLAG_VERDANT_SLUICE', LEVER_B, 'GorgeCalm', 'SluiceAsk', 'SluiceDone', 'SluiceAlready')}{lever_script('Whirl', 'FLAG_VERDANT_WHIRL', LEVER_A, 'WhirlOpen', 'WhirlAsk', 'WhirlDone', 'WhirlAlready')}{NAME}_EventScript_LeverLeave::
	releaseall
	end

{NAME}_EventScript_Guide::
	lock
	faceplayer
	msgbox {NAME}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GuideStay
	msgbox {NAME}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE119, 13, 132
	waitstate
	release
	end

{NAME}_EventScript_GuideStay::
	msgbox {NAME}_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Sign::
	msgbox {NAME}_Text_Sign, MSGBOX_SIGN
	end

{NAME}_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_VERDANT_FROAKIE, {NAME}_EventScript_ElderDone
	goto_if_set FLAG_VERDANT_TEMPLE_REACHED, {NAME}_EventScript_ElderReward
	goto_if_set FLAG_VERDANT_SLUICE, {NAME}_EventScript_ElderSluice
	goto_if_set FLAG_VERDANT_ELDER_TALKED, {NAME}_EventScript_ElderAgain
	setflag FLAG_VERDANT_ELDER_TALKED
	msgbox {NAME}_Text_Elder, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ElderAgain::
	msgbox {NAME}_Text_ElderAgain, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ElderSluice::
	msgbox {NAME}_Text_ElderSluice, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ElderReward::
	msgbox {NAME}_Text_ElderThanks, MSGBOX_DEFAULT
	givemon SPECIES_FROAKIE, 30
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {NAME}_EventScript_ElderFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotFroakie, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_VERDANT_FROAKIE
	msgbox {NAME}_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ElderFull::
	msgbox {NAME}_Text_ElderFull, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ElderDone::
	msgbox {NAME}_Text_ElderDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Kid::
	msgbox {NAME}_Text_Kid, MSGBOX_NPC
	end

{NAME}_EventScript_Builder::
	msgbox {NAME}_Text_Builder, MSGBOX_NPC
	end

{NAME}_EventScript_Lotad::
	lock
	faceplayer
	playmoncry SPECIES_LOTAD, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Lotad, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{trainer_script(NAME, 'REED', text("The fish bite best where the river", "meets the lagoon. And so do I!"),
                text("Off the hook…"), text("Ride the current east of the lagoon.", "Nobody fishes the pool behind it."))}
{trainer_script(NAME, 'KOBE', text("The jungle's full of bugs, and I've", "caught the best of them!"),
                text("Squashed…"), text("HERACROSS love the sap of the trees", "by the river."))}
{trainer_script(NAME, 'LIANA', text("From up here I watch the birds fly", "over the gorge. Want to see mine?"),
                text("Grounded!"), text("The old sluice lever is up ahead.", "Nobody's touched it in years."))}
{trainer_script(NAME, 'MAYA', text("Swimming in the lagoon is the best!", "Battling in it is even better!"),
                text("I'm all washed up."), text("The currents carry you, so don't", "fight them. Just ride along."))}
{trainer_script(NAME, 'ROWAN', text("I swim laps round the lake. Don't get", "caught in the whirl like I did!"),
                text("Swept away…"), text("The whirl always throws you out to", "the west. Look for a lever there."))}
{NAME}_Text_SluiceAsk:
{text("An old sluice lever. Below the", "lookout, the gorge roars with", "rapids.", "", "Pull the lever?")}
{NAME}_Text_SluiceDone:
{text("The sluice gate creaked shut!", "", "The rapids in the gorge calmed.", "You can surf up to the lake now!")}
{NAME}_Text_SluiceAlready:
{text("The lever is pulled down. The gorge", "flows gently.")}
{NAME}_Text_WhirlAsk:
{text("Another old lever, half buried in", "the sand. Out in the lake, the whirl", "spins round the temple island.", "", "Pull the lever?")}
{NAME}_Text_WhirlDone:
{text("Somewhere under the lake, a gate", "groaned open!", "", "The whirl now turns in toward the", "temple island's dock.")}
{NAME}_Text_WhirlAlready:
{text("The lever is pulled down. The whirl", "turns in toward the island.")}
{NAME}_Text_GuideBack:
{text("I paddle travellers up and down the", "river. Back to ROUTE 119?")}
{NAME}_Text_GuideLead:
{text("Hop in the canoe!")}
{NAME}_Text_GuideStay:
{text("Enjoy the river!")}
{NAME}_Text_Sign:
{text("VERDANT RIVER VILLAGE", "Where the river meets the sea of trees.")}
{NAME}_Text_Elder:
{text("Welcome to our village, traveller.", "", "Up the gorge lies a lake, and on its", "island stands the river temple. Its", "guardian keeps the water clean.", "", "But years ago the old sluice stuck", "open. Rapids fill the gorge and a", "whirl rings the island. No one has", "reached the temple since.", "", "The sluice lever is on the lookout,", "up the path behind my hut.")}
{NAME}_Text_ElderAgain:
{text("The sluice lever is on the lookout,", "up the path behind my hut. Pull it", "and the gorge will calm.")}
{NAME}_Text_ElderSluice:
{text("The gorge is calm! You can surf up", "to the lake now.", "", "But beware the whirl round the", "island. They say another lever once", "turned it.")}
{NAME}_Text_ElderThanks:
{text("You reached the temple! The guardian", "has woken, I can feel it. The river", "runs clearer already.", "", "Please take this FROAKIE. It grew up", "in our lagoon, and it wants to see", "the world.")}
{NAME}_Text_GotFroakie:
{text("{PLAYER} received a FROAKIE!")}
{NAME}_Text_ElderFull:
{text("Your party is full. Come back for", "FROAKIE when there's room.")}
{NAME}_Text_ElderDone:
{text("The river sings again. Thank you,", "traveller.")}
{NAME}_Text_Kid:
{text("I tried to swim up the gorge once.", "The rapids pushed me right back down!")}
{NAME}_Text_Builder:
{text("I'm building canoes. That one's", "nearly done. It just needs a hole", "in the right place. Or not.")}
{NAME}_Text_Lotad:
{text("LOTAD: Lo-lo!", "", "It's floating happily in the hidden", "lagoon.")}
''')
print('ok', NAME)
register_wild('MAP_VERDANT_RIVER', 'gVerdantRiver', land=ENC_LAND, water=ENC_WATER, fish=ENC_FISH, land_rate=15)

# ---------------------------------------------------------------- the temple
t = Area(TEMPLE, 22, 18, ruins(), fill='#')
t.rect(3, 3, 16, 12, '.')
t.rect(9, 14, 4, 3, '.')
t.tile(10, 16, EXIT, collision=0, elevation=3, c='o')
t.tile(11, 16, EXIT, collision=0, elevation=3, c='o')
t.warp(10, 16, 'MAP_VERDANT_RIVER', 1)
t.warp(11, 16, 'MAP_VERDANT_RIVER', 1)
t.obj('OBJ_EVENT_GFX_SPECIES(KELDEO)', 11, 5, f'{TEMPLE}_EventScript_Keldeo', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_VERDANT_KELDEO_DONE')
t.item('ITEM_TM_CALM_MIND', 4, 4, 'FLAG_ITEM_VERDANT_TM_CALM_MIND')
t.item('ITEM_MAX_REVIVE', 17, 4, 'FLAG_ITEM_VERDANT_MAX_REVIVE')
for x, y in ((5, 8), (16, 8)):
    t.stamp('LAYOUT_SEALED_CHAMBER_OUTER_ROOM', 15, 5, 3, 2, x - 1, y)
t.write(map_id='MAP_VERDANT_RIVER_TEMPLE', layout_id='LAYOUT_VERDANT_RIVER_TEMPLE', music='MUS_CAVE_OF_ORIGIN', mapsec=MAPSEC, mapsec_name='VERDANT RIVER', map_type='MAP_TYPE_INDOOR',
        after=NAME, show_name=False, allow_cycling=False, scripts=rf'''
{TEMPLE}_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, {TEMPLE}_OnTransition
	.byte 0

{TEMPLE}_OnTransition:
	setflag FLAG_VERDANT_TEMPLE_REACHED
	end

{TEMPLE}_EventScript_Keldeo::
	lock
	faceplayer
	msgbox {TEMPLE}_Text_Sees, MSGBOX_DEFAULT
	setwildbattle SPECIES_KELDEO, 40
	playmoncry SPECIES_KELDEO, CRY_MODE_ENCOUNTER
	msgbox {TEMPLE}_Text_Keldeo, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {TEMPLE}_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {TEMPLE}_EventScript_Stays
	setflag FLAG_VERDANT_KELDEO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {TEMPLE}_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {TEMPLE}_Text_Gone, MSGBOX_DEFAULT
{TEMPLE}_EventScript_End::
	release
	end

{TEMPLE}_EventScript_Stays::
	msgbox {TEMPLE}_Text_Stays, MSGBOX_DEFAULT
	release
	end

{TEMPLE}_Text_Sees:
{text("A young POKéMON stands proudly in", "the temple, its horn shining like", "a blade of water.")}
{TEMPLE}_Text_Keldeo:
{text("Kyuuun!")}
{TEMPLE}_Text_Gone:
{text("KELDEO bounded away across the lake,", "running on the water.")}
{TEMPLE}_Text_Stays:
{text("KELDEO is still standing guard in", "the temple.")}
''')
print('ok', TEMPLE)

# ---------------------------------------------------------------- the healer's hut
interior(HUT, 'MAP_VERDANT_RIVER_HUT', 'LAYOUT_HOUSE3', [(3, 7), (4, 7)], 'MAP_VERDANT_RIVER', 0,
         mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes', music='MUS_ROUTE119',
         objects=[npc('OBJ_EVENT_GFX_WOMAN_5', 6, 3, f'{HUT}_EventScript_Healer', 'MOVEMENT_TYPE_FACE_DOWN')],
         scripts=rf'''
{HUT}_MapScripts::
	.byte 0

{HUT}_EventScript_Healer::
	lock
	faceplayer
	msgbox {HUT}_Text_Healer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {HUT}_EventScript_HealerNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox {HUT}_Text_HealerDone, MSGBOX_DEFAULT
	release
	end

{HUT}_EventScript_HealerNo::
	msgbox {HUT}_Text_HealerNo, MSGBOX_DEFAULT
	release
	end

{HUT}_Text_Healer:
{text("Tired from the river? My herbal tea", "will set your POKéMON right. Rest", "a while?")}
{HUT}_Text_HealerDone:
{text("There! Good as new.")}
{HUT}_Text_HealerNo:
{text("The kettle's always on.")}
''')
print('ok', HUT)

# ---------------------------------------------------------------- the way there, from Route 119
add_object('Route119', {'graphics_id': 'OBJ_EVENT_GFX_MAN_4', 'x': 12, 'y': 132, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': 'Route119_EventScript_VerdantGuide', 'flag': '0'})
register_script_file('jungle', rf'''
@ Verdant River canoe guide on Route 119 (tools/areas/jungle.py).
Route119_EventScript_VerdantGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, Route119_EventScript_VerdantNoBadge
	msgbox Route119_Text_VerdantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_EventScript_VerdantNo
	msgbox Route119_Text_VerdantGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_VERDANT_RIVER, {ARRIVE[0]}, {ARRIVE[1]}
	waitstate
	release
	end

Route119_EventScript_VerdantNoBadge::
	msgbox Route119_Text_VerdantNoBadge, MSGBOX_DEFAULT
	release
	end

Route119_EventScript_VerdantNo::
	msgbox Route119_Text_VerdantNo, MSGBOX_DEFAULT
	release
	end

Route119_Text_VerdantNoBadge:
{text("I paddle travellers to the VERDANT", "RIVER, deep in the jungle.", "", "You'll need to SURF up there. Come", "back once you have five badges.")}
Route119_Text_VerdantAsk:
{text("Five badges! The VERDANT RIVER", "village could use a trainer who", "can SURF.", "", "Shall I paddle you there?")}
Route119_Text_VerdantGo:
{text("Keep your hands inside the canoe!")}
Route119_Text_VerdantNo:
{text("The river will wait.")}
''')
print('ok entrance')
