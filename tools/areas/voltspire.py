"""Voltspire Power Plant: Mauville's power plant, for after the third badge.
A Rotom has got into the wiring and tripped the main generator. Three
breaker panels (yellow, blue and red) each raise and drop the barriers
between the halls; work out the right combination to reach the generator
room and restart it. The Rotom itself hides in the break room's
appliances, hopping to another one every time you look in the wrong place.

New art (breakers, appliances, coils) is drawn in tools/areas/plant_art.py
(gTileset_Plant: the New Mauville tiles plus the art); the halls are learned
from New Mauville by styles.plant(), and the generator is New Mauville's.

    python3 tools/areas/plant_art.py     # the tileset, when the art changes
    python3 tools/areas/voltspire.py
"""
import json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import plant

AREA_NO = 13
NAME = 'VoltspirePlant'
MAPSEC = 'MAPSEC_VOLTSPIRE_PLANT'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'plant_tiles.json')))
MT = ART['objects']

register_constants(NAME, AREA_NO, flags=[
    'VOLT_BREAKER_A', 'VOLT_BREAKER_B', 'VOLT_BREAKER_C', 'VOLT_POWER_ON', 'VOLT_REWARD', 'VOLT_ROTOM_DONE',
    'ITEM_VOLT_MAGNET', 'ITEM_VOLT_ELECTIRIZER', 'ITEM_VOLT_THUNDER_STONE', 'ITEM_VOLT_SUPER_REPEL',
    'ITEM_VOLT_HYPER_POTION', 'HIDDEN_ITEM_VOLT_CELL_BATTERY', 'HIDDEN_ITEM_VOLT_PP_UP', 'VOLT_CHIEF_TALKED',
], trainers=['VOLT_ZAPP', 'VOLT_OHM', 'VOLT_AMPERE'])

register_trainers(NAME, '''
=== AREA_TRAINER_VOLT_ZAPP ===
Name: ZAPP
Class: Guitarist
Pic: Guitarist
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Electrike
Level: 23

Voltorb
Level: 23

Pikachu
Level: 24

=== AREA_TRAINER_VOLT_OHM ===
Name: OHM
Class: Pokemaniac
Pic: Pokemaniac
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Magnemite
Level: 24

Joltik
Level: 24

Tynamo
Level: 25

=== AREA_TRAINER_VOLT_AMPERE ===
Name: AMPERE
Class: Expert
Pic: Expert M
Gender: Male
Music: Intense
Double Battle: No
AI: Smart Trainer

Elekid
Level: 25

Blitzle
Level: 25

Magneton
Level: 27
''')


def place(a, name, x, y):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


# New Mauville's barriers: 4 tall, standing in a 2-tall corridor under a
# 2-tall wall (Top, TopMid over the wall; BottomMid, Bottom on the floor).
BARRIER = {'green': (0x2B6, 0x2BE, 0x2C6, 0x2CE), 'blue': (0x2B7, 0x2BF, 0x2C7, 0x2CF)}
HIDDEN = ((0x269, 'TRUE'), (0x271, 'TRUE'), (0x26D, 'FALSE'), (0x281, 'FALSE'))
GEN_OFF = [0x2F0, 0x2F1, 0x2F2, 0x2F3, 0x2F4, 0x2F5, 0x2F6, 0x2F7]
GEN_ON = [0x2D8, 0x2D9, 0x2DA, 0x2DB, 0x2E0, 0x2E1, 0x2E2, 0x2E3]

st = plant()
W, H = 44, 34
a = Area(NAME, W, H, st, fill='#')
ROOMS = [(2, 4, 12, 26),                         # the coil hall (west)
         (30, 4, 12, 26),                        # the break room wing (east)
         (16, 4, 12, 7),                         # the generator room (north)
         (16, 20, 12, 10)]                       # the lobby (south)
for r in ROOMS:
    a.rect(*r, '.')
# doors: (x, top y of the barrier, colour, condition [(flag, must be set)])
DOORS = [(14, 22, 'green', [('A', True)]),                       # lobby <-> coil hall
         (29, 22, 'green', [('A', False)]),                      # lobby <-> break room wing
         (14, 5, 'blue', [('B', False), ('C', True)]),           # coil hall <-> generator
         (29, 5, 'blue', [('B', True), ('C', False)])]           # break room wing <-> generator
for x, y, _, _ in DOORS:
    a.rect(14 if x == 14 else 28, y + 2, 2, 2, '.')
for i, (x, y, col, _) in enumerate(DOORS):
    for k, m in enumerate(BARRIER[col]):
        a.tile(x, y + k, m, collision=1, elevation=0 if k < 2 else 3, c='o')

GEN = (18, 2)                                    # New Mauville's generator and its machines, 8x5
a.stamp('LAYOUT_NEW_MAUVILLE_INSIDE', 32, 0, 8, 5, *GEN)
GEN_OFF_AT = [(GEN[0] + i % 4, GEN[1] + 2 + i // 4) for i in range(8)]
a.tile(21, 29, 0x218, collision=0, elevation=3, c='o')   # the exit mat
a.tile(22, 29, 0x220, collision=0, elevation=3, c='o')

BREAKERS = [('A', 'yellow', 19, 18), ('B', 'blue', 6, 2), ('C', 'red', 37, 2)]   # 1x2, on the wall
for key, col, x, y in BREAKERS:
    place(a, f'breaker_{col}_off', x, y)
    a.sign(x, y + 1, f'{NAME}_EventScript_Breaker{key}')
APPLIANCES = [('tv', 31), ('fridge', 33), ('washer', 35), ('fan', 37), ('mower', 39)]
for i, (kind, x) in enumerate(APPLIANCES):
    place(a, kind, x, 25)
    a.sign(x, 26, f'{NAME}_EventScript_Appliance{i}')
for x, y in ((4, 10), (11, 10), (4, 16), (11, 16), (4, 22), (11, 22)):
    place(a, 'coil', x, y)

MAUVILLE_WARP = add_warp('MauvilleCity', {'x': 33, 'y': 9, 'elevation': 0, 'dest_map': 'MAP_VOLTSPIRE_PLANT',
                                           'dest_warp_id': '0'})        # where the exit mat drops you
a.warp(21, 29, 'MAP_MAUVILLE_CITY', MAUVILLE_WARP)
a.warp(22, 29, 'MAP_MAUVILLE_CITY', MAUVILLE_WARP)
a.obj('OBJ_EVENT_GFX_SCIENTIST_1', 23, 26, f'{NAME}_EventScript_Chief', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WORKER_M', 17, 23, f'{NAME}_EventScript_Worker', 'MOVEMENT_TYPE_WANDER_AROUND', rx=1, ry=1)
a.obj('OBJ_EVENT_GFX_WORKER_F', 34, 20, f'{NAME}_EventScript_WorkerF', 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_MAN_5', 8, 13, f'{NAME}_EventScript_Zapp', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_MANIAC', 35, 11, f'{NAME}_EventScript_Ohm', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_EXPERT_M', 26, 8, f'{NAME}_EventScript_Ampere', 2, 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_MAGNET', 3, 28, 'FLAG_ITEM_VOLT_MAGNET')
a.item('ITEM_ELECTIRIZER', 16, 9, 'FLAG_ITEM_VOLT_ELECTIRIZER')
a.item('ITEM_THUNDER_STONE', 40, 5, 'FLAG_ITEM_VOLT_THUNDER_STONE')
a.item('ITEM_SUPER_REPEL', 12, 5, 'FLAG_ITEM_VOLT_SUPER_REPEL')
a.item('ITEM_HYPER_POTION', 31, 16, 'FLAG_ITEM_VOLT_HYPER_POTION')
a.hidden(8, 25, 'ITEM_CELL_BATTERY', 'FLAG_HIDDEN_ITEM_VOLT_CELL_BATTERY')
a.hidden(40, 28, 'ITEM_PP_UP', 'FLAG_HIDDEN_ITEM_VOLT_PP_UP')
a.sign(19, 6, f'{NAME}_EventScript_Generator')
a.sign(20, 6, f'{NAME}_EventScript_Generator')


def door_scripts():
    out = f'{NAME}_EventScript_Refresh::\n' + ''.join(f'\tcall {NAME}_EventScript_Door{i}\n' for i in range(len(DOORS)))
    out += f'\tcall {NAME}_EventScript_Panels\n\treturn\n\n'
    for i, (x, y, col, cond) in enumerate(DOORS):
        out += f'{NAME}_EventScript_Door{i}::\n'
        for key, need in cond:
            out += f'\tgoto_if_{"unset" if need else "set"} FLAG_VOLT_BREAKER_{key}, {NAME}_EventScript_Door{i}Shut\n'
        out += ''.join(f'\tsetmetatile {x}, {y + k}, {m}, {c}\n' for k, (m, c) in enumerate(HIDDEN)) + '\treturn\n\n'
        out += f'{NAME}_EventScript_Door{i}Shut::\n'
        out += ''.join(f'\tsetmetatile {x}, {y + k}, {m}, TRUE\n' for k, m in enumerate(BARRIER[col])) + '\treturn\n\n'
    out += f'{NAME}_EventScript_Panels::\n'
    for key, col, x, y in BREAKERS:
        out += f'\tcall_if_set FLAG_VOLT_BREAKER_{key}, {NAME}_EventScript_Panel{key}On\n'
        out += f'\tcall_if_unset FLAG_VOLT_BREAKER_{key}, {NAME}_EventScript_Panel{key}Off\n'
    out += '\treturn\n\n'
    for key, col, x, y in BREAKERS:
        for state in ('on', 'off'):
            mts = MT[f'breaker_{col}_{state}']
            out += f'{NAME}_EventScript_Panel{key}{state.title()}::\n'
            out += f'\tsetmetatile {x}, {y}, {mts[0][0]}, TRUE\n\tsetmetatile {x}, {y + 1}, {mts[1][0]}, TRUE\n\treturn\n\n'
        out += rf'''{NAME}_EventScript_Breaker{key}::
	lockall
	msgbox {NAME}_Text_Breaker{key}, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_BreakerLeave
	playse SE_SWITCH
	goto_if_set FLAG_VOLT_BREAKER_{key}, {NAME}_EventScript_Breaker{key}Down
	setflag FLAG_VOLT_BREAKER_{key}
	goto {NAME}_EventScript_BreakerFlipped

{NAME}_EventScript_Breaker{key}Down::
	clearflag FLAG_VOLT_BREAKER_{key}
	goto {NAME}_EventScript_BreakerFlipped

'''
    out += rf'''{NAME}_EventScript_BreakerFlipped::
	call {NAME}_EventScript_Refresh
	special DrawWholeMapView
	waitse
	playse SE_THUNDERSTORM_STOP
	msgbox {NAME}_Text_BreakerFlipped, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_BreakerLeave::
	releaseall
	end

'''
    return out


def appliance_scripts():
    out = ''
    for i, (kind, x) in enumerate(APPLIANCES):
        out += rf'''{NAME}_EventScript_Appliance{i}::
	lockall
	goto_if_set FLAG_VOLT_ROTOM_DONE, {NAME}_EventScript_Appliance{i}Quiet
	goto_if_eq VAR_TEMP_1, {i}, {NAME}_EventScript_RotomFound
	msgbox {NAME}_Text_Appliance{i}, MSGBOX_DEFAULT
	goto {NAME}_EventScript_RotomHops

{NAME}_EventScript_Appliance{i}Quiet::
	msgbox {NAME}_Text_Appliance{i}Quiet, MSGBOX_DEFAULT
	releaseall
	end

'''
    return out


APPLIANCE_TEXT = {
    'tv': ('The TV flickers on by itself… and', 'snaps off again.'),
    'fridge': ('The fridge door rattles. A cold draft', 'and a spark escape from inside!'),
    'washer': ('The washing machine starts spinning', 'all on its own… then stops dead.'),
    'fan': ('The fan whirs to full speed for a', 'moment, then winds down.'),
    'mower': ('The lawn mower coughs, sputters', 'and goes quiet.'),
}
QUIET = {'tv': "It's an old TV. The picture is fuzzy.", 'fridge': "It's a fridge full of SODA POP.",
         'washer': "It's a washing machine. Someone's", 'fan': "It's an electric fan. It's off.",
         'mower': "It's a lawn mower. Why is it here?"}

texts = ''
for i, (kind, x) in enumerate(APPLIANCES):
    l1, l2 = APPLIANCE_TEXT[kind]
    texts += f'{NAME}_Text_Appliance{i}:\n\t.string "{l1}\\n"\n\t.string "{l2}$"\n\n'
    q = QUIET[kind]
    texts += f'{NAME}_Text_Appliance{i}Quiet:\n\t.string "{q}' + ('\\n"\n\t.string "uniform is in the wash.$"\n\n' if kind == 'washer' else '$"\n\n')

a.write(map_id='MAP_VOLTSPIRE_PLANT', layout_id='LAYOUT_VOLTSPIRE_PLANT', music='MUS_MT_PYRE', mapsec=MAPSEC,
        mapsec_name='VOLTSPIRE PLANT', map_type='MAP_TYPE_INDOOR', after='CobblerockMine_OldShaft',
        allow_cycling=False, allow_escaping=True, scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
	.byte 0

{NAME}_OnLoad:
	call {NAME}_EventScript_Refresh
	call_if_unset FLAG_VOLT_POWER_ON, {NAME}_EventScript_GeneratorOff
	end

@ the Rotom hides in one of the five appliances
{NAME}_OnTransition:
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	end

{NAME}_EventScript_GeneratorOff::
''' + ''.join(f'\tsetmetatile {x}, {y}, {m}, TRUE\n' for (x, y), m in zip(GEN_OFF_AT, GEN_OFF)) + rf'''	return

{door_scripts()}
@ ---- the generator
{NAME}_EventScript_Generator::
	lockall
	goto_if_set FLAG_VOLT_POWER_ON, {NAME}_EventScript_GeneratorHums
	msgbox {NAME}_Text_GeneratorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_BreakerLeave
	setflag FLAG_VOLT_POWER_ON
	playse SE_THUNDER
''' + ''.join(f'\tsetmetatile {x}, {y}, {m}, TRUE\n' for (x, y), m in zip(GEN_OFF_AT, GEN_ON)) + rf'''	special DrawWholeMapView
	waitse
	msgbox {NAME}_Text_GeneratorOn, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_GeneratorHums::
	msgbox {NAME}_Text_GeneratorHums, MSGBOX_DEFAULT
	releaseall
	end

@ ---- the Rotom in the break room
{appliance_scripts()}
{NAME}_EventScript_RotomHops::
	playse SE_M_THUNDERBOLT2
	msgbox {NAME}_Text_RotomHops, MSGBOX_DEFAULT
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	releaseall
	end

{NAME}_EventScript_RotomFound::
	msgbox {NAME}_Text_RotomFound, MSGBOX_DEFAULT
	setwildbattle SPECIES_ROTOM, 25
	playmoncry SPECIES_ROTOM, CRY_MODE_ENCOUNTER
	msgbox {NAME}_Text_Rotom, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {NAME}_EventScript_RotomStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {NAME}_EventScript_RotomStays
	setflag FLAG_VOLT_ROTOM_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {NAME}_EventScript_RotomEnd
	msgbox {NAME}_Text_RotomGone, MSGBOX_DEFAULT
{NAME}_EventScript_RotomEnd::
	releaseall
	end

{NAME}_EventScript_RotomStays::
	msgbox {NAME}_Text_RotomStays, MSGBOX_DEFAULT
	random 5
	copyvar VAR_TEMP_1, VAR_RESULT
	releaseall
	end

@ ---- people
{NAME}_EventScript_Chief::
	lock
	faceplayer
	goto_if_set FLAG_VOLT_REWARD, {NAME}_EventScript_ChiefDone
	goto_if_set FLAG_VOLT_POWER_ON, {NAME}_EventScript_ChiefThanks
	msgbox {NAME}_Text_ChiefAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ChiefThanks::
	msgbox {NAME}_Text_ChiefThanks, MSGBOX_DEFAULT
	giveitem ITEM_TM_SHOCK_WAVE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_VOLT_REWARD
	msgbox {NAME}_Text_ChiefAfter, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_ChiefDone::
	msgbox {NAME}_Text_ChiefDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Worker::
	msgbox {NAME}_Text_Worker, MSGBOX_NPC
	end

{NAME}_EventScript_WorkerF::
	msgbox {NAME}_Text_WorkerF, MSGBOX_NPC
	end

{NAME}_EventScript_Zapp::
	trainerbattle_single TRAINER_VOLT_ZAPP, {NAME}_Text_ZappIntro, {NAME}_Text_ZappDefeat
	msgbox {NAME}_Text_ZappPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Ohm::
	trainerbattle_single TRAINER_VOLT_OHM, {NAME}_Text_OhmIntro, {NAME}_Text_OhmDefeat
	msgbox {NAME}_Text_OhmPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Ampere::
	trainerbattle_single TRAINER_VOLT_AMPERE, {NAME}_Text_AmpereIntro, {NAME}_Text_AmpereDefeat
	msgbox {NAME}_Text_AmperePost, MSGBOX_AUTOCLOSE
	end

{NAME}_Text_BreakerA:
	.string "A breaker panel with a YELLOW band.\n"
	.string "Flip the lever?$"

{NAME}_Text_BreakerB:
	.string "A breaker panel with a BLUE band.\n"
	.string "Flip the lever?$"

{NAME}_Text_BreakerC:
	.string "A breaker panel with a RED band.\n"
	.string "Flip the lever?$"

{NAME}_Text_BreakerFlipped:
	.string "Clunk! Somewhere in the plant, the\n"
	.string "barriers hummed and shifted.$"

{NAME}_Text_GeneratorAsk:
	.string "The main generator is dead quiet.\n"
	.string "Someone has pulled every plug.\p"
	.string "Plug it back in and restart it?$"

{NAME}_Text_GeneratorOn:
	.string "Whirrr… VRMMMM!\p"
	.string "The generator roared back to life!\n"
	.string "The whole plant lights up!$"

{NAME}_Text_GeneratorHums:
	.string "The generator hums steadily. All of\n"
	.string "MAUVILLE has power again.$"

{NAME}_Text_RotomHops:
	.string "Bzzt! Something zipped out and\n"
	.string "into another appliance!$"

{NAME}_Text_RotomFound:
	.string "Something in there is giggling…\n"
	.string "A POKéMON made of electricity!$"

{NAME}_Text_Rotom:
	.string "Rotom-rotom!$"

{NAME}_Text_RotomGone:
	.string "ROTOM zipped off into the wiring.\n"
	.string "The appliances are quiet now.$"

{NAME}_Text_RotomStays:
	.string "ROTOM giggled and dove into\n"
	.string "another appliance!$"

{texts}{NAME}_Text_ChiefAsk:
	.string "You're the trainer WATTSON sent? Good.\p"
	.string "Something got into the wiring and\n"
	.string "tripped the main generator. Now the\l"
	.string "barriers between the halls are\l"
	.string "jammed every which way.\p"
	.string "Three breakers, yellow, blue and\n"
	.string "red, move the barriers. Find a way\l"
	.string "to the generator room up north!$"

{NAME}_Text_ChiefThanks:
	.string "The lights are back! You did it!\p"
	.string "The workers say a ROTOM was behind\n"
	.string "it all. Ha! Here, this is for you.$"

{NAME}_Text_ChiefAfter:
	.string "SHOCK WAVE never misses. A fitting\n"
	.string "move for a trainer this sharp!$"

{NAME}_Text_ChiefDone:
	.string "MAUVILLE runs on this plant. Thanks\n"
	.string "to you, the lights stay on.$"

{NAME}_Text_Worker:
	.string "The yellow breaker swaps the two\n"
	.string "lobby doors. The others… I forget!$"

{NAME}_Text_WorkerF:
	.string "Our break room appliances keep\n"
	.string "turning on by themselves. Spooky!$"

{NAME}_Text_ZappIntro:
	.string "This hall's got the best acoustics!\n"
	.string "Let's make some noise!$"

{NAME}_Text_ZappDefeat:
	.string "You blew my fuse!$"

{NAME}_Text_ZappPost:
	.string "The blue breaker is in this hall.\n"
	.string "Try it, see what happens.$"

{NAME}_Text_OhmIntro:
	.string "I come here to see rare electric\n"
	.string "POKéMON. Are yours rare?$"

{NAME}_Text_OhmDefeat:
	.string "Shocking!$"

{NAME}_Text_OhmPost:
	.string "I heard giggling in the break room.\n"
	.string "Something's hiding in there!$"

{NAME}_Text_AmpereIntro:
	.string "You found the way through? Then\n"
	.string "you're worth a real battle!$"

{NAME}_Text_AmpereDefeat:
	.string "Overloaded!$"

{NAME}_Text_AmperePost:
	.string "Go on, the generator's right there.\n"
	.string "Plug it in and give it a kick!$"
''')
print('ok', NAME)

register_wild('MAP_VOLTSPIRE_PLANT', 'gVoltspirePlant',
              land=[('MAGNEMITE', 22, 24), ('VOLTORB', 22, 24), ('ELECTRIKE', 22, 24), ('PICHU', 22, 24),
                    ('SHINX', 22, 24), ('JOLTIK', 22, 24), ('BLITZLE', 23, 25), ('TYNAMO', 23, 25),
                    ('PLUSLE', 23, 25), ('MINUN', 23, 25), ('ELEKID', 24, 26), ('KLINK', 24, 26)], land_rate=8)

# ---------------------------------------------------------------- the way there, from Mauville City
add_object('MauvilleCity', {'graphics_id': 'OBJ_EVENT_GFX_WORKER_M', 'x': 33, 'y': 8, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'MauvilleCity_EventScript_VoltspireGuide', 'flag': '0'})
register_script_file('voltspire', r'''
@ Voltspire Power Plant guide in Mauville City (tools/areas/voltspire.py).
MauvilleCity_EventScript_VoltspireGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, MauvilleCity_EventScript_VoltspireNoBadge
	msgbox MauvilleCity_Text_VoltspireAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_EventScript_VoltspireNo
	msgbox MauvilleCity_Text_VoltspireGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_VOLTSPIRE_PLANT, 21, 28
	waitstate
	release
	end

MauvilleCity_EventScript_VoltspireNoBadge::
	msgbox MauvilleCity_Text_VoltspireNoBadge, MSGBOX_DEFAULT
	release
	end

MauvilleCity_EventScript_VoltspireNo::
	msgbox MauvilleCity_Text_VoltspireNo, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Text_VoltspireNoBadge:
	.string "I work at VOLTSPIRE POWER PLANT.\n"
	.string "Something's gone haywire in there.\p"
	.string "WATTSON wants a trainer with his\n"
	.string "badge to help. Have you beaten him?$"

MauvilleCity_Text_VoltspireAsk:
	.string "That's WATTSON's badge! Please, the\n"
	.string "power plant needs your help.\p"
	.string "Will you come to VOLTSPIRE?$"

MauvilleCity_Text_VoltspireGo:
	.string "Thank you! This way!$"

MauvilleCity_Text_VoltspireNo:
	.string "Please hurry… the lights in town\n"
	.string "keep flickering.$"
''')
print('ok entrance')
