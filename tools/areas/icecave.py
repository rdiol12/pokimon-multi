"""Frostvein Cave: an ice cave on an islet off Mossdeep. Two floors of
sliding-ice puzzles; at the bottom a sleeping Avalugg guards the Frozen
Shrine, where the Ice Priestess entrusts you with a Frigibax.

    python3 tools/areas/icecave.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import ice, ICE_ROCKS
import icepuzzle

AREA_NO = 4
MAPSEC = 'MAPSEC_FROSTVEIN_CAVE'
LADDER = 0x37F
SNOW = 0x385
ROCK = min(ICE_ROCKS)

register_constants('FrostveinCave', AREA_NO, flags=[
    'HIDE_FROSTVEIN_AVALUGG', 'FROSTVEIN_AVALUGG_DONE', 'FROSTVEIN_FRIGIBAX', 'FROSTVEIN_RESEARCHER_TALKED',
    'ITEM_FROSTVEIN_ICE_STONE', 'ITEM_FROSTVEIN_TM_ICE_BEAM', 'ITEM_FROSTVEIN_NEVER_MELT_ICE', 'ITEM_FROSTVEIN_ULTRA_BALL',
    'ITEM_FROSTVEIN_FULL_HEAL', 'ITEM_FROSTVEIN_MAX_REVIVE', 'ITEM_FROSTVEIN_ICY_ROCK',
    'HIDDEN_ITEM_FROSTVEIN_PEARL', 'HIDDEN_ITEM_FROSTVEIN_STARDUST', 'HIDDEN_ITEM_FROSTVEIN_BIG_PEARL',
], trainers=['FROSTVEIN_GLEN', 'FROSTVEIN_BJORN', 'FROSTVEIN_NIVA', 'FROSTVEIN_SORA'])

register_trainers('FrostveinCave', '''
=== AREA_TRAINER_FROSTVEIN_GLEN ===
Name: GLEN
Class: Cooltrainer
Pic: Cooltrainer M
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Sneasel
Level: 40

Glalie
Level: 41

=== AREA_TRAINER_FROSTVEIN_BJORN ===
Name: BJORN
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Bergmite
Level: 39

Piloswine
Level: 41

=== AREA_TRAINER_FROSTVEIN_NIVA ===
Name: NIVA
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Froslass
Level: 42

Vanillish
Level: 41

Cryogonal
Level: 42

=== AREA_TRAINER_FROSTVEIN_SORA ===
Name: SORA
Class: Psychic
Pic: Psychic M
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Jynx
Level: 42

Smoochum
Level: 40
''')

st = ice()
ENCOUNTERS = dict(
    land=[('SNORUNT', 38, 40), ('SPHEAL', 38, 40), ('SWINUB', 38, 40), ('BERGMITE', 38, 40), ('SNEASEL', 39, 41),
          ('CUBCHOO', 38, 40), ('VANILLITE', 38, 40), ('SNOM', 38, 40), ('GLALIE', 41, 42), ('BEARTIC', 41, 42),
          ('FROSLASS', 41, 42), ('CRYOGONAL', 42, 43)],
)


def ice_field(a, x0, y0, w, h, gap_in, gap_out, rocks, seed, min_moves):
    """An ice field walled in on every side but its way in (bottom) and out (top)."""
    rs, moves = icepuzzle.make(x0, y0, w, h, gap_in, gap_out, rocks=rocks, min_moves=min_moves, seed=seed)
    a.rect(x0, y0, w, h, 'I')
    for x, y in rs:
        a.tile(x, y, ROCK if (x + y) % 2 else max(ICE_ROCKS), collision=1, elevation=0, c='B')
    a.tile(gap_in, y0 + h, SNOW, collision=0, elevation=3, c='.')
    a.tile(gap_out, y0 - 1, SNOW, collision=0, elevation=3, c='.')
    print(f'  ice field at {x0},{y0}: {moves} slides')
    return moves


def ladder(a, x, y):
    a.tile(x, y, LADDER, collision=0, elevation=3, c='o')


# ---------------------------------------------------------------- 1F
a = Area('FrostveinCave_1F', 40, 32, st, fill='#')
a.rect(4, 21, 32, 9, '.')                     # the entry hall
a.rect(6, 3, 28, 5, '.')                      # the upper hall
a.rect(3, 10, 3, 12, '.')                     # west passage and nook
a.ellipse(4, 11, 2, 2, '.')
a.rect(34, 12, 3, 10, '.')                    # east passage and nook
a.ellipse(35, 11, 3, 2, '.')
ice_field(a, 9, 9, 22, 11, 15, 24, rocks=12, seed=7, min_moves=6)
ladder(a, 20, 4)
for x, y in ((6, 23), (7, 28), (33, 22), (27, 28), (11, 28), (14, 6), (29, 4), (35, 15)):
    a.tile(x, y, ROCK if (x + y) % 2 else max(ICE_ROCKS), collision=1, elevation=0, c='B')
W1_DOWN = a.warp(20, 4, 'MAP_FROSTVEIN_CAVE_B1F', 0)
a.obj('OBJ_EVENT_GFX_SAILOR', 21, 28, 'FrostveinCave_1F_EventScript_Sailor', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_SCIENTIST_1', 13, 23, 'FrostveinCave_1F_EventScript_Researcher', 'MOVEMENT_TYPE_FACE_DOWN')
a.sign(15, 21, 'FrostveinCave_1F_EventScript_Sign', elevation=0)
a.trainer('OBJ_EVENT_GFX_MAN_3', 30, 25, 'FrostveinCave_1F_EventScript_Glen', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_HIKER', 10, 5, 'FrostveinCave_1F_EventScript_Bjorn', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.item('ITEM_ICE_STONE', 4, 10, 'FLAG_ITEM_FROSTVEIN_ICE_STONE')
a.item('ITEM_FULL_HEAL', 32, 4, 'FLAG_ITEM_FROSTVEIN_FULL_HEAL')
a.item('ITEM_ULTRA_BALL', 36, 10, 'FLAG_ITEM_FROSTVEIN_ULTRA_BALL')
a.hidden(35, 20, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_FROSTVEIN_PEARL')
a.hidden(7, 6, 'ITEM_STARDUST', 'FLAG_HIDDEN_ITEM_FROSTVEIN_STARDUST')
a.write(map_id='MAP_FROSTVEIN_CAVE_1F', layout_id='LAYOUT_FROSTVEIN_CAVE_1F', music='MUS_SEALED_CHAMBER', mapsec=MAPSEC,
        mapsec_name='FROSTVEIN CAVE', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        battle_scene='MAP_BATTLE_SCENE_NORMAL', after='SkyropeCanyon', scripts=r'''
FrostveinCave_1F_MapScripts::
	.byte 0

FrostveinCave_1F_EventScript_Sailor::
	lock
	faceplayer
	msgbox FrostveinCave_1F_Text_SailorBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FrostveinCave_1F_EventScript_SailorStay
	msgbox FrostveinCave_1F_Text_SailorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MOSSDEEP_CITY, 30, 18
	waitstate
	release
	end

FrostveinCave_1F_EventScript_SailorStay::
	msgbox FrostveinCave_1F_Text_SailorStay, MSGBOX_DEFAULT
	release
	end

FrostveinCave_1F_EventScript_Sign::
	msgbox FrostveinCave_1F_Text_Sign, MSGBOX_SIGN
	end

FrostveinCave_1F_EventScript_Researcher::
	lock
	faceplayer
	msgbox FrostveinCave_1F_Text_Researcher, MSGBOX_DEFAULT
	setflag FLAG_FROSTVEIN_RESEARCHER_TALKED
	release
	end

FrostveinCave_1F_EventScript_Glen::
	trainerbattle_single TRAINER_FROSTVEIN_GLEN, FrostveinCave_1F_Text_GlenIntro, FrostveinCave_1F_Text_GlenDefeat
	msgbox FrostveinCave_1F_Text_GlenPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_1F_EventScript_Bjorn::
	trainerbattle_single TRAINER_FROSTVEIN_BJORN, FrostveinCave_1F_Text_BjornIntro, FrostveinCave_1F_Text_BjornDefeat
	msgbox FrostveinCave_1F_Text_BjornPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_1F_Text_SailorBack:
	.string "Brr! Ready to sail back to\n"
	.string "MOSSDEEP?$"

FrostveinCave_1F_Text_SailorGo:
	.string "Anchors up!$"

FrostveinCave_1F_Text_SailorStay:
	.string "I'll wait by the boat. Don't freeze\n"
	.string "in there!$"

FrostveinCave_1F_Text_Sign:
	.string "FROSTVEIN CAVE\n"
	.string "Ice floors are slippery. Plan your\l"
	.string "path before you step on!$"

FrostveinCave_1F_Text_Researcher:
	.string "I study the ice here. It never\n"
	.string "melts, not even in summer.\p"
	.string "Once you step on the ice, you slide\n"
	.string "until something stops you. Use the\l"
	.string "frozen rocks as brakes!\p"
	.string "They say a shrine lies at the very\n"
	.string "bottom, guarded by a huge AVALUGG.$"

FrostveinCave_1F_Text_GlenIntro:
	.string "Cold battles keep the mind sharp!$"

FrostveinCave_1F_Text_GlenDefeat:
	.string "Chilled to the bone…$"

FrostveinCave_1F_Text_GlenPost:
	.string "Look before you slide. That's the\n"
	.string "whole secret.$"

FrostveinCave_1F_Text_BjornIntro:
	.string "I slid across that ice field six\n"
	.string "times before I got here!$"

FrostveinCave_1F_Text_BjornDefeat:
	.string "Slipped up!$"

FrostveinCave_1F_Text_BjornPost:
	.string "The ladder goes down to an even\n"
	.string "bigger ice field. Good luck!$"
''')
register_wild('MAP_FROSTVEIN_CAVE_1F', 'gFrostveinCave1F', **ENCOUNTERS, land_rate=8)

# ---------------------------------------------------------------- B1F
b = Area('FrostveinCave_B1F', 40, 34, st, fill='#')
b.rect(3, 26, 34, 6, '.')                     # the bottom hall
b.rect(14, 6, 12, 5, '.')                     # the antechamber
b.rect(12, 2, 16, 3, '.')                     # the Frozen Shrine
b.put(20, 5, '.')                             # where Avalugg sleeps
b.rect(33, 14, 4, 12, '.')                    # east passage
b.ellipse(35, 12, 2, 2, '.')
ice_field(b, 8, 12, 24, 13, 9, 20, rocks=16, seed=11, min_moves=8)
ladder(b, 5, 28)
for x, y in ((12, 30), (30, 27), (18, 28), (15, 9), (25, 7), (4, 31), (36, 18)):
    b.tile(x, y, ROCK if (x + y) % 2 else max(ICE_ROCKS), collision=1, elevation=0, c='B')
b.warp(5, 28, 'MAP_FROSTVEIN_CAVE_1F', W1_DOWN)
b.obj('OBJ_EVENT_GFX_SPECIES(AVALUGG)', 20, 5, 'FrostveinCave_B1F_EventScript_Avalugg', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_FROSTVEIN_AVALUGG', local_id='LOCALID_FROSTVEIN_AVALUGG')
b.obj('OBJ_EVENT_GFX_WOMAN_2', 20, 2, 'FrostveinCave_B1F_EventScript_Priestess', 'MOVEMENT_TYPE_FACE_DOWN')
b.trainer('OBJ_EVENT_GFX_WOMAN_5', 24, 29, 'FrostveinCave_B1F_EventScript_Niva', 3, 'MOVEMENT_TYPE_FACE_LEFT')
b.trainer('OBJ_EVENT_GFX_PSYCHIC_M', 16, 8, 'FrostveinCave_B1F_EventScript_Sora', 2, 'MOVEMENT_TYPE_FACE_RIGHT')
b.item('ITEM_NEVER_MELT_ICE', 13, 3, 'FLAG_ITEM_FROSTVEIN_NEVER_MELT_ICE')
b.item('ITEM_TM_ICE_BEAM', 35, 11, 'FLAG_ITEM_FROSTVEIN_TM_ICE_BEAM')
b.item('ITEM_MAX_REVIVE', 26, 3, 'FLAG_ITEM_FROSTVEIN_MAX_REVIVE')
b.item('ITEM_ICY_ROCK', 35, 30, 'FLAG_ITEM_FROSTVEIN_ICY_ROCK')
b.hidden(24, 9, 'ITEM_BIG_PEARL', 'FLAG_HIDDEN_ITEM_FROSTVEIN_BIG_PEARL')
b.write(map_id='MAP_FROSTVEIN_CAVE_B1F', layout_id='LAYOUT_FROSTVEIN_CAVE_B1F', music='MUS_SEALED_CHAMBER', mapsec=MAPSEC,
        mapsec_name='FROSTVEIN CAVE', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        after='FrostveinCave_1F', show_name=False, scripts=r'''
FrostveinCave_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FrostveinCave_B1F_OnTransition
	.byte 0

FrostveinCave_B1F_OnTransition:
	call_if_set FLAG_FROSTVEIN_AVALUGG_DONE, FrostveinCave_B1F_EventScript_HideAvalugg
	end

FrostveinCave_B1F_EventScript_HideAvalugg::
	setflag FLAG_HIDE_FROSTVEIN_AVALUGG
	return

@ Avalugg sleeps in the doorway of the shrine.
FrostveinCave_B1F_EventScript_Avalugg::
	lock
	faceplayer
	msgbox FrostveinCave_B1F_Text_AvaluggSleeps, MSGBOX_DEFAULT
	setwildbattle SPECIES_AVALUGG, 44
	playmoncry SPECIES_AVALUGG, CRY_MODE_ENCOUNTER
	msgbox FrostveinCave_B1F_Text_AvaluggWakes, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, FrostveinCave_B1F_EventScript_AvaluggStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, FrostveinCave_B1F_EventScript_AvaluggStays
	setflag FLAG_FROSTVEIN_AVALUGG_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, FrostveinCave_B1F_EventScript_AvaluggEnd
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_FROSTVEIN_AVALUGG
	setflag FLAG_HIDE_FROSTVEIN_AVALUGG
	fadescreen FADE_FROM_BLACK
	msgbox FrostveinCave_B1F_Text_AvaluggLeft, MSGBOX_DEFAULT
FrostveinCave_B1F_EventScript_AvaluggEnd::
	release
	end

FrostveinCave_B1F_EventScript_AvaluggStays::
	msgbox FrostveinCave_B1F_Text_AvaluggStays, MSGBOX_DEFAULT
	release
	end

@ The Ice Priestess gives a Frigibax to whoever reaches the shrine.
FrostveinCave_B1F_EventScript_Priestess::
	lock
	faceplayer
	goto_if_set FLAG_FROSTVEIN_FRIGIBAX, FrostveinCave_B1F_EventScript_PriestessDone
	msgbox FrostveinCave_B1F_Text_Priestess, MSGBOX_DEFAULT
	givemon SPECIES_FRIGIBAX, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, FrostveinCave_B1F_EventScript_PriestessFull
	setflag FLAG_FROSTVEIN_FRIGIBAX
	playfanfare MUS_OBTAIN_ITEM
	msgbox FrostveinCave_B1F_Text_GotFrigibax, MSGBOX_DEFAULT
	waitfanfare
	msgbox FrostveinCave_B1F_Text_PriestessAfter, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_PriestessFull::
	msgbox FrostveinCave_B1F_Text_PriestessFull, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_PriestessDone::
	msgbox FrostveinCave_B1F_Text_PriestessDone, MSGBOX_DEFAULT
	release
	end

FrostveinCave_B1F_EventScript_Niva::
	trainerbattle_single TRAINER_FROSTVEIN_NIVA, FrostveinCave_B1F_Text_NivaIntro, FrostveinCave_B1F_Text_NivaDefeat
	msgbox FrostveinCave_B1F_Text_NivaPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_B1F_EventScript_Sora::
	trainerbattle_single TRAINER_FROSTVEIN_SORA, FrostveinCave_B1F_Text_SoraIntro, FrostveinCave_B1F_Text_SoraDefeat
	msgbox FrostveinCave_B1F_Text_SoraPost, MSGBOX_AUTOCLOSE
	end

FrostveinCave_B1F_Text_AvaluggSleeps:
	.string "A huge AVALUGG is asleep in the\n"
	.string "doorway. Its breath fogs the air…$"

FrostveinCave_B1F_Text_AvaluggWakes:
	.string "Grrooooh!$"

FrostveinCave_B1F_Text_AvaluggLeft:
	.string "AVALUGG lumbered off into the dark.$"

FrostveinCave_B1F_Text_AvaluggStays:
	.string "AVALUGG settled back down in the\n"
	.string "doorway.$"

FrostveinCave_B1F_Text_Priestess:
	.string "You crossed the ice and woke the\n"
	.string "guardian. Few have done both.\p"
	.string "This shrine looks after a FRIGIBAX\n"
	.string "egg that hatched in the cold.\p"
	.string "It needs a trainer who can keep a\n"
	.string "cool head. Please take it.$"

FrostveinCave_B1F_Text_GotFrigibax:
	.string "{PLAYER} received FRIGIBAX!$"

FrostveinCave_B1F_Text_PriestessAfter:
	.string "Raise it well. One day it may grow\n"
	.string "into a mighty BAXCALIBUR.$"

FrostveinCave_B1F_Text_PriestessFull:
	.string "Your party and boxes are full.\n"
	.string "Make some room and come back.$"

FrostveinCave_B1F_Text_PriestessDone:
	.string "The cave is quieter now. The ice\n"
	.string "remembers you.$"

FrostveinCave_B1F_Text_NivaIntro:
	.string "You made it down here? Then let's\n"
	.string "see how cool you really are!$"

FrostveinCave_B1F_Text_NivaDefeat:
	.string "Frozen solid…$"

FrostveinCave_B1F_Text_NivaPost:
	.string "I always slide too far on that big\n"
	.string "field. There's a trick to it.$"

FrostveinCave_B1F_Text_SoraIntro:
	.string "I sensed you coming across the ice.\n"
	.string "Every slide!$"

FrostveinCave_B1F_Text_SoraDefeat:
	.string "I didn't sense that coming.$"

FrostveinCave_B1F_Text_SoraPost:
	.string "The guardian sleeps, but it dreams\n"
	.string "of the shrine.$"
''')
register_wild('MAP_FROSTVEIN_CAVE_B1F', 'gFrostveinCaveB1F', **ENCOUNTERS, land_rate=8)
print('ok FrostveinCave')

# ---------------------------------------------------------------- the way in, from Mossdeep City
add_object('MossdeepCity', {'graphics_id': 'OBJ_EVENT_GFX_SAILOR', 'x': 31, 'y': 18, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_LEFT', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'MossdeepCity_EventScript_FrostveinSailor', 'flag': '0'})
register_script_file('frostvein', r'''
@ Frostvein Cave boat in Mossdeep City (tools/areas/icecave.py).
MossdeepCity_EventScript_FrostveinSailor::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, MossdeepCity_EventScript_FrostveinSailorNoBadge
	msgbox MossdeepCity_Text_FrostveinSailorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MossdeepCity_EventScript_FrostveinSailorNo
	msgbox MossdeepCity_Text_FrostveinSailorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_FROSTVEIN_CAVE_1F, 20, 28
	waitstate
	release
	end

MossdeepCity_EventScript_FrostveinSailorNoBadge::
	msgbox MossdeepCity_Text_FrostveinSailorNoBadge, MSGBOX_DEFAULT
	release
	end

MossdeepCity_EventScript_FrostveinSailorNo::
	msgbox MossdeepCity_Text_FrostveinSailorNo, MSGBOX_DEFAULT
	release
	end

MossdeepCity_Text_FrostveinSailorNoBadge:
	.string "North of here there's an islet with\n"
	.string "a cave that never thaws.\p"
	.string "It's a dangerous trip. Show me the\n"
	.string "MIND BADGE and I'll sail you there.$"

MossdeepCity_Text_FrostveinSailorAsk:
	.string "The MIND BADGE! Then you can handle\n"
	.string "the cold.\p"
	.string "Shall I sail you to FROSTVEIN CAVE?$"

MossdeepCity_Text_FrostveinSailorGo:
	.string "Wrap up warm!$"

MossdeepCity_Text_FrostveinSailorNo:
	.string "The ice will still be there\n"
	.string "tomorrow. It always is.$"
''')
print('ok entrance')
