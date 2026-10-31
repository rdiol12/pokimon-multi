"""Skyrope Canyon: rock mesas cut by narrow trails, and a deep river gorge
crossed by long rope bridges. The Bridgewright's east bridge is missing its
planks: find his three coils of rope and he mends it, opening the ledge
where a Hawlucha trains.

    python3 tools/areas/canyon.py
"""
import sys, os, math
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import canyon

AREA_NO = 3
NAME = 'SkyropeCanyon'
MAPSEC = 'MAPSEC_SKYROPE_CANYON'

register_constants(NAME, AREA_NO, flags=[
    'SKYROPE_ROPE_1', 'SKYROPE_ROPE_2', 'SKYROPE_ROPE_3', 'SKYROPE_QUEST_STARTED', 'SKYROPE_BRIDGE_FIXED',
    'HIDE_SKYROPE_HAWLUCHA', 'SKYROPE_HAWLUCHA_DONE', 'SKYROPE_ARTIST_GIFT',
    'ITEM_SKYROPE_HARD_STONE', 'ITEM_SKYROPE_MAX_REPEL', 'ITEM_SKYROPE_TM_AERIAL_ACE', 'ITEM_SKYROPE_RARE_CANDY',
    'ITEM_SKYROPE_SHARP_BEAK', 'HIDDEN_ITEM_SKYROPE_STAR_PIECE', 'HIDDEN_ITEM_SKYROPE_ESCAPE_ROPE',
    'HIDDEN_ITEM_SKYROPE_NUGGET',
], trainers=['SKYROPE_HAMISH', 'SKYROPE_WREN', 'SKYROPE_DAISY', 'SKYROPE_KOJI'])

register_trainers(NAME, '''
=== AREA_TRAINER_SKYROPE_HAMISH ===
Name: HAMISH
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Boldore
Level: 31

Rhyhorn
Level: 32

=== AREA_TRAINER_SKYROPE_WREN ===
Name: WREN
Class: Bird Keeper
Pic: Bird Keeper
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Rufflet
Level: 31

Swablu
Level: 31

Skarmory
Level: 33

=== AREA_TRAINER_SKYROPE_DAISY ===
Name: DAISY
Class: Picnicker
Pic: Picnicker
Gender: Female
Music: Girl
Double Battle: No
AI: Basic Trainer

Mudbray
Level: 32

Marill
Level: 32

=== AREA_TRAINER_SKYROPE_KOJI ===
Name: KOJI
Class: Black Belt
Pic: Black Belt
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Machoke
Level: 33

Hawlucha
Level: 34
''')

st = canyon()
W, H = 64, 60
RIVER_Y = 29
BRIDGES = {'west': 9, 'centre': 31, 'east': 51}      # left column of each 4-wide bridge
BRIDGE_TOP, BRIDGE_BOTTOM = 19, 39                   # rows of the bridge ends
BROKEN = range(26, 33)                               # missing planks on the east bridge
WATER = 0x170
CAP = (0x2C0, 0x28B, 0x28B, 0x2C1)                   # Route 114's wooden bridge
DECK = (0x2C8, 0x293, 0x293, 0x2C9)
ELEV = 5                                             # the mesa tops' elevation

a = Area(NAME, W, H, st, fill='#')
# the south basin, where the trail from Fallarbor comes in
a.ellipse(32, 51, 26, 6, '.')
a.rect(29, 54, 6, 6, '.')
# the north mesas: west and centre trails meet in the north-west basin;
# the north-east ledge is only reachable over the east bridge
a.ellipse(14, 9, 10, 6, '.')
a.ellipse(49, 9, 10, 5, '.')
a.line([(10, 18), (12, 14)], 4, '.')
a.line([(33, 18), (28, 14), (20, 11)], 4, '.')
a.line([(53, 18), (50, 12)], 4, '.')
# trails from the basin up to the bridges
a.line([(14, 52), (8, 46), (10, 40)], 4, '.')
a.line([(33, 52), (30, 45), (33, 40)], 4, '.')
a.line([(52, 51), (56, 45), (53, 40)], 4, '.')
a.ellipse(5, 45, 3, 3, '.')                          # a nook off the west trail
a.ellipse(43, 45, 4, 2.5, '.')                       # the Bridgewright's camp
a.line([(43, 45), (53, 45)], 3, '.')
# the gorge
a.line([(x, RIVER_Y + int(2.5 * math.sin(x / 7.0))) for x in range(W)], 9, '~')
# grass
a.ellipse(20, 52, 6, 2.5, '"')
a.ellipse(46, 53, 5, 2, '"')
a.ellipse(11, 7, 5, 2.5, '"')
a.ellipse(50, 8, 4, 2, '"')
a.rect(8, 44, 3, 4, '"')
for name, bx in BRIDGES.items():
    for y in range(BRIDGE_TOP - 2, BRIDGE_TOP + 1):
        a.rect(bx, y, 4, 1, '.')
    for y in range(BRIDGE_BOTTOM, BRIDGE_BOTTOM + 3):
        a.rect(bx, y, 4, 1, '.')

# the rope bridges
for name, bx in BRIDGES.items():
    for y in range(BRIDGE_TOP, BRIDGE_BOTTOM):
        row = CAP if y == BRIDGE_TOP else DECK
        for i, mt in enumerate(row):
            if name == 'east' and y in BROKEN:      # rails left, planks gone
                a.tile(bx + i, y, WATER if i in (1, 2) else mt, collision=1, elevation=ELEV, c='o')
            else:
                a.tile(bx + i, y, mt, collision=0, elevation=ELEV, c='o')

ARRIVE = (32, 57)
a.obj('OBJ_EVENT_GFX_MAN_4', 33, 57, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(30, 55, f'{NAME}_EventScript_Sign', elevation=0)
a.obj('OBJ_EVENT_GFX_HIKER', 43, 44, f'{NAME}_EventScript_Bridgewright', 'MOVEMENT_TYPE_FACE_DOWN',
      local_id='LOCALID_SKYROPE_BRIDGEWRIGHT')
a.obj('OBJ_EVENT_GFX_ARTIST', 22, 49, f'{NAME}_EventScript_Artist', 'MOVEMENT_TYPE_FACE_UP')
a.obj('OBJ_EVENT_GFX_BOY_1', 34, 21, f'{NAME}_EventScript_Kid', 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN', ry=1)
# the three coils of rope
for i, (x, y) in enumerate(((5, 44), (7, 6), (24, 49)), 1):
    a.obj('OBJ_EVENT_GFX_ITEM_BALL', x, y, f'{NAME}_EventScript_Rope{i}', 'MOVEMENT_TYPE_LOOK_AROUND',
          f'FLAG_SKYROPE_ROPE_{i}')
# trainers
a.trainer('OBJ_EVENT_GFX_HIKER', 11, 44, f'{NAME}_EventScript_Hamish', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_CAMPER', 30, 16, f'{NAME}_EventScript_Wren', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_PICNICKER', 47, 52, f'{NAME}_EventScript_Daisy', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_BLACK_BELT', 52, 15, f'{NAME}_EventScript_Koji', 2, 'MOVEMENT_TYPE_FACE_DOWN')
# the ledge across the east bridge
a.obj('OBJ_EVENT_GFX_SPECIES(HAWLUCHA)', 49, 6, f'{NAME}_EventScript_Hawlucha', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_SKYROPE_HAWLUCHA', local_id='LOCALID_SKYROPE_HAWLUCHA')
a.item('ITEM_RARE_CANDY', 57, 9, 'FLAG_ITEM_SKYROPE_RARE_CANDY')
a.item('ITEM_SHARP_BEAK', 41, 8, 'FLAG_ITEM_SKYROPE_SHARP_BEAK')
# scenery: rocks to smash, boulders, and Pokémon going about their day
for x, y in ((20, 12), (45, 48), (4, 47), (56, 41)):
    a.obj('OBJ_EVENT_GFX_BREAKABLE_ROCK', x, y, 'EventScript_RockSmash', 'MOVEMENT_TYPE_LOOK_AROUND')
for x, y in ((16, 9), (37, 53)):
    a.obj('OBJ_EVENT_GFX_PUSHABLE_BOULDER', x, y, 'EventScript_StrengthBoulder', 'MOVEMENT_TYPE_LOOK_AROUND')
a.obj('OBJ_EVENT_GFX_SPECIES(MUDBRAY)', 40, 50, f'{NAME}_EventScript_Mudbray', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_SPECIES(RUFFLET)', 13, 5, f'{NAME}_EventScript_Rufflet', 'MOVEMENT_TYPE_LOOK_AROUND')
# items and secrets
a.item('ITEM_HARD_STONE', 3, 46, 'FLAG_ITEM_SKYROPE_HARD_STONE')
a.item('ITEM_MAX_REPEL', 56, 51, 'FLAG_ITEM_SKYROPE_MAX_REPEL')
a.item('ITEM_TM_AERIAL_ACE', 20, 6, 'FLAG_ITEM_SKYROPE_TM_AERIAL_ACE')
a.hidden(8, 52, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_SKYROPE_STAR_PIECE')
a.hidden(18, 13, 'ITEM_ESCAPE_ROPE', 'FLAG_HIDDEN_ITEM_SKYROPE_ESCAPE_ROPE')
a.hidden(58, 11, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_SKYROPE_NUGGET')

# Route 114 sinks its grass a level below the rock; here the whole mesa
# top is one level, so every open cell (not water, not the bridges) takes it
blocks = a.blocks()
for y in range(H):
    for x in range(W):
        v = int(blocks[y, x])
        if not v & COLL and (x, y) not in a.fixed and v >> 12 in (3, 4):
            blocks[y, x] = (v & 0x0FFF) | ELEV << 12

FIX = ''.join(f'\tsetmetatile {BRIDGES["east"] + i}, {y}, {DECK[i]}, FALSE\n' for y in BROKEN for i in range(4))

a.write(map_id='MAP_SKYROPE_CANYON', layout_id='LAYOUT_SKYROPE_CANYON', blocks=blocks, music='MUS_ROUTE113', mapsec=MAPSEC,
        mapsec_name='SKYROPE CANYON', after='CinderCaverns_Heart', scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
	.byte 0

@ The mended bridge stays mended.
{NAME}_OnLoad:
	call_if_set FLAG_SKYROPE_BRIDGE_FIXED, {NAME}_EventScript_FixBridge
	end

{NAME}_OnTransition:
	call_if_set FLAG_SKYROPE_HAWLUCHA_DONE, {NAME}_EventScript_HideHawlucha
	end

{NAME}_EventScript_HideHawlucha::
	setflag FLAG_HIDE_SKYROPE_HAWLUCHA
	return

{NAME}_EventScript_FixBridge::
{FIX}	return

{NAME}_EventScript_Guide::
	lock
	faceplayer
	msgbox {NAME}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GuideStay
	msgbox {NAME}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FALLARBOR_TOWN, 17, 11
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

@ ---- the Bridgewright and his ropes
{NAME}_EventScript_Bridgewright::
	lock
	faceplayer
	goto_if_set FLAG_SKYROPE_BRIDGE_FIXED, {NAME}_EventScript_BridgewrightDone
	goto_if_unset FLAG_SKYROPE_ROPE_1, {NAME}_EventScript_BridgewrightAsk
	goto_if_unset FLAG_SKYROPE_ROPE_2, {NAME}_EventScript_BridgewrightAsk
	goto_if_unset FLAG_SKYROPE_ROPE_3, {NAME}_EventScript_BridgewrightAsk
	msgbox {NAME}_Text_BridgewrightThanks, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_STRENGTH
	call {NAME}_EventScript_FixBridge
	special DrawWholeMapView
	setflag FLAG_SKYROPE_BRIDGE_FIXED
	delay 30
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_BridgewrightFixed, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_BridgewrightAsk::
	setflag FLAG_SKYROPE_QUEST_STARTED
	msgbox {NAME}_Text_BridgewrightAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_BridgewrightDone::
	msgbox {NAME}_Text_BridgewrightDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Rope1::
	lock
	setflag FLAG_SKYROPE_ROPE_1
	goto {NAME}_EventScript_RopeFound

{NAME}_EventScript_Rope2::
	lock
	setflag FLAG_SKYROPE_ROPE_2
	goto {NAME}_EventScript_RopeFound

{NAME}_EventScript_Rope3::
	lock
	setflag FLAG_SKYROPE_ROPE_3
	goto {NAME}_EventScript_RopeFound

{NAME}_EventScript_RopeFound::
	removeobject VAR_LAST_TALKED
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_RopeFound, MSGBOX_DEFAULT
	waitfanfare
	goto_if_unset FLAG_SKYROPE_ROPE_1, {NAME}_EventScript_RopeMore
	goto_if_unset FLAG_SKYROPE_ROPE_2, {NAME}_EventScript_RopeMore
	goto_if_unset FLAG_SKYROPE_ROPE_3, {NAME}_EventScript_RopeMore
	msgbox {NAME}_Text_RopeAll, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RopeMore::
	release
	end

@ ---- Hawlucha on the ledge
{NAME}_EventScript_Hawlucha::
	lock
	faceplayer
	setwildbattle SPECIES_HAWLUCHA, 36
	playmoncry SPECIES_HAWLUCHA, CRY_MODE_ENCOUNTER
	msgbox {NAME}_Text_Hawlucha, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {NAME}_EventScript_HawluchaStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {NAME}_EventScript_HawluchaStays
	setflag FLAG_SKYROPE_HAWLUCHA_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {NAME}_EventScript_HawluchaEnd
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_SKYROPE_HAWLUCHA
	setflag FLAG_HIDE_SKYROPE_HAWLUCHA
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_HawluchaFlew, MSGBOX_DEFAULT
{NAME}_EventScript_HawluchaEnd::
	release
	end

{NAME}_EventScript_HawluchaStays::
	msgbox {NAME}_Text_HawluchaStays, MSGBOX_DEFAULT
	release
	end

@ ---- people
{NAME}_EventScript_Artist::
	lock
	faceplayer
	goto_if_set FLAG_SKYROPE_ARTIST_GIFT, {NAME}_EventScript_ArtistDone
	msgbox {NAME}_Text_Artist, MSGBOX_DEFAULT
	goto_if_unset FLAG_SKYROPE_BRIDGE_FIXED, {NAME}_EventScript_ArtistEnd
	msgbox {NAME}_Text_ArtistBridge, MSGBOX_DEFAULT
	giveitem ITEM_STAR_PIECE
	goto_if_eq VAR_RESULT, FALSE, {NAME}_EventScript_ArtistEnd
	setflag FLAG_SKYROPE_ARTIST_GIFT
{NAME}_EventScript_ArtistEnd::
	release
	end

{NAME}_EventScript_ArtistDone::
	msgbox {NAME}_Text_ArtistDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Mudbray::
	lock
	faceplayer
	playmoncry SPECIES_MUDBRAY, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Mudbray, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Rufflet::
	lock
	faceplayer
	playmoncry SPECIES_RUFFLET, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Rufflet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Kid::
	msgbox {NAME}_Text_Kid, MSGBOX_NPC
	end

{NAME}_EventScript_Hamish::
	trainerbattle_single TRAINER_SKYROPE_HAMISH, {NAME}_Text_HamishIntro, {NAME}_Text_HamishDefeat
	msgbox {NAME}_Text_HamishPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Wren::
	trainerbattle_single TRAINER_SKYROPE_WREN, {NAME}_Text_WrenIntro, {NAME}_Text_WrenDefeat
	msgbox {NAME}_Text_WrenPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Daisy::
	trainerbattle_single TRAINER_SKYROPE_DAISY, {NAME}_Text_DaisyIntro, {NAME}_Text_DaisyDefeat
	msgbox {NAME}_Text_DaisyPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Koji::
	trainerbattle_single TRAINER_SKYROPE_KOJI, {NAME}_Text_KojiIntro, {NAME}_Text_KojiDefeat
	msgbox {NAME}_Text_KojiPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
{NAME}_Text_GuideBack:
	.string "Had enough wind for one day?\n"
	.string "I can lead you back to FALLARBOR.$"

{NAME}_Text_GuideLead:
	.string "Mind the gusts on the way down!$"

{NAME}_Text_GuideStay:
	.string "Take your time. The canyon is at\n"
	.string "its best when the sun is low.$"

{NAME}_Text_Sign:
	.string "SKYROPE CANYON\n"
	.string "Cross the bridges one at a time!$"

{NAME}_Text_BridgewrightAsk:
	.string "I'm the BRIDGEWRIGHT. I keep the\n"
	.string "canyon's rope bridges in shape.\p"
	.string "A gust took the planks off the east\n"
	.string "bridge, and my three coils of rope\l"
	.string "blew all over the canyon!\p"
	.string "One went west, one went over the\n"
	.string "gorge to the north, and one is out\l"
	.string "in the grass here somewhere.\p"
	.string "Bring them back and I'll fix the\n"
	.string "bridge in no time.$"

{NAME}_Text_BridgewrightThanks:
	.string "All three coils! You've got good\n"
	.string "eyes. Stand back, now!$"

{NAME}_Text_BridgewrightFixed:
	.string "Good as new! There's a ledge on the\n"
	.string "other side nobody has visited in\l"
	.string "years. A HAWLUCHA trains up there.$"

{NAME}_Text_BridgewrightDone:
	.string "Every bridge in the canyon is\n"
	.string "sound. Cross away!$"

{NAME}_Text_RopeFound:
	.string "{{PLAYER}} found a coil of the\n"
	.string "BRIDGEWRIGHT's rope!$"

{NAME}_Text_RopeAll:
	.string "That's all three coils. Better take\n"
	.string "them to the BRIDGEWRIGHT.$"

{NAME}_Text_Hawlucha:
	.string "Hawaa!$"

{NAME}_Text_HawluchaFlew:
	.string "HAWLUCHA leapt off the ledge and\n"
	.string "glided away on the wind.$"

{NAME}_Text_HawluchaStays:
	.string "HAWLUCHA is still striking poses\n"
	.string "on the ledge.$"

{NAME}_Text_Artist:
	.string "I paint the canyon every day. The\n"
	.string "rock changes color with the light.\p"
	.string "If only the east bridge were fixed,\n"
	.string "I could paint the far ledge too.$"

{NAME}_Text_ArtistBridge:
	.string "Oh! The east bridge is fixed? Then\n"
	.string "this painting is done. Thank you!\p"
	.string "Here, I found this in the river\n"
	.string "while washing my brushes.$"

{NAME}_Text_ArtistDone:
	.string "Look at the light on the mesas.\n"
	.string "Perfect.$"

{NAME}_Text_Mudbray:
	.string "MUDBRAY is happily kicking up dust.$"

{NAME}_Text_Rufflet:
	.string "The RUFFLET puffs up its chest and\n"
	.string "glares. It's guarding its nest!$"

{NAME}_Text_Kid:
	.string "If you look down from the bridge,\n"
	.string "you can see BASCULIN fighting in\l"
	.string "the river!$"

{NAME}_Text_HamishIntro:
	.string "These mesas are made of layers and\n"
	.string "layers of old rock. Like my team!$"

{NAME}_Text_HamishDefeat:
	.string "Crumbled…$"

{NAME}_Text_HamishPost:
	.string "Some ledges hide items. Look\n"
	.string "closely at the dead ends.$"

{NAME}_Text_WrenIntro:
	.string "My birds ride the canyon winds all\n"
	.string "day long!$"

{NAME}_Text_WrenDefeat:
	.string "Blown away!$"

{NAME}_Text_WrenPost:
	.string "RUFFLET nest up in the cliffs.\n"
	.string "They're brave little birds.$"

{NAME}_Text_DaisyIntro:
	.string "A picnic in the canyon! Want to\n"
	.string "battle before lunch?$"

{NAME}_Text_DaisyDefeat:
	.string "My sandwiches went flying!$"

{NAME}_Text_DaisyPost:
	.string "The BRIDGEWRIGHT's camp is just up\n"
	.string "the trail. He looks worried.$"

{NAME}_Text_KojiIntro:
	.string "I came over the old bridge to train\n"
	.string "with the HAWLUCHA. Show me your\l"
	.string "strength!$"

{NAME}_Text_KojiDefeat:
	.string "A fine flying press!$"

{NAME}_Text_KojiPost:
	.string "HAWLUCHA only respect those who\n"
	.string "beat them fair and square.$"
''')

register_wild('MAP_SKYROPE_CANYON', 'gSkyropeCanyon',
              land=[('RUFFLET', 30, 32), ('VULLABY', 30, 32), ('DRILBUR', 30, 32), ('SANDILE', 30, 32),
                    ('MUDBRAY', 31, 33), ('GLIGAR', 31, 33), ('ONIX', 31, 33), ('RHYHORN', 31, 33),
                    ('SWABLU', 30, 32), ('HIPPOPOTAS', 32, 34), ('SKARMORY', 33, 34), ('HAWLUCHA', 34, 35)],
              water=[('MARILL', 25, 30), ('BASCULIN', 28, 33), ('GOLDUCK', 30, 35), ('AZUMARILL', 32, 35), ('BASCULIN', 33, 36)],
              fish=[('MAGIKARP', 10, 30), ('BARBOACH', 10, 30), ('BARBOACH', 25, 30), ('BASCULIN', 25, 30), ('GOLDEEN', 25, 30),
                    ('BASCULIN', 30, 35), ('WHISCASH', 30, 35), ('SEAKING', 30, 35), ('BASCULIN', 33, 36), ('GYARADOS', 35, 38)],
              rock=[('ROGGENROLA', 28, 32), ('BOLDORE', 31, 34), ('GEODUDE', 28, 32), ('DWEBBLE', 30, 33), ('ONIX', 32, 34)])
print('ok', NAME)

# ---------------------------------------------------------------- the way in, from Fallarbor Town
add_object('FallarborTown', {'graphics_id': 'OBJ_EVENT_GFX_HIKER', 'x': 18, 'y': 11, 'elevation': 3,
                             'movement_type': 'MOVEMENT_TYPE_FACE_LEFT', 'movement_range_x': 0, 'movement_range_y': 0,
                             'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                             'script': 'FallarborTown_EventScript_SkyropeGuide', 'flag': '0'})
register_script_file('skyrope', r'''
@ Skyrope Canyon guide in Fallarbor Town (tools/areas/canyon.py).
FallarborTown_EventScript_SkyropeGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, FallarborTown_EventScript_SkyropeGuideNoBadge
	msgbox FallarborTown_Text_SkyropeGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FallarborTown_EventScript_SkyropeGuideNo
	msgbox FallarborTown_Text_SkyropeGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SKYROPE_CANYON, 32, 57
	waitstate
	release
	end

FallarborTown_EventScript_SkyropeGuideNoBadge::
	msgbox FallarborTown_Text_SkyropeGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FallarborTown_EventScript_SkyropeGuideNo::
	msgbox FallarborTown_Text_SkyropeGuideNo, MSGBOX_DEFAULT
	release
	end

FallarborTown_Text_SkyropeGuideNoBadge:
	.string "Past the ash fields lies SKYROPE\n"
	.string "CANYON, all mesas and rope bridges.\p"
	.string "The winds are strong. Come back with\n"
	.string "NORMAN's badge and I'll take you.$"

FallarborTown_Text_SkyropeGuideAsk:
	.string "That's NORMAN's badge! You'll be\n"
	.string "fine on the bridges.\p"
	.string "Shall I lead you to SKYROPE\n"
	.string "CANYON?$"

FallarborTown_Text_SkyropeGuideGo:
	.string "Hold on to your hat!$"

FallarborTown_Text_SkyropeGuideNo:
	.string "The canyon isn't going anywhere.\n"
	.string "It took a million years to carve!$"
''')
print('ok entrance')
