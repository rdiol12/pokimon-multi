"""Ironmoor Prison, for after the fourth badge: the ruins of an old island
prison off Route 109, where the ghosts of its guards still keep watch.

A boatman on the Route 109 beach rows you out to Ironmoor Isle. The main
gate is rusted shut, but the wall has fallen in by the old graveyard.
Inside, the watchtowers' searchlights still sweep the yard: step into a
beam and the ghost guards' whistles send you back to the breach. Cut the
power at the generator and the lights go dark, so you can cross to the cell
block (and knock the gate's bolt loose from inside, a shortcut back to the
landing).

In the cell block, the warden's desk holds the key to the iron gate, and
the console in the cell hall opens every cell. Four spirits are bound in
the cells as ghost-fire; set them free and the Odd Keystone in solitary,
below, cracks open: Spiritomb (Lv32) rises from it. An old sword on the
armoury rack turns out to be a Honedge, and it comes with you.

New art (the prison wall and the breach, the portcullis, watchtowers and
searchlight beams, the generator, the cell block front, the pier, cell bars
and gates, bunks, chains, the spirits, the keystone, the sword rack and the
warden's desk) is drawn in tools/areas/prison_art.py (gTileset_Prison: the
Mt. Pyre tiles plus the art).

    python3 tools/areas/prison_art.py   # the tileset, when the art changes
    python3 tools/areas/prison.py
"""
import json, sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import prison

AREA_NO = 21
NAME = 'IronmoorIsle'
CELLS = 'IronmoorPrison'
SOLITARY = 'IronmoorPrison_Solitary'
MAPSEC = 'MAPSEC_IRONMOOR_PRISON'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'prison_tiles.json')))
MT = ART['objects']

register_constants('Ironmoor', AREA_NO, flags=[
    'IRONMOOR_POWER_OFF', 'IRONMOOR_GATE_OPEN', 'IRONMOOR_KEY', 'IRONMOOR_CORRIDOR', 'IRONMOOR_CELLS',
    'IRONMOOR_SPIRIT_1', 'IRONMOOR_SPIRIT_2', 'IRONMOOR_SPIRIT_3', 'IRONMOOR_SPIRIT_4',
    'IRONMOOR_SPIRITOMB_DONE', 'IRONMOOR_HONEDGE',
    'ITEM_IRONMOOR_SPELL_TAG', 'ITEM_IRONMOOR_DUSK_BALL', 'ITEM_IRONMOOR_REAPER_CLOTH', 'ITEM_IRONMOOR_TM_SHADOW_BALL',
    'ITEM_IRONMOOR_FULL_HEAL', 'ITEM_IRONMOOR_RARE_CANDY', 'ITEM_IRONMOOR_DUSK_STONE', 'ITEM_IRONMOOR_MAX_ETHER',
    'HIDDEN_ITEM_IRONMOOR_NUGGET', 'HIDDEN_ITEM_IRONMOOR_PEARL', 'HIDDEN_ITEM_IRONMOOR_ULTRA_BALL',
    'HIDDEN_ITEM_IRONMOOR_CLEANSE_TAG',
], trainers=['IRONMOOR_GARRICK', 'IRONMOOR_KAITO', 'IRONMOOR_MORWEN', 'IRONMOOR_VESPER', 'IRONMOOR_SELMA'])

register_trainers('Ironmoor', '''
=== AREA_TRAINER_IRONMOOR_GARRICK ===
Name: GARRICK
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Baltoy
Level: 28

Golett
Level: 28

Sandslash
Level: 29

=== AREA_TRAINER_IRONMOOR_KAITO ===
Name: KAITO
Class: Ninja Boy
Pic: Ninja Boy
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Sableye
Level: 28

Ninjask
Level: 29

=== AREA_TRAINER_IRONMOOR_MORWEN ===
Name: MORWEN
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Shuppet
Level: 29

Haunter
Level: 29

Misdreavus
Level: 30

=== AREA_TRAINER_IRONMOOR_VESPER ===
Name: VESPER
Class: Psychic
Pic: Psychic M
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Kadabra
Level: 29

Kirlia
Level: 30

=== AREA_TRAINER_IRONMOOR_SELMA ===
Name: SELMA
Class: Hex Maniac
Pic: Hex Maniac
Gender: Female
Music: Suspicious
Double Battle: No
AI: Basic Trainer

Golett
Level: 30

Lampent
Level: 31

Banette
Level: 31
''')


def place(a, name, x, y, blocked=(), elevation=3):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    cells = []
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                open_ = (r, c) in walk and (r, c) not in blocked
                a.tile(x + c, y + r, mt, collision=0 if open_ else 1, elevation=elevation, c='o')
                cells.append((x + c, y + r))
    return cells


def one(name, r=0, c=0):
    return MT[name][r][c]


def swap(name, x, y, walk=None):
    """setmetatile lines that draw object `name` at x, y (walk: the cells
    left open, (row, col) set; default: the object's own walkable cells)."""
    open_ = {tuple(rc) for rc in ART['walkable'][name]} if walk is None else walk
    return ''.join(f'\tsetmetatile {x + c}, {y + r}, {mt}, {"FALSE" if (r, c) in open_ else "TRUE"}\n'
                   for r, row in enumerate(MT[name]) for c, mt in enumerate(row) if mt >= 0)


def text(*lines):
    """Lines of one message: \\n and \\l alternate, \\p where a line is ''."""
    out, n = [], 0
    for i, l in enumerate(lines):
        if l == '':
            out[-1] = out[-1][:-3] + '\\p"'
            n = 0
            continue
        end = '$' if i == len(lines) - 1 else ('\\n' if n == 0 else '\\l')
        out.append(f'\t.string "{l}{end}"')
        n += 1
    return '\n'.join(out) + '\n'


def trainer_script(m, key, intro, defeat, post):
    label = key.title()
    return rf'''{m}_EventScript_{label}::
	trainerbattle_single TRAINER_IRONMOOR_{key}, {m}_Text_{label}Intro, {m}_Text_{label}Defeat
	msgbox {m}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{m}_Text_{label}Intro:
{intro}
{m}_Text_{label}Defeat:
{defeat}
{m}_Text_{label}Post:
{post}
'''


ENC_LAND = [('SHUPPET', 27, 29), ('DUSKULL', 27, 29), ('MURKROW', 27, 29), ('HOUNDOUR', 27, 29),
            ('MIGHTYENA', 28, 30), ('GOLBAT', 28, 30), ('GREAVARD', 28, 30), ('PHANTUMP', 28, 30),
            ('SABLEYE', 28, 30), ('MISDREAVUS', 29, 30), ('HAUNTER', 30, 31), ('ABSOL', 30, 31)]
ENC_INSIDE = [('GASTLY', 27, 29), ('DUSKULL', 27, 29), ('SHUPPET', 27, 29), ('GOLBAT', 28, 30),
              ('LITWICK', 27, 29), ('SABLEYE', 28, 30), ('SINISTEA', 28, 30), ('GOLETT', 28, 30),
              ('HAUNTER', 29, 31), ('MISDREAVUS', 29, 31), ('HAUNTER', 30, 31), ('SPIRITOMB', 30, 31)]

# ---------------------------------------------------------------- the island
W, H = 46, 50
a = Area(NAME, W, H, prison(), fill='#')
a.rect(13, 38, 20, 8, '.')                      # the landing
a.line([(22, 38), (22, 30)], 3, '.')            # up to the gate
a.rect(14, 24, 18, 7, '.')                      # before the gate
a.rect(3, 24, 9, 14, '.')                       # the old graveyard
a.line([(11, 36), (14, 40)], 2, '.')
a.ellipse(38, 33, 5, 6, '"')                    # the east slope
a.rect(31, 27, 5, 3, '.')
a.rect(7, 7, 32, 15, '.')                       # the yard
a.rect(17, 3, 11, 4, '.')                       # before the cell block
a.ellipse(14, 15, 3, 1.5, '"')
a.ellipse(30, 15, 3, 1.5, '"')
a.ellipse(26, 41, 3, 2, '"')
for x in range(W):                              # the sea round the landing
    for y in range(46, H):
        a.put(x, y, '#')

WALL_Y = 22
BREACH_X = 8
GATE_X = 21
TOWERS = [(8, 7), (36, 7)]
BEAM_ROWS = (12, 17)
GEN = (35, 19)
BLOCK = (18, 1)
ARRIVE = (22, 45)

b0 = a.blocks()                                 # the slopes; the art goes on top
for x in range(6, 40):
    place(a, 'wall', x, WALL_Y)
place(a, 'breach', BREACH_X, WALL_Y)
place(a, 'gate', GATE_X, WALL_Y)
for tx, ty in TOWERS:
    place(a, 'tower_on', tx, ty)
place(a, 'gen_on', *GEN)
place(a, 'block', *BLOCK)
for x in range(W):
    for y in range(46, H):
        a.tile(x, y, one('sea'), collision=1, c='o')
for y in range(43, H):
    a.tile(ARRIVE[0], y, one('pier'), collision=0, c='o')
    a.tile(ARRIVE[0] + 1, y, one('pier'), collision=0, c='o')
for x in range(13, 33):                         # a stone quay along the landing
    if x not in (ARRIVE[0], ARRIVE[0] + 1):
        a.tile(x, 46, one('wall', 1), collision=1, c='o')

# graves in the old graveyard (Mt. Pyre's gravestone)
GRAVE = 0x325
GRAVES = [(4, 26), (6, 26), (8, 26), (4, 29), (6, 29), (8, 29), (4, 32), (8, 32), (10, 35)]
for gx, gy in GRAVES:
    a.tile(gx, gy, GRAVE, collision=1, c='o')

a.write(blocks=a.blocks(), map_id='MAP_IRONMOOR_ISLE', layout_id='LAYOUT_IRONMOOR_ISLE', music='MUS_MT_PYRE_EXTERIOR',
        mapsec=MAPSEC, mapsec_name='IRONMOOR PRISON', weather='WEATHER_SHADE', after='VerdantRiver_Hut',
        scripts='')  # written again below, once the beam cells are known
b = a.last_blocks


def walkable(x, y):
    return 0 <= x < W and 0 <= y < H and not (int(b[y, x]) >> 10 & 3)


BEAMS = [(x, y) for y in BEAM_ROWS for x in range(7, 39) if walkable(x, y)]
for x, y in BEAMS:
    a.tile(x, y, one('beam'), collision=0, c='o')
    a.coord(x, y, f'{NAME}_EventScript_Spotted', var='VAR_TEMP_1', var_value='0')
ground = int(b[20, 20]) & 0x3FF                  # plain yard ground, under the beams once the power's off

a.warp(BLOCK[0] + 4, BLOCK[1] + 4, 'MAP_IRONMOOR_PRISON', 0)
a.obj('OBJ_EVENT_GFX_OLD_MAN', ARRIVE[0] - 1, ARRIVE[1], f'{NAME}_EventScript_Boatman', 'MOVEMENT_TYPE_FACE_RIGHT')
a.obj('OBJ_EVENT_GFX_SPECIES(DUSKULL)', 28, 9, f'{NAME}_EventScript_Duskull', 'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT', rx=3)
a.sign(GATE_X + 1, WALL_Y, f'{NAME}_EventScript_GateInside', 'BG_EVENT_PLAYER_FACING_SOUTH')
a.sign(GATE_X + 1, WALL_Y + 1, f'{NAME}_EventScript_GateOutside', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(GEN[0], GEN[1] + 1, f'{NAME}_EventScript_Generator', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(GEN[0] + 1, GEN[1] + 1, f'{NAME}_EventScript_Generator', 'BG_EVENT_PLAYER_FACING_NORTH')
a.sign(17, 30, f'{NAME}_EventScript_Sign')
a.trainer('OBJ_EVENT_GFX_HIKER', 18, 27, f'{NAME}_EventScript_Garrick', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
a.trainer('OBJ_EVENT_GFX_NINJA_BOY', 6, 34, f'{NAME}_EventScript_Kaito', 3, 'MOVEMENT_TYPE_FACE_UP')
a.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 22, 15, f'{NAME}_EventScript_Morwen', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_SPELL_TAG', 4, 36, 'FLAG_ITEM_IRONMOOR_SPELL_TAG')
a.item('ITEM_DUSK_BALL', 40, 30, 'FLAG_ITEM_IRONMOOR_DUSK_BALL')
a.item('ITEM_FULL_HEAL', 10, 9, 'FLAG_ITEM_IRONMOOR_FULL_HEAL')
a.hidden(6, 29 + 1, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_IRONMOOR_NUGGET')
a.hidden(31, 44, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_IRONMOOR_PEARL')

power_off = (f'{NAME}_EventScript_LightsOut::\n'
             + ''.join(f'\tsetmetatile {x}, {y}, {ground}, FALSE\n' for x, y in BEAMS)
             + ''.join(swap('tower_off', tx, ty) for tx, ty in TOWERS)
             + swap('gen_off', *GEN) + '\treturn\n\n')
gate_open = f'{NAME}_EventScript_GateUp::\n' + swap('gate_open', GATE_X, WALL_Y) + '\treturn\n\n'
BREACH_IN = (BREACH_X, WALL_Y - 1)

a.write(blocks=a.blocks(), map_id='MAP_IRONMOOR_ISLE', layout_id='LAYOUT_IRONMOOR_ISLE', music='MUS_MT_PYRE_EXTERIOR',
        mapsec=MAPSEC, mapsec_name='IRONMOOR PRISON', weather='WEATHER_SHADE', after='VerdantRiver_Hut',
        scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
	.byte 0

{NAME}_OnLoad:
	call_if_set FLAG_IRONMOOR_POWER_OFF, {NAME}_EventScript_LightsOut
	call_if_set FLAG_IRONMOOR_GATE_OPEN, {NAME}_EventScript_GateUp
	end

{NAME}_OnTransition:
	call_if_set FLAG_IRONMOOR_POWER_OFF, {NAME}_EventScript_BeamsQuiet
	end

{NAME}_EventScript_BeamsQuiet::
	setvar VAR_TEMP_1, 1
	return

{power_off}{gate_open}{NAME}_EventScript_Spotted::
	lockall
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	msgbox {NAME}_Text_Spotted, MSGBOX_DEFAULT
	closemessage
	playse SE_WARP_IN
	warpsilent MAP_IRONMOOR_ISLE, {BREACH_IN[0]}, {BREACH_IN[1]}
	waitstate
	releaseall
	end

{NAME}_EventScript_Generator::
	lockall
	goto_if_set FLAG_IRONMOOR_POWER_OFF, {NAME}_EventScript_GeneratorOff
	msgbox {NAME}_Text_GeneratorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_Leave
	playse SE_PIN
	delay 20
	playse SE_BANG
	call {NAME}_EventScript_LightsOut
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_POWER_OFF
	setvar VAR_TEMP_1, 1
	delay 30
	msgbox {NAME}_Text_GeneratorDone, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_GeneratorOff::
	msgbox {NAME}_Text_GeneratorIsOff, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_Leave::
	releaseall
	end

{NAME}_EventScript_GateInside::
	lockall
	goto_if_set FLAG_IRONMOOR_GATE_OPEN, {NAME}_EventScript_Leave
	msgbox {NAME}_Text_GateBolt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_Leave
	playse SE_BANG
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_DOOR
	call {NAME}_EventScript_GateUp
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_GATE_OPEN
	msgbox {NAME}_Text_GateUp, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_GateOutside::
	goto_if_set FLAG_IRONMOOR_GATE_OPEN, {NAME}_EventScript_Leave
	msgbox {NAME}_Text_GateShut, MSGBOX_SIGN
	end

{NAME}_EventScript_Boatman::
	lock
	faceplayer
	msgbox {NAME}_Text_BoatmanBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_BoatmanStay
	msgbox {NAME}_Text_BoatmanGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROUTE109, 17, 22
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

{NAME}_EventScript_Duskull::
	lock
	faceplayer
	playmoncry SPECIES_DUSKULL, CRY_MODE_NORMAL
	msgbox {NAME}_Text_Duskull, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{trainer_script(NAME, 'GARRICK', text("Ironmoor was a fortress before it was", "a prison. I'm here for its history!"),
                text("History repeats… my losses."), text("The gate's rusted shut. The wall fell", "in by the graveyard, though."))}
{trainer_script(NAME, 'KAITO', text("I train among the graves. Nothing", "scares a ninja!"),
                text("Eek! I mean… well fought."), text("The searchlights in the yard still", "work. Nobody knows who runs them."))}
{trainer_script(NAME, 'MORWEN', text("The guards never left, you know.", "They still watch the yard…"),
                text("The spirits laugh at me…"), text("The lights run off the generator in", "the corner of the yard."))}
{NAME}_Text_Spotted:
{text("A searchlight swept over you!", "", "Ghostly whistles shriek across the", "yard, and cold hands drag you back", "to the breach…")}
{NAME}_Text_GeneratorAsk:
{text("An old generator, still humming.", "Cables run up to both watchtowers.", "", "Pull the switch?")}
{NAME}_Text_GeneratorDone:
{text("The generator shuddered to a stop.", "", "The searchlights went dark!")}
{NAME}_Text_GeneratorIsOff:
{text("The generator is silent.")}
{NAME}_Text_GateBolt:
{text("The gate's bolt is rusted in place,", "but from this side you could knock", "it loose.", "", "Strike the bolt?")}
{NAME}_Text_GateUp:
{text("The bolt gave way, and the portcullis", "groaned up into the arch!")}
{NAME}_Text_GateShut:
{text("The gate is rusted shut.")}
{NAME}_Text_BoatmanBack:
{text("Seen enough ghosts? I'll row you", "back to ROUTE 109.")}
{NAME}_Text_BoatmanGo:
{text("Mind the fog.")}
{NAME}_Text_BoatmanStay:
{text("I'll be here. I don't go ashore.")}
{NAME}_Text_Sign:
{text("IRONMOOR PRISON", "Closed by order of the HOENN courts.")}
{NAME}_Text_Duskull:
{text("DUSKULL: Duuu…", "", "It's drifting along the yard like a", "guard on patrol.")}
''')
print('ok', NAME)
register_wild('MAP_IRONMOOR_ISLE', 'gIronmoorIsle', land=ENC_LAND, land_rate=15)

# ---------------------------------------------------------------- inside: hand-built stone halls
VOID, WALL_T, WALL_B, FLOOR, FLOOR_SH = 0x201, 0x2EF, 0x2F7, 0x2E6, 0x2E7
EXIT_L, EXIT_R = 0x206, 0x207
STAIRS_DOWN = [[0x2C3, 0x2C4, 0x2C5], [0x2CB, 0x2CC, 0x2CD]]
STAIRS_UP = [[0x2B3, 0x2B4, 0x2B5], [0x2BB, 0x2BC, 0x2BD]]


class Halls:
    """Rooms carved out of the dark: open cells get the stone floor, and
    the two cells above any open cell under the dark become wall."""

    def __init__(self, area):
        self.a, self.w, self.h = area, area.w, area.h
        self.open = np.zeros((self.h, self.w), bool)
        self.fixed = {}

    def room(self, x0, y0, x1, y1):
        self.open[y0:y1 + 1, x0:x1 + 1] = True

    def set(self, x, y, mt, walk):
        self.fixed[(x, y)] = (mt, walk)

    def obj(self, name, x, y, walk=None):
        w = {tuple(rc) for rc in ART['walkable'][name]} if walk is None else walk
        for r, row in enumerate(MT[name]):
            for c, mt in enumerate(row):
                if mt >= 0:
                    self.set(x + c, y + r, mt, (r, c) in w)

    def build(self):
        tiles = {}
        for y in range(self.h):
            for x in range(self.w):
                if self.open[y, x]:
                    tiles[(x, y)] = (FLOOR, True)
        for y in range(self.h):
            for x in range(self.w):
                if self.open[y, x] and y >= 1 and not self.open[y - 1, x] and (x, y - 1) not in self.fixed:
                    tiles[(x, y - 1)] = (WALL_B, False)
                    if y >= 2 and not self.open[y - 2, x]:
                        tiles[(x, y - 2)] = (WALL_T, False)
        tiles.update(self.fixed)
        for (x, y), (mt, walk) in list(tiles.items()):
            if mt == FLOOR and tiles.get((x, y - 1), (None,))[0] in (WALL_B, *[m for r in STAIRS_DOWN + STAIRS_UP for m in r]):
                tiles[(x, y)] = (FLOOR_SH, True)
        for y in range(self.h):
            for x in range(self.w):
                mt, walk = tiles.get((x, y), (VOID, False))
                self.a.tile(x, y, mt, collision=0 if walk else 1, c='o')


def inside_style():
    st = prison()
    return st


# ---------------------------------------------------------------- the cell block
c = Area(CELLS, 30, 24, inside_style(), fill='#')
hc = Halls(c)
hc.room(2, 5, 27, 8)                            # the cell hall
NORTH = [5, 9, 13, 17, 21]                      # north cells: x .. x+2, floor rows 1-2
for x in NORTH:
    hc.room(x, 1, x + 2, 2)
SOUTH = [2, 7, 19, 24]                          # south cells: floor rows 11-12
for x in SOUTH:
    hc.room(x, 11, x + 2, 12)
hc.room(13, 11, 16, 14)                         # the corridor up from the entrance
hc.room(11, 15, 18, 21)                         # the entrance hall
hc.room(2, 16, 8, 20)                           # the guard room
hc.room(9, 19, 10, 20)
hc.room(21, 16, 27, 20)                         # the warden's office and armoury
hc.room(19, 19, 20, 20)
for x in NORTH:                                 # cell fronts: bars with a gate in the middle
    for dx in range(3):
        hc.obj('cell_gate' if dx == 1 else 'bars', x + dx, 3)
    for wx in (x - 1, x + 3):
        hc.set(wx, 1, WALL_T, False)
        hc.set(wx, 2, WALL_B, False)
        hc.set(wx, 3, WALL_T, False)
        hc.set(wx, 4, WALL_B, False)
for x in SOUTH:
    for dx in range(3):
        hc.obj('cell_gate' if dx == 1 else 'bars', x + dx, 9)
for dx, name in enumerate(('bars', 'cell_gate', 'cell_gate', 'bars')):
    hc.obj(name, 13 + dx, 9)                    # the iron gate across the corridor
hc.obj('gen_off', 2, 3)                         # the cell console
hc.set(4, 3, WALL_T, False)
hc.set(4, 4, WALL_B, False)
for r, row in enumerate(STAIRS_DOWN):           # down to solitary
    for col, mt in enumerate(row):
        hc.set(25 + col, 3 + r, mt, (r, col) == (1, 1))
hc.set(14, 22, EXIT_L, True)
hc.set(15, 22, EXIT_R, True)
SPIRITS = [(10, 1), (22, 2), (3, 12), (25, 11)]  # one bound spirit in each of four cells
for x, y in SPIRITS:
    hc.obj('wisp', x, y)
hc.obj('bunk', 5, 1)
hc.obj('bunk', 13, 1)
hc.obj('bunk', 7, 12)
hc.obj('bunk', 19, 12)
for x in (6, 15, 18):
    hc.obj('chains', x, 2 if x != 6 else 1)
hc.obj('desk', 22, 17)
hc.obj('sword_rack', 26, 16)
hc.obj('bunk', 3, 16)
hc.obj('chains', 7, 16)
hc.build()

cell_open = lambda x, y: swap('cell_gate_open', x, y)
CELL_GATES = [(x + 1, 3) for x in NORTH] + [(x + 1, 9) for x in SOUTH]
open_cells = f'{CELLS}_EventScript_CellsOpen::\n' + ''.join(cell_open(x, y) for x, y in CELL_GATES) \
    + swap('gen_on', 2, 3) + '\treturn\n\n'
open_corridor = f'{CELLS}_EventScript_CorridorOpen::\n' + cell_open(14, 9) + cell_open(15, 9) + '\treturn\n\n'
free_spirit = ''.join(
    f'{CELLS}_EventScript_SpiritGone{i}::\n\tsetmetatile {x}, {y}, {FLOOR}, FALSE\n\treturn\n\n'
    for i, (x, y) in enumerate(SPIRITS, 1))
spirit_load = ''.join(f'\tcall_if_set FLAG_IRONMOOR_SPIRIT_{i}, {CELLS}_EventScript_SpiritGone{i}\n'
                      for i in range(1, 5))

c.warp(14, 22, 'MAP_IRONMOOR_ISLE', 0)
c.warp(15, 22, 'MAP_IRONMOOR_ISLE', 0)
c.warp(26, 4, 'MAP_IRONMOOR_PRISON_SOLITARY', 0)
for i, (x, y) in enumerate(SPIRITS, 1):
    c.sign(x, y, f'{CELLS}_EventScript_Spirit{i}')
for x, y in CELL_GATES:
    c.sign(x, y + (1 if y == 3 else 0), f'{CELLS}_EventScript_CellGate')
c.sign(14, 10, f'{CELLS}_EventScript_CorridorGate')
c.sign(15, 10, f'{CELLS}_EventScript_CorridorGate')
c.sign(2, 4, f'{CELLS}_EventScript_Console')
c.sign(3, 4, f'{CELLS}_EventScript_Console')
c.sign(23, 18, f'{CELLS}_EventScript_Desk', 'BG_EVENT_PLAYER_FACING_NORTH')
c.sign(26, 17, f'{CELLS}_EventScript_Rack', 'BG_EVENT_PLAYER_FACING_NORTH')
c.sign(27, 17, f'{CELLS}_EventScript_Rack', 'BG_EVENT_PLAYER_FACING_NORTH')
c.trainer('OBJ_EVENT_GFX_PSYCHIC_M', 5, 19, f'{CELLS}_EventScript_Vesper', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
c.obj('OBJ_EVENT_GFX_SPECIES(SHUPPET)', 20, 6, f'{CELLS}_EventScript_Shuppet', 'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT', rx=4)
c.item('ITEM_RARE_CANDY', 23, 1, 'FLAG_ITEM_IRONMOOR_RARE_CANDY')
c.item('ITEM_REAPER_CLOTH', 9, 1, 'FLAG_ITEM_IRONMOOR_REAPER_CLOTH')
c.item('ITEM_DUSK_STONE', 21, 11, 'FLAG_ITEM_IRONMOOR_DUSK_STONE')
c.item('ITEM_MAX_ETHER', 2, 18, 'FLAG_ITEM_IRONMOOR_MAX_ETHER')
c.hidden(4, 11, 'ITEM_ULTRA_BALL', 'FLAG_HIDDEN_ITEM_IRONMOOR_ULTRA_BALL')

c.write(blocks=c.blocks(), map_id='MAP_IRONMOOR_PRISON', layout_id='LAYOUT_IRONMOOR_PRISON', music='MUS_MT_PYRE',
        mapsec=MAPSEC, mapsec_name='IRONMOOR PRISON', map_type='MAP_TYPE_INDOOR', after=NAME, show_name=True,
        allow_cycling=False, scripts=rf'''
{CELLS}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {CELLS}_OnLoad
	.byte 0

{CELLS}_OnLoad:
	call_if_set FLAG_IRONMOOR_CORRIDOR, {CELLS}_EventScript_CorridorOpen
	call_if_set FLAG_IRONMOOR_CELLS, {CELLS}_EventScript_CellsOpen
{spirit_load}	end

{open_cells}{open_corridor}{free_spirit}{CELLS}_EventScript_CorridorGate::
	lockall
	goto_if_set FLAG_IRONMOOR_CORRIDOR, {CELLS}_EventScript_Leave
	goto_if_unset FLAG_IRONMOOR_KEY, {CELLS}_EventScript_CorridorLocked
	msgbox {CELLS}_Text_CorridorKey, MSGBOX_DEFAULT
	playse SE_UNLOCK
	call {CELLS}_EventScript_CorridorOpen
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_CORRIDOR
	delay 20
	msgbox {CELLS}_Text_CorridorOpen, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_CorridorLocked::
	msgbox {CELLS}_Text_CorridorLocked, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_Leave::
	releaseall
	end

{CELLS}_EventScript_CellGate::
	goto_if_set FLAG_IRONMOOR_CELLS, {CELLS}_EventScript_Leave
	msgbox {CELLS}_Text_CellLocked, MSGBOX_SIGN
	end

{CELLS}_EventScript_Console::
	lockall
	goto_if_set FLAG_IRONMOOR_CELLS, {CELLS}_EventScript_ConsoleDone
	msgbox {CELLS}_Text_ConsoleAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {CELLS}_EventScript_Leave
	playse SE_PIN
	delay 20
	playse SE_DOOR
	call {CELLS}_EventScript_CellsOpen
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_CELLS
	delay 30
	msgbox {CELLS}_Text_ConsoleDone, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_ConsoleDone::
	msgbox {CELLS}_Text_ConsoleIsDone, MSGBOX_DEFAULT
	releaseall
	end

{''.join(f"""{CELLS}_EventScript_Spirit{i}::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRIT_{i}, {CELLS}_EventScript_Leave
	msgbox {CELLS}_Text_Spirit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {CELLS}_EventScript_Leave
	playse SE_M_CONFUSE_RAY
	delay 40
	call {CELLS}_EventScript_SpiritGone{i}
	special DrawWholeMapView
	setflag FLAG_IRONMOOR_SPIRIT_{i}
	goto {CELLS}_EventScript_SpiritFree

""" for i in range(1, 5))}{CELLS}_EventScript_SpiritFree::
	goto_if_unset FLAG_IRONMOOR_SPIRIT_1, {CELLS}_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_2, {CELLS}_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_3, {CELLS}_EventScript_SpiritMore
	goto_if_unset FLAG_IRONMOOR_SPIRIT_4, {CELLS}_EventScript_SpiritMore
	msgbox {CELLS}_Text_SpiritsAll, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_SpiritMore::
	msgbox {CELLS}_Text_SpiritOne, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_Desk::
	lockall
	goto_if_set FLAG_IRONMOOR_KEY, {CELLS}_EventScript_DeskDone
	msgbox {CELLS}_Text_Desk, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	msgbox {CELLS}_Text_GotKey, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_IRONMOOR_KEY
	msgbox {CELLS}_Text_KeyHint, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_DeskDone::
	msgbox {CELLS}_Text_DeskDone, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_Rack::
	lockall
	goto_if_set FLAG_IRONMOOR_HONEDGE, {CELLS}_EventScript_RackEmpty
	msgbox {CELLS}_Text_Rack, MSGBOX_DEFAULT
	playmoncry SPECIES_HONEDGE, CRY_MODE_NORMAL
	msgbox {CELLS}_Text_RackMoves, MSGBOX_DEFAULT
	waitmoncry
	givemon SPECIES_HONEDGE, 25
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {CELLS}_EventScript_RackFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {CELLS}_Text_GotHonedge, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_IRONMOOR_HONEDGE
	releaseall
	end

{CELLS}_EventScript_RackFull::
	msgbox {CELLS}_Text_RackFull, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_RackEmpty::
	msgbox {CELLS}_Text_RackEmpty, MSGBOX_DEFAULT
	releaseall
	end

{CELLS}_EventScript_Shuppet::
	lock
	faceplayer
	playmoncry SPECIES_SHUPPET, CRY_MODE_NORMAL
	msgbox {CELLS}_Text_Shuppet, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{trainer_script(CELLS, 'VESPER', text("I came to speak with the spirits.", "They told me you'd come."),
                text("They didn't tell me this part…"), text("Four spirits are bound in the cells.", "They long to be free."))}
{CELLS}_Text_CorridorLocked:
{text("A heavy iron gate. It's locked.", "", "The warden must have kept the key", "somewhere.")}
{CELLS}_Text_CorridorKey:
{text("{PLAYER} tried the WARDEN'S KEY…")}
{CELLS}_Text_CorridorOpen:
{text("Click! The iron gate swung open.")}
{CELLS}_Text_CellLocked:
{text("The cell is locked. A console must", "open the cells.")}
{CELLS}_Text_ConsoleAsk:
{text("A rusty console. Its brass plate", "reads CELLS: RELEASE ALL.", "", "Pull the lever?")}
{CELLS}_Text_ConsoleDone:
{text("Every cell door clanged open at once!")}
{CELLS}_Text_ConsoleIsDone:
{text("The lever is pulled. The cells stand", "open.")}
{CELLS}_Text_Spirit:
{text("A cold blue flame flickers in the", "cell. It seems to be held here", "against its will…", "", "Reach out to it?")}
{CELLS}_Text_SpiritOne:
{text("The flame brushed past you and", "drifted away, down toward solitary.")}
{CELLS}_Text_SpiritsAll:
{text("The last flame drifted down toward", "solitary.", "", "From below, something cracked like", "splitting stone…")}
{CELLS}_Text_Desk:
{text("The warden's desk. His journal is", "open on it:", "", "“The spirits grow restless. I have", "bound them to the cells and sealed", "their master in the keystone below.”", "", "There's a key in the drawer.")}
{CELLS}_Text_GotKey:
{text("{PLAYER} found the WARDEN'S KEY!")}
{CELLS}_Text_KeyHint:
{text("It's stamped “CORRIDOR GATE”.")}
{CELLS}_Text_DeskDone:
{text("The warden's journal. The last page", "is torn out.")}
{CELLS}_Text_Rack:
{text("An old sword hangs on the armoury", "rack, its blade still bright…")}
{CELLS}_Text_RackMoves:
{text("The sword moved on its own!", "", "Its blue cloth wound round", "{PLAYER}'s arm. It wants to come", "along!")}
{CELLS}_Text_GotHonedge:
{text("{PLAYER} received a HONEDGE!")}
{CELLS}_Text_RackFull:
{text("Your party is full. The sword settles", "back onto the rack to wait.")}
{CELLS}_Text_RackEmpty:
{text("The armoury rack is empty.")}
{CELLS}_Text_Shuppet:
{text("SHUPPET: Shuppa!", "", "It's rattling the cell bars for fun.")}
''')
print('ok', CELLS)
register_wild('MAP_IRONMOOR_PRISON', 'gIronmoorPrison', land=ENC_INSIDE, land_rate=8)

# ---------------------------------------------------------------- solitary
s = Area(SOLITARY, 26, 15, inside_style(), fill='#')
hs = Halls(s)
hs.room(1, 2, 24, 5)                            # the long corridor
for x in (3, 8, 13):                            # solitary cells below it, shut for good
    hs.room(x, 8, x + 2, 9)
    for dx in range(3):
        hs.obj('bars', x + dx, 6)
hs.room(18, 5, 24, 12)                          # the keystone's chamber
for r, row in enumerate(STAIRS_UP):
    for col, mt in enumerate(row):
        hs.set(1 + col, r, mt, (r, col) == (1, 1))
hs.obj('keystone', 21, 10)
hs.obj('chains', 18, 5)
hs.obj('chains', 24, 5)
hs.obj('bunk', 3, 9)
hs.obj('bunk', 13, 8)
hs.obj('chains', 10, 8)
hs.build()
KEYSTONE = (21, 10)
s.warp(2, 1, 'MAP_IRONMOOR_PRISON', 2)
s.sign(*KEYSTONE, f'{SOLITARY}_EventScript_Keystone')
s.trainer('OBJ_EVENT_GFX_HEX_MANIAC', 16, 3, f'{SOLITARY}_EventScript_Selma', 3, 'MOVEMENT_TYPE_FACE_LEFT')
s.item('ITEM_TM_SHADOW_BALL', 24, 12, 'FLAG_ITEM_IRONMOOR_TM_SHADOW_BALL')
s.hidden(18, 12, 'ITEM_CLEANSE_TAG', 'FLAG_HIDDEN_ITEM_IRONMOOR_CLEANSE_TAG')
s.write(blocks=s.blocks(), map_id='MAP_IRONMOOR_PRISON_SOLITARY', layout_id='LAYOUT_IRONMOOR_PRISON_SOLITARY',
        music='MUS_MT_PYRE', mapsec=MAPSEC, mapsec_name='IRONMOOR PRISON', map_type='MAP_TYPE_INDOOR', after=CELLS,
        show_name=False, allow_cycling=False, scripts=rf'''
{SOLITARY}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {SOLITARY}_OnLoad
	.byte 0

{SOLITARY}_OnLoad:
	call_if_set FLAG_IRONMOOR_SPIRITOMB_DONE, {SOLITARY}_EventScript_Cracked
	end

{SOLITARY}_EventScript_Cracked::
	setmetatile {KEYSTONE[0]}, {KEYSTONE[1]}, {one('keystone_cracked')}, TRUE
	return

{SOLITARY}_EventScript_Keystone::
	lockall
	goto_if_set FLAG_IRONMOOR_SPIRITOMB_DONE, {SOLITARY}_EventScript_KeystoneEmpty
	goto_if_unset FLAG_IRONMOOR_SPIRIT_1, {SOLITARY}_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_2, {SOLITARY}_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_3, {SOLITARY}_EventScript_KeystoneCold
	goto_if_unset FLAG_IRONMOOR_SPIRIT_4, {SOLITARY}_EventScript_KeystoneCold
	msgbox {SOLITARY}_Text_KeystoneWakes, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	call {SOLITARY}_EventScript_Cracked
	special DrawWholeMapView
	setwildbattle SPECIES_SPIRITOMB, 32
	playmoncry SPECIES_SPIRITOMB, CRY_MODE_ENCOUNTER
	msgbox {SOLITARY}_Text_Spiritomb, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {SOLITARY}_EventScript_Lingers
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {SOLITARY}_EventScript_Lingers
	setflag FLAG_IRONMOOR_SPIRITOMB_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {SOLITARY}_EventScript_End
	msgbox {SOLITARY}_Text_Gone, MSGBOX_DEFAULT
{SOLITARY}_EventScript_End::
	releaseall
	end

{SOLITARY}_EventScript_Lingers::
	msgbox {SOLITARY}_Text_Lingers, MSGBOX_DEFAULT
	releaseall
	end

{SOLITARY}_EventScript_KeystoneCold::
	msgbox {SOLITARY}_Text_KeystoneCold, MSGBOX_DEFAULT
	releaseall
	end

{SOLITARY}_EventScript_KeystoneEmpty::
	msgbox {SOLITARY}_Text_KeystoneEmpty, MSGBOX_DEFAULT
	releaseall
	end

{trainer_script(SOLITARY, 'SELMA', text("Shh… Can you hear it? The stone at", "the end of the hall is whispering."),
                text("The whispers stopped…"), text("The stone wants its spirits back.", "They're bound up in the cells above."))}
{SOLITARY}_Text_KeystoneCold:
{text("An odd stone sits on a plinth, cold", "and silent. A face seems to stare", "out of it.", "", "Something is missing from it…")}
{SOLITARY}_Text_KeystoneWakes:
{text("Blue flames swirled down the hall", "and poured into the stone!", "", "The keystone is cracking!")}
{SOLITARY}_Text_Spiritomb:
{text("Hehehe…")}
{SOLITARY}_Text_Gone:
{text("SPIRITOMB scattered into the dark,", "laughing.")}
{SOLITARY}_Text_Lingers:
{text("Spirits still swirl round the cracked", "keystone…")}
{SOLITARY}_Text_KeystoneEmpty:
{text("The keystone is split in two. It's", "quiet now.")}
''')
print('ok', SOLITARY)
register_wild('MAP_IRONMOOR_PRISON_SOLITARY', 'gIronmoorPrisonSolitary', land=ENC_INSIDE, land_rate=8)

# ---------------------------------------------------------------- the way there, from Route 109
add_object('Route109', {'graphics_id': 'OBJ_EVENT_GFX_OLD_MAN', 'x': 17, 'y': 21, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': 'Route109_EventScript_IronmoorBoatman', 'flag': '0'})
register_script_file('prison', rf'''
@ Ironmoor Prison boatman on Route 109 (tools/areas/prison.py).
Route109_EventScript_IronmoorBoatman::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, Route109_EventScript_IronmoorNoBadge
	msgbox Route109_Text_IronmoorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route109_EventScript_IronmoorNo
	msgbox Route109_Text_IronmoorGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_IRONMOOR_ISLE, {ARRIVE[0]}, {ARRIVE[1]}
	waitstate
	release
	end

Route109_EventScript_IronmoorNoBadge::
	msgbox Route109_Text_IronmoorNoBadge, MSGBOX_DEFAULT
	release
	end

Route109_EventScript_IronmoorNo::
	msgbox Route109_Text_IronmoorNo, MSGBOX_DEFAULT
	release
	end

Route109_Text_IronmoorNoBadge:
{text("See that island in the fog? IRONMOOR", "PRISON. Empty for fifty years, but", "the lights still burn at night.", "", "I only row out trainers who've", "beaten four GYM LEADERS.")}
Route109_Text_IronmoorAsk:
{text("Four badges, eh? Brave enough for", "IRONMOOR PRISON?", "", "I'll row you out, but I won't", "step ashore.")}
Route109_Text_IronmoorGo:
{text("Hold on tight.")}
Route109_Text_IronmoorNo:
{text("Can't say I blame you.")}
''')
print('ok entrance')
