"""The Prism Caverns, for after the fifth badge: a crystal cave reached from
Meteor Falls, where a team of geologists has made camp.

Four resonance crystals sleep in the caverns, ruby, sapphire, emerald and
topaz. Touch one and it lights up, and every teleport pad of its colour
wakes. The ruby is in the crystal garden past the camp; its red pads lead
to the west gallery and the sapphire; the blue pads lead to the north hall
and the ladder down to the Depths. Down there red and blue pads link rock
islands in a maze: find the emerald, ride its green pads to the topaz, and
with all four lit the crystal seal before the heart chamber shatters.
Diancie waits by the heart crystal. The geologist gives you a Glimmet, and
the white pads wake: one by the camp leads to a hidden geode grotto.

New art (crystal clusters and formations, shards, geodes, the resonance
crystals dull and lit, pads dormant and awake in five colours, the seal,
the heart crystal and the camp) is drawn in tools/areas/crystal_art.py
(gTileset_Crystal: the cave tiles in violet plus the art).

    python3 tools/areas/crystal_art.py   # the tileset, when the art changes
    python3 tools/areas/crystal.py
"""
import json, sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import crystal

AREA_NO = 18
NAME = 'PrismCaverns'
DEPTHS = 'PrismCaverns_Depths'
MAPSEC = 'MAPSEC_PRISM_CAVERNS'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'crystal_tiles.json')))
MT = ART['objects']
LADDER_DOWN, LADDER_UP = 0x23F, 0x23E
MAP_ID = {NAME: 'MAP_PRISM_CAVERNS', DEPTHS: 'MAP_PRISM_CAVERNS_DEPTHS'}
FLAG = {'red': 'FLAG_PRISM_RUBY', 'blue': 'FLAG_PRISM_SAPPHIRE', 'green': 'FLAG_PRISM_EMERALD',
        'gold': 'FLAG_PRISM_TOPAZ', 'white': 'FLAG_PRISM_SEAL_OPEN'}
GEM = {'red': 'RUBY', 'blue': 'SAPPHIRE', 'green': 'EMERALD', 'gold': 'TOPAZ'}

register_constants(NAME, AREA_NO, flags=[
    'PRISM_RUBY', 'PRISM_SAPPHIRE', 'PRISM_EMERALD', 'PRISM_TOPAZ', 'PRISM_SEAL_OPEN', 'PRISM_DIANCIE_DONE',
    'PRISM_GLIMMET', 'PRISM_GEOLOGIST_TALKED',
    'ITEM_PRISM_ULTRA_BALL', 'ITEM_PRISM_HYPER_POTION', 'ITEM_PRISM_MAX_REPEL',
    'ITEM_PRISM_TM_REFLECT', 'ITEM_PRISM_SHINY_STONE', 'ITEM_PRISM_MAX_REVIVE', 'ITEM_PRISM_RARE_CANDY',
    'ITEM_PRISM_TM_LIGHT_SCREEN', 'ITEM_PRISM_COMET_SHARD', 'ITEM_PRISM_PP_UP', 'ITEM_PRISM_FULL_RESTORE',
    'HIDDEN_ITEM_PRISM_STAR_PIECE', 'HIDDEN_ITEM_PRISM_STARDUST', 'HIDDEN_ITEM_PRISM_PEARL',
    'HIDDEN_ITEM_PRISM_BIG_PEARL', 'HIDDEN_ITEM_PRISM_NUGGET',
], trainers=['PRISM_GARNET', 'PRISM_QUINN', 'PRISM_LUCAS', 'PRISM_OSWIN', 'PRISM_BRUNO', 'PRISM_TESSA'])

register_trainers(NAME, '''
=== AREA_TRAINER_PRISM_GARNET ===
Name: GARNET
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Boldore
Level: 31

Graveler
Level: 32

=== AREA_TRAINER_PRISM_QUINN ===
Name: QUINN
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Carbink
Level: 31

Mawile
Level: 32

=== AREA_TRAINER_PRISM_LUCAS ===
Name: LUCAS
Class: Psychic
Pic: Psychic M
Gender: Male
Music: Intense
Double Battle: No
AI: Basic Trainer

Chimecho
Level: 32

Bronzor
Level: 32

Lunatone
Level: 33

=== AREA_TRAINER_PRISM_OSWIN ===
Name: OSWIN
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Sableye
Level: 32

Nosepass
Level: 33

=== AREA_TRAINER_PRISM_BRUNO ===
Name: BRUNO
Class: Hiker
Pic: Hiker
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Lairon
Level: 33

Boldore
Level: 34

=== AREA_TRAINER_PRISM_TESSA ===
Name: TESSA
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Basic Trainer

Sneasel
Level: 33

Bronzong
Level: 34
''')


def place(a, name, x, y):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


def one(name):
    return MT[name][0][0]


def darken(area):
    """The synthesised blocks with the rock above the halls in its darker
    tile (the caves draw it with the plain floor tile), and every floor at
    elevation 3: the caverns are one walking level (Victory Road raises
    some of its floors to 4)."""
    b = area.blocks()
    raised = (b >> 12) == 4
    b[raised] = (b[raised] & 0x0FFF) | (3 << 12)
    top = np.isin(b & 0x3FF, (0x210, 0x211)) & ((b >> 10 & 3) != 0)
    b[top] = (b[top] & 0xFC00) | one('ceiling')
    return b


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


def south_exit(a, x, y):
    """The caves' way out: an arrow on the floor at the bottom edge."""
    a.tile(x, y, 0x207, collision=0, elevation=3, c='o')
    for dx, m in ((-1, 0x258), (0, 0x259), (1, 0x25A)):
        a.tile(x + dx, y + 1, m, collision=1, elevation=0, c='o')


def trainer_script(m, key, intro, defeat, post):
    label = key.title()
    return rf'''{m}_EventScript_{label}::
	trainerbattle_single TRAINER_PRISM_{key}, {m}_Text_{label}Intro, {m}_Text_{label}Defeat
	msgbox {m}_Text_{label}Post, MSGBOX_AUTOCLOSE
	end

{m}_Text_{label}Intro:
{intro}
{m}_Text_{label}Defeat:
{defeat}
{m}_Text_{label}Post:
{post}
'''


# ---------------------------------------------------------------- the pads
# Pairs of pads, one at each end: step on one and you appear on the other.
# Each pad is (map, colour, x, y). A pad sleeps (dull, nothing happens)
# until its colour's crystal is lit; white ones wake when the seal breaks.
PAIRS = [
    ((NAME, 'red', 22, 39), (NAME, 'red', 9, 38)),             # the camp -> the west gallery
    ((NAME, 'blue', 11, 7), (NAME, 'blue', 24, 10)),           # the gallery -> the north hall
    ((NAME, 'blue', 34, 32), (NAME, 'blue', 33, 10)),          # the camp -> the north hall
    ((NAME, 'white', 28, 30), (NAME, 'white', 50, 9)),         # the camp -> the geode grotto
    ((DEPTHS, 'red', 18, 32), (DEPTHS, 'red', 7, 34)),         # the landing -> island A
    ((DEPTHS, 'blue', 30, 32), (DEPTHS, 'blue', 41, 34)),      # the landing -> island B
    ((DEPTHS, 'blue', 10, 31), (DEPTHS, 'blue', 7, 10)),       # A -> C
    ((DEPTHS, 'red', 38, 31), (DEPTHS, 'red', 41, 10)),        # B -> D, a dead end
    ((DEPTHS, 'red', 3, 8), (DEPTHS, 'red', 8, 22)),           # C -> E, the emerald
    ((DEPTHS, 'green', 11, 21), (DEPTHS, 'green', 37, 22)),    # E -> F, the topaz
    ((DEPTHS, 'gold', 44, 22), (DEPTHS, 'gold', 20, 36)),      # F -> the landing, the way home
]
FIXED_WARPS = {NAME: 2, DEPTHS: 1}   # warps before the pads: exits and ladders


def pad_warps():
    """Warp index of every pad, numbered after each map's fixed warps."""
    idx, n = {}, dict(FIXED_WARPS)
    for pair in PAIRS:
        for p in pair:
            idx[p] = n[p[0]]
            n[p[0]] += 1
    return idx


PAD_WARP = pad_warps()


def lay_pads(a):
    """Dormant pads and their warps, in index order."""
    for pair in PAIRS:
        for i, p in enumerate(pair):
            if p[0] != a.name:
                continue
            other = pair[1 - i]
            _, col, x, y = p
            a.tile(x, y, one(f'pad_{col}_dull'), collision=0, c='o')
            a.warp(x, y, MAP_ID[other[0]], PAD_WARP[other])


def wake_scripts(m):
    """On load, wake the pads of every colour already lit; one call per colour."""
    cols = []
    for pair in PAIRS:
        for p in pair:
            if p[0] == m and p[1] not in cols:
                cols.append(p[1])
    load = ''.join(f'\tcall_if_set {FLAG[c]}, {m}_EventScript_Wake{c.title()}\n' for c in cols)
    subs = ''
    for c in cols:
        subs += f'{m}_EventScript_Wake{c.title()}::\n'
        for pair in PAIRS:
            for p in pair:
                if p[0] == m and p[1] == c:
                    subs += f'\tsetmetatile {p[2]}, {p[3]}, {one("pad_" + c)}, FALSE\n'
        subs += '\treturn\n\n'
    return cols, load, subs


def resonance(a, col, x, y):
    """A resonance crystal (1x2) at x, y; you touch it from below."""
    place(a, f'res_{col}', x, y)
    a.sign(x, y + 1, f'{a.name}_EventScript_Res{col.title()}', 'BG_EVENT_PLAYER_FACING_NORTH')


def res_script(m, col, x, y, extra=''):
    lit = MT[f'res_{col}_lit']
    has_pads = col in wake_scripts(m)[0]
    return rf'''{m}_EventScript_Lit{col.title()}::
	setmetatile {x}, {y}, {lit[0][0]}, TRUE
	setmetatile {x}, {y + 1}, {lit[1][0]}, TRUE
	return

{m}_EventScript_Res{col.title()}::
	lockall
	goto_if_set {FLAG[col]}, {m}_EventScript_ResAlreadyLit
	msgbox PrismCaverns_Text_ResTouch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {m}_EventScript_ResLeave
	playse SE_M_HEAL_BELL
	call {m}_EventScript_Lit{col.title()}
	special DrawWholeMapView
	waitse
	setflag {FLAG[col]}
	msgbox PrismCaverns_Text_Res{col.title()}, MSGBOX_DEFAULT
	{f"call {m}_EventScript_Wake{col.title()}" if has_pads else ""}
	playse SE_GLASS_FLUTE
	special DrawWholeMapView
	waitse
	msgbox PrismCaverns_Text_PadsWake{col.title()}, MSGBOX_DEFAULT
{extra}	releaseall
	end

'''


COMMON = rf'''{NAME}_EventScript_ResAlreadyLit::
	msgbox PrismCaverns_Text_ResAlreadyLit, MSGBOX_DEFAULT
	releaseall
	end

{NAME}_EventScript_ResLeave::
	releaseall
	end

PrismCaverns_Text_ResTouch:
{text("A tall crystal on a stone pedestal.", "It's dull and grey, but it hums", "faintly when you come near.", "", "Touch the crystal?")}
PrismCaverns_Text_ResAlreadyLit:
{text("The crystal shines brightly,", "humming a clear, steady note.")}
PrismCaverns_Text_ResRed:
{text("The crystal rang like a bell and", "began to glow a deep ruby red!")}
PrismCaverns_Text_ResBlue:
{text("The crystal rang like a bell and", "began to glow a cool sapphire blue!")}
PrismCaverns_Text_ResGreen:
{text("The crystal rang like a bell and", "began to glow a bright emerald green!")}
PrismCaverns_Text_ResGold:
{text("The crystal rang like a bell and", "began to glow a warm topaz gold!")}
PrismCaverns_Text_PadsWakeRed:
{text("Somewhere in the caverns, the red", "plates in the floor lit up!")}
PrismCaverns_Text_PadsWakeBlue:
{text("Somewhere in the caverns, the blue", "plates in the floor lit up!")}
PrismCaverns_Text_PadsWakeGreen:
{text("Somewhere in the caverns, the green", "plates in the floor lit up!")}
PrismCaverns_Text_PadsWakeGold:
{text("Somewhere in the caverns, the gold", "plates in the floor lit up!")}
'''

ENC_1F = [('GOLBAT', 29, 31), ('BOLDORE', 29, 31), ('SABLEYE', 29, 31), ('NOSEPASS', 29, 31),
          ('BRONZOR', 30, 32), ('GRAVELER', 30, 32), ('LAIRON', 30, 32), ('CARBINK', 30, 32),
          ('MAWILE', 31, 32), ('CHINGLING', 31, 32), ('LUNATONE', 31, 33), ('SOLROCK', 31, 33)]
ENC_DEPTHS = [('GOLBAT', 31, 33), ('BOLDORE', 31, 33), ('SABLEYE', 31, 33), ('CARBINK', 31, 33),
              ('BRONZOR', 32, 33), ('LAIRON', 32, 33), ('NOSEPASS', 32, 33), ('CHIMECHO', 32, 34),
              ('MAWILE', 32, 34), ('LUNATONE', 33, 34), ('CRYOGONAL', 33, 34), ('PROBOPASS', 34, 35)]

# ---------------------------------------------------------------- the caverns (1F)
W, H = 56, 44
a = Area(NAME, W, H, crystal(), fill='#')
# the camp hall and the way out
a.ellipse(28, 35, 9, 6, '.')
a.rect(27, 39, 3, 4, '.')
# the crystal garden, east, and the ruby's alcove
a.line([(36, 35), (42, 35)], 3, '.')
a.ellipse(46, 30, 7.5, 9, '.')
a.rect(44, 15, 5, 8, '.')
# the west gallery: a long hall of crystal, only the red pads reach it
a.line([(9, 39), (9, 31), (6, 25), (10, 18), (8, 9)], 5, '.')
a.ellipse(8, 31, 4.5, 3.5, '.')
a.ellipse(8, 14, 4.5, 3.5, '.')
a.rect(6, 3, 6, 6, '.')
# the north hall, reached by the blue pads, with the ladder down
a.ellipse(30, 8, 11.5, 5.5, '.')
# the geode grotto, reached by the white pads
a.ellipse(50, 7, 4.5, 3.8, '.')

south_exit(a, 28, 42)
ARRIVE = (28, 41)
a.warp(28, 42, 'MAP_METEOR_FALLS_1F_1R', 0)          # fixed up below, once the arrival warp exists
a.tile(38, 5, LADDER_DOWN, collision=0, c='o')
a.warp(38, 5, MAP_ID[DEPTHS], 0)
lay_pads(a)
resonance(a, 'red', 46, 15)
resonance(a, 'blue', 8, 3)

# the camp
place(a, 'tent', 20, 32)
place(a, 'lamp', 23, 31)
place(a, 'lamp', 33, 36)
place(a, 'crate', 19, 36)
place(a, 'crate', 20, 37)
place(a, 'geode', 31, 39)
a.sign(20, 33, f'{NAME}_EventScript_CampSign')
a.obj('OBJ_EVENT_GFX_SCIENTIST_1', 24, 34, f'{NAME}_EventScript_Geologist', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_WOMAN_5', 27, 33, f'{NAME}_EventScript_Medic', 'MOVEMENT_TYPE_FACE_DOWN')
a.obj('OBJ_EVENT_GFX_SCIENTIST_2', 32, 33, f'{NAME}_EventScript_Assistant', 'MOVEMENT_TYPE_WANDER_AROUND', rx=1, ry=1)

# the crystal garden: clusters and formations make a winding way to the ruby
for x, y in ((40, 29), (43, 32), (49, 34), (51, 27), (42, 23), (47, 25)):
    place(a, 'formation', x, y)
for x, y in ((45, 36), (46, 36), (39, 33), (52, 31), (48, 29), (44, 27), (40, 25), (50, 23), (46, 21), (48, 21)):
    place(a, 'cluster', x, y)
for x, y in ((41, 37), (50, 37), (43, 30), (39, 27), (52, 25)):
    place(a, 'small', x, y)
for x, y in ((44, 34), (47, 32), (41, 28), (50, 30), (45, 24)):
    place(a, 'shards', x, y)
a.trainer('OBJ_EVENT_GFX_HIKER', 47, 38, f'{NAME}_EventScript_Garnet', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.trainer('OBJ_EVENT_GFX_COOLTRAINER_F', 49, 26, f'{NAME}_EventScript_Quinn', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.item('ITEM_ULTRA_BALL', 53, 33, 'FLAG_ITEM_PRISM_ULTRA_BALL')
a.hidden(39, 31, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_PRISM_STAR_PIECE')

# the west gallery
for x, y in ((5, 35), (12, 33), (4, 27), (11, 23), (5, 16), (11, 13)):
    place(a, 'cluster', x, y)
place(a, 'formation', 5, 21)
for x, y in ((12, 29), (6, 11), (10, 36)):
    place(a, 'small', x, y)
for x, y in ((8, 28), (7, 18), (9, 6)):
    place(a, 'shards', x, y)
a.trainer('OBJ_EVENT_GFX_PSYCHIC_M', 10, 26, f'{NAME}_EventScript_Lucas', 3, 'MOVEMENT_TYPE_FACE_DOWN')
a.item('ITEM_HYPER_POTION', 4, 31, 'FLAG_ITEM_PRISM_HYPER_POTION')
a.hidden(12, 16, 'ITEM_STARDUST', 'FLAG_HIDDEN_ITEM_PRISM_STARDUST')

# the north hall
for x, y in ((21, 7), (40, 9), (27, 4), (35, 11)):
    place(a, 'cluster', x, y)
place(a, 'formation', 29, 6)
for x, y in ((24, 6), (36, 6)):
    place(a, 'shards', x, y)
a.sign(30, 7, f'{NAME}_EventScript_Carving', 'BG_EVENT_PLAYER_FACING_NORTH')
a.trainer('OBJ_EVENT_GFX_MANIAC', 28, 11, f'{NAME}_EventScript_Oswin', 3, 'MOVEMENT_TYPE_FACE_LEFT')
a.item('ITEM_MAX_REPEL', 20, 10, 'FLAG_ITEM_PRISM_MAX_REPEL')
a.hidden(39, 12, 'ITEM_PEARL', 'FLAG_HIDDEN_ITEM_PRISM_PEARL')

# the geode grotto
for x, y in ((47, 5), (53, 6)):
    place(a, 'cluster', x, y)
for x, y in ((48, 9), (52, 9), (50, 4)):
    place(a, 'geode', x, y)
a.item('ITEM_TM_LIGHT_SCREEN', 49, 5, 'FLAG_ITEM_PRISM_TM_LIGHT_SCREEN')
a.item('ITEM_COMET_SHARD', 52, 7, 'FLAG_ITEM_PRISM_COMET_SHARD')
a.item('ITEM_PP_UP', 47, 8, 'FLAG_ITEM_PRISM_PP_UP')
a.hidden(51, 4, 'ITEM_BIG_PEARL', 'FLAG_HIDDEN_ITEM_PRISM_BIG_PEARL')

cols, load, subs = wake_scripts(NAME)
a.write(blocks=darken(a), map_id=MAP_ID[NAME], layout_id='LAYOUT_PRISM_CAVERNS', music='MUS_VICTORY_ROAD', mapsec=MAPSEC,
        mapsec_name='PRISM CAVERNS', map_type='MAP_TYPE_UNDERGROUND', after='WonderwildPark', allow_cycling=False,
        allow_escaping=True, scripts=rf'''
{NAME}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {NAME}_OnLoad
	.byte 0

{NAME}_OnLoad:
	call_if_set FLAG_PRISM_RUBY, {NAME}_EventScript_LitRed
	call_if_set FLAG_PRISM_SAPPHIRE, {NAME}_EventScript_LitBlue
{load}	end

{subs}
{res_script(NAME, 'red', 46, 15)}
{res_script(NAME, 'blue', 8, 3)}
{COMMON}
{NAME}_EventScript_CampSign::
	msgbox {NAME}_Text_CampSign, MSGBOX_SIGN
	end

{NAME}_EventScript_Carving::
	msgbox {NAME}_Text_Carving, MSGBOX_SIGN
	end

{NAME}_EventScript_Medic::
	lock
	faceplayer
	msgbox {NAME}_Text_Medic, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {NAME}_EventScript_MedicNo
	closemessage
	fadescreen FADE_TO_BLACK
	playfanfare MUS_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox {NAME}_Text_MedicDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_MedicNo::
	msgbox {NAME}_Text_MedicNo, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Assistant::
	lock
	faceplayer
	goto_if_set FLAG_PRISM_SEAL_OPEN, {NAME}_EventScript_AssistantWhite
	msgbox {NAME}_Text_Assistant, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_AssistantWhite::
	msgbox {NAME}_Text_AssistantWhite, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_Geologist::
	lock
	faceplayer
	goto_if_set FLAG_PRISM_GLIMMET, {NAME}_EventScript_GeologistDone
	goto_if_set FLAG_PRISM_SEAL_OPEN, {NAME}_EventScript_GeologistReward
	goto_if_set FLAG_PRISM_SAPPHIRE, {NAME}_EventScript_GeologistTwo
	goto_if_set FLAG_PRISM_RUBY, {NAME}_EventScript_GeologistOne
	goto_if_set FLAG_PRISM_GEOLOGIST_TALKED, {NAME}_EventScript_GeologistAgain
	setflag FLAG_PRISM_GEOLOGIST_TALKED
	msgbox {NAME}_Text_Geologist, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistAgain::
	msgbox {NAME}_Text_GeologistAgain, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistOne::
	msgbox {NAME}_Text_GeologistOne, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistTwo::
	msgbox {NAME}_Text_GeologistTwo, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistReward::
	msgbox {NAME}_Text_GeologistThanks, MSGBOX_DEFAULT
	givemon SPECIES_GLIMMET, 30
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {NAME}_EventScript_GeologistFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {NAME}_Text_GotGlimmet, MSGBOX_DEFAULT
	waitfanfare
	setflag FLAG_PRISM_GLIMMET
	msgbox {NAME}_Text_GeologistDone, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistFull::
	msgbox {NAME}_Text_GeologistFull, MSGBOX_DEFAULT
	release
	end

{NAME}_EventScript_GeologistDone::
	msgbox {NAME}_Text_GeologistDone, MSGBOX_DEFAULT
	release
	end

{trainer_script(NAME, 'GARNET', text("These crystals ring when you knock", "'em! Let's make some noise!"),
                text("Rang my bell, you did."), text("The ruby's past the big formations.", "Follow the shards on the floor."))}
{trainer_script(NAME, 'QUINN', text("Shiny things, shiny POKéMON.", "Show me what sparkles on your team!"),
                text("Outshone!"), text("Light a crystal and its plates wake", "up all over the caverns. Pretty!"))}
{trainer_script(NAME, 'LUCAS', text("The gallery sings to me…", "Can you hear it? No? Then battle!"),
                text("The song went quiet…"), text("The blue crystal sleeps at the end of", "this gallery. Wake it gently."))}
{trainer_script(NAME, 'OSWIN', text("Old carvings, crystal halls… This is", "a ruin maniac's dream! Battle me!"),
                text("I got carried away."), text("Read the carving on the big", "formation. It speaks of a heart."))}
{NAME}_Text_CampSign:
{text("GEOLOGY TEAM CAMP", "Survey of the PRISM CAVERNS.", "Do not lick the crystals.")}
{NAME}_Text_Carving:
{text("Words are cut into the crystal:", "", "RUBY, SAPPHIRE, EMERALD, TOPAZ.", "When four sing as one, the seal", "breaks and the heart awakens.", "", "Then the white way opens for those", "who listen.")}
{NAME}_Text_Medic:
{text("I'm the team's medic. Long day in", "the caverns? Shall I patch up", "your POKéMON?")}
{NAME}_Text_MedicDone:
{text("All better! Mind the sharp edges.")}
{NAME}_Text_MedicNo:
{text("I'll be here if you need me.")}
{NAME}_Text_Assistant:
{text("The plate behind the camp is white.", "We've never seen it light up.", "", "What could wake it?")}
{NAME}_Text_AssistantWhite:
{text("The white plate is glowing now!", "Where does it go? I'm too scared", "to find out. You go first!")}
{NAME}_Text_Geologist:
{text("I'm DR. MICA, the team leader.", "Welcome to the PRISM CAVERNS!", "", "Four resonance crystals sleep in", "here. Wake one by touching it, and", "the plates of its colour light up.", "", "Step on a lit plate and it carries", "you somewhere else. The red crystal", "is east, through the garden.")}
{NAME}_Text_GeologistAgain:
{text("The ruby crystal is at the north end", "of the garden, east of camp.", "", "Once it's lit, try the red plate", "by our crates.")}
{NAME}_Text_GeologistOne:
{text("You woke the ruby! The red plate", "by our crates is glowing.", "", "My guess: it leads to the gallery", "where the sapphire sleeps.")}
{NAME}_Text_GeologistTwo:
{text("Ruby and sapphire, both lit!", "The blue plates reach the north", "hall and the ladder to the DEPTHS.", "", "The emerald and topaz must be down", "there, in the dark.")}
{NAME}_Text_GeologistThanks:
{text("We felt it up here: the seal broke!", "You woke all four crystals!", "", "Please, take this GLIMMET. It hatched", "from a crystal at our camp, and it", "has been following you around.")}
{NAME}_Text_GotGlimmet:
{text("{PLAYER} received a GLIMMET!")}
{NAME}_Text_GeologistFull:
{text("Your party's full! Make room and", "come back for GLIMMET.")}
{NAME}_Text_GeologistDone:
{text("The white plate behind our camp", "lit up when the seal broke.", "", "The carving says the white way", "opens for those who listen…")}
''')
print('ok', NAME)
register_wild(MAP_ID[NAME], 'gPrismCaverns', land=ENC_1F, land_rate=8)

# ---------------------------------------------------------------- the depths (B1F)
W, H = 48, 40
d = Area(DEPTHS, W, H, crystal(), fill='#')
d.ellipse(24, 33, 8.5, 5, '.')                     # the landing
d.rect(23, 17, 3, 12, '.')                         # the passage to the heart, behind the seal
d.ellipse(24, 10, 7.5, 6.5, '.')                   # the heart chamber
for cx, cy in ((7, 8), (7, 21), (7, 33), (41, 8), (41, 21), (41, 33)):   # islands C, E, A / D, F, B
    d.ellipse(cx, cy, 5.5, 3.8 if cy == 21 else 4.5, '.')

d.tile(24, 36, LADDER_UP, collision=0, c='o')
d.warp(24, 36, MAP_ID[NAME], 1)
lay_pads(d)
SEAL = (23, 24)
place(d, 'seal', *SEAL)
d.sign(24, 25, f'{DEPTHS}_EventScript_Seal', 'BG_EVENT_PLAYER_FACING_NORTH')
resonance(d, 'green', 5, 18)
resonance(d, 'gold', 43, 18)
place(d, 'heart', 23, 5)
d.obj('OBJ_EVENT_GFX_SPECIES(DIANCIE)', 24, 8, f'{DEPTHS}_EventScript_Diancie', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_PRISM_DIANCIE_DONE')
for x, y in ((18, 8), (30, 8), (19, 13), (29, 13)):
    place(d, 'cluster', x, y)
for x, y in ((21, 11), (27, 11), (24, 14)):
    place(d, 'shards', x, y)
for x, y in ((17, 31), (31, 30), (28, 35)):
    place(d, 'cluster', x, y)
place(d, 'formation', 20, 34)
for x, y in ((4, 31), (10, 35), (44, 31), (38, 35), (4, 5), (10, 10), (44, 11), (38, 6), (9, 19), (39, 19)):
    place(d, 'small', x, y)
for x, y in ((6, 31), (42, 30), (6, 8), (40, 7), (7, 23), (41, 23)):
    place(d, 'shards', x, y)
d.trainer('OBJ_EVENT_GFX_COOLTRAINER_F', 9, 7, f'{DEPTHS}_EventScript_Tessa', 3, 'MOVEMENT_TYPE_FACE_LEFT')
d.trainer('OBJ_EVENT_GFX_HIKER', 43, 8, f'{DEPTHS}_EventScript_Bruno', 3, 'MOVEMENT_TYPE_FACE_LEFT')
d.item('ITEM_TM_REFLECT', 4, 35, 'FLAG_ITEM_PRISM_TM_REFLECT')
d.item('ITEM_MAX_REVIVE', 44, 35, 'FLAG_ITEM_PRISM_MAX_REVIVE')
d.item('ITEM_SHINY_STONE', 4, 10, 'FLAG_ITEM_PRISM_SHINY_STONE')
d.item('ITEM_RARE_CANDY', 45, 7, 'FLAG_ITEM_PRISM_RARE_CANDY')
d.item('ITEM_FULL_RESTORE', 20, 9, 'FLAG_ITEM_PRISM_FULL_RESTORE')
d.hidden(10, 22, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_PRISM_NUGGET')

shards = one('shards')
seal_open = ''.join(f'\tsetmetatile {SEAL[0] + dx}, {SEAL[1] + dy}, {shards}, FALSE\n' for dy in (0, 1) for dx in range(3))
CHECK = rf'''	goto_if_unset FLAG_PRISM_EMERALD, {DEPTHS}_EventScript_ResDone
	goto_if_unset FLAG_PRISM_TOPAZ, {DEPTHS}_EventScript_ResDone
	goto_if_unset FLAG_PRISM_RUBY, {DEPTHS}_EventScript_ResDone
	goto_if_unset FLAG_PRISM_SAPPHIRE, {DEPTHS}_EventScript_ResDone
	call {DEPTHS}_EventScript_AllFour
'''
cols, load, subs = wake_scripts(DEPTHS)
d.write(blocks=darken(d), map_id=MAP_ID[DEPTHS], layout_id='LAYOUT_PRISM_CAVERNS_DEPTHS', music='MUS_CAVE_OF_ORIGIN', mapsec=MAPSEC,
        mapsec_name='PRISM CAVERNS', map_type='MAP_TYPE_UNDERGROUND', after=NAME, allow_cycling=False,
        allow_escaping=True, show_name=False, scripts=rf'''
{DEPTHS}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {DEPTHS}_OnLoad
	.byte 0

{DEPTHS}_OnLoad:
	call_if_set FLAG_PRISM_EMERALD, {DEPTHS}_EventScript_LitGreen
	call_if_set FLAG_PRISM_TOPAZ, {DEPTHS}_EventScript_LitGold
	call_if_set FLAG_PRISM_SEAL_OPEN, {DEPTHS}_EventScript_SealOpen
{load}	end

{subs}
{DEPTHS}_EventScript_SealOpen::
{seal_open}	return

{res_script(DEPTHS, 'green', 5, 18, CHECK)}
{res_script(DEPTHS, 'gold', 43, 18, CHECK)}
{DEPTHS}_EventScript_ResDone::
	releaseall
	end

{DEPTHS}_EventScript_ResAlreadyLit::
	msgbox PrismCaverns_Text_ResAlreadyLit, MSGBOX_DEFAULT
	releaseall
	end

{DEPTHS}_EventScript_ResLeave::
	releaseall
	end

{DEPTHS}_EventScript_AllFour::
	delay 30
	msgbox {DEPTHS}_Text_AllFour, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 3
	special ShakeCamera
	waitstate
	playse SE_M_BRICK_BREAK
	setflag FLAG_PRISM_SEAL_OPEN
	call {DEPTHS}_EventScript_SealOpen
	special DrawWholeMapView
	waitse
	msgbox {DEPTHS}_Text_SealBroke, MSGBOX_DEFAULT
	return

{DEPTHS}_EventScript_Seal::
	goto_if_set FLAG_PRISM_SEAL_OPEN, {DEPTHS}_EventScript_SealGone
	msgbox {DEPTHS}_Text_Seal, MSGBOX_SIGN
	end

{DEPTHS}_EventScript_SealGone::
	end

{DEPTHS}_EventScript_Diancie::
	lock
	faceplayer
	msgbox {DEPTHS}_Text_Sparkle, MSGBOX_DEFAULT
	setwildbattle SPECIES_DIANCIE, 40
	playmoncry SPECIES_DIANCIE, CRY_MODE_ENCOUNTER
	msgbox {DEPTHS}_Text_Diancie, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {DEPTHS}_EventScript_DiancieStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {DEPTHS}_EventScript_DiancieStays
	setflag FLAG_PRISM_DIANCIE_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {DEPTHS}_EventScript_DiancieEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {DEPTHS}_Text_DiancieGone, MSGBOX_DEFAULT
{DEPTHS}_EventScript_DiancieEnd::
	release
	end

{DEPTHS}_EventScript_DiancieStays::
	msgbox {DEPTHS}_Text_DiancieStays, MSGBOX_DEFAULT
	release
	end

{trainer_script(DEPTHS, 'TESSA', text("You found the way down here? Then", "you can handle the cold, too!"),
                text("Frozen solid…"), text("The red plate on this island goes", "somewhere new. I haven't dared."))}
{trainer_script(DEPTHS, 'BRUNO', text("End of the line, kid! This island", "goes nowhere. But I do battle!"),
                text("Nowhere, indeed."), text("Dead end, this. Take the other", "plate from the landing."))}
{DEPTHS}_Text_Seal:
{text("A wall of pink crystal blocks the", "passage. Four hollows are cut into", "it, each the size of a fist.", "", "Light shines in the first two…" , "but two are still dark.")}
{DEPTHS}_Text_AllFour:
{text("Ruby, sapphire, emerald, topaz…", "All four crystals sing as one!")}
{DEPTHS}_Text_SealBroke:
{text("Somewhere nearby, a wall of crystal", "shattered!")}
{DEPTHS}_Text_Sparkle:
{text("A small POKéMON sits before the", "great heart crystal, glittering", "like a jewel.")}
{DEPTHS}_Text_Diancie:
{text("Dian!")}
{DEPTHS}_Text_DiancieGone:
{text("DIANCIE vanished in a flash of pink", "light…")}
{DEPTHS}_Text_DiancieStays:
{text("DIANCIE is still sitting by the", "heart crystal, sparkling.")}
''')
print('ok', DEPTHS)
register_wild(MAP_ID[DEPTHS], 'gPrismCavernsDepths', land=ENC_DEPTHS, land_rate=8)

# ---------------------------------------------------------------- the way there, from Meteor Falls
BACK = add_warp('MeteorFalls_1F_1R', {'x': 26, 'y': 17, 'elevation': 0, 'dest_map': MAP_ID[NAME], 'dest_warp_id': '0'})
m = json.load(open(f'{ROOT}/data/maps/{NAME}/map.json'))
m['warp_events'][0]['dest_warp_id'] = str(BACK)
with open(f'{ROOT}/data/maps/{NAME}/map.json', 'w') as f:
    json.dump(m, f, indent=2)
    f.write('\n')
add_object('MeteorFalls_1F_1R', {'graphics_id': 'OBJ_EVENT_GFX_SCIENTIST_2', 'x': 25, 'y': 17, 'elevation': 4,
                                 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0,
                                 'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                                 'trainer_sight_or_berry_tree_id': '0',
                                 'script': 'MeteorFalls_1F_1R_EventScript_PrismGuide', 'flag': '0'})
register_script_file('crystal', rf'''
@ Prism Caverns guide in Meteor Falls (tools/areas/crystal.py).
MeteorFalls_1F_1R_EventScript_PrismGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE05_GET, MeteorFalls_1F_1R_EventScript_PrismNoBadge
	msgbox MeteorFalls_1F_1R_Text_PrismAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MeteorFalls_1F_1R_EventScript_PrismNo
	msgbox MeteorFalls_1F_1R_Text_PrismGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_PRISM_CAVERNS, {ARRIVE[0]}, {ARRIVE[1]}
	waitstate
	release
	end

MeteorFalls_1F_1R_EventScript_PrismNoBadge::
	msgbox MeteorFalls_1F_1R_Text_PrismNoBadge, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_1R_EventScript_PrismNo::
	msgbox MeteorFalls_1F_1R_Text_PrismNo, MSGBOX_DEFAULT
	release
	end

MeteorFalls_1F_1R_Text_PrismNoBadge:
{text("I'm with the GEOLOGY TEAM. We found", "a crystal cave behind the falls!", "", "It's deep and full of strong", "POKéMON. I can only take trainers", "with five badges.")}
MeteorFalls_1F_1R_Text_PrismAsk:
{text("Five badges! You could help us.", "There's a crystal cave behind the", "falls: the PRISM CAVERNS.", "", "Our team is camped inside. Shall I", "show you the way?")}
MeteorFalls_1F_1R_Text_PrismGo:
{text("Watch your step. The crystals are", "sharp!")}
MeteorFalls_1F_1R_Text_PrismNo:
{text("I'll be here, taking samples.")}
''')
print('ok entrance')
