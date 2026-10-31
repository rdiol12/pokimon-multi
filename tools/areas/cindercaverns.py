"""Cinder Caverns: a volcano dungeon under Ember Springs. Lava rivers on 1F,
a lava lake on B1F crossed by cooling it with three old valves, and the
Molten Heart where Heatran sleeps (after the Pokémon League).

    python3 tools/areas/cindercaverns.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import magma

AREA_NO = 2
MAPSEC = 'MAPSEC_CINDER_CAVERNS'
COOLED = 0x308          # cave floor
VALVE = 0x35C           # an old Magma machine, used as a valve

register_constants('CinderCaverns', AREA_NO, flags=[
    'CINDER_VALVE_WEST', 'CINDER_VALVE_EAST', 'CINDER_VALVE_SOUTH', 'HIDE_CINDER_HEATRAN', 'CINDER_HEATRAN_DONE',
    'ITEM_CINDER_MAGMARIZER', 'ITEM_CINDER_FULL_RESTORE', 'ITEM_CINDER_CHARCOAL', 'ITEM_CINDER_FIRE_GEM',
    'ITEM_CINDER_ULTRA_BALL', 'ITEM_CINDER_TM_FLAMETHROWER', 'HIDDEN_ITEM_CINDER_HEAT_ROCK',
    'HIDDEN_ITEM_CINDER_STAR_PIECE', 'HIDDEN_ITEM_CINDER_NUGGET', 'CINDER_SCIENTIST_TALKED',
], trainers=['CINDER_IGNATIUS', 'CINDER_ROSA', 'CINDER_BRUNO', 'CINDER_TESS'])

register_trainers('CinderCaverns', '''
=== AREA_TRAINER_CINDER_IGNATIUS ===
Name: IGNATIUS
Class: Kindler
Pic: Kindler
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Magmar
Level: 34

Houndoom
Level: 35

=== AREA_TRAINER_CINDER_ROSA ===
Name: ROSA
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Litwick
Level: 33

Lampent
Level: 35

Sandygast
Level: 34

=== AREA_TRAINER_CINDER_BRUNO ===
Name: BRUNO
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Coalossal
Level: 36

Magcargo
Level: 35

=== AREA_TRAINER_CINDER_TESS ===
Name: TESS
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Arcanine
Level: 36

Salazzle
Level: 36

Centiskorch
Level: 37
''')

st = magma()


def door(a, x, y):
    """A cave door in a wall face: walk up into (x, y)."""
    a.tile(x, y - 1, 0x256, collision=1, elevation=0, c='o')
    a.tile(x, y, 0x25E, collision=0, elevation=0, c='o')


def south_exit(a, x, y):
    """An exit arrow on the floor at the bottom edge: walk down from (x, y)."""
    a.tile(x, y, 0x309, collision=0, elevation=3, c='o')
    for dx, m in ((-1, 0x20D), (0, 0x20E), (1, 0x20F)):
        a.tile(x + dx, y + 1, m, collision=1, elevation=0, c='o')


ENCOUNTERS = dict(
    land=[('SLUGMA', 33, 35), ('NUMEL', 33, 35), ('TORKOAL', 34, 36), ('MAGBY', 33, 35), ('LITWICK', 33, 35),
          ('SIZZLIPEDE', 33, 35), ('CARKOL', 34, 36), ('HOUNDOUR', 34, 36), ('MAGMAR', 36, 38), ('HEATMOR', 36, 38),
          ('LARVESTA', 36, 37), ('MAGCARGO', 38, 39)],
    rock=[('ROLYCOLY', 30, 35), ('CARKOL', 33, 37), ('NOSEPASS', 30, 35), ('GEODUDE', 30, 35), ('COALOSSAL', 38, 40)])

# ---------------------------------------------------------------- 1F: lava rivers
a = Area('CinderCaverns_1F', 44, 36, st, fill='#')
a.ellipse(22, 24, 19, 9, '.')
a.ellipse(22, 9, 15, 6, '.')
a.line([(6, 24), (6, 9), (12, 9)], 3, '.')
a.line([(38, 24), (38, 10)], 3, '.')
a.line([(22, 24), (22, 34)], 3, '.')
a.line([(22, 9), (22, 3)], 3, '.')
a.line([(11, 18), (33, 18)], 2, 'L')       # the river splits the cave: go round by the sides
a.ellipse(13, 28, 4, 2.5, 'L')
a.ellipse(31, 28, 4, 2, 'L')
a.ellipse(31, 7, 4, 2, 'L')
a.ellipse(12, 6, 3, 1.5, 'L')
ring(a, 'L', '#')
door(a, 22, 2)                              # down to B1F
south_exit(a, 22, 34)                       # out to Ember Springs
W1_OUT = a.warp(22, 34, 'MAP_EMBER_SPRINGS', 3)
W1_DOWN = a.warp(22, 2, 'MAP_CINDER_CAVERNS_B1F', 0)
a.trainer('OBJ_EVENT_GFX_MAN_5', 6, 16, 'CinderCaverns_1F_EventScript_Ignatius', 4, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 36, 14, 'CinderCaverns_1F_EventScript_Rosa', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SCIENTIST_1', 25, 30, 'CinderCaverns_1F_EventScript_Scientist', 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_CHARCOAL', 4, 27, 'FLAG_ITEM_CINDER_CHARCOAL')
a.item('ITEM_ULTRA_BALL', 40, 22, 'FLAG_ITEM_CINDER_ULTRA_BALL')
a.item('ITEM_FIRE_GEM', 13, 9, 'FLAG_ITEM_CINDER_FIRE_GEM')
a.hidden(37, 9, 'ITEM_HEAT_ROCK', 'FLAG_HIDDEN_ITEM_CINDER_HEAT_ROCK')
a.obj('OBJ_EVENT_GFX_BREAKABLE_ROCK', 22, 29, 'EventScript_RockSmash', 'MOVEMENT_TYPE_LOOK_AROUND', 'FLAG_TEMP_11')
a.write(map_id='MAP_CINDER_CAVERNS_1F', layout_id='LAYOUT_CINDER_CAVERNS_1F', music='MUS_MT_CHIMNEY', mapsec=MAPSEC,
        mapsec_name='CINDER CAVERNS', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        battle_scene='MAP_BATTLE_SCENE_NORMAL', after='EmberSprings', scripts=r'''
CinderCaverns_1F_MapScripts::
	.byte 0

CinderCaverns_1F_EventScript_Ignatius::
	trainerbattle_single TRAINER_CINDER_IGNATIUS, CinderCaverns_1F_Text_IgnatiusIntro, CinderCaverns_1F_Text_IgnatiusDefeat
	msgbox CinderCaverns_1F_Text_IgnatiusPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_1F_EventScript_Rosa::
	trainerbattle_single TRAINER_CINDER_ROSA, CinderCaverns_1F_Text_RosaIntro, CinderCaverns_1F_Text_RosaDefeat
	msgbox CinderCaverns_1F_Text_RosaPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_1F_EventScript_Scientist::
	lock
	faceplayer
	msgbox CinderCaverns_1F_Text_Scientist, MSGBOX_DEFAULT
	setflag FLAG_CINDER_SCIENTIST_TALKED
	release
	end

CinderCaverns_1F_Text_IgnatiusIntro:
	.string "It's so hot down here my POKéMON\n"
	.string "are glowing! Battle!$"

CinderCaverns_1F_Text_IgnatiusDefeat:
	.string "Burned out…$"

CinderCaverns_1F_Text_IgnatiusPost:
	.string "Don't touch the lava. Seriously.$"

CinderCaverns_1F_Text_RosaIntro:
	.string "The candles flicker… They see your\n"
	.string "soul. Let's battle…$"

CinderCaverns_1F_Text_RosaDefeat:
	.string "The flame went out…$"

CinderCaverns_1F_Text_RosaPost:
	.string "LITWICK love the warmth down here.\n"
	.string "Some say they love souls more…$"

CinderCaverns_1F_Text_Scientist:
	.string "I'm a volcanologist. Long ago, TEAM\n"
	.string "MAGMA built valves on the floor below\l"
	.string "to steer the lava.\p"
	.string "If someone turned all three, the\n"
	.string "lava lake would cool into a path.\p"
	.string "Where would it lead? Deeper than\n"
	.string "anyone has been…$"
''')
register_wild('MAP_CINDER_CAVERNS_1F', 'gCinderCaverns1F', **ENCOUNTERS, land_rate=10)

# ---------------------------------------------------------------- B1F: the lava lake and the valves
b = Area('CinderCaverns_B1F', 48, 40, st, fill='#')
b.ellipse(24, 21, 21, 15, '.')
b.ellipse(24, 19, 15, 9, 'L')               # the lake
b.line([(6, 18), (2, 18)], 3, '.')
b.ellipse(4, 9, 3, 3, '.')                  # west valve nook
b.line([(4, 18), (4, 9)], 2, '.')
b.ellipse(44, 9, 3, 3, '.')                 # east valve nook
b.line([(44, 21), (44, 9)], 2, '.')
b.ellipse(9, 35, 4, 2.5, '.')               # south valve nook
b.line([(9, 33), (13, 30)], 2, '.')
b.ellipse(24, 4, 6, 3, '.')                 # landing in front of the Heart
ring(b, 'L', '#')
for y in range(5, 10):                      # the landing is cut off from the lake shore
    for x in range(14, 35):
        if b.grid[y][x] == '.':
            b.put(x, y, '#')
for y in range(5, 32):                      # the causeway: lava until the valves cool it
    b.rect(23, y, 2, 1, 'L')
    for x in (22, 25):                      # walled in, so it can only be entered from the south
        if b.grid[y][x] == '.':
            b.put(x, y, '#')
b.line([(23, 32), (23, 38)], 3, '.')
door(b, 24, 1)                              # down to the Molten Heart
south_exit(b, 24, 38)
B_UP = b.warp(24, 38, 'MAP_CINDER_CAVERNS_1F', W1_DOWN)
B_HEART = b.warp(24, 1, 'MAP_CINDER_CAVERNS_HEART', 0)
for x, y in ((4, 6), (44, 6), (9, 33)):
    b.tile(x, y, VALVE, collision=1, elevation=0, c='o')
b.sign(4, 6, 'CinderCaverns_B1F_EventScript_ValveWest', 'BG_EVENT_PLAYER_FACING_NORTH')
b.sign(44, 6, 'CinderCaverns_B1F_EventScript_ValveEast', 'BG_EVENT_PLAYER_FACING_NORTH')
b.sign(9, 33, 'CinderCaverns_B1F_EventScript_ValveSouth', 'BG_EVENT_PLAYER_FACING_NORTH')
b.trainer('OBJ_EVENT_GFX_HIKER', 40, 27, 'CinderCaverns_B1F_EventScript_Bruno', 3, 'MOVEMENT_TYPE_FACE_LEFT')
b.trainer('OBJ_EVENT_GFX_WOMAN_2', 8, 22, 'CinderCaverns_B1F_EventScript_Tess', 4, 'MOVEMENT_TYPE_FACE_UP')
b.obj('OBJ_EVENT_GFX_SCIENTIST_1', 24, 2, 'CinderCaverns_B1F_EventScript_Guard', 'MOVEMENT_TYPE_FACE_DOWN',
      local_id='LOCALID_CINDER_GUARD')
b.item('ITEM_MAGMARIZER', 26, 3, 'FLAG_ITEM_CINDER_MAGMARIZER')
b.item('ITEM_FULL_RESTORE', 47 - 2, 12, 'FLAG_ITEM_CINDER_FULL_RESTORE')
b.item('ITEM_TM_FLAMETHROWER', 6, 35, 'FLAG_ITEM_CINDER_TM_FLAMETHROWER')
b.hidden(2, 10, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_CINDER_STAR_PIECE')
b.hidden(37, 31, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_CINDER_NUGGET')

COOL = {'WEST': range(5, 15), 'EAST': range(15, 23), 'SOUTH': range(23, 32)}


def cool_lines(key):
    return ''.join(f'\tsetmetatile {x}, {y}, {COOLED}, FALSE\n' for y in COOL[key] for x in (23, 24))


VALVES = ''.join(f'''
CinderCaverns_B1F_EventScript_Valve{k.title()}::
	lockall
	goto_if_set FLAG_CINDER_VALVE_{k}, CinderCaverns_B1F_EventScript_ValveDone
	msgbox CinderCaverns_B1F_Text_Valve, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CinderCaverns_B1F_EventScript_ValveLeave
	playse SE_PIN
	setflag FLAG_CINDER_VALVE_{k}
	call CinderCaverns_B1F_EventScript_Cool{k.title()}
	special DrawWholeMapView
	playse SE_M_MIST
	waitse
	msgbox CinderCaverns_B1F_Text_Cooled, MSGBOX_DEFAULT
	releaseall
	end

CinderCaverns_B1F_EventScript_Cool{k.title()}::
{cool_lines(k)}	return
''' for k in COOL)

b.write(map_id='MAP_CINDER_CAVERNS_B1F', layout_id='LAYOUT_CINDER_CAVERNS_B1F', music='MUS_MT_CHIMNEY', mapsec=MAPSEC,
        mapsec_name='CINDER CAVERNS', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        after='CinderCaverns_1F', show_name=False, scripts=r'''
CinderCaverns_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, CinderCaverns_B1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, CinderCaverns_B1F_OnTransition
	.byte 0

@ After the League, the volcanologist steps aside from the Molten Heart.
CinderCaverns_B1F_OnTransition:
	goto_if_unset FLAG_SYS_GAME_CLEAR, CinderCaverns_B1F_OnTransitionEnd
	setobjectxyperm LOCALID_CINDER_GUARD, 22, 3
	setobjectmovementtype LOCALID_CINDER_GUARD, MOVEMENT_TYPE_FACE_RIGHT
CinderCaverns_B1F_OnTransitionEnd:
	end

@ Cooled stretches of the causeway stay cool.
CinderCaverns_B1F_OnLoad:
	call_if_set FLAG_CINDER_VALVE_WEST, CinderCaverns_B1F_EventScript_CoolWest
	call_if_set FLAG_CINDER_VALVE_EAST, CinderCaverns_B1F_EventScript_CoolEast
	call_if_set FLAG_CINDER_VALVE_SOUTH, CinderCaverns_B1F_EventScript_CoolSouth
	end

CinderCaverns_B1F_EventScript_ValveDone::
	msgbox CinderCaverns_B1F_Text_ValveDone, MSGBOX_DEFAULT
CinderCaverns_B1F_EventScript_ValveLeave::
	releaseall
	end
''' + VALVES + r'''
CinderCaverns_B1F_EventScript_Bruno::
	trainerbattle_single TRAINER_CINDER_BRUNO, CinderCaverns_B1F_Text_BrunoIntro, CinderCaverns_B1F_Text_BrunoDefeat
	msgbox CinderCaverns_B1F_Text_BrunoPost, MSGBOX_AUTOCLOSE
	end

CinderCaverns_B1F_EventScript_Tess::
	trainerbattle_single TRAINER_CINDER_TESS, CinderCaverns_B1F_Text_TessIntro, CinderCaverns_B1F_Text_TessDefeat
	msgbox CinderCaverns_B1F_Text_TessPost, MSGBOX_AUTOCLOSE
	end

@ The Molten Heart is only for Champions.
CinderCaverns_B1F_EventScript_Guard::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, CinderCaverns_B1F_EventScript_GuardOk
	msgbox CinderCaverns_B1F_Text_GuardNo, MSGBOX_DEFAULT
	release
	end

CinderCaverns_B1F_EventScript_GuardOk::
	msgbox CinderCaverns_B1F_Text_GuardOk, MSGBOX_DEFAULT
	release
	end

CinderCaverns_B1F_Text_Valve:
	.string "A rusty old valve with a MAGMA\n"
	.string "symbol. Turn it?$"

CinderCaverns_B1F_Text_Cooled:
	.string "Steam roars up from the lake!\n"
	.string "Part of the lava has cooled into\l"
	.string "solid rock.$"

CinderCaverns_B1F_Text_ValveDone:
	.string "The valve is turned all the way.$"

CinderCaverns_B1F_Text_BrunoIntro:
	.string "I came to study the lava rocks.\n"
	.string "And to battle, of course!$"

CinderCaverns_B1F_Text_BrunoDefeat:
	.string "You're a hot one!$"

CinderCaverns_B1F_Text_BrunoPost:
	.string "Those valves are TEAM MAGMA's work.\n"
	.string "At least they're useful for once.$"

CinderCaverns_B1F_Text_TessIntro:
	.string "Only the toughest trainers make it\n"
	.string "this deep. Are you one of them?$"

CinderCaverns_B1F_Text_TessDefeat:
	.string "You really are.$"

CinderCaverns_B1F_Text_TessPost:
	.string "There's a valve in the far south-west\n"
	.string "too. Easy to miss!$"

CinderCaverns_B1F_Text_GuardNo:
	.string "Behind me is the MOLTEN HEART. Its\n"
	.string "heat could melt a lesser trainer.\p"
	.string "Come back once you're CHAMPION.$"

CinderCaverns_B1F_Text_GuardOk:
	.string "CHAMPION, you may enter. Something\n"
	.string "is breathing in there… slowly.$"
''')
register_wild('MAP_CINDER_CAVERNS_B1F', 'gCinderCavernsB1F', **ENCOUNTERS, land_rate=10)

# ---------------------------------------------------------------- the Molten Heart
h = Area('CinderCaverns_Heart', 26, 22, st, fill='#')
h.ellipse(13, 11, 11, 8, '.')
h.ellipse(13, 9, 7, 4, 'L')
ring(h, 'L', '#')
h.ellipse(13, 9, 3, 2, '.')                  # Heatran's island
h.rect(12, 11, 3, 9, '.')
h.line([(13, 12), (13, 20)], 3, '.')
south_exit(h, 13, 20)
h.warp(13, 20, 'MAP_CINDER_CAVERNS_B1F', B_HEART)
h.obj('OBJ_EVENT_GFX_SPECIES(HEATRAN)', 13, 8, 'CinderCaverns_Heart_EventScript_Heatran', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_CINDER_HEATRAN', local_id='LOCALID_CINDER_HEATRAN')
h.write(map_id='MAP_CINDER_CAVERNS_HEART', layout_id='LAYOUT_CINDER_CAVERNS_HEART', music='MUS_MT_CHIMNEY', mapsec=MAPSEC,
        mapsec_name='CINDER CAVERNS', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        after='CinderCaverns_B1F', show_name=False, scripts=r'''
CinderCaverns_Heart_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinderCaverns_Heart_OnTransition
	.byte 0

CinderCaverns_Heart_OnTransition:
	call_if_set FLAG_CINDER_HEATRAN_DONE, CinderCaverns_Heart_EventScript_Hide
	end

CinderCaverns_Heart_EventScript_Hide::
	setflag FLAG_HIDE_CINDER_HEATRAN
	return

CinderCaverns_Heart_EventScript_Heatran::
	lock
	faceplayer
	setwildbattle SPECIES_HEATRAN, 50
	playmoncry SPECIES_HEATRAN, CRY_MODE_ENCOUNTER
	msgbox CinderCaverns_Heart_Text_Heatran, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, CinderCaverns_Heart_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, CinderCaverns_Heart_EventScript_Stays
	setflag FLAG_CINDER_HEATRAN_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, CinderCaverns_Heart_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_CINDER_HEATRAN
	setflag FLAG_HIDE_CINDER_HEATRAN
	fadescreen FADE_FROM_BLACK
	msgbox CinderCaverns_Heart_Text_Sank, MSGBOX_DEFAULT
CinderCaverns_Heart_EventScript_End::
	release
	end

CinderCaverns_Heart_EventScript_Stays::
	msgbox CinderCaverns_Heart_Text_Stays, MSGBOX_DEFAULT
	release
	end

CinderCaverns_Heart_Text_Heatran:
	.string "Gyuuoooh!$"

CinderCaverns_Heart_Text_Sank:
	.string "HEATRAN sank back into the magma.$"

CinderCaverns_Heart_Text_Stays:
	.string "HEATRAN is glaring from its island.$"
''')

# ---------------------------------------------------------------- the door on Ember Springs' north hill
STAMP_SKETCH = None
print('ok CinderCaverns')
