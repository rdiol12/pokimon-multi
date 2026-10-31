"""Sunscar Desert and the Sunscar Ruins: a sand basin among rock mesas,
and ruins where three Unown statues ask riddles. Answer each one by
touching the right glyph tablet and the sealed vault opens: the Odd
Keystone, and a Spiritomb.

    python3 tools/areas/desertruins.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import desert, ruins

AREA_NO = 5
MAPSEC = 'MAPSEC_SUNSCAR_DESERT'
SEALED = 'LAYOUT_SEALED_CHAMBER_OUTER_ROOM'
EXIT = 0x207

register_constants('Sunscar', AREA_NO, flags=[
    'SUNSCAR_RIDDLE_1', 'SUNSCAR_RIDDLE_2', 'SUNSCAR_RIDDLE_3', 'HIDE_SUNSCAR_SPIRITOMB', 'SUNSCAR_SPIRITOMB_DONE',
    'SUNSCAR_DIGGER_GIFT', 'ITEM_SUNSCAR_ODD_KEYSTONE', 'ITEM_SUNSCAR_SOFT_SAND', 'ITEM_SUNSCAR_TM_SANDSTORM',
    'ITEM_SUNSCAR_HYPER_POTION', 'ITEM_SUNSCAR_DUSK_BALL', 'ITEM_SUNSCAR_RELIC_GOLD', 'HIDDEN_ITEM_SUNSCAR_STARDUST',
    'HIDDEN_ITEM_SUNSCAR_NUGGET', 'HIDDEN_ITEM_SUNSCAR_RARE_BONE', 'HIDDEN_ITEM_SUNSCAR_RELIC_COPPER',
], trainers=['SUNSCAR_DEXTER', 'SUNSCAR_MILO', 'SUNSCAR_SABLE'])

register_trainers('Sunscar', '''
=== AREA_TRAINER_SUNSCAR_DEXTER ===
Name: DEXTER
Class: Ruin Maniac
Pic: Ruin Maniac
Gender: Male
Music: Hiker
Double Battle: No
AI: Basic Trainer

Baltoy
Level: 30

Sigilyph
Level: 32

=== AREA_TRAINER_SUNSCAR_MILO ===
Name: MILO
Class: Camper
Pic: Camper
Gender: Male
Music: Twins
Double Battle: No
AI: Basic Trainer

Krokorok
Level: 31

Vibrava
Level: 32

=== AREA_TRAINER_SUNSCAR_SABLE ===
Name: SABLE
Class: Expert
Pic: Expert F
Gender: Female
Music: Intense
Double Battle: No
AI: Basic Trainer

Cacturne
Level: 32

Hippowdon
Level: 34

Gabite
Level: 31
''')


def south_exit(a, x, y):
    a.tile(x, y, EXIT, collision=0, elevation=3, c='o')
    for dx, m in ((-1, 0x258), (0, 0x259), (1, 0x25A)):
        a.tile(x + dx, y + 1, m, collision=1, elevation=0, c='o')


# ---------------------------------------------------------------- the desert
d = Area('SunscarDesert', 48, 44, desert(), fill='#')
d.ellipse(24, 24, 21, 18, 's')
d.rect(22, 38, 5, 6, 's')
for x, y, rx, ry in ((12, 16, 4, 3), (36, 31, 5, 3), (31, 13, 3, 2), (17, 30, 3, 2)):
    d.ellipse(x, y, rx, ry, '#')
d.stamp('LAYOUT_ROUTE111', 26, 83, 7, 6, 21, 6)        # the ruin's mouth, door at (24, 10)
for x, y in ((8, 26), (15, 36), (38, 19), (29, 37), (20, 22), (41, 26), (8, 20), (27, 26), (33, 22)):
    d.stamp('LAYOUT_ROUTE111', 25, 82, 2, 2, x, y)    # boulders
d.warp(24, 10, 'MAP_SUNSCAR_RUINS_HALL', 0)
d.obj('OBJ_EVENT_GFX_MAN_4', 25, 41, 'SunscarDesert_EventScript_Guide', 'MOVEMENT_TYPE_FACE_LEFT')
d.obj('OBJ_EVENT_GFX_HIKER', 27, 12, 'SunscarDesert_EventScript_Archaeologist', 'MOVEMENT_TYPE_FACE_LEFT')
d.obj('OBJ_EVENT_GFX_MAN_5', 10, 31, 'SunscarDesert_EventScript_Digger', 'MOVEMENT_TYPE_FACE_DOWN')
d.obj('OBJ_EVENT_GFX_SPECIES(SANDILE)', 33, 27, 'SunscarDesert_EventScript_Sandile', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
d.obj('OBJ_EVENT_GFX_SPECIES(TRAPINCH)', 14, 25, 'SunscarDesert_EventScript_Trapinch', 'MOVEMENT_TYPE_FACE_UP')
d.sign(22, 39, 'SunscarDesert_EventScript_Sign', elevation=0)
d.trainer('OBJ_EVENT_GFX_HIKER', 20, 18, 'SunscarDesert_EventScript_Dexter', 3, 'MOVEMENT_TYPE_FACE_RIGHT')
d.trainer('OBJ_EVENT_GFX_CAMPER', 32, 35, 'SunscarDesert_EventScript_Milo', 3, 'MOVEMENT_TYPE_FACE_LEFT')
d.trainer('OBJ_EVENT_GFX_WOMAN_5', 40, 20, 'SunscarDesert_EventScript_Sable', 3, 'MOVEMENT_TYPE_FACE_DOWN')
d.item('ITEM_SOFT_SAND', 6, 24, 'FLAG_ITEM_SUNSCAR_SOFT_SAND')
d.item('ITEM_TM_SANDSTORM', 43, 24, 'FLAG_ITEM_SUNSCAR_TM_SANDSTORM')
d.item('ITEM_HYPER_POTION', 34, 9, 'FLAG_ITEM_SUNSCAR_HYPER_POTION')
d.hidden(11, 21, 'ITEM_STARDUST', 'FLAG_HIDDEN_ITEM_SUNSCAR_STARDUST')
d.hidden(37, 36, 'ITEM_NUGGET', 'FLAG_HIDDEN_ITEM_SUNSCAR_NUGGET')
d.hidden(24, 30, 'ITEM_RARE_BONE', 'FLAG_HIDDEN_ITEM_SUNSCAR_RARE_BONE')
d.write(map_id='MAP_SUNSCAR_DESERT', layout_id='LAYOUT_SUNSCAR_DESERT', music='MUS_DESERT', mapsec=MAPSEC,
        mapsec_name='SUNSCAR DESERT', weather='WEATHER_SUNNY', after='FrostveinCave_B1F', scripts=r'''
SunscarDesert_MapScripts::
	.byte 0

SunscarDesert_EventScript_Guide::
	lock
	faceplayer
	msgbox SunscarDesert_Text_GuideBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarDesert_EventScript_GuideStay
	msgbox SunscarDesert_Text_GuideLead, MSGBOX_DEFAULT
	closemessage
	warp MAP_MAUVILLE_CITY, 26, 8
	waitstate
	release
	end

SunscarDesert_EventScript_GuideStay::
	msgbox SunscarDesert_Text_GuideStay, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_Sign::
	msgbox SunscarDesert_Text_Sign, MSGBOX_SIGN
	end

SunscarDesert_EventScript_Archaeologist::
	msgbox SunscarDesert_Text_Archaeologist, MSGBOX_NPC
	end

@ The digger gives a Relic Gold once the vault is open.
SunscarDesert_EventScript_Digger::
	lock
	faceplayer
	goto_if_set FLAG_SUNSCAR_DIGGER_GIFT, SunscarDesert_EventScript_DiggerDone
	goto_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarDesert_EventScript_DiggerGift
	msgbox SunscarDesert_Text_Digger, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_DiggerGift::
	msgbox SunscarDesert_Text_DiggerGift, MSGBOX_DEFAULT
	giveitem ITEM_RELIC_GOLD
	goto_if_eq VAR_RESULT, FALSE, SunscarDesert_EventScript_DiggerEnd
	setflag FLAG_SUNSCAR_DIGGER_GIFT
SunscarDesert_EventScript_DiggerEnd::
	release
	end

SunscarDesert_EventScript_DiggerDone::
	msgbox SunscarDesert_Text_DiggerDone, MSGBOX_DEFAULT
	release
	end

SunscarDesert_EventScript_Sandile::
	lock
	faceplayer
	playmoncry SPECIES_SANDILE, CRY_MODE_NORMAL
	msgbox SunscarDesert_Text_Sandile, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarDesert_EventScript_Trapinch::
	lock
	faceplayer
	playmoncry SPECIES_TRAPINCH, CRY_MODE_NORMAL
	msgbox SunscarDesert_Text_Trapinch, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarDesert_EventScript_Dexter::
	trainerbattle_single TRAINER_SUNSCAR_DEXTER, SunscarDesert_Text_DexterIntro, SunscarDesert_Text_DexterDefeat
	msgbox SunscarDesert_Text_DexterPost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_EventScript_Milo::
	trainerbattle_single TRAINER_SUNSCAR_MILO, SunscarDesert_Text_MiloIntro, SunscarDesert_Text_MiloDefeat
	msgbox SunscarDesert_Text_MiloPost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_EventScript_Sable::
	trainerbattle_single TRAINER_SUNSCAR_SABLE, SunscarDesert_Text_SableIntro, SunscarDesert_Text_SableDefeat
	msgbox SunscarDesert_Text_SablePost, MSGBOX_AUTOCLOSE
	end

SunscarDesert_Text_GuideBack:
	.string "Thirsty? I can lead you back to\n"
	.string "MAUVILLE.$"

SunscarDesert_Text_GuideLead:
	.string "Follow my footprints!$"

SunscarDesert_Text_GuideStay:
	.string "Drink plenty of water out there!$"

SunscarDesert_Text_Sign:
	.string "SUNSCAR DESERT\n"
	.string "The ruins to the north are sealed.$"

SunscarDesert_Text_Archaeologist:
	.string "These ruins were built by people\n"
	.string "who worshipped the UNOWN.\p"
	.string "Inside, three UNOWN statues ask\n"
	.string "riddles. The answers are on the\l"
	.string "glyph tablets. Touch the right one!$"

SunscarDesert_Text_Digger:
	.string "I've been digging here for years.\n"
	.string "All I find is sand and old bones.\p"
	.string "If only someone could open that\n"
	.string "sealed vault…$"

SunscarDesert_Text_DiggerGift:
	.string "The vault is open?! You solved the\n"
	.string "riddles! Here, you've earned this.\l"
	.string "I dug it up years ago.$"

SunscarDesert_Text_DiggerDone:
	.string "Back to digging. Maybe there's a\n"
	.string "second vault!$"

SunscarDesert_Text_Sandile:
	.string "SANDILE is basking in the sun with\n"
	.string "its eyes shut.$"

SunscarDesert_Text_Trapinch:
	.string "TRAPINCH is waiting at the bottom\n"
	.string "of its sand pit. Better not step in!$"

SunscarDesert_Text_DexterIntro:
	.string "I've read every glyph in these\n"
	.string "ruins! Except the ones I haven't.$"

SunscarDesert_Text_DexterDefeat:
	.string "That wasn't written in the glyphs!$"

SunscarDesert_Text_DexterPost:
	.string "The UNOWN statues speak in rhymes.\n"
	.string "Think about the sky.$"

SunscarDesert_Text_MiloIntro:
	.string "Camping in the desert is the best.\n"
	.string "No bugs! Well, almost none.$"

SunscarDesert_Text_MiloDefeat:
	.string "Sand in my sleeping bag again.$"

SunscarDesert_Text_MiloPost:
	.string "TRAPINCH dig pits all over. Watch\n"
	.string "your step.$"

SunscarDesert_Text_SableIntro:
	.string "The desert tests everyone. Let me\n"
	.string "test you too.$"

SunscarDesert_Text_SableDefeat:
	.string "You passed.$"

SunscarDesert_Text_SablePost:
	.string "My GABITE hatched here, on the\n"
	.string "hottest day of the year.$"
''')
register_wild('MAP_SUNSCAR_DESERT', 'gSunscarDesert',
              land=[('SANDILE', 29, 31), ('TRAPINCH', 29, 31), ('CACNEA', 29, 31), ('HIPPOPOTAS', 29, 31),
                    ('BALTOY', 30, 32), ('SILICOBRA', 30, 32), ('SANDSHREW', 29, 31), ('MARACTUS', 30, 32),
                    ('KROKOROK', 31, 33), ('VIBRAVA', 32, 33), ('GIBLE', 30, 31), ('SIGILYPH', 32, 33)])

# ---------------------------------------------------------------- the hall of riddles
GLYPHS = {'SUN': (4, 5), 'MOON': (17, 5), 'WAVE': (4, 11), 'FLAME': (17, 11)}
ANSWERS = ['SUN', 'MOON', 'WAVE']
DOOR = (10, 1)                                  # the 5x2 sealed entrance; the door cell is (12, 2)
OPEN = [(0x22A, 0x22B, 0x22C), (0x232, 0x233, 0x234)]
SHUT = [(0x229,) * 3, (0x235,) * 3]

h = Area('SunscarRuins_Hall', 24, 20, ruins(), fill='#')
h.rect(3, 3, 18, 14, '.')
h.stamp(SEALED, 8, 1, 5, 2, *DOOR)
for (x, y) in GLYPHS.values():
    h.stamp(SEALED, 15, 5, 3, 2, x, y)
for dy in range(2):                             # sealed until the riddles are answered
    for i in range(3):
        h.tile(DOOR[0] + 1 + i, DOOR[1] + dy, SHUT[dy][i], collision=1, elevation=0, c='o')
south_exit(h, 12, 17)
h.rect(11, 16, 3, 1, '.')
h.warp(12, 17, 'MAP_SUNSCAR_DESERT', 0)
h.warp(12, 2, 'MAP_SUNSCAR_RUINS_VAULT', 0)
for name, (x, y) in GLYPHS.items():
    h.sign(x + 1, y + 1, f'SunscarRuins_Hall_EventScript_Glyph{name.title()}', 'BG_EVENT_PLAYER_FACING_NORTH')
for i, x in enumerate((8, 12, 16), 1):
    h.obj('OBJ_EVENT_GFX_SPECIES(UNOWN)', x, 9, f'SunscarRuins_Hall_EventScript_Statue{i}', 'MOVEMENT_TYPE_FACE_DOWN')

OPEN_LINES = ''.join(f'\tsetmetatile {DOOR[0] + 1 + i}, {DOOR[1] + dy}, {OPEN[dy][i]}, {"FALSE" if (dy, i) == (1, 1) else "TRUE"}\n'
                     for dy in range(2) for i in range(3))
TEXTS = {
    'SUN': ('the RISING SUN', ('Each dawn I climb the eastern stair', 'and warm the sand and burn the air.')),
    'MOON': ('the CRESCENT MOON', ('I wear the light of one who goes', 'and rule the sky when daylight closes.')),
    'WAVE': ('the ROLLING WAVE', ('Without me every flower dies.', 'The desert dreams of me, and sighs.')),
    'FLAME': ('the DANCING FLAME', None),
}


def glyph_script(name, glyph):
    idx = ANSWERS.index(name) + 1 if name in ANSWERS else None
    s = f'''SunscarRuins_Hall_EventScript_Glyph{name.title()}::
	lockall
	msgbox SunscarRuins_Hall_Text_Glyph{name.title()}, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, SunscarRuins_Hall_EventScript_GlyphLeave
'''
    if idx is None:
        return s + '\tgoto SunscarRuins_Hall_EventScript_Nothing\n\n'
    prev = f'\tgoto_if_unset FLAG_SUNSCAR_RIDDLE_{idx - 1}, SunscarRuins_Hall_EventScript_Nothing\n' if idx > 1 else ''
    s += prev + f'''	goto_if_set FLAG_SUNSCAR_RIDDLE_{idx}, SunscarRuins_Hall_EventScript_Nothing
	setflag FLAG_SUNSCAR_RIDDLE_{idx}
	playse SE_M_REFLECT
	waitse
	msgbox SunscarRuins_Hall_Text_StatueGlows, MSGBOX_DEFAULT
'''
    if idx == 3:
        s += '\tgoto SunscarRuins_Hall_EventScript_OpenVault\n\n'
    else:
        s += '\treleaseall\n\tend\n\n'
    return s


def statue_script(i):
    nxt = ANSWERS[i - 1]
    s = f'''SunscarRuins_Hall_EventScript_Statue{i}::
	lock
	goto_if_set FLAG_SUNSCAR_RIDDLE_{i}, SunscarRuins_Hall_EventScript_StatueSolved
'''
    if i > 1:
        s += f'\tgoto_if_unset FLAG_SUNSCAR_RIDDLE_{i - 1}, SunscarRuins_Hall_EventScript_StatueSilent\n'
    s += f'''	playmoncry SPECIES_UNOWN, CRY_MODE_NORMAL
	msgbox SunscarRuins_Hall_Text_Riddle{i}, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SunscarRuins_Hall_Text_Riddle{i}:
	.string "The UNOWN statue whispers…\\p"
	.string "{TEXTS[nxt][1][0]}\\n"
	.string "{TEXTS[nxt][1][1]}$"

'''
    return s


hall_scripts = r'''
SunscarRuins_Hall_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SunscarRuins_Hall_OnLoad
	.byte 0

@ Once the third riddle is answered the vault stays open.
SunscarRuins_Hall_OnLoad:
	call_if_set FLAG_SUNSCAR_RIDDLE_3, SunscarRuins_Hall_EventScript_OpenDoor
	end

SunscarRuins_Hall_EventScript_OpenDoor::
''' + OPEN_LINES + r'''	return

SunscarRuins_Hall_EventScript_OpenVault::
	msgbox SunscarRuins_Hall_Text_Rumble, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 5
	special ShakeCamera
	waitstate
	playse SE_BANG
	call SunscarRuins_Hall_EventScript_OpenDoor
	special DrawWholeMapView
	delay 40
	msgbox SunscarRuins_Hall_Text_Opened, MSGBOX_DEFAULT
	releaseall
	end

SunscarRuins_Hall_EventScript_Nothing::
	msgbox SunscarRuins_Hall_Text_Nothing, MSGBOX_DEFAULT
SunscarRuins_Hall_EventScript_GlyphLeave::
	releaseall
	end

SunscarRuins_Hall_EventScript_StatueSolved::
	msgbox SunscarRuins_Hall_Text_StatueSolved, MSGBOX_DEFAULT
	release
	end

SunscarRuins_Hall_EventScript_StatueSilent::
	msgbox SunscarRuins_Hall_Text_StatueSilent, MSGBOX_DEFAULT
	release
	end

''' + ''.join(glyph_script(n, g) for n, g in GLYPHS.items()) + ''.join(statue_script(i) for i in (1, 2, 3)) + ''.join(
    f'''SunscarRuins_Hall_Text_Glyph{n.title()}:
	.string "A glyph of {TEXTS[n][0]}.\\n"
	.string "Press your hand to it?$"

''' for n in GLYPHS) + r'''SunscarRuins_Hall_Text_StatueGlows:
	.string "One of the UNOWN statues began to\n"
	.string "glow!$"

SunscarRuins_Hall_Text_Nothing:
	.string "The glyph is cold. Nothing happens.$"

SunscarRuins_Hall_Text_StatueSolved:
	.string "The UNOWN statue glows softly.$"

SunscarRuins_Hall_Text_StatueSilent:
	.string "The UNOWN statue is silent. Maybe\n"
	.string "another one speaks first.$"

SunscarRuins_Hall_Text_Rumble:
	.string "All three statues are glowing!\n"
	.string "The ruins begin to rumble…$"

SunscarRuins_Hall_Text_Opened:
	.string "The sealed door to the north has\n"
	.string "opened!$"
'''
h.write(map_id='MAP_SUNSCAR_RUINS_HALL', layout_id='LAYOUT_SUNSCAR_RUINS_HALL', music='MUS_SEALED_CHAMBER', mapsec=MAPSEC,
        mapsec_name='SUNSCAR DESERT', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False, allow_escaping=True,
        after='SunscarDesert', show_name=False, scripts=hall_scripts)

# ---------------------------------------------------------------- the vault
v = Area('SunscarRuins_Vault', 20, 16, ruins(), fill='#')
v.rect(3, 3, 14, 10, '.')
for x, y in ((5, 5), (12, 5)):
    v.stamp(SEALED, 15, 5, 3, 2, x, y)
south_exit(v, 10, 13)
v.rect(9, 12, 3, 1, '.')
v.warp(10, 13, 'MAP_SUNSCAR_RUINS_HALL', 1)
v.obj('OBJ_EVENT_GFX_SPECIES(SPIRITOMB)', 10, 5, 'SunscarRuins_Vault_EventScript_Spiritomb', 'MOVEMENT_TYPE_FACE_DOWN',
      'FLAG_HIDE_SUNSCAR_SPIRITOMB', local_id='LOCALID_SUNSCAR_SPIRITOMB')
v.item('ITEM_ODD_KEYSTONE', 9, 4, 'FLAG_ITEM_SUNSCAR_ODD_KEYSTONE')
v.item('ITEM_DUSK_BALL', 4, 10, 'FLAG_ITEM_SUNSCAR_DUSK_BALL')
v.item('ITEM_RELIC_GOLD', 15, 10, 'FLAG_ITEM_SUNSCAR_RELIC_GOLD')
v.hidden(4, 4, 'ITEM_RELIC_COPPER', 'FLAG_HIDDEN_ITEM_SUNSCAR_RELIC_COPPER')
v.write(map_id='MAP_SUNSCAR_RUINS_VAULT', layout_id='LAYOUT_SUNSCAR_RUINS_VAULT', music='MUS_SEALED_CHAMBER',
        mapsec=MAPSEC, mapsec_name='SUNSCAR DESERT', map_type='MAP_TYPE_UNDERGROUND', allow_cycling=False,
        allow_escaping=True, after='SunscarRuins_Hall', show_name=False, scripts=r'''
SunscarRuins_Vault_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SunscarRuins_Vault_OnTransition
	.byte 0

SunscarRuins_Vault_OnTransition:
	call_if_set FLAG_SUNSCAR_SPIRITOMB_DONE, SunscarRuins_Vault_EventScript_Hide
	end

SunscarRuins_Vault_EventScript_Hide::
	setflag FLAG_HIDE_SUNSCAR_SPIRITOMB
	return

SunscarRuins_Vault_EventScript_Spiritomb::
	lock
	faceplayer
	msgbox SunscarRuins_Vault_Text_Spirits, MSGBOX_DEFAULT
	setwildbattle SPECIES_SPIRITOMB, 34
	playmoncry SPECIES_SPIRITOMB, CRY_MODE_ENCOUNTER
	msgbox SunscarRuins_Vault_Text_Spiritomb, MSGBOX_DEFAULT
	waitmoncry
	delay 20
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SunscarRuins_Vault_EventScript_Stays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SunscarRuins_Vault_EventScript_Stays
	setflag FLAG_SUNSCAR_SPIRITOMB_DONE
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, SunscarRuins_Vault_EventScript_End
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_SUNSCAR_SPIRITOMB
	setflag FLAG_HIDE_SUNSCAR_SPIRITOMB
	fadescreen FADE_FROM_BLACK
	msgbox SunscarRuins_Vault_Text_Faded, MSGBOX_DEFAULT
SunscarRuins_Vault_EventScript_End::
	release
	end

SunscarRuins_Vault_EventScript_Stays::
	msgbox SunscarRuins_Vault_Text_Stays, MSGBOX_DEFAULT
	release
	end

SunscarRuins_Vault_Text_Spirits:
	.string "Dozens of faint voices whisper from\n"
	.string "the old stone…$"

SunscarRuins_Vault_Text_Spiritomb:
	.string "Kuhehehe!$"

SunscarRuins_Vault_Text_Faded:
	.string "SPIRITOMB faded back into the\n"
	.string "stone.$"

SunscarRuins_Vault_Text_Stays:
	.string "The voices are still whispering.$"
''')
register_wild('MAP_SUNSCAR_RUINS_VAULT', 'gSunscarRuinsVault',
              land=[('UNOWN', 26, 26), ('UNOWN', 28, 28), ('UNOWN', 30, 30), ('UNOWN', 32, 32), ('UNOWN', 26, 32),
                    ('UNOWN', 26, 32), ('UNOWN', 26, 32), ('UNOWN', 26, 32), ('UNOWN', 26, 32), ('UNOWN', 26, 32),
                    ('UNOWN', 26, 32), ('UNOWN', 26, 32)], land_rate=5)
print('ok Sunscar')

# ---------------------------------------------------------------- the way in, from Mauville City
add_object('MauvilleCity', {'graphics_id': 'OBJ_EVENT_GFX_MAN_4', 'x': 26, 'y': 7, 'elevation': 3,
                            'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                            'script': 'MauvilleCity_EventScript_SunscarGuide', 'flag': '0'})
register_script_file('sunscar', r'''
@ Sunscar Desert guide in Mauville City (tools/areas/desertruins.py).
MauvilleCity_EventScript_SunscarGuide::
	lock
	faceplayer
	goto_if_unset FLAG_BADGE04_GET, MauvilleCity_EventScript_SunscarGuideNoBadge
	msgbox MauvilleCity_Text_SunscarGuideAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MauvilleCity_EventScript_SunscarGuideNo
	msgbox MauvilleCity_Text_SunscarGuideGo, MSGBOX_DEFAULT
	closemessage
	warp MAP_SUNSCAR_DESERT, 24, 41
	waitstate
	release
	end

MauvilleCity_EventScript_SunscarGuideNoBadge::
	msgbox MauvilleCity_Text_SunscarGuideNoBadge, MSGBOX_DEFAULT
	release
	end

MauvilleCity_EventScript_SunscarGuideNo::
	msgbox MauvilleCity_Text_SunscarGuideNo, MSGBOX_DEFAULT
	release
	end

MauvilleCity_Text_SunscarGuideNoBadge:
	.string "Deep in the desert there are ruins\n"
	.string "full of UNOWN riddles.\p"
	.string "It's a harsh trip. Come back with\n"
	.string "FLANNERY's badge and I'll guide you.$"

MauvilleCity_Text_SunscarGuideAsk:
	.string "FLANNERY's badge! You're ready for\n"
	.string "the desert.\p"
	.string "Shall I guide you to SUNSCAR\n"
	.string "DESERT?$"

MauvilleCity_Text_SunscarGuideGo:
	.string "Keep your head covered!$"

MauvilleCity_Text_SunscarGuideNo:
	.string "The ruins have kept their secrets\n"
	.string "this long. They can wait.$"
''')
print('ok entrance')
