"""Hollowmere: an abandoned town that only appears at night, with the
haunted Hollowmere Manor. The old caretaker's four ghost children have
wandered off; find them all and she gives you the manor key, opening the
upper floor where a Mimikyu hides.

    python3 tools/areas/ghosttown.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import town, POND

AREA_NO = 6
NAME = 'Hollowmere'
MAPSEC = 'MAPSEC_HOLLOWMERE'
PETAL = 'LAYOUT_PETALBURG_CITY'
GHOSTS = [('GASTLY', 'Gastly'), ('DUSKULL', 'Duskull'), ('SHUPPET', 'Shuppet'), ('MISDREAVUS', 'Misdreavus')]

register_constants(NAME, AREA_NO, flags=[
    'HOLLOW_GHOST_GASTLY', 'HOLLOW_GHOST_DUSKULL', 'HOLLOW_GHOST_SHUPPET', 'HOLLOW_GHOST_MISDREAVUS',
    'HOLLOW_QUEST_STARTED', 'HOLLOW_MANOR_KEY', 'HIDE_HOLLOW_MIMIKYU', 'HOLLOW_MIMIKYU_DONE',
    'ITEM_HOLLOW_DUSK_STONE', 'ITEM_HOLLOW_TM_SHADOW_BALL', 'ITEM_HOLLOW_REAPER_CLOTH', 'ITEM_HOLLOW_MAX_ELIXIR',
    'ITEM_HOLLOW_RARE_CANDY', 'HIDDEN_ITEM_HOLLOW_PEARL', 'HIDDEN_ITEM_HOLLOW_NUGGET', 'HIDDEN_ITEM_HOLLOW_CLEANSE_TAG',
], trainers=['HOLLOW_VESPER', 'HOLLOW_LUNA', 'HOLLOW_MORTIMER', 'HOLLOW_ELVIRA'])

register_trainers(NAME, '''
=== AREA_TRAINER_HOLLOW_VESPER ===
Name: VESPER
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Haunter
Level: 39

Pumpkaboo
Level: 39

Lampent
Level: 40

=== AREA_TRAINER_HOLLOW_LUNA ===
Name: LUNA
Class: Psychic
Pic: Psychic F
Gender: Female
Music: Intense
Double Battle: No
AI: Basic Trainer

Mismagius
Level: 40

Musharna
Level: 40

=== AREA_TRAINER_HOLLOW_MORTIMER ===
Name: MORTIMER
Class: Gentleman
Pic: Gentleman
Gender: Male
Music: Rich
Double Battle: No
AI: Basic Trainer

Sinistea
Level: 40

Dusclops
Level: 41

=== AREA_TRAINER_HOLLOW_ELVIRA ===
Name: ELVIRA
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Banette
Level: 41

Froslass
Level: 41

Gengar
Level: 42
''')

# ---------------------------------------------------------------- the town
a = Area(NAME, 44, 40, town(), fill='T')
a.ellipse(22, 21, 19, 16, 'g')
a.rect(19, 34, 7, 6, 'g')
a.ellipse(34, 30, 5, 3, 'P')
a.autotile3('P', POND)
HOUSES = {'H1': (6, 9), 'H2': (31, 9), 'H3': (6, 25), 'H4': (24, 27)}
for x, y in HOUSES.values():
    a.stamp(PETAL, 9, 16, 4, 4, x, y)
a.stamp(PETAL, 11, 4, 8, 6, 18, 2)                    # the manor, door at (22, 6)
for x, y, mt in ((20, 4, 0x1B3), (21, 4, 0x1B3), (20, 5, 0x1C3), (21, 5, 0x1C3)):
    a.tile(x, y, mt, collision=1, elevation=0, c='o')  # paint over the old gym sign
for pts in ([(22, 38), (22, 8)], [(8, 14), (36, 14)], [(8, 30), (8, 14)], [(26, 31), (22, 31)],
            [(33, 14), (33, 18)]):
    a.line(pts, 1, 's')
a.ellipse(13, 20, 4, 2.5, '"')
a.ellipse(36, 22, 3, 2, '"')
a.ellipse(14, 35, 4, 2, 'T')
a.ellipse(30, 36, 3, 2, 'T')
a.ellipse(28, 18, 2, 2, 'T')
a.snap_trees(ground='g')
W_H1 = a.warp(7, 12, 'MAP_HOLLOWMERE_INN', 0)
W_H3 = a.warp(7, 28, 'MAP_HOLLOWMERE_HOUSE', 0)
W_MANOR = a.warp(22, 6, 'MAP_HOLLOWMERE_MANOR_1F', 0)
a.sign(32, 12, f'{NAME}_EventScript_Boarded', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(25, 30, f'{NAME}_EventScript_Boarded', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(20, 35, f'{NAME}_EventScript_Sign', elevation=0)
a.obj('OBJ_EVENT_GFX_OLD_MAN', 23, 37, f'{NAME}_EventScript_Ferryman', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_OLD_WOMAN', 20, 10, f'{NAME}_EventScript_Caretaker', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SPECIES(GASTLY)', 4, 20, f'{NAME}_EventScript_Gastly', 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN',
      'FLAG_HOLLOW_GHOST_GASTLY', ry=2)
a.obj('OBJ_EVENT_GFX_SPECIES(DUSKULL)', 39, 22, f'{NAME}_EventScript_Duskull', 'MOVEMENT_TYPE_LOOK_AROUND',
      'FLAG_HOLLOW_GHOST_DUSKULL')
a.obj('OBJ_EVENT_GFX_SPECIES(DRIFLOON)', 16, 27, f'{NAME}_EventScript_Drifloon', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=2)
a.obj('OBJ_EVENT_GFX_BOY_1', 27, 22, f'{NAME}_EventScript_Boy', 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 12, 16, f'{NAME}_EventScript_Vesper', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_LASS', 36, 18, f'{NAME}_EventScript_Luna', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_DUSK_STONE', 3, 23, 'FLAG_ITEM_HOLLOW_DUSK_STONE')
a.item('ITEM_TM_SHADOW_BALL', 40, 26, 'FLAG_ITEM_HOLLOW_TM_SHADOW_BALL')
a.hidden(30, 31, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_HOLLOW_PEARL')
a.hidden(10, 33, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_HOLLOW_NUGGET')

GHOST_FOUND = ''.join(f'''{NAME}_EventScript_{name}::
	lock
	faceplayer
	playmoncry SPECIES_{sp}, CRY_MODE_NORMAL
	msgbox {NAME}_Text_{name}, MSGBOX_DEFAULT
	waitmoncry
	goto_if_unset FLAG_HOLLOW_QUEST_STARTED, {NAME}_EventScript_GhostIgnore
	setflag FLAG_HOLLOW_GHOST_{sp}
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_GhostHome, MSGBOX_DEFAULT
	release
	end

''' for sp, name in GHOSTS)

a.write(map_id='MAP_HOLLOWMERE', layout_id='LAYOUT_HOLLOWMERE', music='MUS_MT_PYRE_EXTERIOR', mapsec=MAPSEC,
        mapsec_name='HOLLOWMERE', weather='WEATHER_FOG_HORIZONTAL', map_type='MAP_TYPE_TOWN',
        after='SunscarRuins_Vault', scripts=rf'''
{NAME}_MapScripts::
	.byte 0

{NAME}_EventScript_Ferryman::
	lock
	faceplayer
	msgbox {NAME}_Text_FerrymanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_FerrymanStay
	msgbox {NAME}_Text_FerrymanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_LILYCOVE_CITY, 27, 17
	waitstate
	release
	end

{NAME}_EventScript_FerrymanStay::
	msgbox {NAME}_Text_FerrymanStay, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Sign::
	msgbox {NAME}_Text_Sign, MSGBOX_SIGN
	end

{NAME}_EventScript_Boarded::
	msgbox {NAME}_Text_Boarded, MSGBOX_SIGN
	end

@ ---- the caretaker and her four lost ghosts
{NAME}_EventScript_Caretaker::
	lock
	faceplayer
	goto_if_set FLAG_HOLLOW_MANOR_KEY, {NAME}_EventScript_CaretakerDone
	goto_if_unset FLAG_HOLLOW_GHOST_GASTLY, {NAME}_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_DUSKULL, {NAME}_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_SHUPPET, {NAME}_EventScript_CaretakerAsk
	goto_if_unset FLAG_HOLLOW_GHOST_MISDREAVUS, {NAME}_EventScript_CaretakerAsk
	msgbox {NAME}_Text_CaretakerThanks, MSGBOX_DEFAULT
	giveitem ITEM_SPELL_TAG
	setflag FLAG_HOLLOW_MANOR_KEY
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotKey, MSGBOX_DEFAULT
	waitfanfare
	msgbox {NAME}_Text_CaretakerKey, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_CaretakerAsk::
	setflag FLAG_HOLLOW_QUEST_STARTED
	msgbox {NAME}_Text_CaretakerAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_CaretakerDone::
	msgbox {NAME}_Text_CaretakerDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GhostIgnore::
	release
	end

{GHOST_FOUND}{NAME}_EventScript_Drifloon::
	lock
	faceplayer
	playmoncry SPECIES_DRIFLOON, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Drifloon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Boy::
	msgbox {NAME}_Text_Boy, MSGBOX_NPC
	end

{NAME}_EventScript_Vesper::
	trainerbattle_single TRAINER_HOLLOW_VESPER, {NAME}_Text_VesperIntro, {NAME}_Text_VesperDefeat
	msgbox {NAME}_Text_VesperPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Luna::
	trainerbattle_single TRAINER_HOLLOW_LUNA, {NAME}_Text_LunaIntro, {NAME}_Text_LunaDefeat
	msgbox {NAME}_Text_LunaPost, MSGBOX_AUTOCLOSE
	end

{NAME}_Text_FerrymanBack:
	.string "The mist is lifting soon. Shall I\n"
	.string "row you back to LILYCOVE?$"

{NAME}_Text_FerrymanGo:
	.string "Don't look back at the lights…$"

{NAME}_Text_FerrymanStay:
	.string "I'll wait. I always wait.$"

{NAME}_Text_Sign:
	.string "HOLLOWMERE\n"
	.string "Where the lamps are never lit.$"

{NAME}_Text_Boarded:
	.string "The door is boarded up. Something\n"
	.string "scratches on the other side…$"

{NAME}_Text_CaretakerAsk:
	.string "Oh, a living visitor! How rare.\p"
	.string "I look after this town's little\n"
	.string "ghosts. Four of them slipped out\l"
	.string "tonight and won't come home.\p"
	.string "A GASTLY, a DUSKULL, a SHUPPET and\n"
	.string "a MISDREAVUS. Two are out here, one\l"
	.string "is in a house and one in the manor.\p"
	.string "Please, find them and send them\n"
	.string "home to me.$"

{NAME}_Text_CaretakerThanks:
	.string "All four are home! Thank you, dear.\n"
	.string "Take this, and this old key too.$"

{NAME}_Text_GotKey:
	.string "{{PLAYER}} received the MANOR KEY!$"

{NAME}_Text_CaretakerKey:
	.string "It opens the upper floor of the\n"
	.string "manor. Nobody has gone up there\l"
	.string "for a hundred years…$"

{NAME}_Text_CaretakerDone:
	.string "The little ones are sleeping. Hush,\n"
	.string "now.$"

{NAME}_Text_GhostHome:
	.string "It giggled and drifted off home!$"

{NAME}_Text_Gastly:
	.string "GASTLY is hiding behind a tree,\n"
	.string "giggling.$"

{NAME}_Text_Duskull:
	.string "DUSKULL is staring at the pond.$"

{NAME}_Text_Drifloon:
	.string "DRIFLOON bobs in the air. It seems\n"
	.string "to want to hold your hand…$"

{NAME}_Text_Boy:
	.string "My grandma says this town is only\n"
	.string "here at night. In the morning it's\l"
	.string "just fog over the water!$"

{NAME}_Text_VesperIntro:
	.string "The spirits whispered your name.\n"
	.string "Shall we battle, hehehe?$"

{NAME}_Text_VesperDefeat:
	.string "The spirits lied to me!$"

{NAME}_Text_VesperPost:
	.string "The manor's master never left. They\n"
	.string "say he still walks upstairs.$"

{NAME}_Text_LunaIntro:
	.string "I can see the future. You lose.$"

{NAME}_Text_LunaDefeat:
	.string "I need new glasses.$"

{NAME}_Text_LunaPost:
	.string "A GASTLY was giggling near the west\n"
	.string "trees. Or will be. Time is funny.$"
''')
register_wild('MAP_HOLLOWMERE', 'gHollowmere',
              land=[('GASTLY', 37, 39), ('MISDREAVUS', 37, 39), ('DUSKULL', 37, 39), ('SHUPPET', 37, 39),
                    ('PHANTUMP', 37, 39), ('PUMPKABOO', 37, 39), ('LITWICK', 37, 39), ('DRIFLOON', 37, 39),
                    ('YAMASK', 38, 40), ('GREAVARD', 38, 40), ('HAUNTER', 40, 41), ('MISMAGIUS', 41, 42)],
              water=[('BASCULIN', 30, 35), ('FRILLISH', 35, 40), ('FRILLISH', 35, 40), ('JELLICENT', 38, 42), ('JELLICENT', 38, 42)],
              fish=[('MAGIKARP', 10, 30), ('FEEBAS', 10, 30), ('BASCULIN', 25, 30), ('FRILLISH', 25, 30), ('BARBOACH', 25, 30),
                    ('FRILLISH', 30, 35), ('WHISCASH', 30, 35), ('JELLICENT', 30, 35), ('JELLICENT', 35, 38), ('GYARADOS', 35, 40)])

# ---------------------------------------------------------------- houses
interior('Hollowmere_Inn', 'MAP_HOLLOWMERE_INN', 'LAYOUT_HOUSE1', [(3, 8), (4, 8)], 'MAP_HOLLOWMERE', W_H1, [
    npc('OBJ_EVENT_GFX_WOMAN_2', 5, 4, 'Hollowmere_Inn_EventScript_Keeper', 'MOVEMENT_TYPE_FACE_DOWN'),
], r'''
Hollowmere_Inn_MapScripts::
	.byte 0

Hollowmere_Inn_EventScript_Keeper::
	lock
	faceplayer
	msgbox Hollowmere_Inn_Text_Rest, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Hollowmere_Inn_EventScript_No
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Inn_Text_Rested, MSGBOX_DEFAULT
	release
	end

Hollowmere_Inn_EventScript_No::
	msgbox Hollowmere_Inn_Text_No, MSGBOX_DEFAULT
	release
	end

Hollowmere_Inn_Text_Rest:
	.string "Welcome to the last inn in\n"
	.string "HOLLOWMERE. Will you rest a while?$"

Hollowmere_Inn_Text_Rested:
	.string "You slept soundly. Odd, the bed\n"
	.string "was cold the whole time…$"

Hollowmere_Inn_Text_No:
	.string "The bed will be here. It always is.$"
''', MAPSEC, 'gMapGroup_TownsAndRoutes')

interior('Hollowmere_House', 'MAP_HOLLOWMERE_HOUSE', 'LAYOUT_HOUSE1', [(3, 8), (4, 8)], 'MAP_HOLLOWMERE', W_H3, [
    npc('OBJ_EVENT_GFX_SPECIES(SHUPPET)', 7, 3, 'Hollowmere_EventScript_Shuppet', 'MOVEMENT_TYPE_LOOK_AROUND',
        flag='FLAG_HOLLOW_GHOST_SHUPPET'),
    npc('OBJ_EVENT_GFX_ITEM_BALL', 1, 3, 'Hollowmere_House_EventScript_MaxElixir', 'MOVEMENT_TYPE_LOOK_AROUND',
        flag='FLAG_ITEM_HOLLOW_MAX_ELIXIR'),
], r'''
Hollowmere_House_MapScripts::
	.byte 0

Hollowmere_House_EventScript_MaxElixir::
	finditem ITEM_MAX_ELIXIR
	end

Hollowmere_Text_Shuppet:
	.string "SHUPPET is nibbling on an old,\n"
	.string "sad diary.$"
''', MAPSEC, 'gMapGroup_TownsAndRoutes')

# ---------------------------------------------------------------- the manor (dark: needs Flash to see far)
MUSEUM1, MUSEUM2 = 'LAYOUT_LILYCOVE_CITY_LILYCOVE_MUSEUM_1F', 'LAYOUT_LILYCOVE_CITY_LILYCOVE_MUSEUM_2F'
PORTRAITS = [(1, 1), (2, 1), (5, 1), (6, 1), (9, 1), (11, 1), (19, 1), (20, 1)]
interior('Hollowmere_Manor_1F', 'MAP_HOLLOWMERE_MANOR_1F', MUSEUM1, [(9, 13), (10, 13)], 'MAP_HOLLOWMERE', W_MANOR, [
    npc('OBJ_EVENT_GFX_GENTLEMAN', 16, 2, 'Hollowmere_Manor_1F_EventScript_Butler', 'MOVEMENT_TYPE_FACE_DOWN',
        local_id='LOCALID_HOLLOW_BUTLER'),
    npc('OBJ_EVENT_GFX_SPECIES(MISDREAVUS)', 4, 8, 'Hollowmere_EventScript_Misdreavus', 'MOVEMENT_TYPE_LOOK_AROUND',
        flag='FLAG_HOLLOW_GHOST_MISDREAVUS'),
    {**npc('OBJ_EVENT_GFX_GENTLEMAN', 13, 8, 'Hollowmere_Manor_1F_EventScript_Mortimer', 'MOVEMENT_TYPE_FACE_LEFT'),
     'trainer_type': 'TRAINER_TYPE_NORMAL', 'trainer_sight_or_berry_tree_id': '3', 'movement_range_x': 0,
     'movement_range_y': 0},
    npc('OBJ_EVENT_GFX_ITEM_BALL', 20, 9, 'Hollowmere_Manor_1F_EventScript_RareCandy', 'MOVEMENT_TYPE_LOOK_AROUND',
        flag='FLAG_ITEM_HOLLOW_RARE_CANDY'),
], r'''
Hollowmere_Manor_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Hollowmere_Manor_1F_OnTransition
	.byte 0

@ With the key, the butler stands aside from the stairs.
Hollowmere_Manor_1F_OnTransition:
	goto_if_unset FLAG_HOLLOW_MANOR_KEY, Hollowmere_Manor_1F_OnTransitionEnd
	setobjectxyperm LOCALID_HOLLOW_BUTLER, 17, 3
	setobjectmovementtype LOCALID_HOLLOW_BUTLER, MOVEMENT_TYPE_FACE_LEFT
Hollowmere_Manor_1F_OnTransitionEnd:
	end

Hollowmere_Manor_1F_EventScript_Butler::
	lock
	faceplayer
	goto_if_set FLAG_HOLLOW_MANOR_KEY, Hollowmere_Manor_1F_EventScript_ButlerOk
	msgbox Hollowmere_Manor_1F_Text_ButlerNo, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_1F_EventScript_ButlerOk::
	msgbox Hollowmere_Manor_1F_Text_ButlerOk, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_1F_EventScript_Portrait::
	msgbox Hollowmere_Manor_1F_Text_Portrait, MSGBOX_SIGN
	end

Hollowmere_Manor_1F_EventScript_Mortimer::
	trainerbattle_single TRAINER_HOLLOW_MORTIMER, Hollowmere_Manor_1F_Text_MortimerIntro, Hollowmere_Manor_1F_Text_MortimerDefeat
	msgbox Hollowmere_Manor_1F_Text_MortimerPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_Manor_1F_EventScript_RareCandy::
	finditem ITEM_RARE_CANDY
	end

Hollowmere_Text_Misdreavus:
	.string "MISDREAVUS is humming a song to\n"
	.string "the portraits.$"

Hollowmere_Manor_1F_Text_ButlerNo:
	.string "The upper floor is closed to\n"
	.string "guests. Only the caretaker holds\l"
	.string "the key.$"

Hollowmere_Manor_1F_Text_ButlerOk:
	.string "Ah, you have the key. The master\n"
	.string "is expecting you. Upstairs, please.$"

Hollowmere_Manor_1F_Text_Portrait:
	.string "A dusty portrait. Its eyes seem to\n"
	.string "follow you around the room…$"

Hollowmere_Manor_1F_Text_MortimerIntro:
	.string "Welcome to the manor. I insist you\n"
	.string "stay for a battle.$"

Hollowmere_Manor_1F_Text_MortimerDefeat:
	.string "How rude of me to lose.$"

Hollowmere_Manor_1F_Text_MortimerPost:
	.string "The master upstairs loves dolls.\n"
	.string "One of them moves on its own.$"
''', MAPSEC, 'gMapGroup_TownsAndRoutes', music='MUS_MT_PYRE', requires_flash=True,
         bgs=[{'type': 'sign', 'x': x, 'y': y, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
               'script': 'Hollowmere_Manor_1F_EventScript_Portrait'} for x, y in PORTRAITS],
         extra_warps=[{'x': 16, 'y': 1, 'elevation': 0, 'dest_map': 'MAP_HOLLOWMERE_MANOR_2F', 'dest_warp_id': '0'}])

interior('Hollowmere_Manor_2F', 'MAP_HOLLOWMERE_MANOR_2F', MUSEUM2, [(13, 1)], 'MAP_HOLLOWMERE_MANOR_1F', 2, [
    npc('OBJ_EVENT_GFX_SPECIES(MIMIKYU)', 10, 8, 'Hollowmere_Manor_2F_EventScript_Mimikyu', 'MOVEMENT_TYPE_FACE_DOWN',
        local_id='LOCALID_HOLLOW_MIMIKYU', flag='FLAG_HIDE_HOLLOW_MIMIKYU'),
    {**npc('OBJ_EVENT_GFX_HEX_MANIAC', 19, 10, 'Hollowmere_Manor_2F_EventScript_Elvira', 'MOVEMENT_TYPE_FACE_LEFT'),
     'trainer_type': 'TRAINER_TYPE_NORMAL', 'trainer_sight_or_berry_tree_id': '3', 'movement_range_x': 0,
     'movement_range_y': 0},
    npc('OBJ_EVENT_GFX_ITEM_BALL', 7, 3, 'Hollowmere_Manor_2F_EventScript_ReaperCloth', 'MOVEMENT_TYPE_LOOK_AROUND',
        flag='FLAG_ITEM_HOLLOW_REAPER_CLOTH'),
], r'''
Hollowmere_Manor_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Hollowmere_Manor_2F_OnTransition
	.byte 0

Hollowmere_Manor_2F_OnTransition:
	call_if_set FLAG_HOLLOW_MIMIKYU_DONE, Hollowmere_Manor_2F_EventScript_Hide
	end

Hollowmere_Manor_2F_EventScript_Hide::
	setflag FLAG_HIDE_HOLLOW_MIMIKYU
	return

Hollowmere_Manor_2F_EventScript_Mimikyu::
	lock
	faceplayer
	msgbox Hollowmere_Manor_2F_Text_Doll, MSGBOX_DEFAULT
	setwildbattle SPECIES_MIMIKYU, 40
	playmoncry SPECIES_MIMIKYU, CRY_MODE_ENCOUNTER
	msgbox Hollowmere_Manor_2F_Text_Mimikyu, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, Hollowmere_Manor_2F_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, Hollowmere_Manor_2F_EventScript_Stays
	setflag FLAG_HOLLOW_MIMIKYU_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Hollowmere_Manor_2F_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_HOLLOW_MIMIKYU
	setflag FLAG_HIDE_HOLLOW_MIMIKYU
	fadescreen FADE_FROM_BLACK
	msgbox Hollowmere_Manor_2F_Text_Gone, MSGBOX_DEFAULT
Hollowmere_Manor_2F_EventScript_End::
	release
	end

Hollowmere_Manor_2F_EventScript_Stays::
	msgbox Hollowmere_Manor_2F_Text_Stays, MSGBOX_DEFAULT
	release
	end

Hollowmere_Manor_2F_EventScript_Elvira::
	trainerbattle_single TRAINER_HOLLOW_ELVIRA, Hollowmere_Manor_2F_Text_ElviraIntro, Hollowmere_Manor_2F_Text_ElviraDefeat
	msgbox Hollowmere_Manor_2F_Text_ElviraPost, MSGBOX_AUTOCLOSE
	end

Hollowmere_Manor_2F_EventScript_ReaperCloth::
	finditem ITEM_REAPER_CLOTH
	end

Hollowmere_Manor_2F_EventScript_Case::
	msgbox Hollowmere_Manor_2F_Text_Case, MSGBOX_SIGN
	end

Hollowmere_Manor_2F_Text_Doll:
	.string "A doll in a ragged PIKACHU costume\n"
	.string "sits alone… It just moved!$"

Hollowmere_Manor_2F_Text_Mimikyu:
	.string "Mimikyu!$"

Hollowmere_Manor_2F_Text_Gone:
	.string "MIMIKYU slipped away into the\n"
	.string "shadows.$"

Hollowmere_Manor_2F_Text_Stays:
	.string "The doll is sitting very still\n"
	.string "again.$"

Hollowmere_Manor_2F_Text_Case:
	.string "A glass case full of old dolls.\n"
	.string "One of the spaces is empty.$"

Hollowmere_Manor_2F_Text_ElviraIntro:
	.string "You woke the master's dolls! Now\n"
	.string "you must play with us!$"

Hollowmere_Manor_2F_Text_ElviraDefeat:
	.string "Playtime is over…$"

Hollowmere_Manor_2F_Text_ElviraPost:
	.string "The master was lonely. That's why\n"
	.string "he kept so many dolls.$"
''', MAPSEC, 'gMapGroup_TownsAndRoutes', music='MUS_MT_PYRE', requires_flash=True,
         bgs=[{'type': 'sign', 'x': x, 'y': y, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_ANY',
               'script': 'Hollowmere_Manor_2F_EventScript_Case'} for x, y in ((2, 6), (10, 6), (18, 6), (6, 10), (14, 10))])
register_wild('MAP_HOLLOWMERE_MANOR_1F', 'gHollowmereManor1F',
              land=[('GASTLY', 38, 40), ('SHUPPET', 38, 40), ('SINISTEA', 38, 40), ('HONEDGE', 38, 40),
                    ('SABLEYE', 38, 40), ('HAUNTER', 40, 41), ('BANETTE', 40, 41), ('DUSCLOPS', 40, 41),
                    ('MISDREAVUS', 38, 40), ('LITWICK', 38, 40), ('POLTEAGEIST', 42, 42), ('DOUBLADE', 42, 42)], land_rate=6)
print('ok', NAME)

# ---------------------------------------------------------------- the way in, from Lilycove City at night
add_object('LilycoveCity', {'graphics_id': 'OBJ_EVENT_GFX_OLD_MAN', 'x': 27, 'y': 16, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'LilycoveCity_EventScript_HollowmereFerryman', 'flag': '0'})
register_script_file('hollowmere', r'''
@ The Hollowmere ferryman in Lilycove City (tools/areas/ghosttown.py). He
@ only rows out at night, when the town appears.
LilycoveCity_EventScript_HollowmereFerryman::
	lock
	faceplayer
	gettimeofday
	goto_if_ne VAR_0x8000, TIME_NIGHT, LilycoveCity_EventScript_HollowmereDay
	goto_if_unset FLAG_BADGE07_GET, LilycoveCity_EventScript_HollowmereNoBadge
	msgbox LilycoveCity_Text_HollowmereAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LilycoveCity_EventScript_HollowmereNo
	msgbox LilycoveCity_Text_HollowmereGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_HOLLOWMERE, 22, 37
	waitstate
	release
	end

LilycoveCity_EventScript_HollowmereDay::
	msgbox LilycoveCity_Text_HollowmereDay, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_HollowmereNoBadge::
	msgbox LilycoveCity_Text_HollowmereNoBadge, MSGBOX_DEFAULT
	release
	end

LilycoveCity_EventScript_HollowmereNo::
	msgbox LilycoveCity_Text_HollowmereNo, MSGBOX_DEFAULT
	release
	end

LilycoveCity_Text_HollowmereDay:
	.string "Out past MT. PYRE there's a town\n"
	.string "called HOLLOWMERE.\p"
	.string "You can only find it at night. Come\n"
	.string "back after dark and I'll row you.$"

LilycoveCity_Text_HollowmereNoBadge:
	.string "The mist is thick tonight. Only\n"
	.string "those with the MIND BADGE may cross\l"
	.string "to HOLLOWMERE.$"

LilycoveCity_Text_HollowmereAsk:
	.string "The mist is up and the lamps are\n"
	.string "out. Shall I row you to HOLLOWMERE?$"

LilycoveCity_Text_HollowmereGo:
	.string "Sit still, and don't trail your\n"
	.string "hand in the water…$"

LilycoveCity_Text_HollowmereNo:
	.string "HOLLOWMERE will be there tomorrow\n"
	.string "night. And the next.$"
''')
print('ok entrance')
