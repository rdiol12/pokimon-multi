"""Mirebough Marsh: a rainy swamp of ponds and reeds crossed by wooden
boardwalks. The Bog Witch needs three ingredients for her stew (a Tiny
Mushroom, a Big Root and some Black Sludge) and pays with a Goomy egg. A
Stunfisk lies hidden in the mud.

    python3 tools/areas/swamp.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import marsh

AREA_NO = 7
NAME = 'MireboughMarsh'
MAPSEC = 'MAPSEC_MIREBOUGH_MARSH'
R120 = 'LAYOUT_ROUTE120'

register_constants(NAME, AREA_NO, flags=[
    'MIRE_STEW_DONE', 'MIRE_WITCH_TALKED', 'MIRE_STUNFISK_DONE', 'ITEM_MIRE_TINY_MUSHROOM', 'ITEM_MIRE_BIG_ROOT',
    'ITEM_MIRE_BLACK_SLUDGE', 'ITEM_MIRE_TM_SLUDGE_BOMB', 'ITEM_MIRE_DAMP_ROCK', 'ITEM_MIRE_ULTRA_BALL',
    'ITEM_MIRE_ELIXIR', 'HIDDEN_ITEM_MIRE_BIG_MUSHROOM', 'HIDDEN_ITEM_MIRE_PEARL', 'HIDDEN_ITEM_MIRE_SHOAL_SALT',
], trainers=['MIRE_FINN', 'MIRE_MARSHA', 'MIRE_OTTO', 'MIRE_IVY'])

register_trainers(NAME, '''
=== AREA_TRAINER_MIRE_FINN ===
Name: FINN
Class: Fisherman
Pic: Fisherman
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Whiscash
Level: 37

Quagsire
Level: 37

=== AREA_TRAINER_MIRE_MARSHA ===
Name: MARSHA
Class: Aroma Lady
Pic: Aroma Lady
Gender: Female
Music: Girl
Double Battle: No
AI: Basic Trainer

Lombre
Level: 36

Amoonguss
Level: 38

=== AREA_TRAINER_MIRE_OTTO ===
Name: OTTO
Class: Bug Maniac
Pic: Bug Maniac
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Yanmega
Level: 38

Skorupi
Level: 36

=== AREA_TRAINER_MIRE_IVY ===
Name: IVY
Class: Pkmn Ranger
Pic: Pokemon Ranger F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Toxicroak
Level: 39

Sliggoo
Level: 38
''')

st = marsh()
W, H = 56, 48
a = Area(NAME, W, H, st, fill='T')


def boardwalk(a, x0, x1, y):
    """Route 120's wooden bridge (two rows tall) from x0 to x1: ground at both ends."""
    a.stamp(R120, 7, 15, 4, 2, x0, y)                      # west end: step, post, ramp, deck start
    for x in range(x0 + 4, x1 - 3):
        a.stamp(R120, 12, 15, 1, 2, x, y)                  # deck
    a.stamp(R120, 25, 15, 4, 2, x1 - 3, y)                 # east end


# ground, ponds and reeds
a.ellipse(28, 24, 25, 21, '.')
a.rect(25, 42, 6, 6, '.')
a.ellipse(28, 24, 14, 6, '~')                              # the central pond
a.ellipse(16, 10, 9, 4, '~')                               # the north-west pond
a.ellipse(44, 36, 7, 4, '~')                               # the south-east pond
a.ellipse(41, 10, 6, 3.5, '~')
for x, y, rx, ry in ((10, 22, 5, 4), (45, 23, 5, 4), (18, 36, 6, 3), (34, 6, 4, 2), (8, 32, 3, 3)):
    a.ellipse(x, y, rx, ry, '"')
for x, y, rx, ry in ((4, 14, 3, 3), (50, 14, 3, 3), (30, 36, 3, 2), (24, 14, 2, 2), (52, 30, 2, 3)):
    a.ellipse(x, y, rx, ry, 'T')
for x, y, rx, ry in ((28, 13, 4, 2), (38, 30, 3, 2), (8, 41, 4, 3), (44, 44, 4, 2), (20, 4, 3, 1.5)):
    a.ellipse(x, y, rx, ry, '"')
for x, y, rx, ry in ((36, 16, 2.5, 1.5), (14, 30, 2, 1.5), (49, 41, 2, 1.5)):   # puddles
    a.ellipse(x, y, rx, ry, '~')
for x, y in ((18, 16), (40, 14), (22, 42), (34, 42), (12, 46), (6, 26)):
    a.rect(x, y, 2, 2, 'T')
a.line([(28, 46), (28, 32)], 2, '.')
a.ellipse(4, 9, 3.5, 3, '.')                               # the witch's clearing, only over the boardwalk
a.rect(0, 0, 10, 6, 'T')
a.rect(0, 12, 10, 6, 'T')
a.rect(8, 6, 2, 2, 'T')
a.ellipse(51, 36, 2.5, 3, '.')                             # hidden nooks
a.ellipse(47, 8, 2, 3, '.')
a.snap_trees(ground='.')
boardwalk(a, 12, 44, 23)                                   # across the central pond
boardwalk(a, 6, 26, 9)                                     # across the north-west pond, to the witch
ARRIVE = (28, 45)
a.obj('OBJ_EVENT_GFX_MAN_4', 29, 45, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(26, 43, f'{NAME}_EventScript_Sign', elevation=0)
a.obj('OBJ_EVENT_GFX_OLD_WOMAN', 4, 9, f'{NAME}_EventScript_Witch', 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_SPECIES(WOOPER)', 33, 31, f'{NAME}_EventScript_Wooper', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_SPECIES(GOOMY)', 46, 28, f'{NAME}_EventScript_Goomy', 'MOVEMENT_TYPE_LOOK_AROUND')
a.obj('OBJ_EVENT_GFX_SPECIES(CROAGUNK)', 11, 28, f'{NAME}_EventScript_Croagunk', 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_FISHERMAN', 20, 31, f'{NAME}_EventScript_Finn', 2, 'MOVEMENT_TYPE_FACE_UP')
a.trainer('OBJ_EVENT_GFX_WOMAN_2', 46, 18, f'{NAME}_EventScript_Marsha', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_BUG_CATCHER', 8, 18, f'{NAME}_EventScript_Otto', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_WOMAN_5', 38, 40, f'{NAME}_EventScript_Ivy', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_TINY_MUSHROOM', 50, 22, 'FLAG_ITEM_MIRE_TINY_MUSHROOM')
a.item('ITEM_BIG_ROOT', 33, 4, 'FLAG_ITEM_MIRE_BIG_ROOT')
a.item('ITEM_BLACK_SLUDGE', 8, 36, 'FLAG_ITEM_MIRE_BLACK_SLUDGE')
a.item('ITEM_TM_SLUDGE_BOMB', 52, 36, 'FLAG_ITEM_MIRE_TM_SLUDGE_BOMB')
a.item('ITEM_DAMP_ROCK', 47, 8, 'FLAG_ITEM_MIRE_DAMP_ROCK')
a.item('ITEM_ULTRA_BALL', 14, 41, 'FLAG_ITEM_MIRE_ULTRA_BALL')
a.hidden(22, 18, 'ITEM_BIG_MUSHROOM', 'FLAG_HIDDEN_ITEM_MIRE_BIG_MUSHROOM')
a.hidden(36, 29, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_MIRE_PEARL')
a.hidden(3, 26, 'ITEM_SHOAL_SALT', 'FLAG_HIDDEN_ITEM_MIRE_SHOAL_SALT')
STUNFISK = (24, 40)
a.coord(*STUNFISK, f'{NAME}_EventScript_Stunfisk')

a.write(map_id='MAP_MIREBOUGH_MARSH', layout_id='LAYOUT_MIREBOUGH_MARSH', music='MUS_ROUTE120', mapsec=MAPSEC,
        mapsec_name='MIREBOUGH MARSH', weather='WEATHER_RAIN', after='Hollowmere_Manor_2F', scripts=rf'''
{NAME}_MapScripts::
	.byte 0

{NAME}_EventScript_Guide::
	lock
	faceplayer
	msgbox {NAME}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GuideStay
	msgbox {NAME}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 5, 9
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

@ ---- the Bog Witch's stew
{NAME}_EventScript_Witch::
	lock
	faceplayer
	goto_if_set FLAG_MIRE_STEW_DONE, {NAME}_EventScript_WitchDone
	setflag FLAG_MIRE_WITCH_TALKED
	checkitem ITEM_TINY_MUSHROOM
	goto_if_eq VAR_RESULT, FALSE, {NAME}_EventScript_WitchAsk
	checkitem ITEM_BIG_ROOT
	goto_if_eq VAR_RESULT, FALSE, {NAME}_EventScript_WitchAsk
	checkitem ITEM_BLACK_SLUDGE
	goto_if_eq VAR_RESULT, FALSE, {NAME}_EventScript_WitchAsk
	msgbox {NAME}_Text_WitchHave, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_WitchLater
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, {NAME}_EventScript_WitchFull
	removeitem ITEM_TINY_MUSHROOM
	removeitem ITEM_BIG_ROOT
	removeitem ITEM_BLACK_SLUDGE
	msgbox {NAME}_Text_WitchStew, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_BUBBLE_BEAM
	waitse
	fadescreen FADE_FROM_BLACK
	giveegg SPECIES_GOOMY
	setflag FLAG_MIRE_STEW_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotEgg, MSGBOX_DEFAULT
	waitfanfare
	msgbox {NAME}_Text_WitchAfter, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_WitchAsk::
	msgbox {NAME}_Text_WitchAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_WitchLater::
	msgbox {NAME}_Text_WitchLater, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_WitchFull::
	msgbox {NAME}_Text_WitchFull, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_WitchDone::
	msgbox {NAME}_Text_WitchDone, MSGBOX_DEFAULT
	release
	end

@ ---- a Stunfisk hiding in the mud
{NAME}_EventScript_Stunfisk::
	goto_if_set FLAG_MIRE_STUNFISK_DONE, {NAME}_EventScript_StunfiskEnd
	lockall
	setflag FLAG_MIRE_STUNFISK_DONE
	playse SE_M_THUNDERBOLT
	msgbox {NAME}_Text_Stunfisk, MSGBOX_DEFAULT
	closemessage
	setwildbattle SPECIES_STUNFISK, 38
	dowildbattle
	releaseall
{NAME}_EventScript_StunfiskEnd::
	end

@ ---- Pokémon and people
{NAME}_EventScript_Wooper::
	lock
	faceplayer
	playmoncry SPECIES_WOOPER, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Wooper, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Goomy::
	lock
	faceplayer
	playmoncry SPECIES_GOOMY, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Goomy, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Croagunk::
	lock
	faceplayer
	playmoncry SPECIES_CROAGUNK, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Croagunk, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Finn::
	trainerbattle_single TRAINER_MIRE_FINN, {NAME}_Text_FinnIntro, {NAME}_Text_FinnDefeat
	msgbox {NAME}_Text_FinnPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Marsha::
	trainerbattle_single TRAINER_MIRE_MARSHA, {NAME}_Text_MarshaIntro, {NAME}_Text_MarshaDefeat
	msgbox {NAME}_Text_MarshaPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Otto::
	trainerbattle_single TRAINER_MIRE_OTTO, {NAME}_Text_OttoIntro, {NAME}_Text_OttoDefeat
	msgbox {NAME}_Text_OttoPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Ivy::
	trainerbattle_single TRAINER_MIRE_IVY, {NAME}_Text_IvyIntro, {NAME}_Text_IvyDefeat
	msgbox {NAME}_Text_IvyPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
{NAME}_Text_GuideBack:
	.string "Soaked through? I can lead you back\n"
	.string "to FORTREE.$"

{NAME}_Text_GuideLead:
	.string "Stick to the boardwalks!$"

{NAME}_Text_GuideStay:
	.string "Mind the mud. Some of it bites.$"

{NAME}_Text_Sign:
	.string "MIREBOUGH MARSH\n"
	.string "Boardwalks may be slippery when wet.$"

{NAME}_Text_WitchAsk:
	.string "Heh heh. A visitor in the rain!\p"
	.string "I'm brewing my famous marsh stew,\n"
	.string "but I'm missing three things:\p"
	.string "a TINY MUSHROOM, a BIG ROOT and a\n"
	.string "dab of BLACK SLUDGE.\p"
	.string "They're all somewhere in the marsh.\n"
	.string "Bring them, and I'll make it worth\l"
	.string "your while.$"

{NAME}_Text_WitchHave:
	.string "Oho! You've got all three! Will you\n"
	.string "give them to me for the stew?$"

{NAME}_Text_WitchStew:
	.string "In they go… stir, stir, stir…$"

{NAME}_Text_GotEgg:
	.string "{{PLAYER}} received an EGG from the\n"
	.string "BOG WITCH!$"

{NAME}_Text_WitchAfter:
	.string "Found that egg in the reeds while\n"
	.string "gathering. Something slimy will\l"
	.string "hatch. Slimy things are the best!$"

{NAME}_Text_WitchLater:
	.string "Hmph. The stew can wait. It's been\n"
	.string "waiting forty years.$"

{NAME}_Text_WitchFull:
	.string "Your party's full, dearie. Make\n"
	.string "room and come back.$"

{NAME}_Text_WitchDone:
	.string "The stew is bubbling nicely. Want a\n"
	.string "taste? No? Your loss.$"

{NAME}_Text_Stunfisk:
	.string "Something squelched underfoot!\n"
	.string "It's a STUNFISK!$"

{NAME}_Text_Wooper:
	.string "WOOPER is splashing in the puddles,\n"
	.string "very happy about the rain.$"

{NAME}_Text_Goomy:
	.string "GOOMY is soaking up the rain. It's\n"
	.string "very slimy.$"

{NAME}_Text_Croagunk:
	.string "CROAGUNK puffs out its cheeks and\n"
	.string "croaks at you.$"

{NAME}_Text_FinnIntro:
	.string "The marsh is full of whiskered\n"
	.string "fish! Let me show you!$"

{NAME}_Text_FinnDefeat:
	.string "The big one got away!$"

{NAME}_Text_FinnPost:
	.string "BARBOACH love the muddy water here.$"

{NAME}_Text_MarshaIntro:
	.string "The marsh smells lovely in the rain,\n"
	.string "doesn't it?$"

{NAME}_Text_MarshaDefeat:
	.string "Wilted…$"

{NAME}_Text_MarshaPost:
	.string "I saw a mushroom by the east reeds.\n"
	.string "The witch might want it.$"

{NAME}_Text_OttoIntro:
	.string "Bugs love swamps! I love bugs!$"

{NAME}_Text_OttoDefeat:
	.string "Squashed!$"

{NAME}_Text_OttoPost:
	.string "YANMEGA hunt above the water. Look\n"
	.string "up sometimes!$"

{NAME}_Text_IvyIntro:
	.string "I'm a RANGER. I keep the marsh safe.\n"
	.string "Let's see if you're safe too!$"

{NAME}_Text_IvyDefeat:
	.string "You're clearly no danger.$"

{NAME}_Text_IvyPost:
	.string "Careful near the south reeds. A\n"
	.string "STUNFISK hides in the mud there.$"
''')

register_wild('MAP_MIREBOUGH_MARSH', 'gMireboughMarsh',
              land=[('WOOPER', 34, 36), ('CROAGUNK', 34, 36), ('LOTAD', 34, 36), ('SHROOMISH', 34, 36),
                    ('GOOMY', 34, 36), ('YANMA', 35, 37), ('SKORUPI', 35, 37), ('LOMBRE', 36, 38),
                    ('QUAGSIRE', 37, 38), ('STUNFISK', 36, 38), ('TOXICROAK', 38, 39), ('SLIGGOO', 38, 39)],
              water=[('WOOPER', 30, 35), ('LOTAD', 30, 35), ('QUAGSIRE', 33, 38), ('LOMBRE', 33, 38), ('STUNFISK', 35, 38)],
              fish=[('MAGIKARP', 10, 30), ('BARBOACH', 10, 30), ('BARBOACH', 25, 30), ('POLIWAG', 25, 30), ('GOLDEEN', 25, 30),
                    ('WHISCASH', 30, 35), ('POLIWHIRL', 30, 35), ('SEAKING', 30, 35), ('WHISCASH', 35, 38), ('POLITOED', 38, 40)])
print('ok', NAME)

# ---------------------------------------------------------------- the way in, from Fortree City
add_object('FortreeCity', {'graphics_id': 'OBJ_EVENT_GFX_MAN_4', 'x': 4, 'y': 9, 'elevation': 3,
                           'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                           'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                           'script': 'FortreeCity_EventScript_MireboughGuide', 'flag': '0'})
register_script_file('mirebough', r'''
@ Mirebough Marsh guide in Fortree City (tools/areas/swamp.py).
FortreeCity_EventScript_MireboughGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, FortreeCity_EventScript_MireboughGuideNoBadge
	msgbox FortreeCity_Text_MireboughGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_MireboughGuideNo
	msgbox FortreeCity_Text_MireboughGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_MIREBOUGH_MARSH, 28, 45
	waitstate
	release
	end

FortreeCity_EventScript_MireboughGuideNoBadge::
	msgbox FortreeCity_Text_MireboughGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_MireboughGuideNo::
	msgbox FortreeCity_Text_MireboughGuideNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_MireboughGuideNoBadge:
	.string "West of here, where it always rains,\n"
	.string "is MIREBOUGH MARSH.\p"
	.string "It's easy to get lost. Come back\n"
	.string "with WINONA's badge.$"

FortreeCity_Text_MireboughGuideAsk:
	.string "WINONA's badge! Then you won't get\n"
	.string "lost.\p"
	.string "Shall I take you to MIREBOUGH\n"
	.string "MARSH?$"

FortreeCity_Text_MireboughGuideGo:
	.string "Bring an umbrella!$"

FortreeCity_Text_MireboughGuideNo:
	.string "The rain will keep. It never stops.$"
''')
print('ok entrance')
