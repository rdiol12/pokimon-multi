"""Wonderwild Park, for after the second badge: a theme park reached by tram
from Slateport, with a castle, a hedge maze, a teacup ride whose plates spin
you across the floor, a roller coaster you ride round the park and a
carousel.

The park's three mascots ran off before the parade: Clefairy is lost in the
hedge maze, Jigglypuff is stuck in the middle of the teacup ride, and
Togepi is on the coaster's photo deck, which only the coaster reaches. Find
all three and the park director gives you a Tandemaus, and the parade
starts at the castle.

New art (the coaster, its car, station and decks, the teacup floor, hedges,
the castle, a carousel, ticket booths, a star statue, flower beds and a
popcorn cart) is drawn in tools/areas/park_art.py (gTileset_Park:
Petalburg's tiles plus the art).

    python3 tools/areas/park_art.py    # the tileset, when the art changes
    python3 tools/areas/park.py
"""
import copy, json, random, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import town, PLAZA

AREA_NO = 17
NAME = 'WonderwildPark'
MAPSEC = 'MAPSEC_WONDERWILD_PARK'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'park_tiles.json')))
MT = ART['objects']
RIDE_ELEV = 4             # the coaster and its photo deck stand above the ground

register_constants(NAME, AREA_NO, flags=[
    'PARK_MASCOT_1', 'PARK_MASCOT_2', 'PARK_MASCOT_3', 'PARK_PARADE', 'HIDE_PARK_PARADE',
    'ITEM_PARK_LEMONADE', 'ITEM_PARK_SUPER_POTION', 'ITEM_PARK_GREAT_BALL', 'ITEM_PARK_TM_ATTRACT_X',
    'ITEM_PARK_MOON_STONE', 'HIDDEN_ITEM_PARK_NUGGET', 'HIDDEN_ITEM_PARK_STAR_PIECE', 'HIDDEN_ITEM_PARK_RARE_CANDY',
], trainers=['PARK_TIMMY', 'PARK_POPPY', 'PARK_REMY', 'PARK_GLORIA'])

register_trainers(NAME, '''
=== AREA_TRAINER_PARK_TIMMY ===
Name: TIMMY
Class: Youngster
Pic: Youngster
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Pawmi
Level: 17

Rockruff
Level: 18

=== AREA_TRAINER_PARK_POPPY ===
Name: POPPY
Class: Lass
Pic: Lass
Gender: Female
Music: Female
Double Battle: No
AI: Basic Trainer

Fidough
Level: 18

Swirlix
Level: 18

=== AREA_TRAINER_PARK_REMY ===
Name: REMY
Class: School Kid
Pic: School Kid M
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Minccino
Level: 18

Pichu
Level: 17

Spinda
Level: 19

=== AREA_TRAINER_PARK_GLORIA ===
Name: GLORIA
Class: Pokefan
Pic: Pokefan F
Gender: Female
Music: Twins
Double Battle: No
AI: Basic Trainer

Snubbull
Level: 19

Azurill
Level: 19

Cleffa
Level: 20
''')


def place(a, name, x, y, elevation=3):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, elevation=elevation, c='o')


def one(name):
    return MT[name][0][0]


st = copy.copy(town())
st.secondary = 'gTileset_Park'                 # Petalburg's tiles plus the art, same metatile ids
W, H = 56, 46
a = Area(NAME, W, H, st, fill='g')
a.rect(0, 0, W, 2, 'T')                         # the park's tree line
a.rect(0, 0, 2, H, 'T')
a.rect(W - 2, 0, 2, H, 'T')
a.rect(0, H - 2, 26, 2, 'T')
a.rect(30, H - 2, W - 30, 2, 'T')
# the paved avenues (General's plaza set)
a.rect(26, 7, 4, H - 7, 'P')
a.rect(4, 21, W - 8, 4, 'P')
a.rect(22, 7, 12, 3, 'P')
a.ellipse(28, 23, 6, 4, 'P')
a.rect(22, 38, 12, 4, 'P')
a.autotile3('P', PLAZA)

# ---------------------------------------------------------------- the castle and the plaza
CASTLE = (25, 2)
place(a, 'castle', *CASTLE)
place(a, 'star_statue', 27, 22)
place(a, 'star_statue', 28, 22)
for x, y in ((22, 6), (33, 6), (23, 20), (32, 20), (23, 26), (32, 26), (24, 36), (31, 36)):
    place(a, 'flower_bed', x, y)
place(a, 'carousel', 9, 28)
place(a, 'booth', 22, 40)
place(a, 'booth', 32, 40)
place(a, 'popcorn', 20, 22)
place(a, 'popcorn', 35, 24)
for x, y in ((6, 34), (14, 36), (18, 30), (4, 40), (16, 41)):
    a.rect(x, y, 2, 2, 'T')
a.ellipse(7, 39, 3, 2, '"')

# ---------------------------------------------------------------- the hedge maze (17 x 15 at x 4, y 4)
MX, MY, CW, CH = 4, 4, 8, 7
rnd = random.Random(7)
open_cells = set()
stack, seen = [(CW // 2, CH - 1)], {(CW // 2, CH - 1)}
depth = {(CW // 2, CH - 1): 0}
while stack:
    cx, cy = stack[-1]
    open_cells.add((MX + 1 + 2 * cx, MY + 1 + 2 * cy))
    nbrs = [(cx + dx, cy + dy, dx, dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))
            if 0 <= cx + dx < CW and 0 <= cy + dy < CH and (cx + dx, cy + dy) not in seen]
    if not nbrs:
        stack.pop()
        continue
    nx, ny, dx, dy = rnd.choice(nbrs)
    open_cells.add((MX + 1 + 2 * cx + dx, MY + 1 + 2 * cy + dy))
    seen.add((nx, ny))
    depth[(nx, ny)] = depth[(cx, cy)] + 1
    stack.append((nx, ny))
MAZE_IN = (MX + 1 + 2 * (CW // 2), MY + 2 * CH)          # the gap in the bottom hedge
open_cells.add(MAZE_IN)
far = max(depth, key=depth.get)
CLEFAIRY = (MX + 1 + 2 * far[0], MY + 1 + 2 * far[1])
for y in range(MY, MY + 2 * CH + 1):
    for x in range(MX, MX + 2 * CW + 1):
        if (x, y) in open_cells:
            a.put(x, y, 'g')
        else:
            a.tile(x, y, one('hedge'), collision=1, c='o')
a.rect(MAZE_IN[0], MAZE_IN[1] + 1, 1, 21 - MAZE_IN[1] - 1, 'g')

# ---------------------------------------------------------------- the teacups (rail round x 34..51, y 4..19)
TEACUPS = """\
................
v###............
.#..........<...
.#..........#...
.#.....>....#...
.#######....#...
>.....>oM...#...
.######v#.......
.#..............
.#...........^..
.#...........#..
........>.......
.......o........
................"""
TX, TY = 35, 5
for x in range(TX - 1, TX + 17):
    for y in (TY - 1, TY + 14):
        if (x, y) != (TX + 7, TY + 14):                     # the way in
            a.tile(x, y, one('teacup_rail'), collision=1, c='o')
for y in range(TY - 1, TY + 15):
    a.tile(TX - 1, y, one('teacup_rail'), collision=1, c='o')
    a.tile(TX + 16, y, one('teacup_rail'), collision=1, c='o')
a.tile(TX + 7, TY + 14, one('teacup_plain'), collision=0, c='o')
SPIN = {'>': 'teacup_e', '<': 'teacup_w', '^': 'teacup_n', 'v': 'teacup_s', 'o': 'teacup_stop', '.': 'teacup_plain',
        'M': 'teacup_stop'}
for r, row in enumerate(TEACUPS.split('\n')):
    for c, ch in enumerate(row):
        if ch == '#':
            a.tile(TX + c, TY + r, one('teacup_rail'), collision=1, c='o')
        else:
            a.tile(TX + c, TY + r, one(SPIN[ch]), collision=0, c='o')
        if ch == 'M':
            JIGGLYPUFF = (TX + c, TY + r)
a.rect(TX + 7, TY + 15, 1, 21 - TY - 15, 'g')

# ---------------------------------------------------------------- the roller coaster
def run_cells(*legs):
    cells = []
    for x0, y0, x1, y1 in legs:
        n = max(abs(x1 - x0), abs(y1 - y0))
        for i in range(n + 1):
            p = (x0 + (x1 - x0) * i // n, y0 + (y1 - y0) * i // n)
            if not cells or cells[-1] != p:
                cells.append(p)
    return cells


COASTER = run_cells((40, 30, 52, 30), (52, 30, 52, 41), (52, 41, 36, 41), (36, 41, 36, 34), (36, 34, 46, 34),
                    (46, 34, 46, 36))
VEC = {(1, 0): 'e', (-1, 0): 'w', (0, 1): 's', (0, -1): 'n'}
for (x, y), (nx, ny) in zip(COASTER, COASTER[1:]):
    a.tile(x, y, one(f'coaster_{VEC[(nx - x, ny - y)]}'), collision=0, elevation=RIDE_ELEV, c='o')
for (px, py), (x, y), (nx, ny) in zip(COASTER, COASTER[1:], COASTER[2:]):
    din, dout = VEC[(x - px, y - py)], VEC[(nx - x, ny - y)]
    if din != dout:
        a.tile(x, y, one(f'coaster_{din}{dout}'), collision=0, elevation=RIDE_ELEV, c='o')
DECK = [(x, y) for x in range(44, 49) for y in range(36, 39)]
for x, y in DECK:
    a.tile(x, y, one('deck'), collision=0, elevation=RIDE_ELEV, c='o')
a.tile(44, 38, one('deck_steps'), collision=0, elevation=RIDE_ELEV, c='o')
place(a, 'station_roof', 34, 28)
place(a, 'coaster_car', 38, 30)
for x, y in ((38, 36), (40, 38), (49, 32)):
    a.rect(x, y, 2, 2, 'T')
TOGEPI = (47, 37)

a.snap_trees('T', 'g')
a.connections = None

# ---------------------------------------------------------------- events
ARRIVE = (27, 44)
a.warp(34, 32, f'MAP_WONDERWILD_PARK', 1)                  # 0: where the photo deck's steps bring you
a.warp(44, 38, f'MAP_WONDERWILD_PARK', 0)                  # 1: the photo deck's steps
a.obj('OBJ_EVENT_GFX_GENTLEMAN', 28, 8, f'{NAME}_EventScript_Director', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WOMAN_2', 25, 43, f'{NAME}_EventScript_Gate', 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_MAN_1', 35, 32, f'{NAME}_EventScript_CoasterHost', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_GIRL_2', 42, 21, f'{NAME}_EventScript_TeacupHost', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_BOY_2', 12, 20, f'{NAME}_EventScript_MazeHost', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SPECIES(CLEFAIRY)', *CLEFAIRY, f'{NAME}_EventScript_Mascot1', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_PARK_MASCOT_1')
a.obj('OBJ_EVENT_GFX_SPECIES(JIGGLYPUFF)', *JIGGLYPUFF, f'{NAME}_EventScript_Mascot2', 'MOVEMENT_TYPE_FACE_LEFT',
      'FLAG_PARK_MASCOT_2')
a.obj('OBJ_EVENT_GFX_SPECIES(TOGEPI)', *TOGEPI, f'{NAME}_EventScript_Mascot3', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_PARK_MASCOT_3', elevation=RIDE_ELEV)
for i, (sp, x, y) in enumerate((('CLEFAIRY', 24, 9), ('JIGGLYPUFF', 31, 9), ('TOGEPI', 26, 10))):
    a.obj(f'OBJ_EVENT_GFX_SPECIES({sp})', x, y, f'{NAME}_EventScript_Parade', 'MOVEMENT_TYPE_WALK_IN_PLACE_DOWN',
          'FLAG_HIDE_PARK_PARADE')
a.obj('OBJ_EVENT_GFX_LITTLE_GIRL', 13, 31, f'{NAME}_EventScript_Girl', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
a.obj('OBJ_EVENT_GFX_FAT_MAN', 21, 24, f'{NAME}_EventScript_Popcorn', 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_YOUNGSTER', 8, 23, f'{NAME}_EventScript_Timmy', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_LASS', 48, 22, f'{NAME}_EventScript_Poppy', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_SCHOOL_KID_M', 24, 31, f'{NAME}_EventScript_Remy', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_POKEFAN_F', 6, 31, f'{NAME}_EventScript_Gloria', 2, 'MOVEMENT_TYPE_FACE_DOWN')
a.item('ITEM_LEMONADE', 3, 26, 'FLAG_ITEM_PARK_LEMONADE')
a.item('ITEM_SUPER_POTION', MX + 1, MY + 1, 'FLAG_ITEM_PARK_SUPER_POTION')
a.item('ITEM_GREAT_BALL', 50, 6, 'FLAG_ITEM_PARK_GREAT_BALL')
a.item('ITEM_MOON_STONE', 48, 38, 'FLAG_ITEM_PARK_MOON_STONE')
a.hidden(16, 40, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_PARK_NUGGET')
a.hidden(50, 36, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_PARK_STAR_PIECE')
a.hidden(36, 9, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_PARK_RARE_CANDY')
a.sign(27, 23, f'{NAME}_EventScript_Statue')
a.sign(28, 23, f'{NAME}_EventScript_Statue')
a.sign(22, 41, f'{NAME}_EventScript_Booth')
a.sign(33, 41, f'{NAME}_EventScript_Booth')
a.sign(10, 30, f'{NAME}_EventScript_Carousel')


def mascot(i, sp, text):
    return rf'''{NAME}_EventScript_Mascot{i}::
	lock
	faceplayer
	playmoncry SPECIES_{sp}, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Mascot{i}, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_PARK_MASCOT_{i}
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_MascotGone, MSGBOX_DEFAULT
	release
	end

{NAME}_Text_Mascot{i}:
{text}
'''


def trainer(name, label, intro, defeat, post):
    return rf'''{NAME}_EventScript_{label}::
	trainerbattle_single TRAINER_PARK_{name}, {NAME}_Text_{label}Intro, {NAME}_Text_{label}Defeat
	msgbox {NAME}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{NAME}_Text_{label}Intro:
	.string "{intro}$"

{NAME}_Text_{label}Defeat:
	.string "{defeat}$"

{NAME}_Text_{label}Post:
	.string "{post}$"

'''


a.write(map_id='MAP_WONDERWILD_PARK', layout_id='LAYOUT_WONDERWILD_PARK', music='MUS_CYCLING', mapsec=MAPSEC,
        mapsec_name='WONDERWILD PARK', map_type='MAP_TYPE_TOWN', weather='WEATHER_SUNNY', allow_cycling=False,
        after='MetroOldLine', scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
	.byte 0

{NAME}_OnTransition:
	setflag FLAG_HIDE_PARK_PARADE
	goto_if_unset FLAG_PARK_PARADE, {NAME}_EventScript_NoParade
	clearflag FLAG_HIDE_PARK_PARADE
{NAME}_EventScript_NoParade::
	end

{NAME}_EventScript_Director::
	lock
	faceplayer
	goto_if_set FLAG_PARK_PARADE, {NAME}_EventScript_DirectorDone
	goto_if_unset FLAG_PARK_MASCOT_1, {NAME}_EventScript_DirectorAsk
	goto_if_unset FLAG_PARK_MASCOT_2, {NAME}_EventScript_DirectorAsk
	goto_if_unset FLAG_PARK_MASCOT_3, {NAME}_EventScript_DirectorAsk
	msgbox {NAME}_Text_DirectorThanks, MSGBOX_DEFAULT
	givemon SPECIES_TANDEMAUS, 18
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {NAME}_EventScript_DirectorFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotTandemaus, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_PARK_PARADE
	msgbox {NAME}_Text_DirectorParade, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	clearflag FLAG_HIDE_PARK_PARADE
	addobject 9
	addobject 10
	addobject 11
	playfanfare MUS_LEVEL_UP
	fadescreen FADE_FROM_BLACK
	waitfanfare
	release
	end

{NAME}_EventScript_DirectorFull::
	msgbox {NAME}_Text_DirectorFull, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_DirectorAsk::
	msgbox {NAME}_Text_DirectorAsk, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_DirectorDone::
	msgbox {NAME}_Text_DirectorDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Parade::
	msgbox {NAME}_Text_Parade, MSGBOX_NPC
	end

{NAME}_EventScript_Gate::
	lock
	faceplayer
	msgbox {NAME}_Text_GateBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_GateStay
	closemessage
	warp MAP_SLATEPORT_CITY, 17, 4
	waitstate
	release
	end

{NAME}_EventScript_GateStay::
	msgbox {NAME}_Text_GateStay, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_CoasterHost::
	lock
	faceplayer
	msgbox {NAME}_Text_CoasterAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_CoasterNo
	msgbox {NAME}_Text_CoasterGo, MSGBOX_DEFAULT
	closemessage
	playse SE_TRUCK_DOOR
	waitse
	warp MAP_WONDERWILD_PARK, 40, 30
	waitstate
	release
	end

{NAME}_EventScript_CoasterNo::
	msgbox {NAME}_Text_CoasterNo, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_TeacupHost::
	msgbox {NAME}_Text_TeacupHost, MSGBOX_NPC
	end

{NAME}_EventScript_MazeHost::
	msgbox {NAME}_Text_MazeHost, MSGBOX_NPC
	end

{NAME}_EventScript_Girl::
	msgbox {NAME}_Text_Girl, MSGBOX_NPC
	end

{NAME}_EventScript_Popcorn::
	msgbox {NAME}_Text_Popcorn, MSGBOX_NPC
	end

{NAME}_EventScript_Statue::
	msgbox {NAME}_Text_Statue, MSGBOX_SIGN
	end

{NAME}_EventScript_Booth::
	msgbox {NAME}_Text_Booth, MSGBOX_SIGN
	end

{NAME}_EventScript_Carousel::
	msgbox {NAME}_Text_Carousel, MSGBOX_SIGN
	end

{mascot(1, 'CLEFAIRY', '''	.string "Clefairy! It's been going round in\\n"
	.string "circles in here for hours.\\p"
	.string "CLEFAIRY hopped off back to the\\n"
	.string "castle!$"''')}{mascot(2, 'JIGGLYPUFF', '''	.string "Puuuff… JIGGLYPUFF is dizzy from\\n"
	.string "all the spinning.\\p"
	.string "JIGGLYPUFF floated back to the\\n"
	.string "castle!$"''')}{mascot(3, 'TOGEPI', '''	.string "Togepriii! TOGEPI was posing for\\n"
	.string "the ride photos.\\p"
	.string "TOGEPI toddled back to the castle!$"''')}{NAME}_Text_MascotGone:
	.string "That's one mascot found. Better tell\n"
	.string "the DIRECTOR.$"

{trainer("TIMMY", "Timmy", "I've been on every ride twice! Now\\nlet's battle!", "I'm dizzy…", "The maze is the best. Nobody finds\\nthe middle!")}{trainer("POPPY", "Poppy", "Teacups make me dizzy, but battles\\nmake me happy!", "Round and round I go…", "The arrows on the cups send you\\nflying! Look before you step.")}{trainer("REMY", "Remy", "School trip! Battle me while the\\nteacher's not looking!", "Detention…", "The coaster's last stop is a photo\\ndeck. You can only get there by\\lriding!")}{trainer("GLORIA", "Gloria", "My babies want to see the parade!\\nAnd beat you, of course.", "My little darlings!", "No mascots, no parade. Have you\\nseen them?")}{NAME}_Text_DirectorAsk:
	.string "Welcome to WONDERWILD PARK! I'm the\n"
	.string "DIRECTOR.\p"
	.string "Our mascots CLEFAIRY, JIGGLYPUFF and\n"
	.string "TOGEPI ran off before the parade!\p"
	.string "One went into the hedge maze, one\n"
	.string "onto the teacups, and one rode the\l"
	.string "coaster. Please, find them!$"

{NAME}_Text_DirectorThanks:
	.string "All three mascots are home! The\n"
	.string "parade can go on!\p"
	.string "Here, this TANDEMAUS family has been\n"
	.string "helping backstage. They'd love to\l"
	.string "travel with you.$"

{NAME}_Text_GotTandemaus:
	.string "{{PLAYER}} received a TANDEMAUS!$"

{NAME}_Text_DirectorFull:
	.string "Your party's full! Come back for\n"
	.string "TANDEMAUS when you have room.$"

{NAME}_Text_DirectorParade:
	.string "Let the parade begin!$"

{NAME}_Text_DirectorDone:
	.string "The parade's the best it's ever\n"
	.string "been. Thank you!$"

{NAME}_Text_Parade:
	.string "The mascots are dancing in the\n"
	.string "parade!$"

{NAME}_Text_GateBack:
	.string "Leaving the park? The tram goes\n"
	.string "back to SLATEPORT CITY.$"

{NAME}_Text_GateStay:
	.string "Enjoy the rides!$"

{NAME}_Text_CoasterAsk:
	.string "This is the RAPIDASH RACER, the\n"
	.string "fastest coaster in HOENN!\p"
	.string "Want a ride?$"

{NAME}_Text_CoasterGo:
	.string "Keep your arms inside the car!$"

{NAME}_Text_CoasterNo:
	.string "Chicken!$"

{NAME}_Text_TeacupHost:
	.string "Step on a teacup and it'll spin you\n"
	.string "off the way its arrow points, until\l"
	.string "you land on a gold plate.$"

{NAME}_Text_MazeHost:
	.string "The hedge maze! Some say there's a\n"
	.string "present tucked in the far corner.$"

{NAME}_Text_Girl:
	.string "I want to ride the carousel\n"
	.string "again!$"

{NAME}_Text_Popcorn:
	.string "Popcorn! Get your popcorn!\n"
	.string "…Sorry, I've just sold out.$"

{NAME}_Text_Statue:
	.string "WONDERWILD PARK\n"
	.string "Where every POKéMON is a star!$"

{NAME}_Text_Booth:
	.string "TICKETS\n"
	.string "Free entry for trainers today!$"

{NAME}_Text_Carousel:
	.string "The carousel is turning slowly.\n"
	.string "The music box plays a merry tune.$"
''')
print('ok', NAME)

# ---------------------------------------------------------------- the way there, from Slateport City
add_object('SlateportCity', {'graphics_id': 'OBJ_EVENT_GFX_WOMAN_2', 'x': 17, 'y': 3, 'elevation': 3,
                             'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                             'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                             'script': 'SlateportCity_EventScript_ParkGuide', 'flag': '0'})
register_script_file('park', rf'''
@ Wonderwild Park guide in Slateport City (tools/areas/park.py).
SlateportCity_EventScript_ParkGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE02_GET, SlateportCity_EventScript_ParkNoBadge
	msgbox SlateportCity_Text_ParkAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_ParkNo
	msgbox SlateportCity_Text_ParkGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_WONDERWILD_PARK, {ARRIVE[0]}, {ARRIVE[1]}
	waitstate
	release
	end

SlateportCity_EventScript_ParkNoBadge::
	msgbox SlateportCity_Text_ParkNoBadge, MSGBOX_DEFAULT
	release
	end

SlateportCity_EventScript_ParkNo::
	msgbox SlateportCity_Text_ParkNo, MSGBOX_DEFAULT
	release
	end

SlateportCity_Text_ParkNoBadge:
	.string "The tram to WONDERWILD PARK is for\n"
	.string "trainers with two badges.\p"
	.string "The rides are a little wild for\n"
	.string "beginners!$"

SlateportCity_Text_ParkAsk:
	.string "Two badges! Fancy a trip to\n"
	.string "WONDERWILD PARK? Roller coasters,\l"
	.string "teacups, a hedge maze…\p"
	.string "Hop on the tram?$"

SlateportCity_Text_ParkGo:
	.string "Have fun!$"

SlateportCity_Text_ParkNo:
	.string "The tram runs all day.$"
''')
print('ok entrance')
