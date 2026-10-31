"""Sunfair Carnival: a fairground east of Slateport with a Ferris wheel, a
big top, food and game stalls, and open ground for bowling, mini golf and
soccer. The Ringmaster's Spinda has wandered off dizzy; bring it back and
he gives you a Mime Jr. egg.

The four game hosts call Carnival_EventScript_Bowling, _Golf, _Soccer and
_FerrisWheel, which live in data/scripts/areas/carnival_games.inc. For now
they say "coming soon"; the minigames branch (claude/pastime-carnival)
replaces their bodies with the real games.

    python3 tools/areas/carnival_art.py   # the tileset, when the art changes
    python3 tools/areas/carnival.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import town, POND

AREA_NO = 8
NAME = 'SunfairCarnival'
MAPSEC = 'MAPSEC_SUNFAIR_CARNIVAL'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'carnival_tiles.json')))

register_constants(NAME, AREA_NO, flags=[
    'CARNIVAL_SPINDA_FOUND', 'CARNIVAL_EGG_DONE', 'HIDE_CARNIVAL_SPINDA_LOST', 'HIDE_CARNIVAL_SPINDA_HOME',
    'ITEM_CARNIVAL_STAR_PIECE', 'ITEM_CARNIVAL_PP_UP', 'ITEM_CARNIVAL_TM_ATTRACT',
    'HIDDEN_ITEM_CARNIVAL_BIG_PEARL', 'HIDDEN_ITEM_CARNIVAL_LUXURY_BALL', 'HIDDEN_ITEM_CARNIVAL_HEART_SCALE',
], trainers=['CARNIVAL_BLAZE', 'CARNIVAL_KIT_AND_KAT', 'CARNIVAL_SHADE', 'CARNIVAL_RIFF'])

register_trainers(NAME, '''
=== AREA_TRAINER_CARNIVAL_BLAZE ===
Name: BLAZE
Class: Kindler
Pic: Kindler
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Litleo
Level: 24

Slugma
Level: 24

Magby
Level: 25

=== AREA_TRAINER_CARNIVAL_KIT_AND_KAT ===
Name: KIT & KAT
Class: Twins
Pic: Twins
Gender: Female
Music: Girl
Double Battle: Yes
AI: Basic Trainer

Minun
Level: 24

Plusle
Level: 24

=== AREA_TRAINER_CARNIVAL_SHADE ===
Name: SHADE
Class: Ninja Boy
Pic: Ninja Boy
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Croagunk
Level: 24

Nincada
Level: 23

Zubat
Level: 24

=== AREA_TRAINER_CARNIVAL_RIFF ===
Name: RIFF
Class: Guitarist
Pic: Guitarist
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Toxel
Level: 24

Loudred
Level: 25
''')


def place(a, name, x, y):
    """Stamp one of the carnival's drawn objects with its top left at (x, y)."""
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(ART['objects'][name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


st = copy.copy(town())
st.secondary = 'gTileset_Carnival'      # Petalburg's tiles plus the carnival art, same metatile ids
st.exclude = set(st.exclude) | {0x129, 0x087, 0x0C6, 0x1D4, 0x1E7}  # worn sand, fences, signs
W, H = 60, 48
a = Area(NAME, W, H, st, fill='T')
a.rect(3, 3, 54, 41, 'g')
a.rect(26, 44, 8, 4, 'g')
# paths and the plaza in front of the wheel
a.rect(28, 40, 4, 8, 's')
a.rect(29, 13, 2, 27, 's')
a.rect(6, 22, 49, 2, 's')
a.rect(22, 12, 16, 4, 's')
a.rect(8, 13, 4, 9, 's')
# open ground the games will use: bowling lanes, the soccer pitch, the golf green
LANES = (40, 4, 15, 8)
PITCH = (4, 27, 18, 10)
GREEN = (38, 27, 17, 11)
a.rect(*LANES, 's')
a.ellipse(46, 33, 3, 1.5, 'P')
a.autotile3('P', POND)
# trees and flowers
for x, y in ((22, 4), (36, 4), (16, 14), (22, 18), (36, 18), (24, 30), (34, 30), (4, 40), (54, 40), (16, 4)):
    a.rect(x, y, 2, 2, 'T')
for x, y in ((3, 22), (56, 22), (3, 34), (14, 4), (48, 26), (54, 26)):
    a.rect(x, y, 2, 2, 'T')
a.snap_trees(ground='g')
FLOWERS = ((24, 26), (35, 26), (24, 33), (35, 33), (6, 4), (19, 8), (38, 8), (14, 41), (44, 41), (51, 20),
           (6, 18), (20, 14), (42, 23))
for i, (x, y) in enumerate(FLOWERS):
    for dx, dy in ((0, 0), (1, 0), (0, 1), (1, 1), (2, 0))[:3 + i % 3]:
        a.tile(x + dx, y + dy, 0x004, collision=0)

place(a, 'ferris_wheel', 26, 3)
place(a, 'big_top', 7, 7)
place(a, 'arch', 27, 40)
place(a, 'stall_red', 33, 17)
place(a, 'stall_blue', 24, 17)
place(a, 'stall_red', 18, 24)
place(a, 'big_top', 47, 14)
place(a, 'stall_blue', 17, 8)
place(a, 'balloons', 32, 37)
place(a, 'balloons', 20, 20)
for x, y in ((27, 12), (32, 12), (27, 25), (32, 25), (27, 35), (32, 35), (14, 20), (44, 20)):
    place(a, 'lamp', x, y)

ARRIVE = (29, 46)
a.obj('OBJ_EVENT_GFX_SAILOR', 31, 46, f'{NAME}_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(27, 43, f'{NAME}_EventScript_Sign', elevation=0)
# the four game hosts
HOSTS = {'FerrisWheel': (29, 12), 'Bowling': (9, 13), 'Golf': (39, 26), 'Soccer': (12, 26)}
a.obj('OBJ_EVENT_GFX_GENTLEMAN', *HOSTS['FerrisWheel'], 'Carnival_EventScript_FerrisWheel', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_MAN_2', *HOSTS['Bowling'], 'Carnival_EventScript_Bowling', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_RICH_BOY', *HOSTS['Golf'], 'Carnival_EventScript_Golf', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_BOY_2', *HOSTS['Soccer'], 'Carnival_EventScript_Soccer', 'MOVEMENT_TYPE_FACE_DOWN')
# stall keepers stand behind their counters
a.obj('OBJ_EVENT_GFX_CHEF', 34, 18, f'{NAME}_EventScript_FoodStall', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WOMAN_2', 25, 18, f'{NAME}_EventScript_RingToss', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_COOK', 19, 25, f'{NAME}_EventScript_SweetStall', 'MOVEMENT_TYPE_FACE_DOWN')
# the Ringmaster and his Spinda
a.obj('OBJ_EVENT_GFX_MAN_4', 11, 15, f'{NAME}_EventScript_Ringmaster', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SPECIES(SPINDA)', 52, 6, f'{NAME}_EventScript_SpindaLost', 'MOVEMENT_TYPE_WANDER_AROUND',
      'FLAG_HIDE_CARNIVAL_SPINDA_LOST', rx=1, ry=1)
a.obj('OBJ_EVENT_GFX_SPECIES(SPINDA)', 12, 16, f'{NAME}_EventScript_SpindaHome', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_CARNIVAL_SPINDA_HOME')
SPINDA_HOME = len(a.objects)
# visitors
a.obj('OBJ_EVENT_GFX_LITTLE_GIRL', 34, 38, f'{NAME}_EventScript_BalloonGirl', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_SPECIES(PIKACHU)', 35, 38, f'{NAME}_EventScript_Pikachu', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_OLD_MAN', 23, 14, f'{NAME}_EventScript_OldMan', 'MOVEMENT_TYPE_FACE_UP')
a.obj('OBJ_EVENT_GFX_WOMAN_1', 44, 23, f'{NAME}_EventScript_Woman', 'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT', rx=2)
a.obj('OBJ_EVENT_GFX_SPECIES(MR_MIME)', 21, 30, f'{NAME}_EventScript_MrMime', 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_HIKER', 16, 22, f'{NAME}_EventScript_Blaze', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_WOMAN_4', 50, 19, f'{NAME}_EventScript_Fortune', 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_TWIN', 44, 15, f'{NAME}_EventScript_KitAndKat', 2, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_NINJA_BOY', 4, 6, f'{NAME}_EventScript_Shade', 1, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_MAN_5', 24, 36, f'{NAME}_EventScript_Riff', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.item('ITEM_STAR_PIECE', 55, 4, 'FLAG_ITEM_CARNIVAL_STAR_PIECE')
a.item('ITEM_PP_UP', 4, 42, 'FLAG_ITEM_CARNIVAL_PP_UP')
a.item('ITEM_TM_ATTRACT', 55, 42, 'FLAG_ITEM_CARNIVAL_TM_ATTRACT')
a.hidden(5, 13, 'ITEM_BIG_PEARL', 'FLAG_HIDDEN_ITEM_CARNIVAL_BIG_PEARL')
a.hidden(48, 37, 'ITEM_LUXURY_BALL', 'FLAG_HIDDEN_ITEM_CARNIVAL_LUXURY_BALL')
a.hidden(38, 14, 'ITEM_HEART_SCALE', 'FLAG_HIDDEN_ITEM_CARNIVAL_HEART_SCALE')

a.write(map_id='MAP_SUNFAIR_CARNIVAL', layout_id='LAYOUT_SUNFAIR_CARNIVAL', music='MUS_SLATEPORT', mapsec=MAPSEC,
        mapsec_name='SUNFAIR CARNIVAL', map_type='MAP_TYPE_TOWN', after='MireboughMarsh', scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
	.byte 0

{NAME}_OnTransition:
	goto_if_set FLAG_CARNIVAL_SPINDA_FOUND, {NAME}_OnTransitionHome
	setflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	end

{NAME}_OnTransitionHome:
	setflag FLAG_HIDE_CARNIVAL_SPINDA_LOST
	clearflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	end

{NAME}_EventScript_Guide::
	lock
	faceplayer
	msgbox {NAME}_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GuideStay
	msgbox {NAME}_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_SLATEPORT_CITY, 9, 23
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

@ ---- the Ringmaster's runaway Spinda
{NAME}_EventScript_Ringmaster::
	lock
	faceplayer
	goto_if_set FLAG_CARNIVAL_EGG_DONE, {NAME}_EventScript_RingmasterDone
	goto_if_set FLAG_CARNIVAL_SPINDA_FOUND, {NAME}_EventScript_RingmasterThanks
	msgbox {NAME}_Text_RingmasterAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RingmasterThanks::
	msgbox {NAME}_Text_RingmasterThanks, MSGBOX_DEFAULT
	getpartysize
	goto_if_eq VAR_RESULT, PARTY_SIZE, {NAME}_EventScript_RingmasterFull
	giveegg SPECIES_MIME_JR
	setflag FLAG_CARNIVAL_EGG_DONE
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotEgg, MSGBOX_DEFAULT
	waitfanfare
	msgbox {NAME}_Text_RingmasterEgg, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RingmasterFull::
	msgbox {NAME}_Text_RingmasterFull, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RingmasterDone::
	msgbox {NAME}_Text_RingmasterDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_SpindaLost::
	lock
	faceplayer
	playmoncry SPECIES_SPINDA, CRY_MODE_NORMAL
	msgbox {NAME}_Text_SpindaLost, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	setflag FLAG_CARNIVAL_SPINDA_FOUND
	setflag FLAG_HIDE_CARNIVAL_SPINDA_LOST
	removeobject VAR_LAST_TALKED
	clearflag FLAG_HIDE_CARNIVAL_SPINDA_HOME
	addobject {SPINDA_HOME}
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_SpindaBack, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_SpindaHome::
	lock
	faceplayer
	playmoncry SPECIES_SPINDA, CRY_MODE_NORMAL
	msgbox {NAME}_Text_SpindaHome, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ ---- stalls
{NAME}_EventScript_FoodStall::
	lock
	faceplayer
	message {NAME}_Text_FoodStall
	waitmessage
	pokemart {NAME}_Mart_Food
	msgbox {NAME}_Text_StallBye, MSGBOX_DEFAULT
	release
	end

	.align 2
{NAME}_Mart_Food:
	.2byte ITEM_FRESH_WATER
	.2byte ITEM_SODA_POP
	.2byte ITEM_LEMONADE
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_BERRY_JUICE
	pokemartlistend

{NAME}_EventScript_SweetStall::
	lock
	faceplayer
	message {NAME}_Text_SweetStall
	waitmessage
	pokemart {NAME}_Mart_Sweets
	msgbox {NAME}_Text_StallBye, MSGBOX_DEFAULT
	release
	end

	.align 2
{NAME}_Mart_Sweets:
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_RAGE_CANDY_BAR
	.2byte ITEM_SWEET_HEART
	.2byte ITEM_CASTELIACONE
	.2byte ITEM_LUMIOSE_GALETTE
	pokemartlistend

{NAME}_EventScript_RingToss::
	lock
	faceplayer
	msgbox {NAME}_Text_RingTossAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_RingTossNo
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, {NAME}_EventScript_RingTossBroke
	removemoney 500
	updatemoneybox
	playse SE_BALL_THROW
	msgbox {NAME}_Text_RingTossThrow, MSGBOX_DEFAULT
	random 10
	switch VAR_RESULT
	case 0, {NAME}_EventScript_RingTossCandy
	case 1, {NAME}_EventScript_RingTossScale
	case 2, {NAME}_EventScript_RingTossLemonade
	msgbox {NAME}_Text_RingTossMiss, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RingTossScale::
	playfanfare MUS_LEVEL_UP
	msgbox {NAME}_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_HEART_SCALE
	release
	end

{NAME}_EventScript_RingTossLemonade::
	playfanfare MUS_LEVEL_UP
	msgbox {NAME}_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_LEMONADE
	release
	end

{NAME}_EventScript_RingTossCandy::
	playfanfare MUS_LEVEL_UP
	msgbox {NAME}_Text_RingTossWin, MSGBOX_DEFAULT
	waitfanfare
	giveitem ITEM_RARE_CANDY
	release
	end

{NAME}_EventScript_RingTossNo::
	msgbox {NAME}_Text_RingTossNo, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_RingTossBroke::
	msgbox {NAME}_Text_RingTossBroke, MSGBOX_DEFAULT
	release
	end

@ ---- visitors
{NAME}_EventScript_BalloonGirl::
	msgbox {NAME}_Text_BalloonGirl, MSGBOX_NPC
	end

{NAME}_EventScript_Pikachu::
	lock
	faceplayer
	playmoncry SPECIES_PIKACHU, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Pikachu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_MrMime::
	lock
	faceplayer
	playmoncry SPECIES_MR_MIME, CRY_MODE_NORMAL
	msgbox {NAME}_Text_MrMime, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Fortune::
	lock
	faceplayer
	msgbox {NAME}_Text_FortuneIntro, MSGBOX_DEFAULT
	random 4
	switch VAR_RESULT
	case 0, {NAME}_EventScript_Fortune0
	case 1, {NAME}_EventScript_Fortune1
	case 2, {NAME}_EventScript_Fortune2
	msgbox {NAME}_Text_Fortune3, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Fortune0::
	msgbox {NAME}_Text_Fortune0, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Fortune1::
	msgbox {NAME}_Text_Fortune1, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Fortune2::
	msgbox {NAME}_Text_Fortune2, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_OldMan::
	msgbox {NAME}_Text_OldMan, MSGBOX_NPC
	end

{NAME}_EventScript_Woman::
	msgbox {NAME}_Text_Woman, MSGBOX_NPC
	end

{NAME}_EventScript_Blaze::
	trainerbattle_single TRAINER_CARNIVAL_BLAZE, {NAME}_Text_BlazeIntro, {NAME}_Text_BlazeDefeat
	msgbox {NAME}_Text_BlazePost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_KitAndKat::
	trainerbattle_double TRAINER_CARNIVAL_KIT_AND_KAT, {NAME}_Text_KitIntro, {NAME}_Text_KitDefeat, {NAME}_Text_KitNotEnough
	msgbox {NAME}_Text_KitPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Shade::
	trainerbattle_single TRAINER_CARNIVAL_SHADE, {NAME}_Text_ShadeIntro, {NAME}_Text_ShadeDefeat
	msgbox {NAME}_Text_ShadePost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Riff::
	trainerbattle_single TRAINER_CARNIVAL_RIFF, {NAME}_Text_RiffIntro, {NAME}_Text_RiffDefeat
	msgbox {NAME}_Text_RiffPost, MSGBOX_AUTOCLOSE
	end

@ ---- text
{NAME}_Text_GuideBack:
	.string "Had your fill of fun? I can take you\n"
	.string "back to SLATEPORT.$"

{NAME}_Text_GuideLead:
	.string "All aboard!$"

{NAME}_Text_GuideStay:
	.string "The carnival's open all day and all\n"
	.string "night. Enjoy!$"

{NAME}_Text_Sign:
	.string "SUNFAIR CARNIVAL\n"
	.string "Rides, games and treats for all!$"

{NAME}_Text_RingmasterAsk:
	.string "Ladies and gentlemen… Oh, it's\n"
	.string "just you. Sorry.\p"
	.string "My star performer, SPINDA, spun so\n"
	.string "hard in rehearsal that it wobbled\l"
	.string "right out of the tent!\p"
	.string "It can't walk straight, so it can't\n"
	.string "have gone far. Could you find it?$"

{NAME}_Text_RingmasterThanks:
	.string "SPINDA! You found it! The show can\n"
	.string "go on!\p"
	.string "Here, take this as thanks. A little\n"
	.string "performer of your own.$"

{NAME}_Text_GotEgg:
	.string "{{PLAYER}} received an EGG from the\n"
	.string "RINGMASTER!$"

{NAME}_Text_RingmasterEgg:
	.string "Whatever hatches from it is a born\n"
	.string "mime. Practise your invisible walls!$"

{NAME}_Text_RingmasterFull:
	.string "Oh, your party's full. Come back\n"
	.string "when you have room for my gift.$"

{NAME}_Text_RingmasterDone:
	.string "SPINDA's dizzy dance is the best act\n"
	.string "in the whole show!$"

{NAME}_Text_SpindaLost:
	.string "SPINDA is wobbling about, very dizzy.\n"
	.string "It seems happy to see you.$"

{NAME}_Text_SpindaBack:
	.string "{{PLAYER}} led SPINDA back to the big\n"
	.string "top!$"

{NAME}_Text_SpindaHome:
	.string "SPINDA is practising its dizzy dance.$"

{NAME}_Text_FoodStall:
	.string "Thirsty? We've got the coldest\n"
	.string "drinks in the carnival!$"

{NAME}_Text_SweetStall:
	.string "Sweets! Get your sweets here!$"

{NAME}_Text_StallBye:
	.string "Enjoy the carnival!$"

{NAME}_Text_RingTossAsk:
	.string "Ring toss! Land a ring on a bottle\n"
	.string "and win a prize! Just ¥500 a throw.\l"
	.string "Want to play?$"

{NAME}_Text_RingTossThrow:
	.string "{{PLAYER}} threw the ring…$"

{NAME}_Text_RingTossWin:
	.string "It landed! We have a winner!$"

{NAME}_Text_RingTossMiss:
	.string "Ooh, so close! It bounced off.$"

{NAME}_Text_RingTossNo:
	.string "Maybe later, then!$"

{NAME}_Text_RingTossBroke:
	.string "Sorry, it's ¥500 a throw.$"

{NAME}_Text_BalloonGirl:
	.string "I got a PIKACHU balloon! And a real\n"
	.string "PIKACHU! Best day ever!$"

{NAME}_Text_Pikachu:
	.string "PIKACHU is staring at the balloons.$"

{NAME}_Text_MrMime:
	.string "MR. MIME is miming a wall. Nobody\n"
	.string "can get past it. Wait, there's no\l"
	.string "wall at all…$"

{NAME}_Text_FortuneIntro:
	.string "Welcome to my tent of mysteries.\n"
	.string "Let me gaze into the crystal ball…$"

{NAME}_Text_Fortune0:
	.string "I see… a shiny POKéMON in your\n"
	.string "future. Or a shiny coin. It's\l"
	.string "a bit blurry.$"

{NAME}_Text_Fortune1:
	.string "I see… a long journey across the\n"
	.string "sea. Bring snacks.$"

{NAME}_Text_Fortune2:
	.string "I see… a great battle, and a friend\n"
	.string "who will challenge you online.$"

{NAME}_Text_Fortune3:
	.string "I see… a SPINDA. Dizzy. Somewhere\n"
	.string "north and east of here.$"

{NAME}_Text_OldMan:
	.string "I've ridden the Ferris wheel every\n"
	.string "summer for fifty years. On a clear\l"
	.string "day you can see all of SLATEPORT.$"

{NAME}_Text_Woman:
	.string "The RINGMASTER's SPINDA ran off\n"
	.string "again. Last I saw, it was wobbling\l"
	.string "past the Ferris wheel.$"

{NAME}_Text_BlazeIntro:
	.string "Watch me breathe fire! Well, my\n"
	.string "POKéMON will!$"

{NAME}_Text_BlazeDefeat:
	.string "Fizzled out!$"

{NAME}_Text_BlazePost:
	.string "Don't try fire-breathing at home.$"

{NAME}_Text_KitIntro:
	.string "KIT: We're the carnival's double act!\n"
	.string "KAT: Two against one!$"

{NAME}_Text_KitDefeat:
	.string "KIT and KAT: We lost together!$"

{NAME}_Text_KitNotEnough:
	.string "KIT: You need two POKéMON for our\n"
	.string "double act!$"

{NAME}_Text_KitPost:
	.string "KAT: Next time we'll juggle too!$"

{NAME}_Text_ShadeIntro:
	.string "You found me behind the big top!\n"
	.string "My hiding act is ruined!$"

{NAME}_Text_ShadeDefeat:
	.string "Vanished… in defeat.$"

{NAME}_Text_ShadePost:
	.string "Ninjas love carnivals. So many\n"
	.string "places to hide.$"

{NAME}_Text_RiffIntro:
	.string "Every carnival needs a band! Let me\n"
	.string "play you a battle song!$"

{NAME}_Text_RiffDefeat:
	.string "Broke a string!$"

{NAME}_Text_RiffPost:
	.string "The crowd loved it anyway.$"
''')
print('ok', NAME)

# ---------------------------------------------------------------- the game hosts (stubs for the minigames branch)
register_script_file('carnival_games', r'''
@ Sunfair Carnival game hosts (tools/areas/carnival.py). The minigames
@ branch (claude/pastime-carnival) replaces each body with
@ goto Pastime_EventScript_<game>.
Carnival_EventScript_Bowling::
	msgbox Carnival_Text_BowlingSoon, MSGBOX_NPC
	end

Carnival_EventScript_Golf::
	msgbox Carnival_Text_GolfSoon, MSGBOX_NPC
	end

Carnival_EventScript_Soccer::
	msgbox Carnival_Text_SoccerSoon, MSGBOX_NPC
	end

Carnival_EventScript_FerrisWheel::
	msgbox Carnival_Text_FerrisWheelSoon, MSGBOX_NPC
	end

Carnival_Text_BowlingSoon:
	.string "The bowling lanes are being polished.\n"
	.string "Coming soon!$"

Carnival_Text_GolfSoon:
	.string "Mini golf opens soon! We're still\n"
	.string "mowing the green.$"

Carnival_Text_SoccerSoon:
	.string "We're picking teams for a POKéMON\n"
	.string "soccer match. Coming soon!$"

Carnival_Text_FerrisWheelSoon:
	.string "The Ferris wheel is getting its last\n"
	.string "safety check. Rides start soon!$"
''')

# ---------------------------------------------------------------- the way in, from Slateport City
add_object('SlateportCity', {'graphics_id': 'OBJ_EVENT_GFX_SAILOR', 'x': 8, 'y': 23, 'elevation': 3,
                             'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                             'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                             'script': 'SlateportCity_EventScript_CarnivalGuide', 'flag': '0'})
register_script_file('sunfair', r'''
@ Sunfair Carnival guide in Slateport City (tools/areas/carnival.py).
SlateportCity_EventScript_CarnivalGuide::
	lock
	faceplayer
	msgbox SlateportCity_Text_CarnivalGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_CarnivalGuideNo
	msgbox SlateportCity_Text_CarnivalGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SUNFAIR_CARNIVAL, 29, 46
	waitstate
	release
	end

SlateportCity_EventScript_CarnivalGuideNo::
	msgbox SlateportCity_Text_CarnivalGuideNo, MSGBOX_DEFAULT
	release
	end

SlateportCity_Text_CarnivalGuideAsk:
	.string "The SUNFAIR CARNIVAL is in town!\n"
	.string "Ferris wheel, games, sweets…\p"
	.string "My boat goes there all day. Want a\n"
	.string "ride?$"

SlateportCity_Text_CarnivalGuideGo:
	.string "Off we go!$"

SlateportCity_Text_CarnivalGuideNo:
	.string "Come along whenever you like!$"
''')
print('ok entrance')
