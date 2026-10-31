"""The Nimbus Isles, for after the seventh badge: Nimbus City, a marble town
on a cloud reached by hot-air balloon from Fortree, and the Nimbus Isle, a
ring of grassy islands floating in the sky beyond its rainbow bridge.

A storm blew the three wind chimes off the city's chime tower onto the
islands. Rainbow bridges and one-way wind streams join the islands; bring
the chimes back, and the tower's song wakes the wind that carries you up to
the summit shrine, where Tornadus waits.

New art (sky, clouds, wind streams, rainbows, marble houses, the chime
tower, columns, a fountain, the balloon, the shrine) is drawn in
tools/areas/sky_art.py (gTileset_Sky: Petalburg's tiles plus the art; the
islands' grass and trees are General's).

    python3 tools/areas/sky_art.py     # the tileset, when the art changes
    python3 tools/areas/nimbus.py
"""
import copy, json, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import town

AREA_NO = 14
CITY, ISLE = 'NimbusCity', 'NimbusIsle'
HOUSE, MART = 'NimbusCity_WeatherHouse', 'NimbusCity_Mart'
MAPSEC = 'MAPSEC_NIMBUS_ISLES'
ART = json.load(open(os.path.join(os.path.dirname(__file__), 'sky_tiles.json')))
MT = ART['objects']

register_constants(CITY, AREA_NO, flags=[
    'NIMBUS_CHIME_1', 'NIMBUS_CHIME_2', 'NIMBUS_CHIME_3', 'NIMBUS_CHIMES_DONE', 'NIMBUS_TORNADUS_DONE',
    'ITEM_NIMBUS_SHARP_BEAK', 'ITEM_NIMBUS_WIDE_LENS', 'ITEM_NIMBUS_PP_MAX', 'ITEM_NIMBUS_ULTRA_BALL',
    'ITEM_NIMBUS_TM_STEEL_WING', 'ITEM_NIMBUS_COMET_SHARD', 'HIDDEN_ITEM_NIMBUS_STAR_PIECE',
    'HIDDEN_ITEM_NIMBUS_PRETTY_FEATHER', 'HIDDEN_ITEM_NIMBUS_RARE_CANDY', 'NIMBUS_KEEPER_TALKED',
], trainers=['NIMBUS_CORVIN', 'NIMBUS_DRACO', 'NIMBUS_AURORA', 'NIMBUS_ZEPHYR'])

register_trainers(CITY, '''
=== AREA_TRAINER_NIMBUS_CORVIN ===
Name: CORVIN
Class: Bird Keeper
Pic: Bird Keeper
Gender: Male
Music: Cool
Double Battle: No
AI: Smart Trainer

Swellow
Level: 41

Staraptor
Level: 42

Noctowl
Level: 41

=== AREA_TRAINER_NIMBUS_DRACO ===
Name: DRACO
Class: Dragon Tamer
Pic: Dragon Tamer
Gender: Male
Music: Intense
Double Battle: No
AI: Smart Trainer

Altaria
Level: 43

Flygon
Level: 44

=== AREA_TRAINER_NIMBUS_AURORA ===
Name: AURORA
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Cool
Double Battle: No
AI: Smart Trainer

Togekiss
Level: 43

Drifblim
Level: 42

Chimecho
Level: 43

=== AREA_TRAINER_NIMBUS_ZEPHYR ===
Name: ZEPHYR
Class: Psychic
Pic: Psychic M
Gender: Male
Music: Intense
Double Battle: No
AI: Smart Trainer

Sigilyph
Level: 43

Xatu
Level: 44
''')


def place(a, name, x, y):
    walk = {tuple(rc) for rc in ART['walkable'][name]}
    for r, row in enumerate(MT[name]):
        for c, mt in enumerate(row):
            if mt >= 0:
                a.tile(x + c, y + r, mt, collision=0 if (r, c) in walk else 1, c='o')


def one(name):
    return MT[name][0][0]


def blob(cells, x0, y0, x1, y1, r=2):
    """A rectangle of cells with its corners rounded off."""
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            dx = max(x0 + r - x, 0, x - (x1 - r))
            dy = max(y0 + r - y, 0, y - (y1 - r))
            if dx * dx + dy * dy <= r * r:
                cells.add((x, y))


def skyscape(a, solid, rainbows=(), winds=(), seed=1):
    """Draws the sky and the clouds: solid holds every cell that isn't open
    sky (clouds, islands, buildings); clouds get edges against the sky.
    rainbows: {(x, y): name} bridges over the sky; winds: {(x, y): dir}."""
    import random
    rnd = random.Random(seed)
    rainbows, winds = dict(rainbows), dict(winds)
    is_sky = lambda x, y: (x, y) not in solid
    for y in range(a.h):
        for x in range(a.w):
            if (x, y) in a.fixed and (x, y) not in rainbows and (x, y) not in winds:
                continue
            if (x, y) in rainbows:
                a.tile(x, y, one(rainbows[(x, y)]), collision=0, c='o')
            elif (x, y) in winds:
                a.tile(x, y, one(f'wind_{winds[(x, y)]}'), collision=0, c='o')
            elif (x, y) not in solid:
                a.tile(x, y, one('sky_wisp' if rnd.random() < 0.08 else 'sky'), collision=1, c='o')
            elif a.grid[y][x] == 'c':
                n, s = is_sky(x, y - 1), is_sky(x, y + 1)
                w, e = is_sky(x - 1, y), is_sky(x + 1, y)
                edges = ('n' if n else '') + ('s' if s else '') + ('e' if e else '') + ('w' if w else '')
                if edges in ('ns', 'ew') or len(edges) > 2:
                    name = 'cloud_' + edges if len(edges) == 2 else 'cloud'
                elif edges:
                    name = 'cloud_' + edges
                else:
                    name = 'cloud' if (x + y) % 2 else 'cloud_b'
                    for corner, (dx, dy) in (('nw', (-1, -1)), ('ne', (1, -1)), ('sw', (-1, 1)), ('se', (1, 1))):
                        if is_sky(x + dx, y + dy):
                            name = f'cloud_in_{corner}'
                a.tile(x, y, one(name), collision=0, c='o')


# ---------------------------------------------------------------- Nimbus City
st = copy.copy(town())
st.secondary = 'gTileset_Sky'
st.exclude = set(st.exclude) | {0x129, 0x087, 0x0C6, 0x1D4, 0x1E7, 0x0A1, 0x0B1, 0x0C0, 0x1D9, 0x1D1, 0x1E0, 0x1E1, 0x1E2}
W, H = 40, 32
c = Area(CITY, W, H, st, fill='c')
solid = set()
blob(solid, 3, 4, 36, 28, r=4)
blob(solid, 14, 1, 24, 6, r=2)                   # the tower's cloud
for y in range(H):
    for x in range(W):
        if (x, y) not in solid:
            c.put(x, y, 'o')
TOWER = (18, 3)                                  # 3x6
place(c, 'chime_tower', *TOWER)
WEATHER_HOUSE, SKY_MART = (5, 7), (29, 7)        # 5x4, doors at (+2, +3)
place(c, 'marble_house', *WEATHER_HOUSE)
place(c, 'marble_house', *SKY_MART)
place(c, 'fountain', 18, 14)
for x, y in ((14, 12), (24, 12), (14, 18), (24, 18), (9, 22), (29, 22)):
    place(c, 'column', x, y)
place(c, 'balloon', 7, 15)                       # a moored spare
place(c, 'balloon', 31, 15)
BALLOON = (18, 24)                               # 2x3
place(c, 'balloon', *BALLOON)
BRIDGE_Y = 16
rainbows = {(37, BRIDGE_Y): 'rainbow_h', (38, BRIDGE_Y): 'rainbow_h', (39, BRIDGE_Y): 'rainbow_exit_e'}
skyscape(c, solid, rainbows)
c.border = [one('sky')] * 4

ARRIVE = (20, 27)
c.warp(39, BRIDGE_Y, 'MAP_NIMBUS_ISLE', 0)
c.warp(WEATHER_HOUSE[0] + 2, WEATHER_HOUSE[1] + 3, 'MAP_NIMBUS_CITY_WEATHER_HOUSE', 0)
c.warp(SKY_MART[0] + 2, SKY_MART[1] + 3, 'MAP_NIMBUS_CITY_MART', 0)
c.obj('OBJ_EVENT_GFX_MAN_2', 21, 26, f'{CITY}_EventScript_Pilot', 'MOVEMENT_TYPE_FACE_LEFT')
c.obj('OBJ_EVENT_GFX_OLD_WOMAN', 19, 9, f'{CITY}_EventScript_Keeper', 'MOVEMENT_TYPE_FACE_DOWN')
c.obj('OBJ_EVENT_GFX_SPECIES(ALTARIA)', 12, 16, f'{CITY}_EventScript_Altaria', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=2)
c.obj('OBJ_EVENT_GFX_SPECIES(SWABLU)', 26, 20, f'{CITY}_EventScript_Swablu', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
c.obj('OBJ_EVENT_GFX_LITTLE_GIRL', 16, 20, f'{CITY}_EventScript_Girl', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
c.obj('OBJ_EVENT_GFX_GENTLEMAN', 33, 14, f'{CITY}_EventScript_Gentleman', 'MOVEMENT_TYPE_FACE_RIGHT')
c.sign(17, 22, f'{CITY}_EventScript_Sign')
c.hidden(5, 25, 'ITEM_PRETTY_FEATHER', 'FLAG_HIDDEN_ITEM_NIMBUS_PRETTY_FEATHER')
c.write(map_id='MAP_NIMBUS_CITY', layout_id='LAYOUT_NIMBUS_CITY', music='MUS_ROUTE119', mapsec=MAPSEC,
        mapsec_name='NIMBUS ISLES', map_type='MAP_TYPE_TOWN', after='VoltspirePlant', allow_cycling=False,
        weather='WEATHER_SUNNY_CLOUDS', scripts=rf'''
{CITY}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {CITY}_OnLoad
	.byte 0

{CITY}_OnLoad:
	call_if_set FLAG_NIMBUS_CHIMES_DONE, {CITY}_EventScript_TowerRings
	end

{CITY}_EventScript_TowerRings::
''' + ''.join(f'\tsetmetatile {TOWER[0] + cc}, {TOWER[1] + r}, {m}, TRUE\n'
              for r, row in enumerate(MT['chime_tower_ringing']) for cc, m in enumerate(row) if m >= 0 and r < 3) + rf'''	return

{CITY}_EventScript_Pilot::
	lock
	faceplayer
	msgbox {CITY}_Text_PilotBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {CITY}_EventScript_PilotStay
	msgbox {CITY}_Text_PilotGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_FORTREE_CITY, 3, 11
	waitstate
	release
	end

{CITY}_EventScript_PilotStay::
	msgbox {CITY}_Text_PilotStay, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_Keeper::
	lock
	faceplayer
	goto_if_set FLAG_NIMBUS_CHIMES_DONE, {CITY}_EventScript_KeeperDone
	goto_if_unset FLAG_NIMBUS_CHIME_1, {CITY}_EventScript_KeeperAsk
	goto_if_unset FLAG_NIMBUS_CHIME_2, {CITY}_EventScript_KeeperAsk
	goto_if_unset FLAG_NIMBUS_CHIME_3, {CITY}_EventScript_KeeperAsk
	msgbox {CITY}_Text_KeeperThanks, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_NIMBUS_CHIMES_DONE
	call {CITY}_EventScript_TowerRings
	special DrawWholeMapView
	playse SE_BALL_TRAY_EXIT
	waitse
	playse SE_BALL_TRAY_EXIT
	waitse
	msgbox {CITY}_Text_TowerSings, MSGBOX_DEFAULT
	givemon SPECIES_CASTFORM, 40
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, {CITY}_EventScript_KeeperFull
	playfanfare MUS_OBTAIN_ITEM
	msgbox {CITY}_Text_GotCastform, MSGBOX_DEFAULT
	waitfanfare
	msgbox {CITY}_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_KeeperFull::
	msgbox {CITY}_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_KeeperAsk::
	msgbox {CITY}_Text_KeeperAsk, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_KeeperDone::
	msgbox {CITY}_Text_KeeperWind, MSGBOX_DEFAULT
	release
	end

{CITY}_EventScript_Altaria::
	lock
	faceplayer
	playmoncry SPECIES_ALTARIA, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Altaria, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CITY}_EventScript_Swablu::
	lock
	faceplayer
	playmoncry SPECIES_SWABLU, CRY_MODE_NORMAL
	msgbox {CITY}_Text_Swablu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{CITY}_EventScript_Girl::
	msgbox {CITY}_Text_Girl, MSGBOX_NPC
	end

{CITY}_EventScript_Gentleman::
	msgbox {CITY}_Text_Gentleman, MSGBOX_NPC
	end

{CITY}_EventScript_Sign::
	msgbox {CITY}_Text_Sign, MSGBOX_SIGN
	end

{CITY}_Text_PilotBack:
	.string "Ready to float back down to\n"
	.string "FORTREE CITY?$"

{CITY}_Text_PilotGo:
	.string "Hold on tight!$"

{CITY}_Text_PilotStay:
	.string "Enjoy the view. The balloon will\n"
	.string "be right here.$"

{CITY}_Text_KeeperAsk:
	.string "I keep the CHIME TOWER. Its song\n"
	.string "calls the winds of the isles.\p"
	.string "But last night's storm blew all\n"
	.string "three WIND CHIMES away, onto the\l"
	.string "islands past the rainbow bridge.\p"
	.string "Without their song, the wind up to\n"
	.string "the summit won't blow. Please,\l"
	.string "would you find them?$"

{CITY}_Text_KeeperThanks:
	.string "All three chimes! Let me hang them\n"
	.string "back in the belfry…$"

{CITY}_Text_TowerSings:
	.string "The CHIME TOWER is singing!\p"
	.string "Thank you. Please take this CASTFORM.\n"
	.string "It was born in the tower's wind.$"

{CITY}_Text_GotCastform:
	.string "{{PLAYER}} received a CASTFORM!$"

{CITY}_Text_KeeperWind:
	.string "Hear that? The summit wind is\n"
	.string "blowing again. Ride it from the\l"
	.string "eastern island, up to the shrine.$"

{CITY}_Text_Altaria:
	.string "ALTARIA is humming along with the\n"
	.string "breeze.$"

{CITY}_Text_Swablu:
	.string "SWABLU is trying to hide in a\n"
	.string "cloud. It blends right in!$"

{CITY}_Text_Girl:
	.string "Don't walk off the edge of the\n"
	.string "cloud! Grown-ups always say that.$"

{CITY}_Text_Gentleman:
	.string "The rainbow bridge leads to the\n"
	.string "isles. The winds there blow one\l"
	.string "way only, so mind where you step.$"

{CITY}_Text_Sign:
	.string "NIMBUS CITY\n"
	.string "The town above the clouds.$"
''')
print('ok', CITY)

# ---------------------------------------------------------------- Nimbus Isle
W, H = 52, 40
TREE = {(0, 0): 0x1D6, (1, 0): 0x1D7, (0, 1): 0x1DC, (1, 1): 0x016}   # a General tree, as Route 101 draws it
s = Area(ISLE, W, H, st, fill='c')
solid = set()
ISLANDS = {   # cloud (x0, y0, x1, y1), grass inside
    'A': (5, 14, 19, 28), 'B': (8, 2, 24, 10), 'C': (10, 31, 30, 38), 'D': (37, 14, 49, 27), 'E': (36, 1, 50, 8)}
for x0, y0, x1, y1 in ISLANDS.values():
    blob(solid, x0, y0, x1, y1, r=3)
    s.ellipse((x0 + x1) / 2 + 0.5, (y0 + y1) / 2 + 0.5, (x1 - x0) / 2 - 1.5, (y1 - y0) / 2 - 1.5, 'g')
s.ellipse(9, 24, 3, 2, '"')
s.ellipse(20, 6, 3, 2, '"')
s.ellipse(14, 35, 3, 1.5, '"')
s.ellipse(45, 23, 3, 2, '"')
for x, y in ((8, 16), (16, 16), (12, 26), (12, 6), (22, 4), (24, 34), (28, 32), (40, 16), (46, 16), (38, 4), (48, 6)):
    for (dx, dy), mt in TREE.items():
        s.tile(x + dx, y + dy, mt, collision=1, c='T')
for y in range(H):
    for x in range(W):
        if (x, y) not in solid:
            s.put(x, y, 'o')
rainbows = {(x, 20): 'rainbow_h' for x in range(1, 5)}
rainbows[(0, 20)] = 'rainbow_exit_w'
rainbows.update({(14, y): 'rainbow_v' for y in range(11, 14)})          # A <-> B
rainbows.update({(15, y): 'rainbow_v' for y in range(29, 31)})          # A <-> C
winds = {(x, 18): 'e' for x in range(20, 37)}                           # A -> D
winds.update({(x, 25): 'w' for x in range(20, 37)})                     # D -> A
winds.update({(46, y): 's' for y in range(9, 14)})                      # the summit -> D
SUMMIT_WIND = [(41, y) for y in range(9, 14)]                           # D -> the summit, once the tower sings
place(s, 'shrine', 41, 1)
skyscape(s, solid, rainbows, winds, seed=3)
s.border = [one('sky')] * 4

s.warp(0, 20, 'MAP_NIMBUS_CITY', 0)
s.obj('OBJ_EVENT_GFX_ITEM_BALL', 12, 4, f'{ISLE}_EventScript_Chime1', 'MOVEMENT_TYPE_LOOK_AROUND', 'FLAG_NIMBUS_CHIME_1')
s.obj('OBJ_EVENT_GFX_ITEM_BALL', 28, 35, f'{ISLE}_EventScript_Chime2', 'MOVEMENT_TYPE_LOOK_AROUND', 'FLAG_NIMBUS_CHIME_2')
s.obj('OBJ_EVENT_GFX_ITEM_BALL', 47, 18, f'{ISLE}_EventScript_Chime3', 'MOVEMENT_TYPE_LOOK_AROUND', 'FLAG_NIMBUS_CHIME_3')
s.obj('OBJ_EVENT_GFX_SPECIES(TORNADUS)', 43, 6, f'{ISLE}_EventScript_Tornadus', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_NIMBUS_TORNADUS_DONE')
s.trainer('OBJ_EVENT_GFX_MAN_5', 16, 22, f'{ISLE}_EventScript_Corvin', 3, 'MOVEMENT_TYPE_FACE_LEFT')
s.trainer('OBJ_EVENT_GFX_MAN_3', 18, 8, f'{ISLE}_EventScript_Draco', 3, 'MOVEMENT_TYPE_FACE_LEFT')
s.trainer('OBJ_EVENT_GFX_COOLTRAINER_F', 20, 33, f'{ISLE}_EventScript_Aurora', 3, 'MOVEMENT_TYPE_FACE_DOWN')
s.trainer('OBJ_EVENT_GFX_PSYCHIC_M', 42, 21, f'{ISLE}_EventScript_Zephyr', 3, 'MOVEMENT_TYPE_FACE_LEFT')
s.obj('OBJ_EVENT_GFX_SPECIES(SWABLU)', 10, 20, f'{ISLE}_EventScript_Swablu', 'MOVEMENT_TYPE_WANDER_AROUND', rx=1, ry=1)
s.item('ITEM_SHARP_BEAK', 21, 7, 'FLAG_ITEM_NIMBUS_SHARP_BEAK')
s.item('ITEM_WIDE_LENS', 12, 36, 'FLAG_ITEM_NIMBUS_WIDE_LENS')
s.item('ITEM_ULTRA_BALL', 7, 26, 'FLAG_ITEM_NIMBUS_ULTRA_BALL')
s.item('ITEM_TM_STEEL_WING', 48, 25, 'FLAG_ITEM_NIMBUS_TM_STEEL_WING')
s.item('ITEM_PP_MAX', 48, 4, 'FLAG_ITEM_NIMBUS_PP_MAX')
s.item('ITEM_COMET_SHARD', 37, 5, 'FLAG_ITEM_NIMBUS_COMET_SHARD')
s.hidden(29, 33, 'ITEM_STAR_PIECE', 'FLAG_HIDDEN_ITEM_NIMBUS_STAR_PIECE')
s.hidden(45, 7, 'ITEM_RARE_CANDY', 'FLAG_HIDDEN_ITEM_NIMBUS_RARE_CANDY')


def chime(i):
    return rf'''{ISLE}_EventScript_Chime{i}::
	lock
	playse SE_BALL_TRAY_EXIT
	msgbox {ISLE}_Text_Chime, MSGBOX_DEFAULT
	removeobject VAR_LAST_TALKED
	setflag FLAG_NIMBUS_CHIME_{i}
	release
	end

'''


s.write(map_id='MAP_NIMBUS_ISLE', layout_id='LAYOUT_NIMBUS_ISLE', music='MUS_ROUTE119', mapsec=MAPSEC,
        mapsec_name='NIMBUS ISLES', after=CITY, allow_cycling=False, show_name=False, weather='WEATHER_SUNNY_CLOUDS',
        scripts=rf'''
{ISLE}_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, {ISLE}_OnLoad
	.byte 0

{ISLE}_OnLoad:
	call_if_set FLAG_NIMBUS_CHIMES_DONE, {ISLE}_EventScript_SummitWind
	end

{ISLE}_EventScript_SummitWind::
''' + ''.join(f'\tsetmetatile {x}, {y}, {one("wind_n")}, FALSE\n' for x, y in SUMMIT_WIND) + rf'''	return

{chime(1)}{chime(2)}{chime(3)}{ISLE}_EventScript_Tornadus::
	lock
	faceplayer
	msgbox {ISLE}_Text_Gale, MSGBOX_DEFAULT
	setwildbattle SPECIES_TORNADUS, 50
	playmoncry SPECIES_TORNADUS, CRY_MODE_ENCOUNTER
	msgbox {ISLE}_Text_Tornadus, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, {ISLE}_EventScript_TornadusStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, {ISLE}_EventScript_TornadusStays
	setflag FLAG_NIMBUS_TORNADUS_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, {ISLE}_EventScript_TornadusEnd
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	msgbox {ISLE}_Text_TornadusGone, MSGBOX_DEFAULT
{ISLE}_EventScript_TornadusEnd::
	release
	end

{ISLE}_EventScript_TornadusStays::
	msgbox {ISLE}_Text_TornadusStays, MSGBOX_DEFAULT
	release
	end

{ISLE}_EventScript_Swablu::
	lock
	faceplayer
	playmoncry SPECIES_SWABLU, CRY_MODE_NORMAL
	msgbox {ISLE}_Text_Swablu, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{ISLE}_EventScript_Corvin::
	trainerbattle_single TRAINER_NIMBUS_CORVIN, {ISLE}_Text_CorvinIntro, {ISLE}_Text_CorvinDefeat
	msgbox {ISLE}_Text_CorvinPost, MSGBOX_AUTOCLOSE
	end

{ISLE}_EventScript_Draco::
	trainerbattle_single TRAINER_NIMBUS_DRACO, {ISLE}_Text_DracoIntro, {ISLE}_Text_DracoDefeat
	msgbox {ISLE}_Text_DracoPost, MSGBOX_AUTOCLOSE
	end

{ISLE}_EventScript_Aurora::
	trainerbattle_single TRAINER_NIMBUS_AURORA, {ISLE}_Text_AuroraIntro, {ISLE}_Text_AuroraDefeat
	msgbox {ISLE}_Text_AuroraPost, MSGBOX_AUTOCLOSE
	end

{ISLE}_EventScript_Zephyr::
	trainerbattle_single TRAINER_NIMBUS_ZEPHYR, {ISLE}_Text_ZephyrIntro, {ISLE}_Text_ZephyrDefeat
	msgbox {ISLE}_Text_ZephyrPost, MSGBOX_AUTOCLOSE
	end

{ISLE}_Text_Chime:
	.string "Ting-a-ling…\n"
	.string "{{PLAYER}} found a WIND CHIME!$"

{ISLE}_Text_Gale:
	.string "A fierce wind howls around the\n"
	.string "shrine… and takes shape!$"

{ISLE}_Text_Tornadus:
	.string "Fwoooosh!$"

{ISLE}_Text_TornadusGone:
	.string "TORNADUS whirled away into the\n"
	.string "clouds.$"

{ISLE}_Text_TornadusStays:
	.string "TORNADUS still circles the shrine,\n"
	.string "howling.$"

{ISLE}_Text_Swablu:
	.string "SWABLU fluffed up its cotton wings.$"

{ISLE}_Text_CorvinIntro:
	.string "Up here, my birds fly higher than\n"
	.string "anywhere! Can you keep up?$"

{ISLE}_Text_CorvinDefeat:
	.string "Shot down…$"

{ISLE}_Text_CorvinPost:
	.string "The wind streams only blow one way.\n"
	.string "East out, west back.$"

{ISLE}_Text_DracoIntro:
	.string "Dragons rule the skies! Prepare\n"
	.string "yourself!$"

{ISLE}_Text_DracoDefeat:
	.string "My dragons… grounded!$"

{ISLE}_Text_DracoPost:
	.string "I saw something shiny blow onto\n"
	.string "this island in the storm.$"

{ISLE}_Text_AuroraIntro:
	.string "Isn't the view stunning? Almost as\n"
	.string "stunning as my team!$"

{ISLE}_Text_AuroraDefeat:
	.string "Stunned, indeed.$"

{ISLE}_Text_AuroraPost:
	.string "On a clear day, you can see all of\n"
	.string "HOENN from these islands.$"

{ISLE}_Text_ZephyrIntro:
	.string "I sense a great wind above us,\n"
	.string "waiting to be woken…$"

{ISLE}_Text_ZephyrDefeat:
	.string "My senses… scattered.$"

{ISLE}_Text_ZephyrPost:
	.string "When the CHIME TOWER sings, the\n"
	.string "wind will rise to the shrine.$"
''')
print('ok', ISLE)

register_wild('MAP_NIMBUS_ISLE', 'gNimbusIsle',
              land=[('SWABLU', 40, 42), ('SWELLOW', 40, 42), ('NOCTOWL', 40, 42), ('PELIPPER', 40, 42),
                    ('TROPIUS', 41, 43), ('CHIMECHO', 41, 43), ('DRIFBLIM', 41, 43), ('STARAPTOR', 41, 43),
                    ('SKARMORY', 42, 44), ('SIGILYPH', 42, 44), ('ALTARIA', 43, 45), ('BRAVIARY', 43, 45)])

# ---------------------------------------------------------------- the weather house and the sky mart
interior(HOUSE, 'MAP_NIMBUS_CITY_WEATHER_HOUSE', 'LAYOUT_FORTREE_CITY_HOUSE1', [(3, 5), (4, 5)],
         'MAP_NIMBUS_CITY', 1, mapsec=MAPSEC, group='gMapGroup_TownsAndRoutes',
         objects=[npc('OBJ_EVENT_GFX_SCIENTIST_1', 1, 3, f'{HOUSE}_EventScript_Watcher', 'MOVEMENT_TYPE_FACE_RIGHT'),
                  npc('OBJ_EVENT_GFX_SPECIES(CASTFORM)', 6, 3, f'{HOUSE}_EventScript_Castform',
                      'MOVEMENT_TYPE_WANDER_AROUND')],
         scripts=rf'''
{HOUSE}_MapScripts::
	.byte 0

{HOUSE}_EventScript_Watcher::
	msgbox {HOUSE}_Text_Watcher, MSGBOX_NPC
	end

{HOUSE}_EventScript_Castform::
	lock
	faceplayer
	playmoncry SPECIES_CASTFORM, CRY_MODE_NORMAL
	msgbox {HOUSE}_Text_Castform, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{HOUSE}_Text_Watcher:
	.string "I watch the winds of the isles.\p"
	.string "Two streams blow between the west\n"
	.string "and east islands: east on the\l"
	.string "upper one, west on the lower.\p"
	.string "A third once blew up to the summit,\n"
	.string "but it's been still since the storm.$"

{HOUSE}_Text_Castform:
	.string "CASTFORM changes shape with the\n"
	.string "weather. Today it's sunny!$"
''')
interior(MART, 'MAP_NIMBUS_CITY_MART', 'LAYOUT_MART', [(3, 7), (4, 7)], 'MAP_NIMBUS_CITY', 2, mapsec=MAPSEC,
         group='gMapGroup_TownsAndRoutes', music='MUS_POKE_MART',
         objects=[npc('OBJ_EVENT_GFX_MART_EMPLOYEE', 1, 3, f'{MART}_EventScript_Clerk', 'MOVEMENT_TYPE_FACE_RIGHT'),
                  npc('OBJ_EVENT_GFX_WOMAN_5', 8, 5, f'{MART}_EventScript_Woman', 'MOVEMENT_TYPE_FACE_LEFT')],
         scripts=rf'''
{MART}_MapScripts::
	.byte 0

{MART}_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart {MART}_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
{MART}_Pokemart:
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_REVIVE
	.2byte ITEM_PRETTY_FEATHER
	.2byte ITEM_SHARP_BEAK
	pokemartlistend

{MART}_EventScript_Woman::
	msgbox {MART}_Text_Woman, MSGBOX_NPC
	end

{MART}_Text_Woman:
	.string "Everything here comes up by\n"
	.string "balloon. Even the shop's floor!$"
''')
print('ok interiors')

# ---------------------------------------------------------------- the way there, from Fortree City
add_object('FortreeCity', {'graphics_id': 'OBJ_EVENT_GFX_MAN_2', 'x': 2, 'y': 11, 'elevation': 3,
                           'movement_type': 'MOVEMENT_TYPE_FACE_RIGHT', 'movement_range_x': 0, 'movement_range_y': 0,
                           'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                           'script': 'FortreeCity_EventScript_NimbusPilot', 'flag': '0'})
register_script_file('nimbus', r'''
@ Nimbus Isles balloon pilot in Fortree City (tools/areas/nimbus.py).
FortreeCity_EventScript_NimbusPilot::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE07_GET, FortreeCity_EventScript_NimbusNoBadge
	msgbox FortreeCity_Text_NimbusAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FortreeCity_EventScript_NimbusNo
	msgbox FortreeCity_Text_NimbusGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_NIMBUS_CITY, 20, 27
	waitstate
	release
	end

FortreeCity_EventScript_NimbusNoBadge::
	msgbox FortreeCity_Text_NimbusNoBadge, MSGBOX_DEFAULT
	release
	end

FortreeCity_EventScript_NimbusNo::
	msgbox FortreeCity_Text_NimbusNo, MSGBOX_DEFAULT
	release
	end

FortreeCity_Text_NimbusNoBadge:
	.string "My balloon flies up to NIMBUS CITY,\n"
	.string "the town above the clouds.\p"
	.string "The winds up there are rough,\n"
	.string "though. Come back when you've\l"
	.string "earned seven badges.$"

FortreeCity_Text_NimbusAsk:
	.string "Seven badges! You can handle the\n"
	.string "winds of the NIMBUS ISLES.\p"
	.string "Shall we fly up to NIMBUS CITY?$"

FortreeCity_Text_NimbusGo:
	.string "Up, up and away!$"

FortreeCity_Text_NimbusNo:
	.string "The balloon's ready when you are.$"
''')
print('ok entrance')
