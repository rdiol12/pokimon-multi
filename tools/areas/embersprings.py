"""Ember Springs: a hot-spring town in a volcanic valley above Lavaridge.
Soak in the springs to heal and grow closer to your Pokémon, chase three
Slugma off the town's steam vents, and earn a soak in the hidden Old Spring.

    python3 tools/areas/embersprings.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import volcanic, SAND

NAME = 'EmberSprings'
AREA_NO = 1
MAPSEC = 'MAPSEC_EMBER_SPRINGS'

register_constants(NAME, AREA_NO, flags=[
    'EMBER_SLUGMA_WEST', 'EMBER_SLUGMA_NORTH', 'EMBER_SLUGMA_EAST', 'EMBER_QUEST_DONE', 'EMBER_EGG_GIVEN',
    'ITEM_EMBER_FIRE_STONE', 'ITEM_EMBER_HEAT_ROCK', 'ITEM_EMBER_MAX_REVIVE', 'ITEM_EMBER_LAVA_COOKIE',
    'HIDDEN_ITEM_EMBER_CHARCOAL', 'HIDDEN_ITEM_EMBER_PP_UP', 'HIDDEN_ITEM_EMBER_STAR_PIECE',
    'EMBER_HERMIT_GIFT', 'HIDE_EMBER_SPRING_GUARD',
], trainers=['EMBER_BLAZE', 'EMBER_CINDY', 'EMBER_ROCCO', 'EMBER_SAKI'])

register_trainers(NAME, '''
=== AREA_TRAINER_EMBER_BLAZE ===
Name: BLAZE
Class: Kindler
Pic: Kindler
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Magmar
Level: 31

Torkoal
Level: 32

=== AREA_TRAINER_EMBER_CINDY ===
Name: CINDY
Class: Beauty
Pic: Beauty
Gender: Female
Music: Female
Double Battle: No
AI: Basic Trainer

Ponyta
Level: 31

Vulpix
Level: 31

Fletchinder
Level: 32

=== AREA_TRAINER_EMBER_ROCCO ===
Name: ROCCO
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Rolycoly
Level: 30

Carkol
Level: 32

Camerupt
Level: 33

=== AREA_TRAINER_EMBER_SAKI ===
Name: SAKI
Class: Expert
Pic: Expert F
Gender: Female
Music: Intense
Double Battle: No
AI: Basic Trainer

Salandit
Level: 32

Litleo
Level: 32

Turtonator
Level: 34
''')

st = volcanic()
a = Area(NAME, 46, 40, st, fill='#')

# the valley
a.ellipse(23, 22, 21, 17, 'g')
a.ellipse(8, 7, 6, 4.5, 'r')        # west vent hill
a.ellipse(23, 4, 7, 3, 'r')         # north vent hill
a.ellipse(39, 8, 5, 4.5, 'r')       # east vent hill
a.rect(21, 4, 4, 6, 'r')
a.rect(7, 7, 3, 6, 'r')
a.rect(37, 8, 3, 6, 'r')
a.ellipse(33, 31, 6, 4.5, 'r')      # the Old Spring hollow, behind the guard
a.rect(2, 26, 6, 8, 'T')
a.rect(40, 16, 6, 8, 'T')
a.rect(12, 30, 6, 4, 'T')

a.ellipse(18, 36, 5, 2, '"')
a.ellipse(40, 25, 3, 2, '"')
a.line([(22, 33), (22, 39)], 2, 'g')
a.snap_trees(ground='g')
# sand paths
a.line([(22, 39), (22, 9)], 2, 'S')
a.line([(6, 13), (40, 13)], 2, 'S')
a.line([(8, 13), (8, 8)], 2, 'S')
a.line([(38, 13), (38, 9)], 2, 'S')
a.line([(22, 24), (33, 24)], 2, 'S')
a.line([(33, 24), (33, 26)], 2, 'S')
a.autotile3('S', SAND)

# buildings and springs, copied from Lavaridge
a.stamp('LAYOUT_LAVARIDGE_TOWN', 8, 3, 4, 4, 14, 8)      # Pokémon Center, door (15, 11)
a.stamp('LAYOUT_LAVARIDGE_TOWN', 11, 12, 4, 4, 25, 8)    # inn, door (26, 11)
a.stamp('LAYOUT_LAVARIDGE_TOWN', 15, 12, 4, 4, 30, 8)    # hermit's house, door (31, 11)
a.stamp('LAYOUT_LAVARIDGE_TOWN', 2, 2, 6, 9, 9, 15)      # public spring, entered from (13, 15)
a.stamp('LAYOUT_LAVARIDGE_TOWN', 2, 2, 6, 9, 30, 27)     # the Old Spring, entered from (34, 27)
a.scatter(0x004, 18, 4, 16, 44, 36, seed=3)

W_PC = a.warp(15, 11, 'MAP_EMBER_SPRINGS_POKEMON_CENTER_1F', 0)
W_INN = a.warp(26, 11, 'MAP_EMBER_SPRINGS_INN', 0)
W_HERMIT = a.warp(31, 11, 'MAP_EMBER_SPRINGS_HERMIT_HOUSE', 0)
# the cave on the north hill leads down into Cinder Caverns (tools/areas/cindercaverns.py)
a.tile(19, 0, 0x256, collision=1, elevation=0, c='o')
a.tile(19, 1, 0x25E, collision=0, elevation=0, c='o')
W_CAVE = a.warp(19, 1, 'MAP_CINDER_CAVERNS_1F', 0)

a.obj('OBJ_EVENT_GFX_HIKER', 20, 37, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_RIGHT')
a.sign(24, 36, f'{NAME}_EventScript_TownSign')
a.sign(17, 12, f'{NAME}_EventScript_SpringSign')
a.obj('OBJ_EVENT_GFX_WOMAN_3', 15, 20, f'{NAME}_EventScript_Attendant', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_OLD_MAN', 21, 15, f'{NAME}_EventScript_SpringKeeper', 'MOVEMENT_TYPE_FACE_DOWN')
# the guard in front of the Old Spring
a.obj('OBJ_EVENT_GFX_BLACK_BELT', 34, 26, f'{NAME}_EventScript_Guard', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_EMBER_SPRING_GUARD', local_id='LOCALID_EMBER_GUARD')
a.obj('OBJ_EVENT_GFX_SPECIES(TORKOAL)', 37, 30, f'{NAME}_EventScript_Torkoal', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_OLD_WOMAN', 35, 33, f'{NAME}_EventScript_OldSpringLady', 'MOVEMENT_TYPE_FACE_UP')
# three Slugma on the steam vents
for key, x, y in (('WEST', 6, 6), ('NORTH', 23, 3), ('EAST', 41, 7)):
    a.obj('OBJ_EVENT_GFX_SPECIES(SLUGMA)', x, y, f'{NAME}_EventScript_Slugma{key.title()}', 'MOVEMENT_TYPE_FACE_DOWN',
          f'FLAG_EMBER_SLUGMA_{key}', local_id=f'LOCALID_EMBER_SLUGMA_{key}')
# trainers
a.trainer('OBJ_EVENT_GFX_MAN_5', 12, 13, f'{NAME}_EventScript_Blaze', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_BEAUTY', 11, 22, f'{NAME}_EventScript_Cindy', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_HIKER', 35, 10, f'{NAME}_EventScript_Rocco', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_EXPERT_F', 27, 24, f'{NAME}_EventScript_Saki', 4, 'MOVEMENT_TYPE_FACE_LEFT')
# townsfolk
a.obj('OBJ_EVENT_GFX_LITTLE_BOY', 25, 18, f'{NAME}_EventScript_Boy', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=2)
a.obj('OBJ_EVENT_GFX_SPECIES(NUMEL)', 27, 19, f'{NAME}_EventScript_Numel', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_FAT_MAN', 30, 15, f'{NAME}_EventScript_Tourist', 'MOVEMENT_TYPE_FACE_DOWN')
# items
a.item('ITEM_FIRE_STONE', 4, 8, 'FLAG_ITEM_EMBER_FIRE_STONE')
a.item('ITEM_HEAT_ROCK', 26, 3, 'FLAG_ITEM_EMBER_HEAT_ROCK')
a.item('ITEM_MAX_REVIVE', 43, 9, 'FLAG_ITEM_EMBER_MAX_REVIVE')
a.item('ITEM_LAVA_COOKIE', 6, 22, 'FLAG_ITEM_EMBER_LAVA_COOKIE')
a.hidden(10, 5, 'ITEM_CHARCOAL', 'FLAG_HIDDEN_ITEM_EMBER_CHARCOAL')
a.hidden(38, 33, 'ITEM_PP_UP', 'FLAG_HIDDEN_ITEM_EMBER_PP_UP')
a.hidden(20, 4, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_EMBER_STAR_PIECE')

SLUGMA = ''.join(f'''
EmberSprings_EventScript_Slugma{k.title()}::
	lock
	faceplayer
	playmoncry SPECIES_SLUGMA, CRY_MODE_ENCOUNTER
	msgbox EmberSprings_Text_SlugmaVent, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_SLUGMA, {lv}
	dowildbattle
	setflag FLAG_EMBER_SLUGMA_{k}
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_EMBER_SLUGMA_{k}
	fadescreen FADE_FROM_BLACK
	goto EmberSprings_EventScript_VentCleared
''' for k, lv in (('WEST', 31), ('NORTH', 32), ('EAST', 33)))

SCRIPTS = r'''
EmberSprings_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, EmberSprings_OnTransition
	.byte 0

EmberSprings_OnTransition:
	call_if_set FLAG_EMBER_QUEST_DONE, EmberSprings_EventScript_HideGuard
	end

EmberSprings_EventScript_HideGuard::
	setflag FLAG_HIDE_EMBER_SPRING_GUARD
	return

EmberSprings_EventScript_Guide::
	lock
	faceplayer
	msgbox EmberSprings_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_GuideStay
	closemessage
	warp MAP_LAVARIDGE_TOWN, 17, 10
	waitstate
	release
	end

EmberSprings_EventScript_GuideStay::
	msgbox EmberSprings_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_TownSign::
	msgbox EmberSprings_Text_TownSign, MSGBOX_SIGN
	end

EmberSprings_EventScript_SpringSign::
	msgbox EmberSprings_Text_SpringSign, MSGBOX_SIGN
	end

@ A soak heals the party and raises friendship (src/area_specials.c).
EmberSprings_EventScript_Attendant::
	lock
	faceplayer
	msgbox EmberSprings_Text_Soak, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_SoakNo
	closemessage
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	callnative Area_SoakInHotSpring
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox EmberSprings_Text_Soaked, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_SoakNo::
	msgbox EmberSprings_Text_SoakNo, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_SpringKeeper::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_QUEST_DONE, EmberSprings_EventScript_KeeperAfter
	goto_if_unset FLAG_EMBER_SLUGMA_WEST, EmberSprings_EventScript_KeeperAsk
	goto_if_unset FLAG_EMBER_SLUGMA_NORTH, EmberSprings_EventScript_KeeperAsk
	goto_if_unset FLAG_EMBER_SLUGMA_EAST, EmberSprings_EventScript_KeeperAsk
	msgbox EmberSprings_Text_KeeperThanks, MSGBOX_DEFAULT
	giveitem ITEM_SOOTHE_BELL
	giveitem ITEM_TM_OVERHEAT
	setflag FLAG_EMBER_QUEST_DONE
	msgbox EmberSprings_Text_KeeperOldSpring, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_KeeperAsk::
	msgbox EmberSprings_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_KeeperAfter::
	msgbox EmberSprings_Text_KeeperAfter, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_VentCleared::
	goto_if_unset FLAG_EMBER_SLUGMA_WEST, EmberSprings_EventScript_VentsLeft
	goto_if_unset FLAG_EMBER_SLUGMA_NORTH, EmberSprings_EventScript_VentsLeft
	goto_if_unset FLAG_EMBER_SLUGMA_EAST, EmberSprings_EventScript_VentsLeft
	msgbox EmberSprings_Text_AllVents, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_VentsLeft::
	msgbox EmberSprings_Text_VentCleared, MSGBOX_DEFAULT
	release
	end
''' + SLUGMA + r'''
EmberSprings_EventScript_Guard::
	msgbox EmberSprings_Text_Guard, MSGBOX_NPC
	end

EmberSprings_EventScript_Torkoal::
	lock
	faceplayer
	playmoncry SPECIES_TORKOAL, CRY_MODE_NORMAL
	msgbox EmberSprings_Text_Torkoal, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ The Old Spring: a free soak, and once, an Egg from the lady who keeps it.
EmberSprings_EventScript_OldSpringLady::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_EGG_GIVEN, EmberSprings_EventScript_OldSpringSoak
	msgbox EmberSprings_Text_EggOffer, MSGBOX_DEFAULT
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, EmberSprings_EventScript_EggNoRoom
	setflag FLAG_EMBER_EGG_GIVEN
	playfanfare MUS_OBTAIN_ITEM
	message EmberSprings_Text_GotEgg
	waitfanfare
	giveegg SPECIES_CHARCADET
	msgbox EmberSprings_Text_EggCare, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_EggNoRoom::
	msgbox EmberSprings_Text_EggNoRoom, MSGBOX_DEFAULT
	release
	end

EmberSprings_EventScript_OldSpringSoak::
	msgbox EmberSprings_Text_OldSoak, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_EventScript_OldSoakNo
	closemessage
	fadescreen FADE_TO_WHITE
	playfanfare MUS_HEAL
	callnative Area_SoakInHotSpring
	callnative Area_SoakInHotSpring
	waitfanfare
	fadescreen FADE_FROM_WHITE
	msgbox EmberSprings_Text_OldSoaked, MSGBOX_DEFAULT
EmberSprings_EventScript_OldSoakNo::
	release
	end

EmberSprings_EventScript_Blaze::
	trainerbattle_single TRAINER_EMBER_BLAZE, EmberSprings_Text_BlazeIntro, EmberSprings_Text_BlazeDefeat
	msgbox EmberSprings_Text_BlazePost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Cindy::
	trainerbattle_single TRAINER_EMBER_CINDY, EmberSprings_Text_CindyIntro, EmberSprings_Text_CindyDefeat
	msgbox EmberSprings_Text_CindyPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Rocco::
	trainerbattle_single TRAINER_EMBER_ROCCO, EmberSprings_Text_RoccoIntro, EmberSprings_Text_RoccoDefeat
	msgbox EmberSprings_Text_RoccoPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Saki::
	trainerbattle_single TRAINER_EMBER_SAKI, EmberSprings_Text_SakiIntro, EmberSprings_Text_SakiDefeat
	msgbox EmberSprings_Text_SakiPost, MSGBOX_AUTOCLOSE
	end

EmberSprings_EventScript_Boy::
	msgbox EmberSprings_Text_Boy, MSGBOX_NPC
	end

EmberSprings_EventScript_Numel::
	lock
	faceplayer
	playmoncry SPECIES_NUMEL, CRY_MODE_NORMAL
	msgbox EmberSprings_Text_Numel, MSGBOX_DEFAULT
	waitmoncry
	release
	end

EmberSprings_EventScript_Tourist::
	msgbox EmberSprings_Text_Tourist, MSGBOX_NPC
	end

EmberSprings_Text_GuideBack:
	.string "Heading back down to LAVARIDGE?\n"
	.string "I can show you the short trail.$"

EmberSprings_Text_GuideStay:
	.string "Stay as long as you like. The water\n"
	.string "is always warm here.$"

EmberSprings_Text_TownSign:
	.string "EMBER SPRINGS\n"
	.string "“Where the mountain keeps you warm.”$"

EmberSprings_Text_SpringSign:
	.string "PUBLIC SPRING\n"
	.string "Ask the attendant for a soak.$"

EmberSprings_Text_Soak:
	.string "Welcome to the springs! A soak heals\n"
	.string "your POKéMON and brings you closer.\p"
	.string "Would you like to take a soak?$"

EmberSprings_Text_Soaked:
	.string "Your POKéMON look relaxed and happy.\n"
	.string "They seem to like you even more!$"

EmberSprings_Text_SoakNo:
	.string "Come back whenever you're weary.$"

EmberSprings_Text_KeeperAsk:
	.string "I keep the springs flowing. But the\n"
	.string "steam vents on the hills are plugged!\p"
	.string "SLUGMA have curled up on all three\n"
	.string "of them to keep warm.\p"
	.string "West, north and east… Could you\n"
	.string "chase them off for me?$"

EmberSprings_Text_KeeperThanks:
	.string "The vents are steaming again! The\n"
	.string "springs will be hot for years.\p"
	.string "Please, take these.$"

EmberSprings_Text_KeeperOldSpring:
	.string "And I've told my son to stop\n"
	.string "guarding the OLD SPRING.\p"
	.string "Go and soak in it. It's the oldest\n"
	.string "water in HOENN.$"

EmberSprings_Text_KeeperAfter:
	.string "Steam, steam, glorious steam!$"

EmberSprings_Text_SlugmaVent:
	.string "A SLUGMA is curled up on a steam\n"
	.string "vent. It won't budge!$"

EmberSprings_Text_VentCleared:
	.string "The vent hisses back to life!$"

EmberSprings_Text_AllVents:
	.string "That was the last plugged vent!\n"
	.string "The whole valley fills with steam.\p"
	.string "The SPRING KEEPER will want to\n"
	.string "hear about this.$"

EmberSprings_Text_Guard:
	.string "The OLD SPRING is only for those\n"
	.string "my father trusts.\p"
	.string "He's worried about the steam vents\n"
	.string "on the hills, you know.$"

EmberSprings_Text_Torkoal:
	.string "TORKOAL: Koooh…\p"
	.string "It's puffing happy smoke rings.$"

EmberSprings_Text_EggOffer:
	.string "Oh, you cleared the vents! The OLD\n"
	.string "SPRING is warm enough to hatch eggs\l"
	.string "again.\p"
	.string "This one was laid in the warm sand.\n"
	.string "Please raise it.$"

EmberSprings_Text_GotEgg:
	.string "{PLAYER} received an EGG!$"

EmberSprings_Text_EggCare:
	.string "Keep it warm and it'll hatch into\n"
	.string "something fiery.$"

EmberSprings_Text_EggNoRoom:
	.string "Your party is full. Make room and\n"
	.string "come back.$"

EmberSprings_Text_OldSoak:
	.string "The OLD SPRING is extra warm today.\n"
	.string "Take a soak?$"

EmberSprings_Text_OldSoaked:
	.string "Your POKéMON look blissful.\n"
	.string "They trust you more than ever!$"

EmberSprings_Text_BlazeIntro:
	.string "My POKéMON and I train in the\n"
	.string "steam. Feel the heat!$"

EmberSprings_Text_BlazeDefeat:
	.string "I'm steamed!$"

EmberSprings_Text_BlazePost:
	.string "After a battle, a soak is the best.$"

EmberSprings_Text_CindyIntro:
	.string "The springs keep my skin glowing.\n"
	.string "And my POKéMON's fur!$"

EmberSprings_Text_CindyDefeat:
	.string "Oh, I'm all flustered.$"

EmberSprings_Text_CindyPost:
	.string "PONYTA's mane gets so shiny after\n"
	.string "a soak.$"

EmberSprings_Text_RoccoIntro:
	.string "I climbed all the way up here just\n"
	.string "for a battle!$"

EmberSprings_Text_RoccoDefeat:
	.string "That rocked!$"

EmberSprings_Text_RoccoPost:
	.string "I found a HEAT ROCK on the north\n"
	.string "hill once.$"

EmberSprings_Text_SakiIntro:
	.string "I guard the path to the OLD\n"
	.string "SPRING's hollow. Prove yourself!$"

EmberSprings_Text_SakiDefeat:
	.string "Your spirit burns bright.$"

EmberSprings_Text_SakiPost:
	.string "Some say a fiery legend sleeps\n"
	.string "deep inside the volcano.$"

EmberSprings_Text_Boy:
	.string "My NUMEL won't get out of the\n"
	.string "springs. It's too comfy!$"

EmberSprings_Text_Numel:
	.string "NUMEL: Nuuu…$"

EmberSprings_Text_Tourist:
	.string "I came all the way from SLATEPORT\n"
	.string "for the springs. Worth the climb!$"
'''

a.write(map_id='MAP_EMBER_SPRINGS', layout_id='LAYOUT_EMBER_SPRINGS', music='MUS_FALLARBOR', mapsec=MAPSEC,
        mapsec_name='EMBER SPRINGS', weather='WEATHER_NONE', map_type='MAP_TYPE_TOWN', scripts=SCRIPTS)

register_wild('MAP_EMBER_SPRINGS', 'gEmberSprings',
              land=[('NUMEL', 29, 31), ('SLUGMA', 29, 31), ('TORKOAL', 30, 32), ('LITLEO', 29, 31),
                    ('SALANDIT', 29, 31), ('ROLYCOLY', 29, 31), ('SIZZLIPEDE', 30, 32), ('CHARCADET', 30, 32),
                    ('PONYTA', 31, 33), ('MAGBY', 30, 32), ('TURTONATOR', 33, 34), ('HEATMOR', 33, 34)])

pc = pokemon_center('EmberSprings', MAPSEC, 'gMapGroup_IndoorLavaridge', 'MAP_EMBER_SPRINGS', W_PC, [
    ('OBJ_EVENT_GFX_GENTLEMAN', 2, 5, ['A soak in the springs and a rest at', 'the CENTER. Perfect holiday!$']),
    ('OBJ_EVENT_GFX_TWIN', 10, 6, ['The SPRING KEEPER looks worried', 'about the steam vents.$']),
])

interior('EmberSprings_Inn', 'MAP_EMBER_SPRINGS_INN', 'LAYOUT_HOUSE3', [(3, 7), (4, 7)], 'MAP_EMBER_SPRINGS', W_INN, [
    npc('OBJ_EVENT_GFX_WOMAN_5', 4, 3, 'EmberSprings_Inn_EventScript_Innkeeper'),
    npc('OBJ_EVENT_GFX_SPECIES(ESPURR)', 7, 4, 'EmberSprings_Inn_EventScript_Espurr', 'MOVEMENT_TYPE_WANDER_AROUND'),
], r'''
EmberSprings_Inn_MapScripts::
	.byte 0

EmberSprings_Inn_EventScript_Innkeeper::
	lock
	faceplayer
	msgbox EmberSprings_Inn_Text_Rest, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EmberSprings_Inn_EventScript_NoRest
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	msgbox EmberSprings_Inn_Text_Rested, MSGBOX_DEFAULT
EmberSprings_Inn_EventScript_NoRest::
	release
	end

EmberSprings_Inn_EventScript_Espurr::
	lock
	faceplayer
	playmoncry SPECIES_ESPURR, CRY_MODE_NORMAL
	msgbox EmberSprings_Inn_Text_Espurr, MSGBOX_DEFAULT
	waitmoncry
	release
	end

EmberSprings_Inn_Text_Rest:
	.string "Welcome to the EMBER INN!\n"
	.string "Would you like to rest a while?$"

EmberSprings_Inn_Text_Rested:
	.string "Good morning! Your POKéMON are\n"
	.string "full of energy.$"

EmberSprings_Inn_Text_Espurr:
	.string "ESPURR: Mew?$"
''', MAPSEC, 'gMapGroup_IndoorLavaridge')

interior('EmberSprings_HermitHouse', 'MAP_EMBER_SPRINGS_HERMIT_HOUSE', 'LAYOUT_HOUSE3', [(3, 7), (4, 7)], 'MAP_EMBER_SPRINGS', W_HERMIT, [
    npc('OBJ_EVENT_GFX_EXPERT_M', 2, 4, 'EmberSprings_HermitHouse_EventScript_Hermit'),
], r'''
EmberSprings_HermitHouse_MapScripts::
	.byte 0

EmberSprings_HermitHouse_EventScript_Hermit::
	lock
	faceplayer
	goto_if_set FLAG_EMBER_HERMIT_GIFT, EmberSprings_HermitHouse_EventScript_After
	msgbox EmberSprings_HermitHouse_Text_Hermit, MSGBOX_DEFAULT
	giveitem ITEM_FLAME_ORB
	goto_if_eq VAR_RESULT, FALSE, EmberSprings_HermitHouse_EventScript_End
	setflag FLAG_EMBER_HERMIT_GIFT
EmberSprings_HermitHouse_EventScript_After::
	msgbox EmberSprings_HermitHouse_Text_After, MSGBOX_DEFAULT
EmberSprings_HermitHouse_EventScript_End::
	release
	end

EmberSprings_HermitHouse_Text_Hermit:
	.string "I've lived by the volcano for sixty\n"
	.string "years. It speaks, if you listen.\p"
	.string "Here. This orb was warmed in its\n"
	.string "heart.$"

EmberSprings_HermitHouse_Text_After:
	.string "Deep in the mountain, the lava runs\n"
	.string "in rivers. One day someone brave\l"
	.string "will walk them.$"
''', MAPSEC, 'gMapGroup_IndoorLavaridge')

# ---------------------------------------------------------------- the way in, from Lavaridge Town
add_object('LavaridgeTown', npc('OBJ_EVENT_GFX_HIKER', 18, 10, 'LavaridgeTown_EventScript_EmberSpringsGuide',
                                'MOVEMENT_TYPE_FACE_LEFT'))
register_script_file('embersprings', r'''
@ Ember Springs guide in Lavaridge Town (tools/areas/embersprings.py).
LavaridgeTown_EventScript_EmberSpringsGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, LavaridgeTown_EventScript_EmberSpringsNoBadge
	msgbox LavaridgeTown_Text_EmberSpringsAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LavaridgeTown_EventScript_EmberSpringsNo
	closemessage
	warp MAP_EMBER_SPRINGS, 22, 38
	waitstate
	release
	end

LavaridgeTown_EventScript_EmberSpringsNoBadge::
	msgbox LavaridgeTown_Text_EmberSpringsNoBadge, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_EventScript_EmberSpringsNo::
	msgbox LavaridgeTown_Text_EmberSpringsNo, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_Text_EmberSpringsNoBadge:
	.string "Up the mountain is EMBER SPRINGS,\n"
	.string "a town of hot springs.\p"
	.string "The trail is steep. Beat FLANNERY\n"
	.string "first, then I'll show you the way.$"

LavaridgeTown_Text_EmberSpringsAsk:
	.string "You beat FLANNERY! Want me to show\n"
	.string "you the trail up to EMBER SPRINGS?$"

LavaridgeTown_Text_EmberSpringsNo:
	.string "The springs aren't going anywhere!$"
''')
print('ok', NAME)
