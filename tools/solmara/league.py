"""Victory Road and the Solmara League in the middle of the ring.

Victory Road is a cave north of Port Solace (open with all eight Solmara
badges). It comes out below the League, an Elite Four of four rooms and the
Champion. The run restarts from the first room whenever you walk back into
the lobby, so nobody's progress depends on anybody else, and the lobby has
screens for watching live PvP battles and the ladder: the region's meeting
place for its strongest trainers.
"""
from sol import *
from styles import TREES

LOBBY = 'SolmaraLeague_Lobby'
OUT = 'SolmaraLeague'
VR = 'SolmaraVictoryRoad'
MAPSEC = 'MAPSEC_SOLMARA_LEAGUE'

E4 = [  # key, name, pic, ow, layout, team, intro, defeat
    ('VESPER', 'VESPER', 'Gentleman', 'OBJ_EVENT_GFX_GENTLEMAN', 'LAYOUT_EVER_GRANDE_CITY_SIDNEYS_ROOM',
     [('Honchkrow', 58), ('Krookodile', 58), ('Zoroark', 59), ('Umbreon', 59), ('Kingambit', 60)],
     ['Good evening. I am VESPER of the', 'ELITE FOUR.', '', 'Darkness hides many things.', 'Let\'s see what yours hides!'],
     ['The night is over.']),
    ('NETTLE', 'NETTLE', 'Psychic F', 'OBJ_EVENT_GFX_WOMAN_4', 'LAYOUT_EVER_GRANDE_CITY_PHOEBES_ROOM',
     [('Toxapex', 58), ('Salazzle', 59), ('Glimmora', 59), ('Toxtricity', 59), ('Nidoking', 60)],
     ['I\'m NETTLE! My POKéMON sting, bite', 'and poison.', '', 'Still want to battle? Of course', 'you do!'],
     ['Ouch! That stung!']),
    ('DUNE', 'DUNE', 'Ruin Maniac', 'OBJ_EVENT_GFX_HIKER', 'LAYOUT_EVER_GRANDE_CITY_GLACIAS_ROOM',
     [('Hippowdon', 58), ('Gliscor', 59), ('Donphan', 59), ('Clodsire', 59), ('Rhyperior', 60)],
     ['DUNE\'s the name. The ground', 'under your feet is mine.', '', 'Let\'s shake things up!'],
     ['The ground gave way...']),
    ('ORION', 'ORION', 'Psychic M', 'OBJ_EVENT_GFX_PSYCHIC_M', 'LAYOUT_EVER_GRANDE_CITY_DRAKES_ROOM',
     [('Slowking', 58), ('Espeon', 59), ('Armarouge', 59), ('Reuniclus', 59), ('Alakazam', 60)],
     ['I am ORION, last of the ELITE', 'FOUR. I saw this battle in the', 'stars.', '', 'I did not see who won.'],
     ['The stars were right about you.']),
]
CHAMPION = ('SOLENNE', 'Expert F', 'OBJ_EVENT_GFX_EXPERT_F',
            [('Togekiss', 63), ('Lucario', 63), ('Gyarados', 63), ('Volcarona', 63), ('Metagross', 64), ('Tyranitar', 65)])


def south_exit(a, x, y):
    """A cave way out: an arrow on the floor with the wall below it."""
    a.tile(x, y, 0x207, collision=0, elevation=3, c='o')
    for dx, m in ((-1, 0x258), (0, 0x259), (1, 0x25A)):
        a.tile(x + dx, y + 1, m, collision=1, elevation=0, c='o')


def victory_road(ctx):
    st = Style('gTileset_General', 'gTileset_Cave', ['LAYOUT_VICTORY_ROAD_1F', 'LAYOUT_VICTORY_ROAD_B1F'])
    st.min_count = 2
    st.exclude |= {0x207, 0x258, 0x259, 0x25A, 0x205, 0x209}
    W, H = 40, 44
    a = Area(VR, W, H, st, fill='#')
    # Wide galleries joined by 3-4 wide tunnels; two ways up so crowds split.
    a.rect(15, 34, 10, 7, '.')        # the entrance hall
    a.rect(4, 26, 12, 8, '.')         # west gallery
    a.rect(25, 24, 11, 10, '.')       # east gallery
    a.rect(17, 14, 8, 10, '.')        # the central cavern
    a.rect(4, 8, 10, 10, '.')         # north-west gallery
    a.rect(24, 4, 12, 8, '.')         # north-east gallery, with the exit
    a.line([(18, 36), (9, 30)], 4, '.')
    a.line([(22, 36), (30, 28)], 4, '.')
    a.line([(9, 28), (8, 12)], 3, '.')
    a.line([(30, 26), (21, 18)], 3, '.')
    a.line([(9, 12), (20, 16)], 3, '.')
    a.line([(21, 15), (30, 7)], 3, '.')
    a.rect(14, 41, 12, 1, '.')
    entrance = (20, 41)
    south_exit(a, entrance[0], entrance[1])
    exit_ = (31, 11)
    a.rect(29, 4, 6, 7, '.')
    south_exit(a, exit_[0], exit_[1])
    a.warp(*entrance, map_id('PortSolace'), 4)
    a.warp(*exit_, map_id(OUT), 0)
    C = f'{VR}_EventScript'
    sc = f'{VR}_MapScripts::\n\t.byte 0\n\n'
    trainers = [
        ('ACE_KAI', 'OBJ_EVENT_GFX_BLACK_BELT', 'Cooltrainer', 'Cooltrainer M', 'Male', 'Cool', 'KAI',
         [('Lucario', 54), ('Conkeldurr', 54), ('Hawlucha', 55)], (12, 29), 'MOVEMENT_TYPE_FACE_RIGHT'),
        ('ACE_LYRA', 'OBJ_EVENT_GFX_WOMAN_5', 'Cooltrainer', 'Cooltrainer F', 'Female', 'Cool', 'LYRA',
         [('Gardevoir', 54), ('Milotic', 55), ('Roserade', 54)], (27, 27), 'MOVEMENT_TYPE_FACE_LEFT'),
        ('ACE_ROLF', 'OBJ_EVENT_GFX_EXPERT_M', 'Expert', 'Expert M', 'Male', 'Intense', 'ROLF',
         [('Steelix', 55), ('Golem', 54), ('Tyranitar', 55)], (18, 19), 'MOVEMENT_TYPE_FACE_RIGHT'),
        ('ACE_NOVA', 'OBJ_EVENT_GFX_EXPERT_F', 'Expert', 'Expert F', 'Female', 'Intense', 'NOVA',
         [('Dragapult', 55), ('Hydreigon', 55), ('Flygon', 54)], (6, 11), 'MOVEMENT_TYPE_FACE_DOWN'),
    ]
    for key, gfx, cls, pic, gender, music, tname, mons, (x, y), move in trainers:
        t = REG.trainer(f'VR_{key}', party(tname, cls, pic, gender, music, mons))
        a.trainer(gfx, x, y, f'{C}_{key.title()}', 3, move)
        sc += trainer_script(f'{C}_{key.title()}', t, ['Only the best get this far.', 'Are you one of them?'],
                             ['You are one of them.'], ['The LEAGUE is just ahead. Rest', 'up before you go in.'])
    for i, (it, x, y) in enumerate([('ITEM_FULL_RESTORE', 6, 31), ('ITEM_MAX_ELIXIR', 34, 31), ('ITEM_RARE_CANDY', 23, 16),
                                    ('ITEM_TM_EARTHQUAKE', 5, 9), ('ITEM_MAX_REVIVE', 26, 5)]):
        a.item(it, x, y, REG.flag(f'ITEM_VR_{i}'))
    a.hidden(24, 39, 'ITEM_BIG_NUGGET', REG.flag('HIDDEN_VR_0'))
    register_wild(map_id(VR), f'g{VR}', land=[
        ('GOLURK', 50, 52), ('GARGANACL', 50, 52), ('KLINKLANG', 50, 52), ('CROBAT', 50, 52), ('GRAVELER', 50, 52),
        ('DURALUDON', 51, 53), ('STEELIX', 50, 52), ('CROBAT', 50, 52), ('HAXORUS', 52, 54), ('GOODRA', 52, 54),
        ('GARCHOMP', 53, 55), ('SALAMENCE', 53, 55)], land_rate=8)
    a.write(map_id=map_id(VR), layout_id='LAYOUT_SOLMARA_VICTORY_ROAD', music='MUS_VICTORY_ROAD', mapsec=MAPSEC,
            mapsec_name='SOLMARA LEAGUE', map_type='MAP_TYPE_UNDERGROUND', group=GROUP, scripts=sc,
            battle_scene='MAP_BATTLE_SCENE_NORMAL', allow_cycling=False, allow_escaping=True)


def outside(ctx):
    st = Style('gTileset_General', 'gTileset_EverGrande', ['LAYOUT_EVER_GRANDE_CITY'], wall_classes={'T': TREES},
               colors=True)
    W, H = 30, 34
    a = Area(OUT, W, H, st, fill='#')
    a.stamp('LAYOUT_EVER_GRANDE_CITY', 5, 0, 30, 22, 0, 0)       # the League and its terraces
    a.rect(4, 22, 22, 8, 'g')
    a.rect(10, 20, 10, 3, 'g')
    a.stamp('LAYOUT_EVER_GRANDE_CITY', 14, 37, 9, 5, 10, 28)     # a cliff with a cave mouth (Victory Road)
    cave = (14, 31)
    door = (13, 5)
    a.warp(*cave, map_id(VR), 1)
    a.warp(*door, map_id(LOBBY), 0)
    a.warp(door[0] + 1, door[1], map_id(LOBBY), 0)
    C = f'{OUT}_EventScript'
    sc = f"""{OUT}_MapScripts::
\t.byte 0

""" + signpost(f'{C}_Sign', ['SOLMARA POKéMON LEAGUE', 'Where the ring\'s strongest meet.'])
    a.sign(16, 20, f'{C}_Sign')
    a.obj('OBJ_EVENT_GFX_COOLTRAINER_M' if False else 'OBJ_EVENT_GFX_MAN_3', 8, 24, f'{C}_Fan', 'MOVEMENT_TYPE_WANDER_AROUND', rx=2, ry=1)
    sc += talker(f'{C}_Fan', ['Everyone who made it up VICTORY', 'ROAD hangs out here. It\'s like a', 'club for champions!'])
    a.write(map_id=map_id(OUT), layout_id='LAYOUT_SOLMARA_LEAGUE', music='MUS_EVER_GRANDE', mapsec=MAPSEC,
            mapsec_name='SOLMARA LEAGUE', map_type='MAP_TYPE_CITY', group=GROUP, scripts=sc)
    return cave


def lobby(ctx):
    S = f'{LOBBY}_EventScript'
    nurse = npc('OBJ_EVENT_GFX_NURSE', 3, 2, f'{S}_Nurse', local_id='LOCALID_SOLMARA_LEAGUE_NURSE')
    clerk = npc('OBJ_EVENT_GFX_MART_EMPLOYEE', 16, 2, f'{S}_Clerk')
    flight = npc('OBJ_EVENT_GFX_GENTLEMAN', 14, 5, f'{S}_Flight', 'MOVEMENT_TYPE_FACE_LEFT')
    screens = npc('OBJ_EVENT_GFX_REPORTER_F', 5, 5, f'{S}_Screens', 'MOVEMENT_TYPE_FACE_RIGHT')
    for o in (nurse, clerk, flight, screens):
        o['movement_range_x'] = o['movement_range_y'] = 0
    fans = [npc('OBJ_EVENT_GFX_MAN_4', 8, 8, f'{S}_Fan', 'MOVEMENT_TYPE_WANDER_AROUND')]
    reset = ''.join(f'\tclearflag {REG.flag("E4_" + k)}\n' for k, *_ in E4)
    sc = f"""{LOBBY}_MapScripts::
\tmap_script MAP_SCRIPT_ON_TRANSITION, {LOBBY}_OnTransition
\t.byte 0

@ Walking into the lobby starts a fresh run at the Elite Four.
{LOBBY}_OnTransition:
{reset}\tend

{S}_Nurse::
\tsetvar VAR_0x800B, LOCALID_SOLMARA_LEAGUE_NURSE
\tcall Common_EventScript_PkmnCenterNurse
\twaitmessage
\twaitbuttonpress
\trelease
\tend

{S}_Clerk::
\tlock
\tfaceplayer
\tmessage gText_HowMayIServeYou
\twaitmessage
\tpokemart {S}_Items
\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
\trelease
\tend

\t.align 2
{S}_Items:
\t.2byte ITEM_ULTRA_BALL
\t.2byte ITEM_FULL_RESTORE
\t.2byte ITEM_MAX_POTION
\t.2byte ITEM_MAX_REVIVE
\t.2byte ITEM_FULL_HEAL
\t.2byte ITEM_MAX_ETHER
\t.2byte ITEM_MAX_REPEL
\tpokemartlistend

{S}_Flight::
\tsetvar VAR_0x8008, 255
\tgoto Solmara_EventScript_FlightDesk

@ Live PvP battles and the ladder, on the lobby's big screens.
{S}_Screens::
\tlock
\tfaceplayer
\tmessage {S}_Text_Screens
\twaitmessage
\tmultichoice 18, 6, MULTI_YESNO, FALSE
\tgoto_if_eq VAR_RESULT, 0, {S}_Watch
\tmsgbox {S}_Text_Ladder, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, YES, {S}_Ladder
\trelease
\tend

{S}_Watch::
\tclosemessage
\tlockall
\tcallnative ShowNetplayWatch
\tend

{S}_Ladder::
\tclosemessage
\tlockall
\tcallnative ShowNetplayLadder
\tend

{S}_Text_Screens:
{text('Welcome to the SOLMARA LEAGUE!', 'Want to watch the live PvP', 'battles on the big screen?')}
{S}_Text_Ladder:
{text('Then how about the PvP LADDER?')}
""" + talker(f'{S}_Fan', ['When it\'s busy, there\'s a crowd', 'watching the big screen and', 'cheering for their friends!'])
    bgs = [{'type': 'sign', 'x': x, 'y': 1, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
            'script': f'{S}_Screens'} for x in (6, 7)]
    interior(LOBBY, map_id(LOBBY), 'LAYOUT_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F', [(9, 11), (10, 11)], map_id(OUT), 1,
             [nurse, clerk, flight, screens] + fans, sc, MAPSEC, GROUP, music='MUS_EVER_GRANDE',
             extra_warps=[{'x': 9, 'y': 1, 'elevation': 0, 'dest_map': map_id('SolmaraLeague_Vesper'), 'dest_warp_id': '0'},
                          {'x': 10, 'y': 1, 'elevation': 0, 'dest_map': map_id('SolmaraLeague_Vesper'), 'dest_warp_id': '0'}])


def rooms(ctx):
    names = [f'SolmaraLeague_{k.title()}' for k, *_ in E4] + ['SolmaraLeague_Champion', 'SolmaraLeague_Hall']
    for i, (key, nm, pic, ow, layout, team, intro, defeat) in enumerate(E4):
        name = names[i]
        S = f'{name}_EventScript'
        fl = REG.flag(f'E4_{key}')
        tid = REG.trainer(f'E4_{key}', party(nm, 'Elite Four', pic, 'Male', 'Elite Four', team))
        o = npc(ow, 6, 5, f'{S}_Member', 'MOVEMENT_TYPE_FACE_DOWN')
        o['movement_range_x'] = o['movement_range_y'] = 0
        sc = f"""{name}_MapScripts::
\tmap_script MAP_SCRIPT_ON_LOAD, {name}_OnLoad
\t.byte 0

{name}_OnLoad:
\tcall_if_set {fl}, PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
\tend

{S}_Member::
\tlock
\tfaceplayer
\tgoto_if_set {fl}, {S}_After
\tplaybgm MUS_ENCOUNTER_ELITE_FOUR, FALSE
\tmsgbox {S}_Text_Intro, MSGBOX_DEFAULT
\tcleartrainerflag {tid}
\ttrainerbattle_no_intro {tid}, {S}_Text_Defeat
\tsetflag {fl}
\tcall PokemonLeague_EliteFour_SetAdvanceToNextRoomMetatiles
\tmsgbox {S}_Text_After, MSGBOX_DEFAULT
\trelease
\tend

{S}_After::
\tmsgbox {S}_Text_After, MSGBOX_DEFAULT
\trelease
\tend

{S}_Text_Intro:
{text(*intro)}
{S}_Text_Defeat:
{text(*defeat)}
{S}_Text_After:
{text('Go on. The next room awaits.')}
"""
        interior(name, map_id(name), layout, [(6, 13)], map_id(LOBBY), 0, [o], sc, MAPSEC, GROUP,
                 music='MUS_RG_VICTORY_ROAD' if False else 'MUS_NONE',
                 extra_warps=[{'x': 6, 'y': 2, 'elevation': 0, 'dest_map': map_id(names[i + 1]), 'dest_warp_id': '0'}])
    # The Champion.
    name = names[4]
    S = f'{name}_EventScript'
    cn, cpic, cow, cteam = CHAMPION
    tid = REG.trainer('CHAMPION_SOLENNE', party(cn, 'Champion', cpic, 'Female', 'Elite Four', cteam))
    champ = REG.flag('CHAMPION')
    beat = REG.flag('CHAMPION_THIS_RUN')
    o = npc(cow, 6, 5, f'{S}_Champion', 'MOVEMENT_TYPE_FACE_DOWN')
    o['movement_range_x'] = o['movement_range_y'] = 0
    sc = f"""{name}_MapScripts::
\tmap_script MAP_SCRIPT_ON_TRANSITION, {name}_OnTransition
\t.byte 0

{name}_OnTransition:
\tclearflag {beat}
\tend

{S}_Champion::
\tlock
\tfaceplayer
\tgoto_if_set {beat}, {S}_After
\tplaybgm MUS_ENCOUNTER_CHAMPION, FALSE
\tmsgbox {S}_Text_Intro, MSGBOX_DEFAULT
\tcleartrainerflag {tid}
\ttrainerbattle_no_intro {tid}, {S}_Text_Defeat
\tsetflag {beat}
\tcallnative Solmara_RecordLeagueClear
\tgoto_if_set {champ}, {S}_Again
\tsetflag {champ}
\tmsgbox {S}_Text_First, MSGBOX_DEFAULT
\tgiveitem ITEM_MASTER_BALL
\tmsgbox {S}_Text_Hall, MSGBOX_DEFAULT
\trelease
\tend

{S}_Again::
\tbuffernumberstring STR_VAR_1, VAR_RESULT
\tmsgbox {S}_Text_Again, MSGBOX_DEFAULT
\tgiveitem ITEM_RARE_CANDY, 3
\tmsgbox {S}_Text_Hall, MSGBOX_DEFAULT
\trelease
\tend

{S}_After::
\tmsgbox {S}_Text_Hall, MSGBOX_DEFAULT
\trelease
\tend

{S}_Text_Intro:
{text('So you crossed the whole ring and', 'climbed VICTORY ROAD.', '', 'I am SOLENNE, Champion of', 'SOLMARA. Every trainer who came', 'over from HOENN has to get past', 'me. Very few have!', '', 'Show me everything!')}
{S}_Text_Defeat:
{text('Magnificent! You are SOLMARA\'s', 'new Champion!')}
{S}_Text_First:
{text('The first time is special. Take', 'this, Champion.')}
{S}_Text_Again:
{text('That\'s {{STR_VAR_1}} wins at this LEAGUE!', 'You keep getting stronger.').replace('{{STR_VAR_1}}', '{STR_VAR_1}')}
{S}_Text_Hall:
{text('Go through to the hall. Your', 'name belongs there.')}
"""
    interior(name, map_id(name), 'LAYOUT_EVER_GRANDE_CITY_CHAMPIONS_ROOM', [(6, 12)], map_id(LOBBY), 0, [o], sc, MAPSEC,
             GROUP, music='MUS_NONE',
             extra_warps=[{'x': 6, 'y': 2, 'elevation': 0, 'dest_map': map_id(names[5]), 'dest_warp_id': '0'}])
    # The hall: a keeper, and the way back down.
    name = names[5]
    S = f'{name}_EventScript'
    keeper = npc('OBJ_EVENT_GFX_GENTLEMAN', 7, 4, f'{S}_Keeper', 'MOVEMENT_TYPE_FACE_DOWN')
    keeper['movement_range_x'] = keeper['movement_range_y'] = 0
    sc = f"""{name}_MapScripts::
\t.byte 0

{S}_Keeper::
\tlock
\tfaceplayer
\tgoto_if_unset {beat}, {S}_NotYet
\tmsgbox {S}_Text_Keeper, MSGBOX_DEFAULT
\tclosemessage
\twarp {map_id(LOBBY)}, 9, 10
\twaitstate
\trelease
\tend

{S}_NotYet::
\tmsgbox {S}_Text_NotYet, MSGBOX_DEFAULT
\trelease
\tend

{S}_Text_Keeper:
{text('Your team\'s names are written in', 'the SOLMARA HALL forever.', '', 'Your friends will see them on', 'the HALL OF FAME wall online.', 'Back to the lobby you go!')}
{S}_Text_NotYet:
{text('Only a Champion may stand here.')}
"""
    interior(name, map_id(name), 'LAYOUT_EVER_GRANDE_CITY_HALL_OF_FAME', [(7, 11)], map_id(LOBBY), 0, [keeper], sc,
             MAPSEC, GROUP, music='MUS_HALL_OF_FAME')


def build(ctx):
    victory_road(ctx)
    outside(ctx)
    lobby(ctx)
    rooms(ctx)
