"""Whisperstem Valley: a bamboo valley west of Petalburg, an early-game
area (no badge needed). Bamboo groves wall off a winding set of glades: a
shrine gate, a teahouse whose three Pancham have run off into the bamboo,
a koi pond with a plank bridge, a meadow of trainers, and a secret grove
behind a stand of bamboo you can slip through (a Mienfoo trains there).

New art (bamboo, teahouse, lanterns, shrine gate) is drawn in
tools/areas/bamboo_art.py (the bamboo tileset: Rustboro's plus the art).

    python3 tools/areas/bamboo_art.py     # the tileset, when the art changes
    python3 tools/areas/whisperstem.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import woods

AREA_NO = 11
NAME, TEA = 'WhisperstemValley', 'WhisperstemValley_Teahouse'
MAPSEC = 'MAPSEC_WHISPERSTEM_VALLEY'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'bamboo_tiles.json')))
MT = ART['objects']

register_constants(NAME, AREA_NO, flags=[
    'BAMBOO_PANCHAM_1', 'BAMBOO_PANCHAM_2', 'BAMBOO_PANCHAM_3', 'HIDE_BAMBOO_HOME_1', 'HIDE_BAMBOO_HOME_2',
    'HIDE_BAMBOO_HOME_3', 'BAMBOO_TEA_DONE', 'BAMBOO_MIENFOO_DONE', 'ITEM_BAMBOO_SUPER_POTION',
    'ITEM_BAMBOO_GREAT_BALL', 'ITEM_BAMBOO_BIG_PEARL', 'ITEM_BAMBOO_TM_BULK_UP', 'HIDDEN_ITEM_BAMBOO_TINY_MUSHROOM',
    'HIDDEN_ITEM_BAMBOO_RARE_CANDY', 'HIDDEN_ITEM_BAMBOO_REVIVE', 'ITEM_BAMBOO_ETHER',
], trainers=['BAMBOO_JUN', 'BAMBOO_LILA', 'BAMBOO_MEI', 'BAMBOO_KAITO'])

register_trainers(NAME, '''
=== AREA_TRAINER_BAMBOO_JUN ===
Name: JUN
Class: Bug Catcher
Pic: Bug Catcher
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Nincada
Level: 10

Sewaddle
Level: 11

=== AREA_TRAINER_BAMBOO_LILA ===
Name: LILA
Class: Picnicker
Pic: Picnicker
Gender: Female
Music: Girl
Double Battle: No
AI: Basic Trainer

Bounsweet
Level: 11

Skiddo
Level: 12

=== AREA_TRAINER_BAMBOO_MEI ===
Name: MEI
Class: Battle Girl
Pic: Battle Girl
Gender: Female
Music: Intense
Double Battle: No
AI: Basic Trainer

Makuhita
Level: 12

Meditite
Level: 12

=== AREA_TRAINER_BAMBOO_KAITO ===
Name: KAITO
Class: Ninja Boy
Pic: Ninja Boy
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Shroomish
Level: 12

Nincada
Level: 12

Pancham
Level: 13
''')


def place(a, name, x, y, ground_char='o'):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c=ground_char)


# ---------------------------------------------------------------- the valley
st = copy.copy(woods())
st.secondary = 'gTileset_Bamboo'              # Rustboro's tiles plus the bamboo art, same metatile ids
st.exclude = set(st.exclude) | {0x122, 0x333, 0x334, 0x344, 0x346, 0x347, 0x34C, 0x34D, 0x34E, 0x34F, 0x350, 0x351}  # bridges, sand
W, H = 56, 48
a = Area(NAME, W, H, st, fill='B')
OPEN = [(18, 40, 20, 6),                        # the entrance glade
        (26, 30, 4, 10),                        # the path through the shrine gate
        (18, 24, 22, 6),                        # the crossroads
        (26, 18, 4, 6), (20, 4, 16, 14),        # the teahouse glade
        (16, 26, 2, 2), (2, 2, 14, 38),         # the bamboo maze in the west
        (10, 40, 8, 4),
        (38, 14, 16, 28), (36, 40, 2, 2),       # the koi pond and the meadow
        (40, 10, 14, 4),                        # the north bank, only over the bridge
        (42, 2, 12, 6)]                         # the secret grove
for r in OPEN:
    a.rect(*r, '.')
for r in ((2, 10, 10, 2), (6, 18, 10, 2), (2, 26, 10, 2), (6, 32, 10, 2),   # the maze walls
          (8, 4, 2, 2), (12, 22, 2, 2), (6, 36, 2, 2),
          (20, 4, 2, 4), (34, 4, 2, 2), (18, 40, 2, 2), (36, 44, 2, 2)):
    a.rect(*r, 'B')
a.rect(40, 14, 14, 12, '~')                     # the koi pond
for e in ((4, 4, 3, 2), (12, 14, 3, 2), (4, 22, 3, 2), (12, 30, 2, 1), (4, 37, 3, 1),
          (44, 36, 6, 3), (50, 29, 3, 2), (40, 39, 2, 1)):
    a.ellipse(*e, '"')
a.snap_trees('B', ground='.')

# the parted bamboo into the secret grove, and the bamboo itself
PARTED = {(46, 8), (47, 8), (46, 9), (47, 9)}
cells = {(x, y) for y in range(H) for x in range(W) if a.grid[y][x] == 'B'}
for (x, y) in cells:
    up = y == 0 or (x, y - 1) in cells
    dn = y == H - 1 or (x, y + 1) in cells
    part = 'mid' if up and dn else ('top' if not up else 'bottom')
    if (x, y) in PARTED:
        part = 'top' if y == 8 else 'bottom'
        a.tile(x, y, MT[f'bamboo_{part}_parted'][0][x % 2], collision=0, c='o')
    else:
        a.tile(x, y, MT[f'bamboo_{part}'][0][x % 2], collision=1, c='o')
a.border = [MT['bamboo_mid'][0][0], MT['bamboo_mid'][0][1], MT['bamboo_mid'][0][0], MT['bamboo_mid'][0][1]]

# the plank bridge across the pond (Route 104's)
a.tile(46, 14, 0x350, elevation=3, c='o')
a.tile(47, 14, 0x351, elevation=3, c='o')
for y in range(15, 26):
    a.tile(46, y, 0x34E, elevation=3, c='o')
    a.tile(47, y, 0x34F, elevation=3, c='o')

TEAHOUSE = (25, 5)                              # 5x4; its door at (27, 8)
place(a, 'teahouse', *TEAHOUSE)
GATE = (27, 34)                                 # the path runs through (28, 34..36)
place(a, 'gate', *GATE, ground_char='.')
for x, y in ((26, 35), (23, 7), (31, 7), (24, 25), (37, 25), (19, 43), (40, 11)):
    place(a, 'lantern', x, y)
for y in range(9, 18, 2):
    place(a, 'stepping_stone', 27, y, ground_char='.')
    place(a, 'stepping_stone', 28, y + 1, ground_char='.')

ARRIVE = (29, 44)
a.obj('OBJ_EVENT_GFX_WOMAN_2', 30, 44, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(26, 36, f'{NAME}_EventScript_Sign')
a.obj('OBJ_EVENT_GFX_OLD_MAN', 33, 12, f'{NAME}_EventScript_OldMan', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_FISHERMAN', 42, 26, f'{NAME}_EventScript_Fisher', 'MOVEMENT_TYPE_FACE_UP')
a.obj('OBJ_EVENT_GFX_LITTLE_GIRL', 22, 27, f'{NAME}_EventScript_Girl', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
PANCHAM = [(3, 3), (52, 11), (3, 22)]
for i, (x, y) in enumerate(PANCHAM, 1):
    a.obj('OBJ_EVENT_GFX_SPECIES(PANCHAM)', x, y, f'{NAME}_EventScript_Pancham{i}', 'MOVEMENT_TYPE_LOOK_AROUND',
          f'FLAG_BAMBOO_PANCHAM_{i}')
a.obj('OBJ_EVENT_GFX_SPECIES(MIENFOO)', 47, 4, f'{NAME}_EventScript_Mienfoo', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_BAMBOO_MIENFOO_DONE')
a.trainer('OBJ_EVENT_GFX_BUG_CATCHER', 8, 14, f'{NAME}_EventScript_Jun', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_PICNICKER', 13, 29, f'{NAME}_EventScript_Lila', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_CRUSH_GIRL', 34, 27, f'{NAME}_EventScript_Mei', 4, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_NINJA_BOY', 48, 34, f'{NAME}_EventScript_Kaito', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.item('ITEM_SUPER_POTION', 13, 3, 'FLAG_ITEM_BAMBOO_SUPER_POTION')
a.item('ITEM_GREAT_BALL', 2, 38, 'FLAG_ITEM_BAMBOO_GREAT_BALL')
a.item('ITEM_BIG_PEARL', 43, 11, 'FLAG_ITEM_BAMBOO_BIG_PEARL')
a.item('ITEM_TM_BULK_UP', 52, 6, 'FLAG_ITEM_BAMBOO_TM_BULK_UP')
a.item('ITEM_ETHER', 52, 40, 'FLAG_ITEM_BAMBOO_ETHER')
a.hidden(19, 29, 'ITEM_TINY_MUSHROOM', 'FLAG_HIDDEN_ITEM_BAMBOO_TINY_MUSHROOM')
a.hidden(43, 2, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_BAMBOO_RARE_CANDY')
a.hidden(53, 41, 'ITEM_REVIVE', 'FLAG_HIDDEN_ITEM_BAMBOO_REVIVE')
a.warp(27, 8, 'MAP_WHISPERSTEM_VALLEY_TEAHOUSE', 0)

a.write(map_id='MAP_WHISPERSTEM_VALLEY', layout_id='LAYOUT_WHISPERSTEM_VALLEY', music='MUS_PETALBURG_WOODS',
        mapsec=MAPSEC, mapsec_name='WHISPERSTEM VALLEY', after='GullwingPoint_PirateHold', scripts=rf'''
{NAME}_MapScripts::
	.byte 0

{NAME}_EventScript_Guide::
	lock
	faceplayer
	msgbox {NAME}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GuideStay
	msgbox {NAME}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_PETALBURG_CITY, 7, 12
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

{NAME}_EventScript_OldMan::
	msgbox {NAME}_Text_OldMan, MSGBOX_NPC
	end

{NAME}_EventScript_Fisher::
	msgbox {NAME}_Text_Fisher, MSGBOX_NPC
	end

{NAME}_EventScript_Girl::
	msgbox {NAME}_Text_Girl, MSGBOX_NPC
	end

@ ---- the tea master's runaway Pancham
{NAME}_EventScript_Pancham1::
	setflag FLAG_BAMBOO_PANCHAM_1
	goto {NAME}_EventScript_PanchamFound

{NAME}_EventScript_Pancham2::
	setflag FLAG_BAMBOO_PANCHAM_2
	goto {NAME}_EventScript_PanchamFound

{NAME}_EventScript_Pancham3::
	setflag FLAG_BAMBOO_PANCHAM_3
	goto {NAME}_EventScript_PanchamFound

{NAME}_EventScript_PanchamFound::
	lock
	faceplayer
	playmoncry SPECIES_PANCHAM, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Pancham, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_PanchamHome, MSGBOX_DEFAULT
	release
	end

@ ---- the secret grove
{NAME}_EventScript_Mienfoo::
	lock
	faceplayer
	msgbox {NAME}_Text_MienfooTrains, MSGBOX_DEFAULT
	setwildbattle SPECIES_MIENFOO, 14
	playmoncry SPECIES_MIENFOO, CRY_MODE_ENCOUNTER
	msgbox {NAME}_Text_Mienfoo, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {NAME}_EventScript_MienfooStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {NAME}_EventScript_MienfooStays
	setflag FLAG_BAMBOO_MIENFOO_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {NAME}_EventScript_MienfooEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_MienfooGone, MSGBOX_DEFAULT
{NAME}_EventScript_MienfooEnd::
	release
	end

{NAME}_EventScript_MienfooStays::
	msgbox {NAME}_Text_MienfooStays, MSGBOX_DEFAULT
	release
	end

@ ---- trainers
{NAME}_EventScript_Jun::
	trainerbattle_single TRAINER_BAMBOO_JUN, {NAME}_Text_JunIntro, {NAME}_Text_JunDefeat
	msgbox {NAME}_Text_JunPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Lila::
	trainerbattle_single TRAINER_BAMBOO_LILA, {NAME}_Text_LilaIntro, {NAME}_Text_LilaDefeat
	msgbox {NAME}_Text_LilaPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Mei::
	trainerbattle_single TRAINER_BAMBOO_MEI, {NAME}_Text_MeiIntro, {NAME}_Text_MeiDefeat
	msgbox {NAME}_Text_MeiPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Kaito::
	trainerbattle_single TRAINER_BAMBOO_KAITO, {NAME}_Text_KaitoIntro, {NAME}_Text_KaitoDefeat
	msgbox {NAME}_Text_KaitoPost, MSGBOX_AUTOCLOSE
	end

{NAME}_Text_GuideBack:
	.string "Shall I walk you back to PETALBURG\n"
	.string "CITY?$"

{NAME}_Text_GuideLead:
	.string "Mind the bamboo shoots on the path!$"

{NAME}_Text_GuideStay:
	.string "Take your time. The valley is\n"
	.string "loveliest when the wind blows.$"

{NAME}_Text_Sign:
	.string "WHISPERSTEM VALLEY\p"
	.string "Listen to the bamboo. It knows\n"
	.string "where the paths go.$"

{NAME}_Text_OldMan:
	.string "When the wind comes off the pond,\n"
	.string "I watch the bamboo sway.\p"
	.string "Past the north bank, one stand\n"
	.string "of stalks always leans apart…\l"
	.string "as if something walks through it.$"

{NAME}_Text_Fisher:
	.string "The koi in this pond are GOLDEEN.\n"
	.string "The plank bridge is the only way\l"
	.string "over to the north bank.$"

{NAME}_Text_Girl:
	.string "The TEA MASTER's PANCHAM ran off\n"
	.string "into the bamboo again!\p"
	.string "They hide where the paths stop.\n"
	.string "One even crossed the pond!$"

{NAME}_Text_Pancham:
	.string "A PANCHAM is chewing on a bamboo\n"
	.string "leaf. It looks at you sheepishly…\p"
	.string "It must be one of the TEA MASTER's!$"

{NAME}_Text_PanchamHome:
	.string "PANCHAM scampered off toward the\n"
	.string "teahouse.$"

{NAME}_Text_MienfooTrains:
	.string "A MIENFOO is training alone in\n"
	.string "the hidden grove…\p"
	.string "It turns to face you and takes\n"
	.string "a stance!$"

{NAME}_Text_Mienfoo:
	.string "Hyaah!$"

{NAME}_Text_MienfooGone:
	.string "MIENFOO bowed, and vanished into\n"
	.string "the bamboo.$"

{NAME}_Text_MienfooStays:
	.string "MIENFOO is still waiting, perfectly\n"
	.string "balanced on one foot.$"

{NAME}_Text_JunIntro:
	.string "Bug POKéMON love bamboo! Let's\n"
	.string "see whose are tougher!$"

{NAME}_Text_JunDefeat:
	.string "Bzzt! You win!$"

{NAME}_Text_JunPost:
	.string "My SEWADDLE wants to make leaf\n"
	.string "clothes out of bamboo leaves.$"

{NAME}_Text_LilaIntro:
	.string "I came for a picnic, but I'll\n"
	.string "take a battle too!$"

{NAME}_Text_LilaDefeat:
	.string "That was a sweet battle.$"

{NAME}_Text_LilaPost:
	.string "Have you tried the tea at the\n"
	.string "teahouse? It's so good.$"

{NAME}_Text_MeiIntro:
	.string "I train among the bamboo every\n"
	.string "morning. Show me your stance!$"

{NAME}_Text_MeiDefeat:
	.string "Your form is better than mine!$"

{NAME}_Text_MeiPost:
	.string "Bamboo bends but never breaks.\n"
	.string "A fighter should be the same.$"

{NAME}_Text_KaitoIntro:
	.string "You saw me? Then my hiding\n"
	.string "practice has failed! Battle!$"

{NAME}_Text_KaitoDefeat:
	.string "I vanish… in defeat.$"

{NAME}_Text_KaitoPost:
	.string "A real ninja could slip right\n"
	.string "through a bamboo wall.$"
''')
print('ok', NAME)

register_wild('MAP_WHISPERSTEM_VALLEY', 'gWhisperstemValley',
              land=[('SEWADDLE', 9, 11), ('NINCADA', 9, 11), ('SHROOMISH', 9, 11), ('SEEDOT', 9, 11),
                    ('HOOTHOOT', 9, 11), ('SPINARAK', 9, 11), ('BELLSPROUT', 10, 12), ('SKIDDO', 10, 12),
                    ('COMBEE', 10, 12), ('BOUNSWEET', 10, 12), ('MEDITITE', 11, 12), ('MAKUHITA', 11, 12)],
              water=[('LOTAD', 10, 15), ('GOLDEEN', 10, 15), ('SURSKIT', 10, 15), ('MARILL', 10, 15), ('MARILL', 10, 15)],
              fish=[('MAGIKARP', 5, 10), ('GOLDEEN', 5, 10), ('MAGIKARP', 10, 15), ('GOLDEEN', 10, 15),
                    ('GOLDEEN', 10, 15), ('GOLDEEN', 20, 25), ('SEAKING', 25, 30), ('BARBOACH', 20, 25),
                    ('BARBOACH', 20, 25), ('SEAKING', 25, 30)])

# ---------------------------------------------------------------- the teahouse (the Lavaridge herb shop's room)
interior(TEA, 'MAP_WHISPERSTEM_VALLEY_TEAHOUSE', 'LAYOUT_LAVARIDGE_TOWN_HERB_SHOP', [(3, 7), (4, 7)],
         'MAP_WHISPERSTEM_VALLEY', 0, mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes', music='MUS_PETALBURG_WOODS',
         objects=[npc('OBJ_EVENT_GFX_OLD_WOMAN', 3, 2, f'{TEA}_EventScript_Master'),
                  npc('OBJ_EVENT_GFX_SPECIES(PANCHAM)', 7, 4, f'{TEA}_EventScript_Pancham',
                      'MOVEMENT_TYPE_LOOK_AROUND', flag='FLAG_HIDE_BAMBOO_HOME_1'),
                  npc('OBJ_EVENT_GFX_SPECIES(PANCHAM)', 2, 5, f'{TEA}_EventScript_Pancham',
                      'MOVEMENT_TYPE_LOOK_AROUND', flag='FLAG_HIDE_BAMBOO_HOME_2'),
                  npc('OBJ_EVENT_GFX_SPECIES(PANCHAM)', 8, 6, f'{TEA}_EventScript_Pancham',
                      'MOVEMENT_TYPE_LOOK_AROUND', flag='FLAG_HIDE_BAMBOO_HOME_3'),
                  npc('OBJ_EVENT_GFX_WOMAN_5', 9, 4, f'{TEA}_EventScript_Guest', 'MOVEMENT_TYPE_FACE_LEFT')],
         scripts=rf'''
{TEA}_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, {TEA}_OnTransition
	.byte 0

{TEA}_OnTransition:
	setflag FLAG_HIDE_BAMBOO_HOME_1
	setflag FLAG_HIDE_BAMBOO_HOME_2
	setflag FLAG_HIDE_BAMBOO_HOME_3
	goto_if_set FLAG_BAMBOO_TEA_DONE, {TEA}_OnTransitionDone
	call_if_set FLAG_BAMBOO_PANCHAM_1, {TEA}_EventScript_Home1
	call_if_set FLAG_BAMBOO_PANCHAM_2, {TEA}_EventScript_Home2
	call_if_set FLAG_BAMBOO_PANCHAM_3, {TEA}_EventScript_Home3
	end

@ once the quest is done, one of the three has gone with you
{TEA}_OnTransitionDone:
	clearflag FLAG_HIDE_BAMBOO_HOME_1
	clearflag FLAG_HIDE_BAMBOO_HOME_3
	end

{TEA}_EventScript_Home1::
	clearflag FLAG_HIDE_BAMBOO_HOME_1
	return

{TEA}_EventScript_Home2::
	clearflag FLAG_HIDE_BAMBOO_HOME_2
	return

{TEA}_EventScript_Home3::
	clearflag FLAG_HIDE_BAMBOO_HOME_3
	return

{TEA}_EventScript_Master::
	lock
	faceplayer
	goto_if_set FLAG_BAMBOO_TEA_DONE, {TEA}_EventScript_MasterDone
	goto_if_unset FLAG_BAMBOO_PANCHAM_1, {TEA}_EventScript_MasterAsk
	goto_if_unset FLAG_BAMBOO_PANCHAM_2, {TEA}_EventScript_MasterAsk
	goto_if_unset FLAG_BAMBOO_PANCHAM_3, {TEA}_EventScript_MasterAsk
	msgbox {TEA}_Text_MasterThanks, MSGBOX_DEFAULT
	givemon SPECIES_PANCHAM, 12
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {TEA}_EventScript_MasterFull
	setflag FLAG_BAMBOO_TEA_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox {TEA}_Text_GotPancham, MSGBOX_DEFAULT
	waitfanfare
	msgbox {TEA}_Text_MasterAfter, MSGBOX_DEFAULT
	release
	end

{TEA}_EventScript_MasterAsk::
	msgbox {TEA}_Text_MasterAsk, MSGBOX_DEFAULT
	release
	end

{TEA}_EventScript_MasterFull::
	msgbox {TEA}_Text_MasterFull, MSGBOX_DEFAULT
	release
	end

{TEA}_EventScript_MasterDone::
	msgbox {TEA}_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

{TEA}_EventScript_Pancham::
	lock
	faceplayer
	playmoncry SPECIES_PANCHAM, CRY_MODE_NORMAL
	msgbox {TEA}_Text_Pancham, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{TEA}_EventScript_Guest::
	msgbox {TEA}_Text_Guest, MSGBOX_NPC
	end

{TEA}_Text_MasterAsk:
	.string "Welcome, welcome. Forgive the\n"
	.string "mess… my three PANCHAM ran off\l"
	.string "into the bamboo again.\p"
	.string "They hide where the paths end.\n"
	.string "Would you bring them home?$"

{TEA}_Text_MasterThanks:
	.string "All three are home! Thank you.\p"
	.string "This little one keeps looking at\n"
	.string "you. I think it wants to go with\l"
	.string "you. Please, take it along.$"

{TEA}_Text_GotPancham:
	.string "{{PLAYER}} received a PANCHAM!$"

{TEA}_Text_MasterAfter:
	.string "Raise it well. It may grow into a\n"
	.string "fine PANGORO one day.$"

{TEA}_Text_MasterFull:
	.string "Your party is full. Come back when\n"
	.string "you have room for one more.$"

{TEA}_Text_MasterDone:
	.string "Have some tea before you go. It's\n"
	.string "brewed from young bamboo leaves.$"

{TEA}_Text_Pancham:
	.string "PANCHAM is chewing on a leaf and\n"
	.string "trying to look tough.$"

{TEA}_Text_Guest:
	.string "I come here for the quiet. Listen…\n"
	.string "you can hear the bamboo whisper.$"
''')
print('ok', TEA)

# ---------------------------------------------------------------- the way there, from Petalburg City
add_object('PetalburgCity', {'graphics_id': 'OBJ_EVENT_GFX_WOMAN_2', 'x': 6, 'y': 12, 'elevation': 3,
                             'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                             'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                             'script': 'PetalburgCity_EventScript_WhisperstemGuide', 'flag': '0'})
register_script_file('whisperstem', r'''
@ Whisperstem Valley guide in Petalburg City (tools/areas/whisperstem.py).
PetalburgCity_EventScript_WhisperstemGuide::
	lock
	faceplayer
	msgbox PetalburgCity_Text_WhisperstemAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_EventScript_WhisperstemNo
	msgbox PetalburgCity_Text_WhisperstemGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_WHISPERSTEM_VALLEY, 29, 44
	waitstate
	release
	end

PetalburgCity_EventScript_WhisperstemNo::
	msgbox PetalburgCity_Text_WhisperstemNo, MSGBOX_DEFAULT
	release
	end

PetalburgCity_Text_WhisperstemAsk:
	.string "West of town there's a hidden\n"
	.string "valley full of whispering bamboo.\p"
	.string "There's a teahouse, a koi pond…\n"
	.string "Shall I show you the way?$"

PetalburgCity_Text_WhisperstemGo:
	.string "Follow me!$"

PetalburgCity_Text_WhisperstemNo:
	.string "Any time. The bamboo isn't going\n"
	.string "anywhere.$"
''')
print('ok entrance')
