"""Gullwing Point: a windy cape with a lighthouse and a fishing village. A
pirate ship hides in the fog of the bay. The keeper's beam can be turned
with four mirrors on the cliff; aim it at the ship and the pirates lower
their gangplank. Beat the crew and Captain Barnacle on deck, and their
treasure is waiting in the hold.

New art (lighthouse, mirrors, beam, pirate ship, deck pieces) is drawn in
tools/areas/carnival_art.py (the carnival tileset).

    python3 tools/areas/carnival_art.py   # the tileset, when the art changes
    python3 tools/areas/gullwing.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import town

AREA_NO = 10
NAME, DECK, HOLD = 'GullwingPoint', 'GullwingPoint_PirateShip', 'GullwingPoint_PirateHold'
MAPSEC = 'MAPSEC_GULLWING_POINT'
PETAL = 'LAYOUT_PETALBURG_CITY'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'carnival_tiles.json')))
MT = {k: v for k, v in ART['objects'].items()}

register_constants(NAME, AREA_NO, flags=[
    'GULL_MIRROR_1', 'GULL_MIRROR_2', 'GULL_MIRROR_3', 'GULL_MIRROR_4', 'GULL_BEAM_SOLVED', 'GULL_KEEPER_TALKED',
    'GULL_CAPTAIN_BEATEN', 'GULL_TREASURE_TAKEN', 'ITEM_GULL_NET_BALL', 'ITEM_GULL_TM_RAIN_DANCE',
    'ITEM_GULL_SHELL_BELL', 'ITEM_HOLD_BIG_NUGGET', 'ITEM_HOLD_KINGS_ROCK', 'ITEM_HOLD_PEARL_STRING',
    'HIDDEN_ITEM_GULL_PEARL', 'HIDDEN_ITEM_GULL_HEART_SCALE', 'GULL_PELIPPER_TALKED',
], trainers=['GULL_SKERRY', 'GULL_BILGE', 'GULL_MAREE', 'GULL_CAPTAIN', 'GULL_PEG', 'GULL_FINN'])

register_trainers(NAME, '''
=== AREA_TRAINER_GULL_SKERRY ===
Name: SKERRY
Class: Sailor
Pic: Sailor
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Mankey
Level: 16

Murkrow
Level: 17

=== AREA_TRAINER_GULL_BILGE ===
Name: BILGE
Class: Sailor
Pic: Sailor
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Krabby
Level: 16

Corphish
Level: 16

Wingull
Level: 16

=== AREA_TRAINER_GULL_MAREE ===
Name: MAREE
Class: Tuber F
Pic: Tuber F
Gender: Female
Music: Girl
Double Battle: No
AI: Basic Trainer

Marill
Level: 14

Wingull
Level: 14

=== AREA_TRAINER_GULL_PEG ===
Name: PEG
Class: Sailor
Pic: Sailor
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Mankey
Level: 18

Chatot
Level: 18

=== AREA_TRAINER_GULL_FINN ===
Name: FINN
Class: Sailor
Pic: Sailor
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Carvanha
Level: 18

Krabby
Level: 19

=== AREA_TRAINER_GULL_CAPTAIN ===
Name: BARNACLE
Class: Expert
Pic: Expert M
Gender: Male
Music: Intense
Double Battle: No
AI: Smart Trainer

Corphish
Level: 20

Cramorant
Level: 20

Murkrow
Level: 21

Carvanha
Level: 22
''')


def place(a, name, x, y, ground_char='o'):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c=ground_char)


def one(name):
    return MT[name][0][0]


# ---------------------------------------------------------------- the cape
st = copy.copy(town())
st.secondary = 'gTileset_Carnival'
st.exclude = set(st.exclude) | {0x129, 0x087, 0x0C6, 0x1D4, 0x1E7, 0x0A1, 0x0B1, 0x0C0, 0x1D9, 0x1D1, 0x1E0, 0x1E1, 0x1E2}
W, H = 56, 48
a = Area(NAME, W, H, st, fill='~')
a.ellipse(20, 20, 22, 19, 'g')                     # the cape
a.ellipse(38, 12, 9, 9, 'g')                       # the lighthouse headland
a.rect(0, 0, 30, 40, 'g')
a.rect(0, 0, 4, 48, 'T')
a.rect(0, 0, 50, 3, 'T')
a.rect(0, 40, 26, 8, 'g')
a.rect(4, 44, 22, 4, 'T')
a.ellipse(8, 8, 4, 4, 'T')
a.ellipse(6, 30, 3, 6, 'T')
a.ellipse(22, 8, 3, 2, '"')
a.ellipse(14, 22, 4, 3, '"')
a.ellipse(10, 39, 4, 2, '"')
a.rect(26, 33, 1, 5, 's')                          # the pier to the pirate ship
a.rect(16, 35, 11, 2, 's')
a.line([(14, 46), (14, 30), (24, 30), (24, 12), (38, 12)], 2, 's')
a.snap_trees(ground='g')
for x, y in ((6, 12), (6, 18), (16, 13)):         # the fishing village
    a.stamp(PETAL, 9, 16, 4, 4, x, y)
LIGHTHOUSE = (40, 4)                               # 3x7; its door at (41, 10)
place(a, 'lighthouse', *LIGHTHOUSE)
a.rect(27, 30, 13, 9, '~')
SHIP = (28, 31)                                    # 11x7 on the water; its hatch at (28, 35)
place(a, 'pirate_ship', *SHIP, ground_char='~')
PLANK = (27, 35)
# the mirrors and the beam's path when they're set right
MIRRORS = [(41, 15, 'mirror_a', 'mirror_b'), (33, 15, 'mirror_a', 'mirror_b'),
           (33, 22, 'mirror_b', 'mirror_a'), (37, 22, 'mirror_b', 'mirror_a')]   # (x, y, wrong, right)
BEAM = ([(41, y, 'beam_v') for y in range(11, 15)] + [(x, 15, 'beam_h') for x in range(34, 41)] +
        [(33, y, 'beam_v') for y in range(16, 22)] + [(x, 22, 'beam_h') for x in range(34, 37)] +
        [(37, y, 'beam_v') for y in range(23, 31)])
for x, y, wrong, right in MIRRORS:
    a.tile(x, y, one(wrong), collision=1, c='o')
for x, y, _ in BEAM:
    a.put(x, y, 'g')
a.ellipse(37, 20, 6, 8.5, 'g')
a.rect(32, 14, 11, 17, 'g')
for x, y, wrong, right in MIRRORS:
    a.tile(x, y, one(wrong), collision=1, c='o')

ARRIVE = (15, 45)
a.obj('OBJ_EVENT_GFX_SAILOR', 16, 45, f'{NAME}_EventScript_Boatman', 'MOVEMENT_TYPE_FACE_LEFT')
a.sign(13, 43, f'{NAME}_EventScript_Sign', elevation=0)
a.obj('OBJ_EVENT_GFX_OLD_MAN', 42, 11, f'{NAME}_EventScript_Keeper', 'MOVEMENT_TYPE_FACE_DOWN')
a.sign(38, 13, f'{NAME}_EventScript_Plaque', elevation=0)
for i, (x, y, _, _) in enumerate(MIRRORS, 1):
    a.sign(x, y, f'{NAME}_EventScript_Mirror{i}', elevation=0)
a.obj('OBJ_EVENT_GFX_FISHERMAN', 20, 37, f'{NAME}_EventScript_Fisher', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WOMAN_3', 9, 23, f'{NAME}_EventScript_Woman', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_LITTLE_BOY', 19, 18, f'{NAME}_EventScript_Boy', 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN', ry=2)
a.obj('OBJ_EVENT_GFX_SPECIES(PELIPPER)', 30, 9, f'{NAME}_EventScript_Pelipper', 'MOVEMENT_TYPE_LOOK_AROUND')
a.obj('OBJ_EVENT_GFX_SPECIES(WINGULL)', 12, 34, f'{NAME}_EventScript_Wingull', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.trainer('OBJ_EVENT_GFX_SAILOR', 24, 26, f'{NAME}_EventScript_Skerry', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_SAILOR', 22, 35, f'{NAME}_EventScript_Bilge', 2, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_TUBER_F', 10, 28, f'{NAME}_EventScript_Maree', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.item('ITEM_NET_BALL', 5, 40, 'FLAG_ITEM_GULL_NET_BALL')
a.item('ITEM_TM_RAIN_DANCE', 46, 7, 'FLAG_ITEM_GULL_TM_RAIN_DANCE')
a.item('ITEM_SHELL_BELL', 27, 4, 'FLAG_ITEM_GULL_SHELL_BELL')
a.hidden(28, 22, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_GULL_PEARL')
a.hidden(8, 36, 'ITEM_HEART_SCALE', 'FLAG_HIDDEN_ITEM_GULL_HEART_SCALE')
a.warp(SHIP[0], SHIP[1] + 4, 'MAP_GULLWING_POINT_PIRATE_SHIP', 0)


def mirror_script(i, x, y, wrong, right):
    return rf'''
{NAME}_EventScript_Mirror{i}::
	lockall
	goto_if_set FLAG_GULL_BEAM_SOLVED, {NAME}_EventScript_MirrorDone
	msgbox {NAME}_Text_MirrorTurn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_MirrorLeave
	playse SE_CLICK
	goto_if_set FLAG_GULL_MIRROR_{i}, {NAME}_EventScript_Mirror{i}Back
	setflag FLAG_GULL_MIRROR_{i}
	setmetatile {x}, {y}, {one(right)}, TRUE
	special DrawWholeMapView
	goto {NAME}_EventScript_CheckBeam

{NAME}_EventScript_Mirror{i}Back::
	clearflag FLAG_GULL_MIRROR_{i}
	setmetatile {x}, {y}, {one(wrong)}, TRUE
	special DrawWholeMapView
	releaseall
	end
'''


mirrors = ''.join(mirror_script(i, *m) for i, m in enumerate(MIRRORS, 1))
on_load_mirrors = ''.join(f'\tcall_if_set FLAG_GULL_MIRROR_{i}, {NAME}_OnLoadMirror{i}\n' for i in range(1, 5))
on_load_subs = ''.join(f'{NAME}_OnLoadMirror{i}:\n\tsetmetatile {x}, {y}, {one(r)}, TRUE\n\treturn\n\n'
                       for i, (x, y, w, r) in enumerate(MIRRORS, 1))
beam_lines = ''.join(f'\tsetmetatile {x}, {y}, {one(b)}, FALSE\n' for x, y, b in BEAM)
plank_line = f'\tsetmetatile {PLANK[0]}, {PLANK[1]}, {one("gangplank")}, FALSE\n'

a.write(map_id='MAP_GULLWING_POINT', layout_id='LAYOUT_GULLWING_POINT', music='MUS_ROUTE104', mapsec=MAPSEC,
        mapsec_name='GULLWING POINT', after='Abyssia_Temple', scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	.byte 0

{NAME}_OnLoad:
{on_load_mirrors}	call_if_set FLAG_GULL_BEAM_SOLVED, {NAME}_ShowBeam
	end

{on_load_subs}{NAME}_ShowBeam:
{beam_lines}{plank_line}	return

{NAME}_EventScript_Boatman::
	lock
	faceplayer
	msgbox {NAME}_Text_BoatmanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_BoatmanStay
	msgbox {NAME}_Text_BoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_DEWFORD_TOWN, 5, 12
	waitstate
	release
	end

{NAME}_EventScript_BoatmanStay::
	msgbox {NAME}_Text_BoatmanStay, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Sign::
	msgbox {NAME}_Text_Sign, MSGBOX_SIGN
	end

{NAME}_EventScript_Plaque::
	msgbox {NAME}_Text_Plaque, MSGBOX_SIGN
	end

@ ---- the keeper and the mirror puzzle
{NAME}_EventScript_Keeper::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, {NAME}_EventScript_KeeperThanks
	goto_if_set FLAG_GULL_BEAM_SOLVED, {NAME}_EventScript_KeeperGo
	setflag FLAG_GULL_KEEPER_TALKED
	msgbox {NAME}_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_KeeperGo::
	msgbox {NAME}_Text_KeeperGo, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_KeeperThanks::
	msgbox {NAME}_Text_KeeperThanks, MSGBOX_DEFAULT
	release
	end
{mirrors}
{NAME}_EventScript_MirrorDone::
	msgbox {NAME}_Text_MirrorDone, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_MirrorLeave::
	releaseall
	end

{NAME}_EventScript_CheckBeam::
	goto_if_unset FLAG_GULL_MIRROR_1, {NAME}_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_2, {NAME}_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_3, {NAME}_EventScript_BeamNo
	goto_if_unset FLAG_GULL_MIRROR_4, {NAME}_EventScript_BeamNo
	setflag FLAG_GULL_BEAM_SOLVED
	playse SE_M_REFLECT
	call {NAME}_ShowBeam
	special DrawWholeMapView
	waitse
	msgbox {NAME}_Text_BeamHits, MSGBOX_DEFAULT
	playse SE_M_SURF
	waitse
	msgbox {NAME}_Text_Gangplank, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_BeamNo::
	releaseall
	end

@ ---- people and Pokémon
{NAME}_EventScript_Fisher::
	msgbox {NAME}_Text_Fisher, MSGBOX_NPC
	end

{NAME}_EventScript_Woman::
	msgbox {NAME}_Text_Woman, MSGBOX_NPC
	end

{NAME}_EventScript_Boy::
	msgbox {NAME}_Text_Boy, MSGBOX_NPC
	end

{NAME}_EventScript_Pelipper::
	lock
	faceplayer
	playmoncry SPECIES_PELIPPER, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Pelipper, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Wingull::
	lock
	faceplayer
	playmoncry SPECIES_WINGULL, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Wingull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{NAME}_EventScript_Skerry::
	trainerbattle_single TRAINER_GULL_SKERRY, {NAME}_Text_SkerryIntro, {NAME}_Text_SkerryDefeat
	msgbox {NAME}_Text_SkerryPost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Bilge::
	trainerbattle_single TRAINER_GULL_BILGE, {NAME}_Text_BilgeIntro, {NAME}_Text_BilgeDefeat
	msgbox {NAME}_Text_BilgePost, MSGBOX_AUTOCLOSE
	end

{NAME}_EventScript_Maree::
	trainerbattle_single TRAINER_GULL_MAREE, {NAME}_Text_MareeIntro, {NAME}_Text_MareeDefeat
	msgbox {NAME}_Text_MareePost, MSGBOX_AUTOCLOSE
	end

@ ---- text
{NAME}_Text_BoatmanBack:
	.string "Shall I row you back to DEWFORD?$"

{NAME}_Text_BoatmanGo:
	.string "Hold on to your hat!$"

{NAME}_Text_BoatmanStay:
	.string "Mind the pirates. They're not as\n"
	.string "friendly as the WINGULL.$"

{NAME}_Text_Sign:
	.string "GULLWING POINT\n"
	.string "Where the sea wind never rests.$"

{NAME}_Text_Plaque:
	.string "“The keeper's beam runs south from\n"
	.string "the lamp. Each mirror turns it to\l"
	.string "the left or to the right.\p"
	.string "Set them well and the light will\n"
	.string "reach any ship in the bay.”$"

{NAME}_Text_KeeperAsk:
	.string "Ah, a traveller. You've seen that\n"
	.string "ship in the bay? Pirates!\p"
	.string "They hide in the fog and slip out at\n"
	.string "night to rob the fishing boats.\p"
	.string "My beam could catch them red-handed,\n"
	.string "but the mirrors on the cliff are all\l"
	.string "turned the wrong way.\p"
	.string "Turn them so the beam lands on that\n"
	.string "ship, and the whole coast will see\l"
	.string "them!$"

{NAME}_Text_KeeperGo:
	.string "You did it! They've lowered their\n"
	.string "gangplank. They want a fight!\p"
	.string "Go and show them GULLWING POINT\n"
	.string "won't be pushed around.$"

{NAME}_Text_KeeperThanks:
	.string "The bay is quiet again, thanks to\n"
	.string "you. My lamp will shine for you\l"
	.string "every night.$"

{NAME}_Text_MirrorTurn:
	.string "A big mirror on a stone base. It\n"
	.string "can be turned. Turn it?$"

{NAME}_Text_MirrorDone:
	.string "The mirror is sending the beam on\n"
	.string "its way.$"

{NAME}_Text_BeamHits:
	.string "The lighthouse beam bounced from\n"
	.string "mirror to mirror… and lit up the\l"
	.string "pirate ship!$"

{NAME}_Text_Gangplank:
	.string "Someone on the ship shouted, and a\n"
	.string "gangplank thudded onto the pier!$"

{NAME}_Text_Fisher:
	.string "Those pirates took my best rod. If\n"
	.string "you go aboard, give them a thump\l"
	.string "for me.$"

{NAME}_Text_Woman:
	.string "We hang our washing out on the cape.\n"
	.string "The wind dries it in minutes!$"

{NAME}_Text_Boy:
	.string "The lighthouse keeper's mirrors are\n"
	.string "like a puzzle. I can never solve it.$"

{NAME}_Text_Pelipper:
	.string "PELIPPER is carrying a letter in its\n"
	.string "beak. It won't let go of it.$"

{NAME}_Text_Wingull:
	.string "WINGULL rides the wind without\n"
	.string "flapping its wings.$"

{NAME}_Text_SkerryIntro:
	.string "Arr! Ye shouldn't be snooping round\n"
	.string "the mirrors, landlubber!$"

{NAME}_Text_SkerryDefeat:
	.string "Shiver me timbers!$"

{NAME}_Text_SkerryPost:
	.string "The CAPTAIN won't be happy…$"

{NAME}_Text_BilgeIntro:
	.string "Nobody walks this pier without\n"
	.string "paying the toll! The toll is a\l"
	.string "battle!$"

{NAME}_Text_BilgeDefeat:
	.string "Toll paid… by me.$"

{NAME}_Text_BilgePost:
	.string "Our ship only shows itself when the\n"
	.string "fog lifts. Or when someone shines a\l"
	.string "light on it.$"

{NAME}_Text_MareeIntro:
	.string "I'm going to be a pirate when I grow\n"
	.string "up! Practice battle!$"

{NAME}_Text_MareeDefeat:
	.string "Walked the plank!$"

{NAME}_Text_MareePost:
	.string "Maybe I'll be a lighthouse keeper\n"
	.string "instead.$"
''')
register_wild('MAP_GULLWING_POINT', 'gGullwingPoint',
              land=[('WINGULL', 13, 15), ('ZIGZAGOON', 13, 15), ('MURKROW', 13, 15), ('MANKEY', 13, 15),
                    ('WINGULL', 14, 16), ('KRABBY', 14, 16), ('CHATOT', 15, 17), ('ZIGZAGOON', 15, 17),
                    ('MANKEY', 16, 17), ('CORPHISH', 15, 17), ('CRAMORANT', 17, 18), ('MURKROW', 17, 18)],
              water=[('TENTACOOL', 15, 25), ('WINGULL', 15, 25), ('PELIPPER', 25, 30), ('WAILMER', 20, 30), ('CARVANHA', 20, 30)],
              fish=[('MAGIKARP', 5, 10), ('TENTACOOL', 5, 10), ('KRABBY', 10, 20), ('CORPHISH', 10, 20), ('CARVANHA', 10, 20),
                    ('KRABBY', 20, 25), ('CORPHISH', 20, 25), ('CARVANHA', 20, 25), ('GYARADOS', 25, 30), ('SHARPEDO', 25, 30)])
print('ok', NAME)

# ---------------------------------------------------------------- the pirate ship's deck
d = Area(DECK, 22, 14, st, fill='~')
DW, DH = 22, 14
for y in range(2, 12):
    for x in range(2, 20):
        d.tile(x, y, one('deck'), collision=0, c='o')
for x in range(2, 20):
    d.tile(x, 2, one('rail_top'), collision=1, c='o')
    d.tile(x, 11, one('rail_bottom'), collision=1, c='o')
for y in range(3, 11):
    d.tile(2, y, one('rail_left'), collision=1, c='o')
    d.tile(19, y, one('rail_right'), collision=1, c='o')
GANG = (2, 7)
d.tile(*GANG, one('gangplank'), collision=0, c='o')
d.tile(1, 7, one('gangplank_exit'), collision=0, c='o')
place(d, 'mast', 7, 3)
place(d, 'mast', 13, 3)
place(d, 'helm', 17, 4)
place(d, 'hatch', 9, 8)                            # its door row is (9..10, 9)
for x in (4, 15):
    place(d, 'cannon', x, 10 - 1)
for x, y in ((3, 3), (4, 3), (18, 9), (3, 10)):
    place(d, 'barrel', x, y)
place(d, 'chest', 18, 3)
d.warp(1, 7, 'MAP_GULLWING_POINT', 0)              # back down the gangplank
d.warp(9, 9, 'MAP_GULLWING_POINT_PIRATE_HOLD', 0)
d.warp(10, 9, 'MAP_GULLWING_POINT_PIRATE_HOLD', 0)
d.trainer('OBJ_EVENT_GFX_SAILOR', 6, 6, f'{DECK}_EventScript_Peg', 2, 'MOVEMENT_TYPE_FACE_LEFT')
d.trainer('OBJ_EVENT_GFX_SAILOR', 12, 9, f'{DECK}_EventScript_Finn', 2, 'MOVEMENT_TYPE_FACE_UP')
d.obj('OBJ_EVENT_GFX_EXPERT_M', 16, 5, f'{DECK}_EventScript_Captain', 'MOVEMENT_TYPE_FACE_LEFT')
d.obj('OBJ_EVENT_GFX_SPECIES(MURKROW)', 18, 6, f'{DECK}_EventScript_Murkrow', 'MOVEMENT_TYPE_FACE_LEFT')
d.write(map_id='MAP_GULLWING_POINT_PIRATE_SHIP', layout_id='LAYOUT_GULLWING_POINT_PIRATE_SHIP', music='MUS_ABANDONED_SHIP',
        mapsec=MAPSEC, mapsec_name='GULLWING POINT', after=NAME, show_name=False, allow_cycling=False, scripts=rf'''
{DECK}_MapScripts::
	.byte 0

{DECK}_EventScript_Captain::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, {DECK}_EventScript_CaptainAfter
	msgbox {DECK}_Text_CaptainIntro, MSGBOX_DEFAULT
	trainerbattle_no_intro TRAINER_GULL_CAPTAIN, {DECK}_Text_CaptainDefeat
	setflag FLAG_GULL_CAPTAIN_BEATEN
	msgbox {DECK}_Text_CaptainGives, MSGBOX_DEFAULT
	release
	end

{DECK}_EventScript_CaptainAfter::
	msgbox {DECK}_Text_CaptainAfter, MSGBOX_DEFAULT
	release
	end

{DECK}_EventScript_Murkrow::
	lock
	faceplayer
	playmoncry SPECIES_MURKROW, CRY_MODE_NORMAL
	msgbox {DECK}_Text_Murkrow, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{DECK}_EventScript_Peg::
	trainerbattle_single TRAINER_GULL_PEG, {DECK}_Text_PegIntro, {DECK}_Text_PegDefeat
	msgbox {DECK}_Text_PegPost, MSGBOX_AUTOCLOSE
	end

{DECK}_EventScript_Finn::
	trainerbattle_single TRAINER_GULL_FINN, {DECK}_Text_FinnIntro, {DECK}_Text_FinnDefeat
	msgbox {DECK}_Text_FinnPost, MSGBOX_AUTOCLOSE
	end

{DECK}_Text_CaptainIntro:
	.string "So YOU'RE the one who lit up my ship\n"
	.string "for the whole coast to see!\p"
	.string "I'm CAPTAIN BARNACLE, terror of the\n"
	.string "seven routes! Prepare to be boarded!$"

{DECK}_Text_CaptainDefeat:
	.string "Blast and barnacles! Beaten by a\n"
	.string "child with a flashlight!$"

{DECK}_Text_CaptainGives:
	.string "A deal's a deal. The treasure in the\n"
	.string "hold is yours, and we'll give the\l"
	.string "fishermen their things back.\p"
	.string "Maybe it's time we became honest\n"
	.string "sailors. Maybe.$"

{DECK}_Text_CaptainAfter:
	.string "Honest sailing is hard work. I miss\n"
	.string "the fog already.$"

{DECK}_Text_Murkrow:
	.string "MURKROW squawks: “Treasure! Treasure!”$"

{DECK}_Text_PegIntro:
	.string "Arr! Ye came up the gangplank? Then\n"
	.string "ye'll go down it in defeat!$"

{DECK}_Text_PegDefeat:
	.string "I'm the one going down…$"

{DECK}_Text_PegPost:
	.string "The CAPTAIN's at the helm. Good\n"
	.string "luck, ye'll need it.$"

{DECK}_Text_FinnIntro:
	.string "Stay away from the hatch! That's\n"
	.string "where the loot is!$"

{DECK}_Text_FinnDefeat:
	.string "Oops. I said too much.$"

{DECK}_Text_FinnPost:
	.string "Only the CAPTAIN can say who gets\n"
	.string "the treasure.$"
''')
print('ok', DECK)

# ---------------------------------------------------------------- the hold (the S.S. Tidal's cargo deck)
interior(HOLD, 'MAP_GULLWING_POINT_PIRATE_HOLD', 'LAYOUT_SS_TIDAL_LOWER_DECK', [(15, 2)], 'MAP_GULLWING_POINT_PIRATE_SHIP', 1,
         mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes', music='MUS_ABANDONED_SHIP',
         objects=[npc('OBJ_EVENT_GFX_ITEM_BALL', 1, 9, f'{HOLD}_EventScript_BigNugget', flag='FLAG_ITEM_HOLD_BIG_NUGGET'),
                  npc('OBJ_EVENT_GFX_ITEM_BALL', 9, 9, f'{HOLD}_EventScript_KingsRock', flag='FLAG_ITEM_HOLD_KINGS_ROCK'),
                  npc('OBJ_EVENT_GFX_ITEM_BALL', 15, 12, f'{HOLD}_EventScript_PearlString', flag='FLAG_ITEM_HOLD_PEARL_STRING'),
                  npc('OBJ_EVENT_GFX_SAILOR', 7, 4, f'{HOLD}_EventScript_Guard')],
         scripts=rf'''
{HOLD}_MapScripts::
	.byte 0

{HOLD}_EventScript_Guard::
	lock
	faceplayer
	goto_if_set FLAG_GULL_CAPTAIN_BEATEN, {HOLD}_EventScript_GuardOk
	msgbox {HOLD}_Text_GuardNo, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, {HOLD}_Movement_Out
	waitmovement 0
	warp MAP_GULLWING_POINT_PIRATE_SHIP, 9, 10
	waitstate
	release
	end

{HOLD}_EventScript_GuardOk::
	msgbox {HOLD}_Text_GuardOk, MSGBOX_DEFAULT
	release
	end

{HOLD}_Movement_Out:
	walk_up
	step_end

{HOLD}_EventScript_BigNugget::
	finditem ITEM_BIG_NUGGET
	end

{HOLD}_EventScript_KingsRock::
	finditem ITEM_KINGS_ROCK
	end

{HOLD}_EventScript_PearlString::
	finditem ITEM_PEARL_STRING
	end

{HOLD}_Text_GuardNo:
	.string "Oi! Nobody touches the loot without\n"
	.string "the CAPTAIN's say-so! Out!$"

{HOLD}_Text_GuardOk:
	.string "The CAPTAIN says it's yours. Take it\n"
	.string "before I change my mind.$"
''')
print('ok', HOLD)

# ---------------------------------------------------------------- the way there, from Dewford Town
add_object('DewfordTown', {'graphics_id': 'OBJ_EVENT_GFX_SAILOR', 'x': 6, 'y': 12, 'elevation': 3,
                           'movement_type': 'MOVEMENT_TYPE_FACE_LEFT', 'movement_range_x': 0, 'movement_range_y': 0,
                           'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                           'script': 'DewfordTown_EventScript_GullwingBoatman', 'flag': '0'})
register_script_file('gullwing', r'''
@ Gullwing Point boatman in Dewford Town (tools/areas/gullwing.py).
DewfordTown_EventScript_GullwingBoatman::
	lock
	faceplayer
	msgbox DewfordTown_Text_GullwingBoatmanAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_GullwingBoatmanNo
	msgbox DewfordTown_Text_GullwingBoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_GULLWING_POINT, 15, 45
	waitstate
	release
	end

DewfordTown_EventScript_GullwingBoatmanNo::
	msgbox DewfordTown_Text_GullwingBoatmanNo, MSGBOX_DEFAULT
	release
	end

DewfordTown_Text_GullwingBoatmanAsk:
	.string "I row folk out to GULLWING POINT.\n"
	.string "There's a lighthouse, a fishing\l"
	.string "village… and pirates, they say.\p"
	.string "Want to go?$"

DewfordTown_Text_GullwingBoatmanGo:
	.string "Heave ho!$"

DewfordTown_Text_GullwingBoatmanNo:
	.string "The boat's here when you want it.$"
''')
print('ok entrance')
