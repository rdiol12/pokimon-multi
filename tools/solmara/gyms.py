"""Solmara's eight gyms. Each is a new hall drawn from a Hoenn gym's tiles,
laid out with lanes at least two tiles wide so several challengers can be
inside at once without blocking each other. Gym trainers stand beside the
lanes. The leader can be challenged again after the badge (rematches grow
with your badge count too), so friends can come back together.
"""
from sol import *

# 22 x 20 halls: '#' wall, '.' floor. Bottom 3 rows: the entrance is stamped
# in the middle. Leader at the top middle.
PILLARS = '''
######################
######################
#....................#
#....................#
#...####......####...#
#...####......####...#
#....................#
#....................#
#..##....####....##..#
#..##....####....##..#
#....................#
#....................#
#...####......####...#
#...####......####...#
#....................#
#....................#
#....................#
######################
######################
######################
'''
HALLS = '''
######################
######################
#....................#
#....................#
#..######....######..#
#..#....#....#....#..#
#..#....#....#....#..#
#..#..........#...#..#
#........##..........#
#........##..........#
#..#.............##..#
#..#....#....#....#..#
#..######....######..#
#....................#
#....................#
#....................#
#....................#
######################
######################
######################
'''
ZIGZAG = '''
######################
######################
#....................#
#....................#
#..##############....#
#....................#
#....................#
#....##############..#
#....................#
#....................#
#..##############....#
#....................#
#....................#
#....##############..#
#....................#
#....................#
#....................#
######################
######################
######################
'''
ISLANDS = '''
######################
######################
#....................#
#....................#
#..###..######..###..#
#..###..######..###..#
#....................#
#....................#
#.####..........####.#
#.####...####...####.#
#........####........#
#....................#
#..###..######..###..#
#..###..######..###..#
#....................#
#....................#
#....................#
######################
######################
######################
'''

#  town, kind, sketch, leader (name, ow gfx, pic, gender), badge #, TM, team, trainers, texts
GYMS = [
    ('PortSolace', 'water', ISLANDS, ('MARINA', 'OBJ_EVENT_GFX_BEAUTY', 'Swimmer F', 'Female'), 1, 'ITEM_TM_WATER_PULSE',
     [('Pelipper', 52), ('Lanturn', 52), ('Kingdra', 53), ('Golisopod', 53), ('Quaquaval', 55)],
     [('SWIMMER', 'OBJ_EVENT_GFX_SWIMMER_M', 'Swimmer M', 'Swimmer M', 'Male', 'Swimmer', 'DYLAN', [('Floatzel', 50), ('Clawitzer', 50)]),
      ('TUBER', 'OBJ_EVENT_GFX_SWIMMER_F', 'Swimmer F', 'Swimmer F', 'Female', 'Swimmer', 'PERLA', [('Dewgong', 50), ('Araquanid', 51)])],
     'I am MARINA, keeper of PORT SOLACE.', 'Every trainer from HOENN washes', 'up here first. Let\'s see what', 'the tide brought in!'),
    ('Gearholt', 'steel', PILLARS, ('BRANT', 'OBJ_EVENT_GFX_EXPERT_M', 'Expert M', 'Male'), 2, 'ITEM_TM_STEEL_WING',
     [('Magnezone', 52), ('Bronzong', 52), ('Corviknight', 53), ('Excadrill', 53), ('Kingambit', 55)],
     [('WORKER', 'OBJ_EVENT_GFX_HIKER', 'Hiker', 'Hiker', 'Male', 'Hiker', 'RIVET', [('Probopass', 50), ('Klang', 50)]),
      ('ENGINEER', 'OBJ_EVENT_GFX_SCIENTIST_1', 'Ruin Maniac', 'Ruin Maniac', 'Male', 'Hiker', 'BOLT', [('Magneton', 50), ('Ferrothorn', 51)])],
     'BRANT\'s the name. I forge steel', 'and trainers.', 'Let\'s see if you hold your shape', 'under the hammer!'),
    ('Ashcombe', 'fire', ZIGZAG, ('CINDRA', 'OBJ_EVENT_GFX_WOMAN_5', 'Battle Girl', 'Female'), 3, 'ITEM_TM_FLAMETHROWER',
     [('Torkoal', 52), ('Arcanine', 52), ('Ceruledge', 53), ('Volcarona', 53), ('Skeledirge', 55)],
     [('KINDLER', 'OBJ_EVENT_GFX_MAN_5', 'Kindler', 'Kindler', 'Male', 'Male', 'FLINT', [('Magmar', 50), ('Camerupt', 50)]),
      ('COOL', 'OBJ_EVENT_GFX_WOMAN_4', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'EMBER', [('Ninetales', 50), ('Houndoom', 51)])],
     'I\'m CINDRA! The springs keep this', 'town warm, and I keep it hot!', 'Show me your fighting spirit!'),
    ('Silkwood', 'bug', HALLS, ('THISTLE', 'OBJ_EVENT_GFX_MANIAC', 'Bug Maniac', 'Male'), 4, 'ITEM_TM_GIGA_DRAIN',
     [('Galvantula', 52), ('Vikavolt', 52), ('Kleavor', 53), ('Lokix', 53), ('Scizor', 55)],
     [('BUGCATCHER', 'OBJ_EVENT_GFX_BUG_CATCHER', 'Bug Catcher', 'Bug Catcher', 'Male', 'Male', 'MOSS', [('Beautifly', 50), ('Dustox', 50), ('Ninjask', 50)]),
      ('MANIAC', 'OBJ_EVENT_GFX_MANIAC', 'Bug Maniac', 'Bug Maniac', 'Male', 'Suspicious', 'NETT', [('Crustle', 50), ('Durant', 51)])],
     'Welcome to the hive! I\'m', 'THISTLE.', 'Bugs work together. Let\'s see', 'how your team works together!'),
    ('Rosemead', 'fairy', ISLANDS, ('BELLE', 'OBJ_EVENT_GFX_WOMAN_2', 'Lady', 'Female'), 5, 'ITEM_TM_CALM_MIND',
     [('Tinkaton', 52), ('Grimmsnarl', 52), ('Hatterene', 53), ('Sylveon', 53), ('Gardevoir', 55)],
     [('LADY', 'OBJ_EVENT_GFX_WOMAN_1', 'Lady', 'Lady', 'Female', 'Rich', 'ROSE', [('Clefable', 50), ('Togekiss', 51)]),
      ('GIRL', 'OBJ_EVENT_GFX_LASS', 'Lass', 'Lass', 'Female', 'Female', 'PIXIE', [('Azumarill', 50), ('Whimsicott', 50)])],
     'Oh, a visitor! I\'m BELLE.', 'Pretty things can be strong too.', 'Let me show you!'),
    ('Duskmere', 'ghost', ZIGZAG, ('MORWYN', 'OBJ_EVENT_GFX_HEX_MANIAC', 'Hex Maniac', 'Female'), 6, 'ITEM_TM_SHADOW_BALL',
     [('Dhelmise', 52), ('Mimikyu', 52), ('Chandelure', 53), ('Gholdengo', 53), ('Gengar', 55)],
     [('HEX', 'OBJ_EVENT_GFX_HEX_MANIAC', 'Hex Maniac', 'Hex Maniac', 'Female', 'Suspicious', 'NOCTA', [('Banette', 50), ('Cofagrigus', 51)]),
      ('PSYCHIC', 'OBJ_EVENT_GFX_PSYCHIC_M', 'Psychic', 'Psychic M', 'Male', 'Intense', 'SILAS', [('Golurk', 50), ('Drifblim', 50)])],
     'The mist told me you were coming.', 'I am MORWYN.', 'Let us see if your spirit is as', 'strong as your team...'),
    ('Frostholm', 'water', HALLS, ('SIGRID', 'OBJ_EVENT_GFX_EXPERT_F', 'Expert F', 'Female'), 7, 'ITEM_TM_ICE_BEAM',
     [('Weavile', 52), ('Glaceon', 52), ('Cetitan', 53), ('Lapras', 53), ('Baxcalibur', 55)],
     [('SKIER', 'OBJ_EVENT_GFX_WOMAN_5', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'ASTRID', [('Froslass', 50), ('Avalugg', 51)]),
      ('BOARDER', 'OBJ_EVENT_GFX_BOY_2', 'Cooltrainer', 'Cooltrainer M', 'Male', 'Cool', 'ODIN', [('Mamoswine', 50), ('Walrein', 51)])],
     'I am SIGRID. In the north, the', 'cold teaches patience.', 'Your patience ends here!'),
    ('Drakespire', 'dragon', PILLARS, ('VALEN', 'OBJ_EVENT_GFX_MAN_3', 'Dragon Tamer', 'Male'), 8, 'ITEM_TM_DRAGON_CLAW',
     [('Haxorus', 53), ('Goodra', 53), ('Kommo-o', 53), ('Dragonite', 54), ('Garchomp', 55)],
     [('TAMER', 'OBJ_EVENT_GFX_MAN_3', 'Dragon Tamer', 'Dragon Tamer', 'Male', 'Intense', 'WYATT', [('Flygon', 51), ('Altaria', 51)]),
      ('COOLM', 'OBJ_EVENT_GFX_BLACK_BELT', 'Cooltrainer', 'Cooltrainer M', 'Male', 'Cool', 'DRAKE', [('Salamence', 51), ('Noivern', 51)])],
     'I am VALEN. Dragons have nested on', 'this spire for a thousand years.', 'Face their fury!'),
]
# Frostholm's hall uses Sootopolis' ice tiles (the Water gym uses them too;
# MARINA's gym is drawn from the Sootopolis set, SIGRID's from it as well).
BADGE_NAMES = ['TIDE', 'ANVIL', 'CINDER', 'HIVE', 'BLOOM', 'MIST', 'RIME', 'WYRM']


def build(ctx):
    for town, kind, sk, leader, n, tm, team, trainers, *intro in GYMS:
        build_gym_map(ctx, town, kind, sk, leader, n, tm, team, trainers, intro)


def build_gym_map(ctx, town, kind, sk, leader, n, tm, team, trainers, intro):
    name = f'{town}_Gym'
    lname, lgfx, lpic, lgender = leader
    C = f'{name}_EventScript'
    tid = REG.trainer(f'LEADER_{lname}', party(lname, 'Leader', lpic, lgender, 'Intense', team))
    badge = ctx.badge(n)
    bn = BADGE_NAMES[n - 1]
    rows = sk.strip('\n').split('\n')
    w = len(rows[0])
    objs = []
    leader_obj = npc(lgfx, w // 2 - 1, 2, f'{C}_Leader', 'MOVEMENT_TYPE_FACE_DOWN')
    leader_obj['movement_range_x'] = leader_obj['movement_range_y'] = 0
    objs.append(leader_obj)
    sc = f"""{name}_MapScripts::
\t.byte 0

{C}_Leader::
\tgoto_if_set {badge}, {C}_Rematch
\ttrainerbattle_single {tid}, {C}_Text_Intro, {C}_Text_Defeat, {C}_Won, NO_MUSIC
\trelease
\tend

{C}_Won::
\tmessage {C}_Text_GotBadge
\twaitmessage
\tcall Common_EventScript_PlayGymBadgeFanfare
\tsetflag {badge}
\tcallnative Solmara_GetBadgeCount
\tbuffernumberstring STR_VAR_1, VAR_RESULT
\tmsgbox {C}_Text_BadgeInfo, MSGBOX_DEFAULT
\tgiveitem {tm}
\tmsgbox {C}_Text_After, MSGBOX_DEFAULT
\trelease
\tend

{C}_Rematch::
\tlock
\tfaceplayer
\tmsgbox {C}_Text_RematchAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, {C}_RematchNo
\tcleartrainerflag {tid}
\ttrainerbattle_single {tid}, {C}_Text_RematchIntro, {C}_Text_Defeat, {C}_RematchWon, NO_MUSIC
\trelease
\tend

{C}_RematchWon::
\tmsgbox {C}_Text_RematchDone, MSGBOX_DEFAULT
\trelease
\tend

{C}_RematchNo::
\tmsgbox {C}_Text_After, MSGBOX_DEFAULT
\trelease
\tend

{C}_Guide::
\tlock
\tfaceplayer
\tcallnative Solmara_GetBadgeCount
\tbuffernumberstring STR_VAR_1, VAR_RESULT
\tmsgbox {C}_Text_Guide, MSGBOX_DEFAULT
\trelease
\tend

{C}_Text_Intro:
{text(*intro)}
{C}_Text_Defeat:
{text('Well fought! You\'ve earned this.')}
{C}_Text_GotBadge:
{text('{PLAYER} received the', bn + ' BADGE from ' + lname + '!')}
{C}_Text_BadgeInfo:
{text('That makes {STR_VAR_1} SOLMARA badges.', 'Every leader you face from now', 'on will battle a little harder.', '', 'And take this, too!')}
{C}_Text_After:
{text('Come back with your friends', 'any time. A rematch keeps us', 'all sharp!')}
{C}_Text_RematchAsk:
{text('Back for more? My team has grown', 'along with your badges. Battle?')}
{C}_Text_RematchIntro:
{text('Then show me how much stronger', 'you\'ve become!')}
{C}_Text_RematchDone:
{text('Even better than last time!')}
{C}_Text_Guide:
{text('Yo, challenger! This is the ' + town.upper().replace('PORTSOLACE', 'PORT SOLACE') + ' GYM.', '',
      'You have {STR_VAR_1} SOLMARA badges.', 'The leader\'s team grows with', 'every badge you hold.', '',
      'Lots of room in here, so bring', 'your friends along!')}
"""
    guide = npc('OBJ_EVENT_GFX_MAN_2', w // 2 - 5, len(rows) - 4, f'{C}_Guide', 'MOVEMENT_TYPE_FACE_DOWN')
    guide['movement_range_x'] = guide['movement_range_y'] = 0
    objs.append(guide)
    spots = [(3, 10, 'MOVEMENT_TYPE_FACE_RIGHT'), (w - 4, 6, 'MOVEMENT_TYPE_FACE_LEFT')]
    for (key, gfx, cls, pic, gender, music, tname, mons), (x, y, move) in zip(trainers, spots):
        t = REG.trainer(f'GYM_{lname}_{key}', party(tname, cls, pic, gender, music, mons))
        label = f'{C}_{key.title()}'
        while rows[y][x] != '.':
            y += 1
        objs.append({'graphics_id': gfx, 'x': x, 'y': y, 'elevation': 3, 'movement_type': move,
                     'movement_range_x': 0, 'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NORMAL',
                     'trainer_sight_or_berry_tree_id': '3', 'script': label, 'flag': '0'})
        sc += trainer_script(label, t, ['Think you can reach the leader?', 'Get past me first!'], ['Go on, then!'],
                             ['The leader is just ahead. Good', 'luck!'])
    build_gym(name, kind, sk, f'MAPSEC_{snake(town)}', map_id(town), 0, objs, sc)
