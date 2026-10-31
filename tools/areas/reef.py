"""Coralline Reef and the sunken city of Abyssia, on the sea floor south of
Pacifidlog Town.

- Coralline Reef: coral gardens between rock walls, a giant clam, and a
  shipwreck you can swim into, where a Dhelmise guards the captain's
  chest.
- Abyssia, further north: domes, columns and four old lamp pillars around
  a sealed temple. Relight the lamps and the temple opens; a Relicanth
  rests inside.

A diver in Pacifidlog takes you down once you have the Mind Badge.
New art (coral, domes, temple, wreck) comes from tools/areas/abyss_art.py.

    python3 tools/areas/abyss_art.py   # the tileset, when the art changes
    python3 tools/areas/reef.py
"""
import json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import abyss

AREA_NO = 9
REEF, CITY, WRECK, TEMPLE = 'CorallineReef', 'Abyssia', 'CorallineReef_Wreck', 'Abyssia_Temple'
MAPSEC = 'MAPSEC_CORALLINE_REEF'
CITY_MAPSEC = 'MAPSEC_ABYSSIA'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'abyss_tiles.json')))
UW = dict(map_type='MAP_TYPE_UNDERWATER', weather='WEATHER_UNDERWATER_BUBBLES', music='MUS_UNDERWATER',
          allow_cycling=False, allow_running=False)

register_constants(REEF, AREA_NO, flags=[
    'ABYSS_LAMP_1', 'ABYSS_LAMP_2', 'ABYSS_LAMP_3', 'ABYSS_LAMP_4', 'ABYSS_TEMPLE_OPEN', 'ABYSS_MARINA_TALKED',
    'ABYSS_MARINA_REWARD', 'ABYSS_RELICANTH_DONE', 'ABYSS_DHELMISE_DONE', 'REEF_CLAM_PEARL',
    'ITEM_REEF_DEEP_SEA_SCALE', 'ITEM_REEF_DEEP_SEA_TOOTH', 'ITEM_REEF_TM_WATER_PULSE', 'ITEM_REEF_MAX_REVIVE',
    'ITEM_WRECK_NUGGET', 'ITEM_WRECK_RARE_CANDY', 'ITEM_ABYSS_MYSTIC_WATER', 'ITEM_ABYSS_PRISM_SCALE',
    'ITEM_TEMPLE_HEART_SCALE', 'HIDDEN_ITEM_REEF_PEARL', 'HIDDEN_ITEM_REEF_STAR_PIECE', 'HIDDEN_ITEM_ABYSS_BIG_PEARL',
    'HIDE_ABYSS_RELICANTH',
], trainers=['REEF_NERISSA', 'REEF_DORIAN', 'REEF_PELAGIA', 'ABYSS_TRITON'])

register_trainers(REEF, '''
=== AREA_TRAINER_REEF_NERISSA ===
Name: NERISSA
Class: Swimmer F
Pic: Swimmer F
Gender: Female
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Lumineon
Level: 42

Corsola
Level: 42

=== AREA_TRAINER_REEF_DORIAN ===
Name: DORIAN
Class: Swimmer M
Pic: Swimmer M
Gender: Male
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Seaking
Level: 41

Qwilfish
Level: 41

Sharpedo
Level: 43

=== AREA_TRAINER_REEF_PELAGIA ===
Name: PELAGIA
Class: Swimmer F
Pic: Swimmer F
Gender: Female
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Mantine
Level: 43

Lanturn
Level: 43

=== AREA_TRAINER_ABYSS_TRITON ===
Name: TRITON
Class: Swimmer M
Pic: Swimmer M
Gender: Male
Music: Swimmer
Double Battle: No
AI: Basic Trainer

Gorebyss
Level: 44

Huntail
Level: 44

Kingdra
Level: 45
''')


def place(a, name, x, y):
    """Stamp a drawn object with its top left at (x, y); returns its metatile rows."""
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    rows = ART['objects'][name]
    for r, row in enumerate(rows):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')
    return rows


# ---------------------------------------------------------------- Coralline Reef
W, H = 56, 44
a = Area(REEF, W, H, abyss(), fill='#')
a.ellipse(28, 40, 4, 3, '.')                                # where the diver drops you
a.line([(28, 40), (28, 30)], 3, '.')
a.ellipse(28, 25, 15, 7, '.')                               # the coral garden
a.ellipse(26, 25, 6, 3, '"')
a.line([(16, 24), (11, 16)], 3, '.')                        # west to the wreck
a.ellipse(11, 12, 10, 6, '.')
a.line([(40, 24), (46, 14)], 3, '.')                        # east to the clam
a.ellipse(46, 11, 7, 5, '.')
a.ellipse(47, 14, 3, 1.5, '"')
a.line([(28, 18), (28, 0)], 3, '.')                         # north to Abyssia
a.ellipse(20, 33, 4, 3, '.')
a.ellipse(19, 34, 2.5, 1.5, '"')
a.ellipse(38, 34, 5, 3, '.')
a.line([(31, 36), (35, 34)], 2, '.')
a.ellipse(8, 18, 3, 2, '"')
place(a, 'wreck', 3, 6)                                     # the hole is at (7, 10)
WRECK_DOOR = (7, 10)
place(a, 'clam', 45, 7)
for name, x, y in (('coral_branch', 18, 21), ('coral_fan', 34, 21), ('coral_tubes', 22, 29), ('coral_branch', 37, 27),
                   ('coral_fan', 14, 26), ('coral_tubes', 30, 19), ('coral_branch', 41, 31), ('coral_fan', 17, 31),
                   ('coral_tubes', 49, 9), ('coral_branch', 13, 8), ('anchor', 16, 13), ('coral_fan', 24, 36),
                   ('coral_tubes', 40, 36)):
    place(a, name, x, y)
a.connections = [{'map': 'MAP_ABYSSIA', 'offset': 0, 'direction': 'up'}]
a.warp(*WRECK_DOOR, 'MAP_CORALLINE_REEF_WRECK', 0)
ARRIVE = (28, 41)
a.obj('OBJ_EVENT_GFX_SWIMMER_M_WATER', 29, 41, f'{REEF}_EventScript_Diver', 'MOVEMENT_TYPE_FACE_LEFT')
a.obj('OBJ_EVENT_GFX_SPECIES(CORSOLA)', 30, 24, f'{REEF}_EventScript_Corsola', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_SPECIES(LUVDISC)', 22, 26, f'{REEF}_EventScript_Luvdisc', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_SPECIES(CLAMPERL)', 43, 12, f'{REEF}_EventScript_Clamperl', 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_SWIMMER_F_WATER', 21, 24, f'{REEF}_EventScript_Nerissa', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_SWIMMER_M_WATER', 28, 13, f'{REEF}_EventScript_Dorian', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.trainer('OBJ_EVENT_GFX_SWIMMER_F_WATER', 12, 16, f'{REEF}_EventScript_Pelagia', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.item('ITEM_DEEP_SEA_SCALE', 51, 12, 'FLAG_ITEM_REEF_DEEP_SEA_SCALE')
a.item('ITEM_DEEP_SEA_TOOTH', 4, 15, 'FLAG_ITEM_REEF_DEEP_SEA_TOOTH')
a.item('ITEM_TM_WATER_PULSE', 42, 35, 'FLAG_ITEM_REEF_TM_WATER_PULSE')
a.item('ITEM_MAX_REVIVE', 17, 34, 'FLAG_ITEM_REEF_MAX_REVIVE')
a.hidden(24, 31, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_REEF_PEARL')
a.hidden(38, 22, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_REEF_STAR_PIECE')
a.sign(46, 9, f'{REEF}_EventScript_Clam', 'BG_EVENT_PLAYER_FACING_NORTH', elevation=3)
a.write(map_id='MAP_CORALLINE_REEF', layout_id='LAYOUT_CORALLINE_REEF', mapsec=MAPSEC, mapsec_name='CORALLINE REEF',
        after='SunfairCarnival', **UW, scripts=rf'''
{REEF}_MapScripts::
	.byte 0

{REEF}_EventScript_Diver::
	lock
	faceplayer
	msgbox {REEF}_Text_DiverBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {REEF}_EventScript_DiverStay
	msgbox {REEF}_Text_DiverUp, MSGBOX_DEFAULT
	closemessage
	warp MAP_PACIFIDLOG_TOWN, 8, 19
	waitstate
	release
	end

{REEF}_EventScript_DiverStay::
	msgbox {REEF}_Text_DiverStay, MSGBOX_DEFAULT
	release
	end

{REEF}_EventScript_Clam::
	lockall
	goto_if_set FLAG_REEF_CLAM_PEARL, {REEF}_EventScript_ClamEmpty
	msgbox {REEF}_Text_ClamOpen, MSGBOX_DEFAULT
	setflag FLAG_REEF_CLAM_PEARL
	giveitem ITEM_BIG_PEARL
	releaseall
	end

{REEF}_EventScript_ClamEmpty::
	msgbox {REEF}_Text_ClamEmpty, MSGBOX_DEFAULT
	releaseall
	end

{REEF}_EventScript_Corsola::
	lock
	faceplayer
	playmoncry SPECIES_CORSOLA, CRY_MODE_NORMAL
	msgbox {REEF}_Text_Corsola, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{REEF}_EventScript_Luvdisc::
	lock
	faceplayer
	playmoncry SPECIES_LUVDISC, CRY_MODE_NORMAL
	msgbox {REEF}_Text_Luvdisc, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{REEF}_EventScript_Clamperl::
	lock
	faceplayer
	playmoncry SPECIES_CLAMPERL, CRY_MODE_NORMAL
	msgbox {REEF}_Text_Clamperl, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{REEF}_EventScript_Nerissa::
	trainerbattle_single TRAINER_REEF_NERISSA, {REEF}_Text_NerissaIntro, {REEF}_Text_NerissaDefeat
	msgbox {REEF}_Text_NerissaPost, MSGBOX_AUTOCLOSE
	end

{REEF}_EventScript_Dorian::
	trainerbattle_single TRAINER_REEF_DORIAN, {REEF}_Text_DorianIntro, {REEF}_Text_DorianDefeat
	msgbox {REEF}_Text_DorianPost, MSGBOX_AUTOCLOSE
	end

{REEF}_EventScript_Pelagia::
	trainerbattle_single TRAINER_REEF_PELAGIA, {REEF}_Text_PelagiaIntro, {REEF}_Text_PelagiaDefeat
	msgbox {REEF}_Text_PelagiaPost, MSGBOX_AUTOCLOSE
	end

{REEF}_Text_DiverBack:
	.string "Running low on air? I can take you\n"
	.string "back up to PACIFIDLOG.$"

{REEF}_Text_DiverUp:
	.string "Up we go. Slowly, now!$"

{REEF}_Text_DiverStay:
	.string "The sunken city is north of the\n"
	.string "coral garden. Swim safe!$"

{REEF}_Text_ClamOpen:
	.string "The giant clam slowly opened…\n"
	.string "There's a pearl inside!$"

{REEF}_Text_ClamEmpty:
	.string "The giant clam is fast asleep.$"

{REEF}_Text_Corsola:
	.string "CORSOLA is tending the coral like a\n"
	.string "little gardener.$"

{REEF}_Text_Luvdisc:
	.string "LUVDISC drifts by and gives you a\n"
	.string "friendly nudge.$"

{REEF}_Text_Clamperl:
	.string "CLAMPERL is guarding the giant clam.\n"
	.string "It looks like a proud little sibling.$"

{REEF}_Text_NerissaIntro:
	.string "The reef is the prettiest place in\n"
	.string "the whole sea! Let's battle in it!$"

{REEF}_Text_NerissaDefeat:
	.string "Swept away!$"

{REEF}_Text_NerissaPost:
	.string "Don't break the coral, okay?$"

{REEF}_Text_DorianIntro:
	.string "Nobody gets to the sunken city\n"
	.string "without getting past me!$"

{REEF}_Text_DorianDefeat:
	.string "You swim faster than a SHARPEDO!$"

{REEF}_Text_DorianPost:
	.string "The city's lamps went dark ages\n"
	.string "ago. Maybe you can light them.$"

{REEF}_Text_PelagiaIntro:
	.string "That old wreck is haunted! Or maybe\n"
	.string "I'm just scared. Battle me!$"

{REEF}_Text_PelagiaDefeat:
	.string "I'm still scared…$"

{REEF}_Text_PelagiaPost:
	.string "Something moved inside the wreck.\n"
	.string "Something like an anchor…$"
''')
register_wild('MAP_CORALLINE_REEF', 'gCorallineReef',
              water=[('CLAMPERL', 38, 42), ('CHINCHOU', 38, 42), ('LUVDISC', 38, 42), ('CORSOLA', 40, 44), ('RELICANTH', 40, 44)])
print('ok', REEF)

# ---------------------------------------------------------------- the shipwreck (Abandoned Ship's flooded hold)
interior(WRECK, 'MAP_CORALLINE_REEF_WRECK', 'LAYOUT_ABANDONED_SHIP_UNDERWATER2', [(3, 1)], 'MAP_CORALLINE_REEF', 0,
         mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes', music='MUS_UNDERWATER', map_type='MAP_TYPE_UNDERWATER',
         weather='WEATHER_UNDERWATER_BUBBLES',
         objects=[npc('OBJ_EVENT_GFX_SPECIES(DHELMISE)', 17, 3, f'{WRECK}_EventScript_Dhelmise',
                      flag='FLAG_ABYSS_DHELMISE_DONE'),
                  npc('OBJ_EVENT_GFX_ITEM_BALL', 19, 5, f'{WRECK}_EventScript_Nugget', flag='FLAG_ITEM_WRECK_NUGGET'),
                  npc('OBJ_EVENT_GFX_ITEM_BALL', 1, 5, f'{WRECK}_EventScript_RareCandy', flag='FLAG_ITEM_WRECK_RARE_CANDY')],
         bgs=[{'type': 'sign', 'x': 9, 'y': 1, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
               'script': f'{WRECK}_EventScript_Log'}],
         scripts=rf'''
{WRECK}_MapScripts::
	.byte 0

{WRECK}_EventScript_Dhelmise::
	lock
	faceplayer
	playmoncry SPECIES_DHELMISE, CRY_MODE_ENCOUNTER
	msgbox {WRECK}_Text_Dhelmise, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_DHELMISE, 44
	dowildbattle
	setflag FLAG_ABYSS_DHELMISE_DONE
	removeobject VAR_LAST_TALKED
	release
	end

{WRECK}_EventScript_Nugget::
	finditem ITEM_NUGGET
	end

{WRECK}_EventScript_RareCandy::
	finditem ITEM_RARE_CANDY
	end

{WRECK}_EventScript_Log::
	msgbox {WRECK}_Text_Log, MSGBOX_SIGN
	end

{WRECK}_Text_Dhelmise:
	.string "The old anchor came to life!$"

{WRECK}_Text_Log:
	.string "A waterlogged captain's log…\p"
	.string "“Day 40. We found the lights of a\n"
	.string "city under the sea, then lost our\l"
	.string "anchor in a storm. The crew say\l"
	.string "it swam away on its own.”$"
''')
print('ok', WRECK)

# ---------------------------------------------------------------- Abyssia, the sunken city
c = Area(CITY, W, H, abyss(), fill='#')
c.ellipse(28, 24, 25, 17, '.')
c.line([(28, 43), (28, 36)], 3, '.')
c.rect(18, 2, 20, 10, '.')
c.ellipse(10, 38, 5, 3, '"')
c.ellipse(47, 38, 5, 3, '"')
c.ellipse(6, 24, 2.5, 4, '"')
c.ellipse(50, 24, 2.5, 4, '"')
TEMPLE_AT = (24, 4)                                          # 8x6; door blocks (3..4, 2..4), steps on row 5
place(c, 'temple', *TEMPLE_AT)
TEMPLE_DOOR = [(TEMPLE_AT[0] + 3, TEMPLE_AT[1] + 4), (TEMPLE_AT[0] + 4, TEMPLE_AT[1] + 4)]
OPEN = ART['objects']['temple_open']
OPEN_LINES = ''.join(f'\tsetmetatile {TEMPLE_AT[0] + cx}, {TEMPLE_AT[1] + cy}, {OPEN[cy][cx]}, {"FALSE" if cy == 4 else "TRUE"}\n'
                     for cy in (2, 3, 4) for cx in (3, 4))
for x, y in ((9, 12), (42, 12), (9, 28), (42, 28)):
    place(c, 'dome', x, y)
LAMPS = [(18, 15), (37, 15), (18, 31), (37, 31)]              # the globe; the post is below it
for x, y in LAMPS:
    place(c, 'lamp_dark', x, y)
LIT = ART['objects']['lamp_lit'][0][0]
for x, y in ((23, 13), (32, 13), (23, 19), (32, 19), (23, 25), (32, 25)):
    place(c, 'column', x, y)
for x, y in ((15, 22), (40, 22), (26, 34), (12, 36), (44, 36)):
    place(c, 'column_broken', x, y)
for name, x, y in (('coral_branch', 5, 20), ('coral_fan', 49, 20), ('coral_tubes', 14, 9), ('coral_tubes', 40, 9),
                   ('coral_branch', 33, 37), ('coral_fan', 20, 37), ('clam', 4, 31)):
    place(c, name, x, y)
c.connections = [{'map': 'MAP_CORALLINE_REEF', 'offset': 0, 'direction': 'down'}]
for x, y in TEMPLE_DOOR:
    c.warp(x, y, 'MAP_ABYSSIA_TEMPLE', 0)
LAMP_LINES = []
for i, (x, y) in enumerate(LAMPS, 1):
    c.sign(x, y + 1, f'{CITY}_EventScript_Lamp{i}', 'BG_EVENT_PLAYER_FACING_NORTH', elevation=3)
c.obj('OBJ_EVENT_GFX_SCIENTIST_1', 28, 32, f'{CITY}_EventScript_Marina', 'MOVEMENT_TYPE_FACE_DOWN')
c.obj('OBJ_EVENT_GFX_SPECIES(LANTURN)', 13, 18, f'{CITY}_EventScript_Lanturn', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=2)
c.obj('OBJ_EVENT_GFX_SPECIES(MANTINE)', 38, 24, f'{CITY}_EventScript_Mantine', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
c.obj('OBJ_EVENT_GFX_SPECIES(ALOMOMOLA)', 46, 33, f'{CITY}_EventScript_Alomomola', 'MOVEMENT_TYPE_LOOK_AROUND')
c.obj('OBJ_EVENT_GFX_SPECIES(LUMINEON)', 20, 27, f'{CITY}_EventScript_Lumineon', 'MOVEMENT_TYPE_WANDER_AROUND', rx=1, ry=2)
c.trainer('OBJ_EVENT_GFX_SWIMMER_M_WATER', 35, 34, f'{CITY}_EventScript_Triton', 3, 'MOVEMENT_TYPE_FACE_LEFT')
c.item('ITEM_MYSTIC_WATER', 51, 27, 'FLAG_ITEM_ABYSS_MYSTIC_WATER')
c.item('ITEM_PRISM_SCALE', 21, 3, 'FLAG_ITEM_ABYSS_PRISM_SCALE')
c.hidden(8, 34, 'ITEM_BIG_PEARL', 'FLAG_HIDDEN_ITEM_ABYSS_BIG_PEARL')
lamp_scripts = ''
for i, (x, y) in enumerate(LAMPS, 1):
    lamp_scripts += rf'''
{CITY}_EventScript_Lamp{i}::
	lockall
	goto_if_set FLAG_ABYSS_LAMP_{i}, {CITY}_EventScript_LampLit
	msgbox {CITY}_Text_LampAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {CITY}_EventScript_LampLeave
	setflag FLAG_ABYSS_LAMP_{i}
	setmetatile {x}, {y}, {LIT}, TRUE
	special DrawWholeMapView
	playse SE_M_CHARGE
	waitse
	msgbox {CITY}_Text_LampGlows, MSGBOX_DEFAULT
	goto {CITY}_EventScript_CheckLamps
'''
on_load_lamps = ''.join(f'\tcall_if_set FLAG_ABYSS_LAMP_{i}, {CITY}_OnLoadLamp{i}\n' for i in range(1, 5))
on_load_subs = ''.join(f'{CITY}_OnLoadLamp{i}:\n\tsetmetatile {x}, {y}, {LIT}, TRUE\n\treturn\n\n'
                       for i, (x, y) in enumerate(LAMPS, 1))
c.write(map_id='MAP_ABYSSIA', layout_id='LAYOUT_ABYSSIA', mapsec=CITY_MAPSEC, mapsec_name='ABYSSIA', after=REEF, **UW,
        scripts=rf'''
{CITY}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {CITY}_OnLoad
	.byte 0

{CITY}_OnLoad:
{on_load_lamps}	call_if_set FLAG_ABYSS_TEMPLE_OPEN, {CITY}_OpenTemple
	end

{on_load_subs}{CITY}_OpenTemple:
{OPEN_LINES}	return
{lamp_scripts}
{CITY}_EventScript_LampLit::
	msgbox {CITY}_Text_LampLit, MSGBOX_DEFAULT
	releaseall
	end

{CITY}_EventScript_LampLeave::
	releaseall
	end

{CITY}_EventScript_CheckLamps::
	goto_if_unset FLAG_ABYSS_LAMP_1, {CITY}_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_2, {CITY}_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_3, {CITY}_EventScript_LampsLeft
	goto_if_unset FLAG_ABYSS_LAMP_4, {CITY}_EventScript_LampsLeft
	setflag FLAG_ABYSS_TEMPLE_OPEN
	playse SE_M_EARTHQUAKE
	special ShakeCamera
	call {CITY}_OpenTemple
	special DrawWholeMapView
	msgbox {CITY}_Text_TempleOpens, MSGBOX_DEFAULT
	releaseall
	end

{CITY}_EventScript_LampsLeft::
	msgbox {CITY}_Text_LampsLeft, MSGBOX_DEFAULT
	releaseall
	end

{CITY}_EventScript_Marina::
	lock
	faceplayer
	goto_if_set FLAG_ABYSS_MARINA_REWARD, {CITY}_EventScript_MarinaDone
	goto_if_set FLAG_ABYSS_TEMPLE_OPEN, {CITY}_EventScript_MarinaReward
	setflag FLAG_ABYSS_MARINA_TALKED
	msgbox {CITY}_Text_MarinaAsk, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_MarinaReward::
	msgbox {CITY}_Text_MarinaThanks, MSGBOX_DEFAULT
	giveitem ITEM_DEEP_SEA_SCALE
	giveitem ITEM_DEEP_SEA_TOOTH
	setflag FLAG_ABYSS_MARINA_REWARD
	msgbox {CITY}_Text_MarinaAfter, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_MarinaDone::
	msgbox {CITY}_Text_MarinaDone, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_Lanturn::
	lock
	faceplayer
	playmoncry SPECIES_LANTURN, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Lanturn, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CITY}_EventScript_Mantine::
	lock
	faceplayer
	playmoncry SPECIES_MANTINE, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Mantine, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CITY}_EventScript_Alomomola::
	lock
	faceplayer
	playmoncry SPECIES_ALOMOMOLA, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Alomomola, MSGBOX_DEFAULT
	waitmoncry
	special HealPlayerParty
	msgbox {CITY}_Text_Healed, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_Lumineon::
	lock
	faceplayer
	playmoncry SPECIES_LUMINEON, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Lumineon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CITY}_EventScript_Triton::
	trainerbattle_single TRAINER_ABYSS_TRITON, {CITY}_Text_TritonIntro, {CITY}_Text_TritonDefeat
	msgbox {CITY}_Text_TritonPost, MSGBOX_AUTOCLOSE
	end

{CITY}_Text_LampAsk:
	.string "An old lamp pillar. There's a crank\n"
	.string "on its side. Turn it?$"

{CITY}_Text_LampGlows:
	.string "The crank creaked… and the lamp\n"
	.string "began to glow!$"

{CITY}_Text_LampLit:
	.string "The lamp is glowing softly.$"

{CITY}_Text_LampsLeft:
	.string "Somewhere in the city, something is\n"
	.string "still waiting in the dark.$"

{CITY}_Text_TempleOpens:
	.string "All four lamps are shining!\p"
	.string "The temple doors are grinding open!$"

{CITY}_Text_MarinaAsk:
	.string "A visitor, down here? I'm MARINA.\n"
	.string "I study this sunken city, ABYSSIA.\p"
	.string "Its people built four lamp pillars\n"
	.string "around the temple.\p"
	.string "The carvings say: “When the four\n"
	.string "lights shine, the temple wakes.”\p"
	.string "My arms aren't strong enough for\n"
	.string "the cranks. Could you try?$"

{CITY}_Text_MarinaThanks:
	.string "The temple is open! Decades of study\n"
	.string "and you did it in an afternoon!\p"
	.string "Please, take these. I found them in\n"
	.string "the ruins.$"

{CITY}_Text_MarinaAfter:
	.string "Something ancient sleeps inside. Be\n"
	.string "gentle with it.$"

{CITY}_Text_MarinaDone:
	.string "Every carving here tells of a fish\n"
	.string "older than the city itself.$"

{CITY}_Text_Lanturn:
	.string "LANTURN's light flickers. It seems\n"
	.string "to miss the old city lamps.$"

{CITY}_Text_Mantine:
	.string "MANTINE glides slowly over the\n"
	.string "rooftops.$"

{CITY}_Text_Alomomola:
	.string "ALOMOMOLA wrapped your POKéMON in\n"
	.string "its caring fins…$"

{CITY}_Text_Healed:
	.string "Your POKéMON are fully healed!$"

{CITY}_Text_Lumineon:
	.string "LUMINEON's fins glow in the dark\n"
	.string "water.$"

{CITY}_Text_TritonIntro:
	.string "I've been treasure hunting down\n"
	.string "here for weeks. You're not taking\l"
	.string "my treasure!$"

{CITY}_Text_TritonDefeat:
	.string "Fine, fine…$"

{CITY}_Text_TritonPost:
	.string "The real treasure is in the temple.\n"
	.string "But it's sealed tight.$"
''')
register_wild('MAP_ABYSSIA', 'gAbyssia',
              water=[('CHINCHOU', 40, 44), ('LANTURN', 42, 45), ('CLAMPERL', 40, 44), ('FINNEON', 40, 44), ('LUMINEON', 43, 45)])
print('ok', CITY)

# ---------------------------------------------------------------- the temple
t = Area(TEMPLE, 20, 16, abyss(), fill='#')
t.rect(3, 3, 14, 12, '.')
for x in range(4, 17, 4):
    for y in (4, 9):
        place(t, 'column', x - 1, y - 1 if y == 4 else y)
place(t, 'clam', 9, 2)
place(t, 'exit_glyph', 9, 14)
t.warp(9, 14, 'MAP_ABYSSIA', 0)
t.obj('OBJ_EVENT_GFX_SPECIES(RELICANTH)', 10, 5, f'{TEMPLE}_EventScript_Relicanth', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_ABYSS_RELICANTH')
t.item('ITEM_HEART_SCALE', 4, 11, 'FLAG_ITEM_TEMPLE_HEART_SCALE')
t.write(map_id='MAP_ABYSSIA_TEMPLE', layout_id='LAYOUT_ABYSSIA_TEMPLE', mapsec=CITY_MAPSEC, mapsec_name='ABYSSIA',
        after=CITY, show_name=False, **UW, scripts=rf'''
{TEMPLE}_MapScripts::
	.byte 0

{TEMPLE}_EventScript_Relicanth::
	lock
	faceplayer
	playmoncry SPECIES_RELICANTH, CRY_MODE_ENCOUNTER
	msgbox {TEMPLE}_Text_Relicanth, MSGBOX_DEFAULT
	waitmoncry
	setwildbattle SPECIES_RELICANTH, 48
	dowildbattle
	setflag FLAG_HIDE_ABYSS_RELICANTH
	setflag FLAG_ABYSS_RELICANTH_DONE
	removeobject VAR_LAST_TALKED
	release
	end

{TEMPLE}_Text_Relicanth:
	.string "An ancient fish rests before the\n"
	.string "altar. It opens one old eye…$"
''')
print('ok', TEMPLE)

# ---------------------------------------------------------------- the way down, from Pacifidlog Town
add_object('PacifidlogTown', {'graphics_id': 'OBJ_EVENT_GFX_SWIMMER_M_LAND', 'x': 8, 'y': 18, 'elevation': 3,
                              'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                              'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                              'script': 'PacifidlogTown_EventScript_ReefDiver', 'flag': '0'})
register_script_file('coralline', r'''
@ Coralline Reef diver in Pacifidlog Town (tools/areas/reef.py).
PacifidlogTown_EventScript_ReefDiver::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, PacifidlogTown_EventScript_ReefDiverNoBadge
	msgbox PacifidlogTown_Text_ReefDiverAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PacifidlogTown_EventScript_ReefDiverNo
	msgbox PacifidlogTown_Text_ReefDiverGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_CORALLINE_REEF, 28, 41
	waitstate
	release
	end

PacifidlogTown_EventScript_ReefDiverNoBadge::
	msgbox PacifidlogTown_Text_ReefDiverNoBadge, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_EventScript_ReefDiverNo::
	msgbox PacifidlogTown_Text_ReefDiverNo, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_Text_ReefDiverNoBadge:
	.string "Below us is CORALLINE REEF, and\n"
	.string "past it a city under the sea!\p"
	.string "It's deep, though. Come back once\n"
	.string "TATE and LIZA say you're ready.$"

PacifidlogTown_Text_ReefDiverAsk:
	.string "The MIND BADGE! Then you can handle\n"
	.string "the deep.\p"
	.string "Want me to guide you down to\n"
	.string "CORALLINE REEF?$"

PacifidlogTown_Text_ReefDiverGo:
	.string "Big breath… and down we go!$"

PacifidlogTown_Text_ReefDiverNo:
	.string "The reef will wait for you.$"
''')
print('ok entrance')
