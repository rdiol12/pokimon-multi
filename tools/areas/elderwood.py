"""Elderwood Forest: an ancient rainforest east of Fortree with treehouses
joined by canopy walkways, a quest to bring the Elder Tree's three keepers
home, and a hidden grove where Zarude appears.

    python3 tools/areas/elderwood.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import jungle, POND

NAME = 'ElderwoodForest'
AREA_NO = 0

register_constants('ElderwoodForest', AREA_NO, flags=[
    'ELDERWOOD_BOUNSWEET_HOME', 'ELDERWOOD_ROWLET_HOME', 'ELDERWOOD_GROOKEY_HOME', 'ELDERWOOD_QUEST_DONE',
    'HIDE_ELDERWOOD_ZARUDE', 'ELDERWOOD_ZARUDE_DONE', 'ITEM_ELDERWOOD_MIRACLE_SEED', 'ITEM_ELDERWOOD_LEAF_STONE',
    'ITEM_ELDERWOOD_REVIVE', 'ITEM_ELDERWOOD_ULTRA_BALL', 'HIDDEN_ITEM_ELDERWOOD_BIG_MUSHROOM',
    'HIDDEN_ITEM_ELDERWOOD_RARE_CANDY', 'HIDDEN_ITEM_ELDERWOOD_TINY_MUSHROOM', 'ELDERWOOD_LOOKOUT_GIFT',
    'HIDDEN_ITEM_ELDERWOOD_ABILITY_CAPSULE',
], trainers=['ELDERWOOD_IVY', 'ELDERWOOD_HAL', 'ELDERWOOD_FERN'])

register_trainers('ElderwoodForest', '''
=== AREA_TRAINER_ELDERWOOD_IVY ===
Name: IVY
Class: Pkmn Ranger
Pic: Pokemon Ranger F
Gender: Female
Music: Female
Double Battle: No
AI: Basic Trainer

Dartrix
Level: 36

Tropius
Level: 37

Leavanny
Level: 38

=== AREA_TRAINER_ELDERWOOD_HAL ===
Name: HAL
Class: Bug Maniac
Pic: Bug Maniac
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Heracross
Level: 36

Pinsir
Level: 36

Yanmega
Level: 37

=== AREA_TRAINER_ELDERWOOD_FERN ===
Name: FERN
Class: Aroma Lady
Pic: Aroma Lady
Gender: Female
Music: Female
Double Battle: No
AI: Basic Trainer

Steenee
Level: 36

Comfey
Level: 37

Breloom
Level: 37
''')

st = jungle()
a = Area(NAME, 56, 52, st, fill='T')

# Canopy: four treehouses joined by a walkway, copied from Fortree City.
FORTREE_SKIP = {0xC6, 0xC7, 0x017}
a.stamp('LAYOUT_FORTREE_CITY', 8, 0, 27, 8, 14, 3)
# doors at (16,6) (23,6) (31,6) (38,5); walkway at y = 7 (elevation 4);
# ladders come down to the ground at x = 16, 31, 38 (y = 10)

# The great clearing and the paths up to the ladders.
a.ellipse(28, 31, 21, 12, '.')
for x in (16, 30, 38):
    a.line([(x, 10), (x, 22)], 2, '.')
a.line([(16, 14), (46, 14)], 2, '.')
a.line([(46, 14), (46, 22)], 2, '.')
a.line([(27, 40), (27, 51)], 4, '.')            # south way in
a.line([(8, 25), (14, 25)], 2, '.')             # west: the hidden grove, behind a Cut tree
a.ellipse(6, 23, 4.5, 5, '.')
a.ellipse(49, 31, 5, 6, '.')                    # east shrine nook
a.line([(44, 31), (49, 31)], 2, '.')
# tall grass
a.ellipse(15, 31, 6, 4, '"')
a.ellipse(30, 40, 6, 2.5, '"')
a.ellipse(46, 22, 3, 2, '"')
a.ellipse(49, 36, 3, 2, '"')
# a few big trees inside the clearing
a.rect(20, 24, 2, 2, 'T')
a.rect(34, 25, 2, 2, 'T')
a.rect(22, 36, 2, 2, 'T')
a.snap_trees()
# pond
a.rect(33, 31, 8, 5, 'P')
a.autotile3('P', POND)
a.scatter(0x004, 40, 8, 18, 50, 46, seed=7)

NPC = 'MOVEMENT_TYPE_FACE_DOWN'
# arrival / return
a.obj('OBJ_EVENT_GFX_MAN_4', 25, 48, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_RIGHT')
a.sign(29, 46, f'{NAME}_EventScript_EntranceSign')
# doors (treehouses); warp ids 0..1
a.warp(16, 6, 'MAP_ELDERWOOD_FOREST_ELDER_HOUSE', 0)
a.warp(23, 6, 'MAP_ELDERWOOD_FOREST_LOOKOUT', 0)
# the three keepers
a.obj('OBJ_EVENT_GFX_SPECIES(BOUNSWEET)', 13, 32, f'{NAME}_EventScript_Bounsweet', 'MOVEMENT_TYPE_WANDER_AROUND',
      'FLAG_ELDERWOOD_BOUNSWEET_HOME', rx=2, ry=1, local_id='LOCALID_ELDERWOOD_BOUNSWEET')
a.obj('OBJ_EVENT_GFX_SPECIES(ROWLET)', 35, 7, f'{NAME}_EventScript_Rowlet', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_ELDERWOOD_ROWLET_HOME', local_id='LOCALID_ELDERWOOD_ROWLET', elevation=4)
a.obj('OBJ_EVENT_GFX_SPECIES(GROOKEY)', 50, 29, f'{NAME}_EventScript_Grookey', 'MOVEMENT_TYPE_WANDER_AROUND',
      'FLAG_ELDERWOOD_GROOKEY_HOME', rx=1, ry=2, local_id='LOCALID_ELDERWOOD_GROOKEY')
# hidden grove: a Cut tree, then Zarude by the old stump
a.obj('OBJ_EVENT_GFX_CUTTABLE_TREE', 12, 25, 'EventScript_CutTree', 'MOVEMENT_TYPE_LOOK_AROUND', 'FLAG_TEMP_11')
a.obj('OBJ_EVENT_GFX_SPECIES(ZARUDE)', 5, 21, f'{NAME}_EventScript_Zarude', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_ELDERWOOD_ZARUDE', local_id='LOCALID_ELDERWOOD_ZARUDE')
a.sign(7, 20, f'{NAME}_EventScript_GroveStone')
# trainers
a.trainer('OBJ_EVENT_GFX_PICNICKER', 27, 7, f'{NAME}_EventScript_Ivy', 4, 'MOVEMENT_TYPE_FACE_LEFT', elevation=4)
a.trainer('OBJ_EVENT_GFX_BUG_CATCHER', 17, 29, f'{NAME}_EventScript_Hal', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_WOMAN_2', 37, 37, f'{NAME}_EventScript_Fern', 3, 'MOVEMENT_TYPE_FACE_UP')
# villagers
a.obj('OBJ_EVENT_GFX_FISHERMAN', 32, 33, f'{NAME}_EventScript_Fisher', 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_HIKER', 47, 16, f'{NAME}_EventScript_Botanist', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_TWIN', 25, 27, f'{NAME}_EventScript_Kid', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=2)
# items
a.item('ITEM_MIRACLE_SEED', 4, 26, 'FLAG_ITEM_ELDERWOOD_MIRACLE_SEED')
a.item('ITEM_LEAF_STONE', 52, 33, 'FLAG_ITEM_ELDERWOOD_LEAF_STONE')
a.item('ITEM_REVIVE', 40, 7, 'FLAG_ITEM_ELDERWOOD_REVIVE')
a.item('ITEM_ULTRA_BALL', 9, 34, 'FLAG_ITEM_ELDERWOOD_ULTRA_BALL')
a.hidden(51, 27, 'ITEM_BIG_MUSHROOM', 'FLAG_HIDDEN_ITEM_ELDERWOOD_BIG_MUSHROOM')
a.hidden(8, 22, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_ELDERWOOD_RARE_CANDY')
a.hidden(21, 41, 'ITEM_TINY_MUSHROOM', 'FLAG_HIDDEN_ITEM_ELDERWOOD_TINY_MUSHROOM')
a.hidden(20, 7, 'ITEM_ABILITY_CAPSULE', 'FLAG_HIDDEN_ITEM_ELDERWOOD_ABILITY_CAPSULE', elevation=4)
a.sign(48, 30, f'{NAME}_EventScript_ShrineSign')

SCRIPTS = r'''
ElderwoodForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ElderwoodForest_OnTransition
	.byte 0

@ Zarude only comes to the grove once the keepers are home.
ElderwoodForest_OnTransition:
	setflag FLAG_HIDE_ELDERWOOD_ZARUDE
	goto_if_set FLAG_ELDERWOOD_ZARUDE_DONE, ElderwoodForest_OnTransitionEnd
	goto_if_unset FLAG_ELDERWOOD_QUEST_DONE, ElderwoodForest_OnTransitionEnd
	clearflag FLAG_HIDE_ELDERWOOD_ZARUDE
ElderwoodForest_OnTransitionEnd:
	end

ElderwoodForest_EventScript_Guide::
	lock
	faceplayer
	msgbox ElderwoodForest_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ElderwoodForest_EventScript_GuideStay
	msgbox ElderwoodForest_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 37, 10
	waitstate
	release
	end

ElderwoodForest_EventScript_GuideStay::
	msgbox ElderwoodForest_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_EntranceSign::
	msgbox ElderwoodForest_Text_EntranceSign, MSGBOX_SIGN
	end

ElderwoodForest_EventScript_ShrineSign::
	msgbox ElderwoodForest_Text_ShrineSign, MSGBOX_SIGN
	end

ElderwoodForest_EventScript_GroveStone::
	msgbox ElderwoodForest_Text_GroveStone, MSGBOX_SIGN
	end

@ The three keepers: each one heads home to the Elder Tree when found.
ElderwoodForest_EventScript_Bounsweet::
	lock
	faceplayer
	playmoncry SPECIES_BOUNSWEET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Bounsweet, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_BOUNSWEET_HOME
	goto ElderwoodForest_EventScript_KeeperGoesHome_Bounsweet

ElderwoodForest_EventScript_KeeperGoesHome_Bounsweet::
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_BOUNSWEET
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_Rowlet::
	lock
	faceplayer
	playmoncry SPECIES_ROWLET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Rowlet, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_ROWLET_HOME
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_ROWLET
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_Grookey::
	lock
	faceplayer
	playmoncry SPECIES_GROOKEY, CRY_MODE_NORMAL
	msgbox ElderwoodForest_Text_Grookey, MSGBOX_DEFAULT
	waitmoncry
	setflag FLAG_ELDERWOOD_GROOKEY_HOME
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_GROOKEY
	fadescreen FADE_FROM_BLACK
	goto ElderwoodForest_EventScript_KeeperCount

ElderwoodForest_EventScript_KeeperCount::
	goto_if_unset FLAG_ELDERWOOD_BOUNSWEET_HOME, ElderwoodForest_EventScript_KeepersLeft
	goto_if_unset FLAG_ELDERWOOD_ROWLET_HOME, ElderwoodForest_EventScript_KeepersLeft
	goto_if_unset FLAG_ELDERWOOD_GROOKEY_HOME, ElderwoodForest_EventScript_KeepersLeft
	msgbox ElderwoodForest_Text_AllKeepersHome, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_KeepersLeft::
	msgbox ElderwoodForest_Text_KeeperWentHome, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_Zarude::
	lock
	faceplayer
	setwildbattle SPECIES_ZARUDE, 45
	playmoncry SPECIES_ZARUDE, CRY_MODE_ENCOUNTER
	msgbox ElderwoodForest_Text_Zarude, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, ElderwoodForest_EventScript_ZarudeGone
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, ElderwoodForest_EventScript_ZarudeStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, ElderwoodForest_EventScript_ZarudeStays
	setflag FLAG_ELDERWOOD_ZARUDE_DONE
	release
	end

ElderwoodForest_EventScript_ZarudeGone::
	setflag FLAG_ELDERWOOD_ZARUDE_DONE
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ELDERWOOD_ZARUDE
	setflag FLAG_HIDE_ELDERWOOD_ZARUDE
	fadescreen FADE_FROM_BLACK
	msgbox ElderwoodForest_Text_ZarudeFled, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_ZarudeStays::
	msgbox ElderwoodForest_Text_ZarudeWatches, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_EventScript_Ivy::
	trainerbattle_single TRAINER_ELDERWOOD_IVY, ElderwoodForest_Text_IvyIntro, ElderwoodForest_Text_IvyDefeat
	msgbox ElderwoodForest_Text_IvyPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Hal::
	trainerbattle_single TRAINER_ELDERWOOD_HAL, ElderwoodForest_Text_HalIntro, ElderwoodForest_Text_HalDefeat
	msgbox ElderwoodForest_Text_HalPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Fern::
	trainerbattle_single TRAINER_ELDERWOOD_FERN, ElderwoodForest_Text_FernIntro, ElderwoodForest_Text_FernDefeat
	msgbox ElderwoodForest_Text_FernPost, MSGBOX_AUTOCLOSE
	end

ElderwoodForest_EventScript_Fisher::
	msgbox ElderwoodForest_Text_Fisher, MSGBOX_NPC
	end

ElderwoodForest_EventScript_Botanist::
	msgbox ElderwoodForest_Text_Botanist, MSGBOX_NPC
	end

ElderwoodForest_EventScript_Kid::
	msgbox ElderwoodForest_Text_Kid, MSGBOX_NPC
	end

ElderwoodForest_Text_GuideBack:
	.string "The canopy paths can make a person\n"
	.string "dizzy. Shall I lead you back\l"
	.string "to FORTREE CITY?$"

ElderwoodForest_Text_GuideLead:
	.string "Follow the vines, and mind your step!$"

ElderwoodForest_Text_GuideStay:
	.string "Take your time. ELDERWOOD has been\n"
	.string "growing for a thousand years.$"

ElderwoodForest_Text_EntranceSign:
	.string "ELDERWOOD FOREST\n"
	.string "“Walk softly. The trees remember.”$"

ElderwoodForest_Text_ShrineSign:
	.string "A mossy stone shrine.\n"
	.string "“Here rest the keepers of the\l"
	.string "ELDER TREE: the sweet, the swift,\l"
	.string "and the drummer.”$"

ElderwoodForest_Text_GroveStone:
	.string "Words are carved into the stump.\p"
	.string "“When the three keepers sleep\n"
	.string "at home, the king of the jungle\l"
	.string "comes to listen.”$"

ElderwoodForest_Text_Bounsweet:
	.string "BOUNSWEET: Bou! Bou!\p"
	.string "It smells wonderfully sweet.\n"
	.string "It looks like it got lost in the\l"
	.string "tall grass.$"

ElderwoodForest_Text_Rowlet:
	.string "ROWLET: Hoo-hoo!\p"
	.string "It was dozing on the walkway.\n"
	.string "It blinks, then flutters off\l"
	.string "toward the ELDER TREE.$"

ElderwoodForest_Text_Grookey:
	.string "GROOKEY: Grook! Grook!\p"
	.string "It drums its stick on the shrine,\n"
	.string "then scampers off home.$"

ElderwoodForest_Text_KeeperWentHome:
	.string "The keeper went home to the\n"
	.string "ELDER TREE.$"

ElderwoodForest_Text_AllKeepersHome:
	.string "That was the last keeper!\n"
	.string "The whole forest seems to sigh\l"
	.string "with relief.\p"
	.string "You should tell the ELDER in the\n"
	.string "treehouse.$"

ElderwoodForest_Text_Zarude:
	.string "Uuh-rah!$"

ElderwoodForest_Text_ZarudeFled:
	.string "ZARUDE swung away into the canopy.$"

ElderwoodForest_Text_ZarudeWatches:
	.string "ZARUDE is watching you from the\n"
	.string "branches.$"

ElderwoodForest_Text_IvyIntro:
	.string "Rangers patrol the canopy so no one\n"
	.string "falls. Show me you're sure-footed!$"

ElderwoodForest_Text_IvyDefeat:
	.string "Steady hands, steady team!$"

ElderwoodForest_Text_IvyPost:
	.string "There's a stump in the west woods\n"
	.string "behind a thin tree. A cut would\l"
	.string "open the way.$"

ElderwoodForest_Text_HalIntro:
	.string "The biggest bugs in HOENN live\n"
	.string "right here. Want to see?$"

ElderwoodForest_Text_HalDefeat:
	.string "Bzzzt! Out-bugged!$"

ElderwoodForest_Text_HalPost:
	.string "I've seen a YANMEGA fly so fast\n"
	.string "it left a gust behind.$"

ElderwoodForest_Text_FernIntro:
	.string "Breathe in. Can you smell the\n"
	.string "flowers? My team grew up here.$"

ElderwoodForest_Text_FernDefeat:
	.string "Such a fresh breeze!$"

ElderwoodForest_Text_FernPost:
	.string "COMFEY likes to braid the flowers\n"
	.string "by the pond into garlands.$"

ElderwoodForest_Text_Fisher:
	.string "This pond never dries up, not even\n"
	.string "in summer. The roots drink deep.$"

ElderwoodForest_Text_Botanist:
	.string "I'm cataloguing every plant in\n"
	.string "ELDERWOOD. I'm on number 4,012.\p"
	.string "Some of these mushrooms only grow\n"
	.string "where nobody walks…$"

ElderwoodForest_Text_Kid:
	.string "Grandma says the forest has a king\n"
	.string "who swings on vines! Have you\l"
	.string "seen him?$"
'''

a.write(map_id='MAP_ELDERWOOD_FOREST', layout_id='LAYOUT_ELDERWOOD_FOREST', music='MUS_ROUTE119',
        mapsec='MAPSEC_ELDERWOOD_FOREST', mapsec_name='ELDERWOOD FOREST', weather='WEATHER_SHADE',
        scripts=SCRIPTS)

register_wild('MAP_ELDERWOOD_FOREST', 'gElderwoodForest',
              land=[('ROWLET', 33, 35), ('BOUNSWEET', 33, 35), ('SHROOMISH', 33, 35), ('SEWADDLE', 33, 35),
                    ('TROPIUS', 34, 36), ('HERACROSS', 34, 36), ('PINSIR', 34, 36), ('COMBEE', 33, 35),
                    ('STEENEE', 35, 37), ('SWADLOON', 35, 37), ('GROOKEY', 33, 35), ('YANMEGA', 37, 38)],
              water=[('LOTAD', 30, 35), ('WOOPER', 30, 35), ('LOMBRE', 33, 37), ('QUAGSIRE', 33, 37), ('ARAQUANID', 35, 38)],
              fish=[('MAGIKARP', 10, 30), ('GOLDEEN', 10, 30), ('POLIWAG', 25, 30), ('GOLDEEN', 25, 30), ('POLIWHIRL', 25, 30),
                    ('POLIWHIRL', 30, 35), ('SEAKING', 30, 35), ('DEWPIDER', 30, 35), ('ARAQUANID', 35, 38), ('POLITOED', 38, 40)])

# ---------------------------------------------------------------- interiors
def interior(name, map_id, layout, door_warp, objects, scripts, mapsec='MAPSEC_ELDERWOOD_FOREST'):
    m = {'id': map_id, 'name': name, 'layout': layout, 'music': 'MUS_FORTREE', 'region': 'REGION_HOENN',
         'region_map_section': mapsec, 'requires_flash': False, 'weather': 'WEATHER_NONE', 'map_type': 'MAP_TYPE_INDOOR',
         'allow_cycling': False, 'allow_escaping': False, 'allow_running': False, 'show_map_name': False,
         'battle_scene': 'MAP_BATTLE_SCENE_NORMAL', 'connections': None, 'object_events': objects,
         'warp_events': [{'x': 3, 'y': 5, 'elevation': 0, 'dest_map': 'MAP_ELDERWOOD_FOREST', 'dest_warp_id': str(door_warp)},
                         {'x': 4, 'y': 5, 'elevation': 0, 'dest_map': 'MAP_ELDERWOOD_FOREST', 'dest_warp_id': str(door_warp)}],
         'coord_events': [], 'bg_events': []}
    d = f'{ROOT}/data/maps/{name}'
    os.makedirs(d, exist_ok=True)
    with open(f'{d}/map.json', 'w') as f:
        json.dump(m, f, indent=2, ensure_ascii=False)
        f.write('\n')
    open(f'{d}/scripts.inc', 'w').write(scripts.strip('\n') + '\n')
    register_group('gMapGroup_IndoorFortree', name)
    register_scripts(name)


def npc(gfx, x, y, script, move='MOVEMENT_TYPE_FACE_DOWN', local_id=None):
    o = {'graphics_id': gfx, 'x': x, 'y': y, 'elevation': 3, 'movement_type': move, 'movement_range_x': 1,
         'movement_range_y': 1, 'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
         'script': script, 'flag': '0'}
    return {'local_id': local_id, **o} if local_id else o


interior('ElderwoodForest_ElderHouse', 'MAP_ELDERWOOD_FOREST_ELDER_HOUSE', 'LAYOUT_FORTREE_CITY_HOUSE1', 0, [
    npc('OBJ_EVENT_GFX_OLD_WOMAN', 2, 2, 'ElderwoodForest_ElderHouse_EventScript_Elder'),
    npc('OBJ_EVENT_GFX_SPECIES(BOUNSWEET)', 6, 3, 'ElderwoodForest_ElderHouse_EventScript_Bounsweet', 'MOVEMENT_TYPE_WANDER_AROUND'),
], r'''
ElderwoodForest_ElderHouse_MapScripts::
	.byte 0

ElderwoodForest_ElderHouse_EventScript_Elder::
	lock
	faceplayer
	goto_if_set FLAG_ELDERWOOD_QUEST_DONE, ElderwoodForest_ElderHouse_EventScript_ElderAfter
	goto_if_unset FLAG_ELDERWOOD_BOUNSWEET_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	goto_if_unset FLAG_ELDERWOOD_ROWLET_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	goto_if_unset FLAG_ELDERWOOD_GROOKEY_HOME, ElderwoodForest_ElderHouse_EventScript_ElderAsk
	msgbox ElderwoodForest_ElderHouse_Text_Thanks, MSGBOX_DEFAULT
	giveitem ITEM_MIRACLE_SEED
	giveitem ITEM_SITRUS_BERRY, 5
	giveitem ITEM_TM_GIGA_DRAIN
	setflag FLAG_ELDERWOOD_QUEST_DONE
	msgbox ElderwoodForest_ElderHouse_Text_KingComes, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_ElderAsk::
	msgbox ElderwoodForest_ElderHouse_Text_Ask, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_ElderAfter::
	msgbox ElderwoodForest_ElderHouse_Text_After, MSGBOX_DEFAULT
	release
	end

ElderwoodForest_ElderHouse_EventScript_Bounsweet::
	lock
	faceplayer
	playmoncry SPECIES_BOUNSWEET, CRY_MODE_NORMAL
	msgbox ElderwoodForest_ElderHouse_Text_Bounsweet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

ElderwoodForest_ElderHouse_Text_Ask:
	.string "I am the ELDER of this wood.\p"
	.string "Three keepers tend the ELDER TREE:\n"
	.string "BOUNSWEET, ROWLET and GROOKEY.\p"
	.string "A storm scattered them across the\n"
	.string "forest. The tree droops without\l"
	.string "them.\p"
	.string "Please find them. Look in the tall\n"
	.string "grass, up on the walkways, and by\l"
	.string "the old shrine.$"

ElderwoodForest_ElderHouse_Text_Thanks:
	.string "All three keepers are home!\n"
	.string "The ELDER TREE is green again.\p"
	.string "Take these, with the forest's\n"
	.string "thanks.$"

ElderwoodForest_ElderHouse_Text_KingComes:
	.string "Listen… the drumming has stopped.\p"
	.string "When the keepers are home, the\n"
	.string "king of the jungle visits the old\l"
	.string "stump in the west woods.$"

ElderwoodForest_ElderHouse_Text_After:
	.string "The forest is at peace, thanks\n"
	.string "to you, {PLAYER}.$"

ElderwoodForest_ElderHouse_Text_Bounsweet:
	.string "BOUNSWEET: Bou-bou!$"
''')

interior('ElderwoodForest_Lookout', 'MAP_ELDERWOOD_FOREST_LOOKOUT', 'LAYOUT_FORTREE_CITY_HOUSE2', 1, [
    npc('OBJ_EVENT_GFX_MAN_3', 2, 2, 'ElderwoodForest_Lookout_EventScript_Ranger'),
    npc('OBJ_EVENT_GFX_LITTLE_GIRL', 5, 3, 'ElderwoodForest_Lookout_EventScript_Girl', 'MOVEMENT_TYPE_WANDER_AROUND'),
], r'''
ElderwoodForest_Lookout_MapScripts::
	.byte 0

ElderwoodForest_Lookout_EventScript_Ranger::
	lock
	faceplayer
	goto_if_set FLAG_ELDERWOOD_LOOKOUT_GIFT, ElderwoodForest_Lookout_EventScript_RangerAfter
	msgbox ElderwoodForest_Lookout_Text_Ranger, MSGBOX_DEFAULT
	giveitem ITEM_BIG_ROOT
	goto_if_eq VAR_RESULT, FALSE, ElderwoodForest_Lookout_EventScript_End
	setflag FLAG_ELDERWOOD_LOOKOUT_GIFT
ElderwoodForest_Lookout_EventScript_RangerAfter::
	msgbox ElderwoodForest_Lookout_Text_RangerAfter, MSGBOX_DEFAULT
ElderwoodForest_Lookout_EventScript_End::
	release
	end

ElderwoodForest_Lookout_EventScript_Girl::
	msgbox ElderwoodForest_Lookout_Text_Girl, MSGBOX_NPC
	end

ElderwoodForest_Lookout_Text_Ranger:
	.string "Welcome to the ranger lookout!\n"
	.string "From up here you can see the whole\l"
	.string "canopy.\p"
	.string "Here, a souvenir. Roots like this\n"
	.string "hold the whole forest together.$"

ElderwoodForest_Lookout_Text_RangerAfter:
	.string "Mushroom hunters say the best ones\n"
	.string "grow in the far corners, where the\l"
	.string "light hardly reaches.$"

ElderwoodForest_Lookout_Text_Girl:
	.string "I counted the treehouses: four!\n"
	.string "But only two doors open. The\l"
	.string "others are full of BOUNSWEET nests.$"
''')
print('ok', NAME)

# ---------------------------------------------------------------- the way in, from Fortree City
add_object('FortreeCity', {'graphics_id': 'OBJ_EVENT_GFX_MAN_4', 'x': 37, 'y': 9, 'elevation': 3,
                           'movement_type': 'MOVEMENT_TYPE_FACE_LEFT', 'movement_range_x': 0, 'movement_range_y': 0,
                           'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                           'script': 'FortreeCity_EventScript_ElderwoodGuide', 'flag': '0'})
register_script_file('elderwood', r'''
@ Elderwood Forest guide in Fortree City (tools/areas/elderwood.py).
FortreeCity_EventScript_ElderwoodGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE06_GET, FortreeCity_EventScript_ElderwoodGuideNoBadge
	msgbox FortreeCity_Text_ElderwoodGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_ElderwoodGuideNo
	msgbox FortreeCity_Text_ElderwoodGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_ELDERWOOD_FOREST, 27, 49
	waitstate
	release
	end

FortreeCity_EventScript_ElderwoodGuideNoBadge::
	msgbox FortreeCity_Text_ElderwoodGuideNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_ElderwoodGuideNo::
	msgbox FortreeCity_Text_ElderwoodGuideNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_ElderwoodGuideNoBadge:
	.string "East of here lies ELDERWOOD, the\n"
	.string "oldest forest in HOENN.\p"
	.string "The canopy paths are dangerous.\n"
	.string "Come back with WINONA's badge and\l"
	.string "I'll take you there.$"

FortreeCity_Text_ElderwoodGuideAsk:
	.string "That's WINONA's badge! Then you can\n"
	.string "handle the canopy.\p"
	.string "Shall I lead you to ELDERWOOD\n"
	.string "FOREST?$"

FortreeCity_Text_ElderwoodGuideGo:
	.string "Stay close. The trail is hidden\n"
	.string "under the vines!$"

FortreeCity_Text_ElderwoodGuideNo:
	.string "The forest has waited a thousand\n"
	.string "years. It can wait for you.$"
''')
print('ok entrance')
