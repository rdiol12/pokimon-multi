"""The Hoenn Metro, for after the third badge: an underground line joining
Mauville, Slateport and Verdanturf, with a train you ride between the
three stations, and the closed old line behind a shutter at Mauville.

Stamp rally: each station has a stamp stand. Stamp all three and the
station master in Mauville opens the old line, a disused tunnel of
maintenance rooms and a rusted train, ending at the forgotten Ghost
Station, where the fourth and last stamp waits.

The halls are learned from New Mauville (styles.plant()); the track, the
trains, the platform edge, turnstiles, ticket machines, benches, station
boards, stamp stands, stairs, the shutter and the tunnel mouth are drawn
in tools/areas/subway_art.py (gTileset_Subway).

    python3 tools/areas/subway_art.py   # the tileset, when the art changes
    python3 tools/areas/metro.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import plant

AREA_NO = 16
MAPSEC = 'MAPSEC_HOENN_METRO'
OLD = 'MetroOldLine'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'subway_tiles.json')))
MT = ART['objects']
FLOOR = 0x210

register_constants('MetroMauville', AREA_NO, flags=[
    'METRO_STAMP_1', 'METRO_STAMP_2', 'METRO_STAMP_3', 'METRO_STAMP_4', 'METRO_OLD_LINE_OPEN',
    'METRO_RALLY_DONE', 'METRO_GHOST_SEEN', 'ITEM_METRO_HYPER_POTION', 'ITEM_METRO_MAGNET',
    'ITEM_METRO_METAL_COAT', 'ITEM_METRO_SPELL_TAG', 'ITEM_METRO_TM_THUNDERBOLT', 'HIDDEN_ITEM_METRO_NUGGET',
    'HIDDEN_ITEM_METRO_RARE_CANDY', 'HIDDEN_ITEM_METRO_PP_UP', 'HIDDEN_ITEM_METRO_STARDUST',
], trainers=['METRO_STRUM', 'METRO_DIGBY', 'METRO_GEARS', 'METRO_ODETTE'])

register_trainers('MetroMauville', '''
=== AREA_TRAINER_METRO_STRUM ===
Name: STRUM
Class: Guitarist
Pic: Guitarist
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Toxel
Level: 23

Electrike
Level: 24

=== AREA_TRAINER_METRO_DIGBY ===
Name: DIGBY
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Drilbur
Level: 25

Boldore
Level: 25

=== AREA_TRAINER_METRO_GEARS ===
Name: GEARS
Class: Pokemaniac
Pic: Pokemaniac
Gender: Male
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Klink
Level: 25

Magneton
Level: 26

=== AREA_TRAINER_METRO_ODETTE ===
Name: ODETTE
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Litwick
Level: 25

Haunter
Level: 26

Sinistea
Level: 27
''')


def place(a, name, x, y):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


def one(name):
    return MT[name][0][0]


st = copy.copy(plant())
st.secondary = 'gTileset_Subway'               # New Mauville's tiles plus the art, same metatile ids

EDGE_Y, TRACK_Y = 13, 14                       # the platform edge, then three rows of track
TRAIN_X = 9

STATIONS = {   # name: (map, width, board, stamp no, city map, city attendant cell)
    'Mauville': ('MetroMauville', 36, 'board_green', 1, 'MauvilleCity', (25, 6)),
    'Slateport': ('MetroSlateport', 30, 'board_blue', 2, 'SlateportCity', (21, 20)),
    'Verdanturf': ('MetroVerdanturf', 30, 'board_orange', 3, 'VerdanturfTown', (17, 5)),
}
CITY_IDS = {'MauvilleCity': 'MAP_MAUVILLE_CITY', 'SlateportCity': 'MAP_SLATEPORT_CITY',
            'VerdanturfTown': 'MAP_VERDANTURF_TOWN'}


def map_id(name):
    import re
    return 'MAP_' + re.sub(r'(?<!^)(?=[A-Z])', '_', name).upper()


def station(key, extra_objects, extra_scripts, shutter=False):
    name, W, board, stamp, city, (cx, cy) = STATIONS[key]
    H = 18
    a = Area(name, W, H, st, fill='#')
    a.rect(1, 2, W - 2, 11, '.')
    for x in range(W):                                   # the track runs on through the tunnel walls
        for y in range(TRACK_Y, TRACK_Y + 3):
            a.tile(x, y, one('track'), collision=1, c='o')
        a.tile(x, TRACK_Y + 3, one('track_wall'), collision=1, c='o')
    for x in range(1, W - 1):
        a.tile(x, EDGE_Y, one('edge'), collision=0, c='o')
    place(a, 'train', TRAIN_X, TRACK_Y)
    place(a, 'stairs', 3, 0)
    place(a, 'ticket_machine', 7, 2)
    place(a, 'ticket_machine', 8, 2)
    place(a, board, 12, 2)
    for x in range(1, W - 1):                            # the turnstiles, with two ways through
        if x not in (10, 11, W - 12, W - 11):
            a.tile(x, 7, one('turnstile'), collision=1, c='o')
    for x in (6, W - 7):
        place(a, 'pillar', x, 9)
    place(a, 'bench', 13, 11)
    place(a, 'bench', W - 15, 11)
    sx = W // 2
    place(a, 'stamp_stand', sx, 8)
    if shutter:
        place(a, 'shutter', W - 2, 9)
    # the city above: an arrival point beside the attendant
    back = add_warp(city, {'x': cx, 'y': cy + 1, 'elevation': 0, 'dest_map': map_id(name), 'dest_warp_id': '0'})
    a.warp(3, 1, CITY_IDS[city], back)
    a.warp(4, 1, CITY_IDS[city], back)
    for x in range(TRAIN_X, TRAIN_X + 10):
        a.sign(x, TRACK_Y, f'{name}_EventScript_Train', 'BG_EVENT_PLAYER_FACING_SOUTH')
    a.sign(sx, 9, f'{name}_EventScript_Stamp', 'BG_EVENT_PLAYER_FACING_NORTH')
    a.sign(12, 3, f'{name}_EventScript_Board')
    a.sign(13, 3, f'{name}_EventScript_Board')
    for o in extra_objects:
        a.objects.append(o)
    others = [k for k in STATIONS if k != key]
    ride = rf'''
{name}_EventScript_Train::
	lockall
	message Metro_Text_WhereTo
	waitmessage
''' + ''.join(f'\tdynmultipush Metro_Text_{o}, {i}\n' for i, o in enumerate(others)) + rf'''	dynmultipush Metro_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, NULL
	closemessage
''' + ''.join(f'\tgoto_if_eq VAR_RESULT, {i}, Metro_EventScript_RideTo{o}\n' for i, o in enumerate(others)) + rf'''	releaseall
	end

{name}_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_{stamp}, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_{stamp}
	playse SE_BANG
	msgbox {name}_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

{name}_EventScript_Board::
	msgbox {name}_Text_Board, MSGBOX_SIGN
	end

{name}_Text_Stamp:
	.string "{{PLAYER}} stamped the rally card!\n"
	.string "It's the {key.upper()} STATION stamp.$"

{name}_Text_Board:
	.string "HOENN METRO\n"
	.string "{key.upper()} STATION$"
'''
    return a, ride + extra_scripts


def write_station(a, scripts, onload=''):
    a.write(map_id=map_id(a.name), layout_id='LAYOUT_' + map_id(a.name)[4:], music='MUS_RG_SILPH',
            mapsec=MAPSEC, mapsec_name='HOENN METRO', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False,
            after='MtFrostcrown', scripts=f'{a.name}_MapScripts::\n' + (
                f'\tmap_script MAP_SCRIPT_ON_LOAD, {a.name}_OnLoad\n' if onload else '') + '\t.byte 0\n'
            + onload + scripts)
    print('ok', a.name)


# ---------------------------------------------------------------- Mauville Central, with the old line's shutter
M = 'MetroMauville'
mw = STATIONS['Mauville'][1]
SHUTTER = [(mw - 2, 9), (mw - 1, 9), (mw - 2, 10), (mw - 1, 10)]
a, s = station('Mauville', [
    npc('OBJ_EVENT_GFX_WORKER_M', 28, 4, f'{M}_EventScript_Master', 'MOVEMENT_TYPE_FACE_DOWN'),
    npc('OBJ_EVENT_GFX_WOMAN_5', 20, 10, f'{M}_EventScript_Commuter', 'MOVEMENT_TYPE_WANDER_AROUND'),
    npc('OBJ_EVENT_GFX_SCHOOL_KID_M', 25, 4, f'{M}_EventScript_Kid', 'MOVEMENT_TYPE_FACE_LEFT'),
], rf'''
{M}_EventScript_Master::
	lock
	faceplayer
	goto_if_set FLAG_METRO_RALLY_DONE, {M}_EventScript_MasterDone
	goto_if_set FLAG_METRO_STAMP_4, {M}_EventScript_MasterReward
	goto_if_set FLAG_METRO_OLD_LINE_OPEN, {M}_EventScript_MasterOldLine
	goto_if_unset FLAG_METRO_STAMP_1, {M}_EventScript_MasterAsk
	goto_if_unset FLAG_METRO_STAMP_2, {M}_EventScript_MasterAsk
	goto_if_unset FLAG_METRO_STAMP_3, {M}_EventScript_MasterAsk
	msgbox {M}_Text_MasterThree, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_METRO_OLD_LINE_OPEN
	playse SE_TRUCK_DOOR
	call {M}_EventScript_OpenShutter
	special DrawWholeMapView
	waitse
	msgbox {M}_Text_MasterOldLine, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_MasterAsk::
	msgbox {M}_Text_MasterAsk, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_MasterOldLine::
	msgbox {M}_Text_MasterOldLine, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_MasterReward::
	msgbox {M}_Text_MasterFour, MSGBOX_DEFAULT
	givemon SPECIES_KLINK, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {M}_EventScript_MasterFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {M}_Text_GotKlink, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_METRO_RALLY_DONE
	msgbox {M}_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_MasterFull::
	msgbox {M}_Text_MasterFull, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_MasterDone::
	msgbox {M}_Text_MasterDone, MSGBOX_DEFAULT
	release
	end

{M}_EventScript_Commuter::
	msgbox {M}_Text_Commuter, MSGBOX_NPC
	end

{M}_EventScript_Kid::
	msgbox {M}_Text_Kid, MSGBOX_NPC
	end

{M}_Text_MasterAsk:
	.string "I'm the station master here at\n"
	.string "MAUVILLE CENTRAL.\p"
	.string "Fancy a stamp rally? Every station\n"
	.string "has a stamp stand on its platform:\l"
	.string "MAUVILLE, SLATEPORT and VERDANTURF.\p"
	.string "Fill your card and come and see me.\n"
	.string "There's something special in it.$"

{M}_Text_MasterThree:
	.string "All three stamps! Well done.\p"
	.string "Here's the special part. There used\n"
	.string "to be a fourth station, on the old\l"
	.string "line. It closed long ago.\p"
	.string "Its stamp stand is still down there.\n"
	.string "I'll open the shutter for you.$"

{M}_Text_MasterOldLine:
	.string "The old line runs east, past the\n"
	.string "shutter. Watch your step, and…\l"
	.string "they say the last station isn't\l"
	.string "as empty as it should be.$"

{M}_Text_MasterFour:
	.string "The GHOST STATION's stamp! I\n"
	.string "haven't seen that one in years.\p"
	.string "A full card deserves a prize. This\n"
	.string "KLINK lives in the signal box. It\l"
	.string "wants to see the world.$"

{M}_Text_GotKlink:
	.string "{{PLAYER}} received a KLINK!$"

{M}_Text_MasterFull:
	.string "Your party's full. Come back when\n"
	.string "you have room for KLINK.$"

{M}_Text_MasterDone:
	.string "Thanks for riding the HOENN METRO!$"

{M}_Text_Commuter:
	.string "The METRO gets me to SLATEPORT\n"
	.string "before my coffee's cold.$"

{M}_Text_Kid:
	.string "I've got two stamps! I just need\n"
	.string "VERDANTURF.$"
''', shutter=True)
a.warp(mw - 1, 9, 'MAP_METRO_OLD_LINE', 0)
a.warp(mw - 1, 10, 'MAP_METRO_OLD_LINE', 1)
a.hidden(1, 12, 'ITEM_STARDUST', 'FLAG_HIDDEN_ITEM_METRO_STARDUST')
write_station(a, s, onload=rf'''
{M}_OnLoad:
	call_if_set FLAG_METRO_OLD_LINE_OPEN, {M}_EventScript_OpenShutter
	end

{M}_EventScript_OpenShutter::
''' + ''.join(f'\tsetmetatile {x}, {y}, {one("tunnel_e")}, FALSE\n' for x, y in SHUTTER) + '\treturn\n')

# ---------------------------------------------------------------- Slateport and Verdanturf
S = 'MetroSlateport'
a, s = station('Slateport', [
    npc('OBJ_EVENT_GFX_SAILOR', 20, 4, f'{S}_EventScript_Sailor', 'MOVEMENT_TYPE_FACE_DOWN'),
    npc('OBJ_EVENT_GFX_WOMAN_2', 8, 10, f'{S}_EventScript_Woman', 'MOVEMENT_TYPE_WANDER_AROUND'),
], rf'''
{S}_EventScript_Sailor::
	msgbox {S}_Text_Sailor, MSGBOX_NPC
	end

{S}_EventScript_Woman::
	msgbox {S}_Text_Woman, MSGBOX_NPC
	end

{S}_EventScript_Strum::
	trainerbattle_single TRAINER_METRO_STRUM, {S}_Text_StrumIntro, {S}_Text_StrumDefeat
	msgbox {S}_Text_StrumPost, MSGBOX_AUTOCLOSE
	end

{S}_Text_Sailor:
	.string "Ten minutes from the harbour to\n"
	.string "MAUVILLE. The sea was never this\l"
	.string "quick!$"

{S}_Text_Woman:
	.string "The stamp stand's by the platform.\n"
	.string "The kids love it.$"

{S}_Text_StrumIntro:
	.string "The acoustics down here are\n"
	.string "electric! Battle me to the beat!$"

{S}_Text_StrumDefeat:
	.string "You broke a string…$"

{S}_Text_StrumPost:
	.string "Busking pays better on the METRO.\n"
	.string "Everyone's waiting for a train.$"
''')
a.trainer('OBJ_EVENT_GFX_MAN_5', 22, 11, f'{S}_EventScript_Strum', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.hidden(28, 12, 'ITEM_PP_UP', 'FLAG_HIDDEN_ITEM_METRO_PP_UP')
write_station(a, s)

V = 'MetroVerdanturf'
a, s = station('Verdanturf', [
    npc('OBJ_EVENT_GFX_OLD_WOMAN', 20, 4, f'{V}_EventScript_Granny', 'MOVEMENT_TYPE_FACE_DOWN'),
    npc('OBJ_EVENT_GFX_SPECIES(SKITTY)', 9, 11, f'{V}_EventScript_Skitty', 'MOVEMENT_TYPE_WANDER_AROUND'),
], rf'''
{V}_EventScript_Granny::
	msgbox {V}_Text_Granny, MSGBOX_NPC
	end

{V}_EventScript_Skitty::
	lock
	faceplayer
	playmoncry SPECIES_SKITTY, CRY_MODE_NORMAL
	msgbox {V}_Text_Skitty, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{V}_Text_Granny:
	.string "The air's so clean in VERDANTURF,\n"
	.string "they built the station deep so the\l"
	.string "trains wouldn't spoil it.$"

{V}_Text_Skitty:
	.string "SKITTY is chasing its tail around\n"
	.string "the bench.$"
''')
write_station(a, s)

register_script_file('metro', r'''
@ Hoenn Metro: riding the train, and the stamp rally (tools/areas/metro.py).
Metro_EventScript_RideToMauville::
	call Metro_EventScript_Depart
	warp MAP_METRO_MAUVILLE, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_RideToSlateport::
	call Metro_EventScript_Depart
	warp MAP_METRO_SLATEPORT, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_RideToVerdanturf::
	call Metro_EventScript_Depart
	warp MAP_METRO_VERDANTURF, 13, 12
	waitstate
	releaseall
	end

Metro_EventScript_Depart::
	msgbox Metro_Text_AllAboard, MSGBOX_DEFAULT
	closemessage
	playse SE_TRUCK_DOOR
	waitse
	playse SE_TRUCK_MOVE
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 4
	special ShakeCamera
	waitstate
	fadescreen FADE_TO_BLACK
	delay 60
	playse SE_TRUCK_STOP
	waitse
	return

Metro_EventScript_Stamped::
	msgbox Metro_Text_Stamped, MSGBOX_DEFAULT
	releaseall
	end

Metro_Text_WhereTo:
	.string "The train's doors are open.\n"
	.string "Where would you like to go?$"

Metro_Text_Mauville:
	.string "MAUVILLE$"

Metro_Text_Slateport:
	.string "SLATEPORT$"

Metro_Text_Verdanturf:
	.string "VERDANTURF$"

Metro_Text_Cancel:
	.string "CANCEL$"

Metro_Text_AllAboard:
	.string "All aboard! Mind the doors!$"

Metro_Text_Stamped:
	.string "This stamp is already on your card.$"
''')

# ---------------------------------------------------------------- the old line
W, H = 52, 16
o = Area(OLD, W, H, st, fill='#')
o.rect(1, 8, 36, 4, '.')                       # the walkway beside the track
o.rect(3, 2, 9, 4, '.')                        # the signal room
o.rect(6, 6, 2, 2, '.')
o.rect(16, 1, 12, 5, '.')                      # the depot office
o.rect(21, 6, 2, 2, '.')
o.rect(30, 3, 5, 3, '.')                       # a store cupboard
o.rect(31, 6, 2, 2, '.')
o.rect(37, 3, 14, 9, '.')                      # the Ghost Station's platform
for x in range(W):
    for y in range(12, 15):
        o.tile(x, y, one('track_bare' if (x * 7 + y) % 5 == 0 else 'track'), collision=1, c='o')
    o.tile(x, 15, one('track_wall'), collision=1, c='o')
for x in range(37, W - 1):
    o.tile(x, 11, one('edge'), collision=0, c='o')
place(o, 'train_old', 15, 12)
place(o, 'train_old', 39, 12)
place(o, 'tunnel_w', 0, 8)
place(o, 'tunnel_w', 0, 9)
place(o, 'tunnel_w', 0, 10)
place(o, 'tunnel_w', 0, 11)
place(o, 'board_rust', 41, 3)
place(o, 'stamp_stand', 46, 4)
place(o, 'bench', 38, 8)
place(o, 'bench', 48, 8)
place(o, 'pillar', 44, 7)
o.warp(0, 9, 'MAP_METRO_MAUVILLE', 2)
o.warp(0, 10, 'MAP_METRO_MAUVILLE', 3)
o.sign(46, 5, f'{OLD}_EventScript_Stamp', 'BG_EVENT_PLAYER_FACING_NORTH')
o.sign(41, 4, f'{OLD}_EventScript_Board')
o.sign(42, 4, f'{OLD}_EventScript_Board')
o.obj('OBJ_EVENT_GFX_OLD_MAN', 44, 9, f'{OLD}_EventScript_Conductor', 'MOVEMENT_TYPE_FACE_DOWN', 'FLAG_METRO_GHOST_SEEN')
o.trainer('OBJ_EVENT_GFX_HIKER', 12, 9, f'{OLD}_EventScript_Digby', 3, 'MOVEMENT_TYPE_FACE_LEFT')
o.trainer('OBJ_EVENT_GFX_MANIAC', 22, 3, f'{OLD}_EventScript_Gears', 2, 'MOVEMENT_TYPE_FACE_DOWN')
o.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 34, 9, f'{OLD}_EventScript_Odette', 3, 'MOVEMENT_TYPE_FACE_LEFT')
o.obj('OBJ_EVENT_GFX_SPECIES(KLINK)', 5, 3, f'{OLD}_EventScript_Klink', 'MOVEMENT_TYPE_WANDER_AROUND', rx=1, ry=1)
o.item('ITEM_HYPER_POTION', 11, 4, 'FLAG_ITEM_METRO_HYPER_POTION')
o.item('ITEM_MAGNET', 3, 5, 'FLAG_ITEM_METRO_MAGNET')
o.item('ITEM_METAL_COAT', 27, 5, 'FLAG_ITEM_METRO_METAL_COAT')
o.item('ITEM_TM_THUNDERBOLT', 33, 3, 'FLAG_ITEM_METRO_TM_THUNDERBOLT')
o.item('ITEM_SPELL_TAG', 50, 10, 'FLAG_ITEM_METRO_SPELL_TAG')
o.hidden(17, 5, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_METRO_NUGGET')
o.hidden(49, 3, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_METRO_RARE_CANDY')
o.write(map_id='MAP_METRO_OLD_LINE', layout_id='LAYOUT_METRO_OLD_LINE', music='MUS_ABANDONED_SHIP', mapsec=MAPSEC,
        mapsec_name='HOENN METRO', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, after='MetroVerdanturf',
        scripts=rf'''
{OLD}_MapScripts::
	.byte 0

{OLD}_EventScript_Stamp::
	lockall
	goto_if_set FLAG_METRO_STAMP_4, Metro_EventScript_Stamped
	setflag FLAG_METRO_STAMP_4
	playse SE_BANG
	msgbox {OLD}_Text_Stamp, MSGBOX_DEFAULT
	releaseall
	end

{OLD}_EventScript_Board::
	msgbox {OLD}_Text_Board, MSGBOX_SIGN
	end

{OLD}_EventScript_Conductor::
	lock
	faceplayer
	msgbox {OLD}_Text_Conductor, MSGBOX_DEFAULT
	closemessage
	playse SE_M_FAINT_ATTACK
	fadescreen FADE_TO_WHITE
	removeobject VAR_LAST_TALKED
	setflag FLAG_METRO_GHOST_SEEN
	fadescreen FADE_FROM_WHITE
	msgbox {OLD}_Text_ConductorGone, MSGBOX_DEFAULT
	release
	end

{OLD}_EventScript_Klink::
	lock
	faceplayer
	playmoncry SPECIES_KLINK, CRY_MODE_NORMAL
	msgbox {OLD}_Text_Klink, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{OLD}_EventScript_Digby::
	trainerbattle_single TRAINER_METRO_DIGBY, {OLD}_Text_DigbyIntro, {OLD}_Text_DigbyDefeat
	msgbox {OLD}_Text_DigbyPost, MSGBOX_AUTOCLOSE
	end

{OLD}_EventScript_Gears::
	trainerbattle_single TRAINER_METRO_GEARS, {OLD}_Text_GearsIntro, {OLD}_Text_GearsDefeat
	msgbox {OLD}_Text_GearsPost, MSGBOX_AUTOCLOSE
	end

{OLD}_EventScript_Odette::
	trainerbattle_single TRAINER_METRO_ODETTE, {OLD}_Text_OdetteIntro, {OLD}_Text_OdetteDefeat
	msgbox {OLD}_Text_OdettePost, MSGBOX_AUTOCLOSE
	end

{OLD}_Text_Stamp:
	.string "{{PLAYER}} stamped the rally card!\n"
	.string "It's the GHOST STATION stamp.$"

{OLD}_Text_Board:
	.string "HOENN METRO\n"
	.string "…ST STATION. The rest has rusted off.$"

{OLD}_Text_Conductor:
	.string "Tickets, please…\p"
	.string "Ah, a rally card. Nobody's stamped\n"
	.string "it here since the last train left.\p"
	.string "That was… a long time ago now.\n"
	.string "Thank you for visiting.$"

{OLD}_Text_ConductorGone:
	.string "The conductor was gone. Only his\n"
	.string "cap was left on the platform…$"

{OLD}_Text_Klink:
	.string "KLINK is turning round and round,\n"
	.string "like a gear with nothing to drive.$"

{OLD}_Text_DigbyIntro:
	.string "These tunnels are full of old\n"
	.string "relics! Don't touch my finds!$"

{OLD}_Text_DigbyDefeat:
	.string "Buried!$"

{OLD}_Text_DigbyPost:
	.string "I found a ticket from fifty years\n"
	.string "ago. It still says GHOST STATION.$"

{OLD}_Text_GearsIntro:
	.string "The depot's machines still hum. My\n"
	.string "POKéMON love it here!$"

{OLD}_Text_GearsDefeat:
	.string "My gears… stripped!$"

{OLD}_Text_GearsPost:
	.string "The signal room still has power.\n"
	.string "Nobody knows where it comes from.$"

{OLD}_Text_OdetteIntro:
	.string "Fu fu fu… The last train never\n"
	.string "left this line…$"

{OLD}_Text_OdetteDefeat:
	.string "Fu fu… you're no ghost.$"

{OLD}_Text_OdettePost:
	.string "An old conductor still walks the\n"
	.string "GHOST STATION platform. Say hello.$"
''')
print('ok', OLD)

register_wild('MAP_METRO_OLD_LINE', 'gMetroOldLine',
              land=[('ZUBAT', 22, 24), ('RATTATA', 22, 24), ('TRUBBISH', 22, 24), ('WOOBAT', 22, 24),
                    ('KLINK', 23, 25), ('MAGNEMITE', 23, 25), ('VOLTORB', 23, 25), ('GRIMER', 23, 25),
                    ('DRILBUR', 24, 26), ('ROGGENROLA', 24, 26), ('LITWICK', 25, 26), ('TYNAMO', 25, 26)])

# ---------------------------------------------------------------- the attendants in the cities
for key, (name, W, board, stamp, city, (cx, cy)) in STATIONS.items():
    add_object(city, {'graphics_id': 'OBJ_EVENT_GFX_WORKER_M', 'x': cx, 'y': cy, 'elevation': 3,
                      'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                      'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                      'script': f'{city}_EventScript_MetroAttendant', 'flag': '0'})
register_script_file('metro_entrances', '@ Hoenn Metro attendants in the cities (tools/areas/metro.py).\n' + ''.join(rf'''
{city}_EventScript_MetroAttendant::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE03_GET, Metro_EventScript_AttendantClosed
	msgbox Metro_Text_AttendantAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Metro_EventScript_AttendantNo
	closemessage
	warp {map_id(name)}, 3, 2
	waitstate
	release
	end
''' for key, (name, W, board, stamp, city, _) in STATIONS.items()) + r'''
Metro_EventScript_AttendantClosed::
	msgbox Metro_Text_AttendantClosed, MSGBOX_DEFAULT
	release
	end

Metro_EventScript_AttendantNo::
	msgbox Metro_Text_AttendantNo, MSGBOX_DEFAULT
	release
	end

Metro_Text_AttendantClosed:
	.string "The HOENN METRO's still being\n"
	.string "built. WATTSON's checking the\l"
	.string "power lines.\p"
	.string "Come back when you've beaten him\n"
	.string "at the MAUVILLE GYM.$"

Metro_Text_AttendantAsk:
	.string "Welcome to the HOENN METRO! Trains\n"
	.string "to MAUVILLE, SLATEPORT and\l"
	.string "VERDANTURF. Rides are free!\p"
	.string "Go down to the platform?$"

Metro_Text_AttendantNo:
	.string "Have a nice day!$"
''')
print('ok entrances')
