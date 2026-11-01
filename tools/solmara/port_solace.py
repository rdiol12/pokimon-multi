"""Port Solace: where the S.S. Meridian docks, the Water gym, and Harbor
Square, a wide open plaza where players arriving from Hoenn meet up. The
cave in the north cliff is Victory Road, up to the Solmara League (all 8
Solmara badges). Routes 201 (east) and 208 (west).
"""
from sol import *
from styles import TREES, PLAZA

NAME = 'PortSolace'
MAPSEC = 'MAPSEC_PORT_SOLACE'
W, H = 44, 38
EXITS = {'left': 15, 'right': 15}   # first row of each 4-wide exit
EXIT_ROWS = (15, 19)
ARRIVE = (21, 31)             # off the gangway
ITEMS = ['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_DIVE_BALL', 'ITEM_NET_BALL', 'ITEM_HYPER_POTION',
         'ITEM_MAX_POTION', 'ITEM_FULL_HEAL', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE']


def build(ctx):
    st = Style('gTileset_General', 'gTileset_Dewford', ['LAYOUT_DEWFORD_TOWN', 'LAYOUT_ROUTE105', 'LAYOUT_ROUTE106',
                                                        'LAYOUT_ROUTE107'], wall_classes={'T': TREES})
    a = Area(NAME, W, H, st, fill='.')
    # Trees round the edge, gaps for the exits.
    a.rect(0, 0, W, 2, 'T')
    a.rect(0, 0, 4, 30, 'T')
    a.rect(W - 4, 0, 4, 30, 'T')
    a.rect(0, EXIT_ROWS[0], 4, EXIT_ROWS[1] - EXIT_ROWS[0], '.')
    a.rect(W - 4, EXIT_ROWS[0], 4, EXIT_ROWS[1] - EXIT_ROWS[0], '.')
    # The sea, and the pier the ship ties up at.
    a.rect(0, 30, W, H - 30, '~')
    a.rect(0, 28, 6, 2, '~')
    a.rect(W - 6, 28, 6, 2, '~')
    a.rect(18, 30, 8, 3, '.')                       # the pier
    a.snap_trees()
    # Victory Road: the north cliff with its cave.
    a.stamp('LAYOUT_ROUTE106', 40, 12, 11, 6, 16, 0)
    cave = (24, 4)
    a.rect(14, 0, 2, 6, 'T')
    a.rect(27, 0, 3, 6, 'T')
    a.snap_trees()
    # Buildings: the gym at the top of the square, PC west, Mart east.
    gym = put_general(a, 'gym', 19, 7)
    pc_door = put_general(a, 'pc', 8, 17)
    homes = HOUSES['gTileset_Dewford']
    h1 = put_building(a, homes[0], 5, 6)
    h2 = put_building(a, homes[1], 34, 6)
    # Harbor Square: paved, wide open, the place to meet.
    a.rect(14, 14, 16, 10, 'P')
    a.autotile3('P', PLAZA)
    # A few palms and rocks along the beach, kept off the walkways.
    for x in (7, 13, 30, 37):
        a.rect(x, 25, 2, 2, 'T')
    a.snap_trees()

    import flight
    flight.LANDING[NAME] = (pc_door[0], pc_door[1] + 1)
    a.warp(*gym, map_id(f'{NAME}_Gym'), 0)
    a.warp(*pc_door, map_id(f'{NAME}_PokemonCenter_1F'), 0)
    a.warp(*h1, map_id(f'{NAME}_House1'), 0)
    a.warp(*h2, map_id(f'{NAME}_House2'), 0)
    a.warp(*cave, map_id('SolmaraVictoryRoad'), 0)
    # Not enough badges: the ranger sends you back (a trigger, not a body in
    # the doorway, so nobody else is ever blocked).
    for x in (23, 24, 25):
        a.coord(x, 5, f'{NAME}_EventScript_VictoryCheck')

    C = f'{NAME}_EventScript'
    a.obj('OBJ_EVENT_GFX_SS_TIDAL', 21, 33, '0x0', 'MOVEMENT_TYPE_FACE_DOWN')
    a.obj('OBJ_EVENT_GFX_SAILOR', 19, 31, f'{C}_Ship', 'MOVEMENT_TYPE_FACE_RIGHT')
    a.obj('OBJ_EVENT_GFX_POKEMON_RANGER_M' if False else 'OBJ_EVENT_GFX_CAMPER', 27, 5, f'{C}_Ranger', 'MOVEMENT_TYPE_FACE_LEFT')
    people = [
        ('OBJ_EVENT_GFX_FISHERMAN', 6, 28, 'MOVEMENT_TYPE_FACE_DOWN', 'Fisher', [
            'Everyone who sails in from HOENN', 'walks through HARBOR SQUARE.', '',
            'Wait here and you\'ll meet a', 'traveling partner in no time!']),
        ('OBJ_EVENT_GFX_SWIMMER_F', 35, 26, 'MOVEMENT_TYPE_WANDER_AROUND', 'Swimmer', [
            'SOLMARA has eight gyms, and you', 'can take them in any order!', '',
            'The leaders size up your badges', 'and raise their game to match.']),
        ('OBJ_EVENT_GFX_OLD_MAN', 15, 13, 'MOVEMENT_TYPE_FACE_DOWN', 'OldMan', [
            'The towns form a great ring. Head', 'east or west, you\'ll come home', 'to PORT SOLACE either way.']),
        ('OBJ_EVENT_GFX_TWIN', 29, 13, 'MOVEMENT_TYPE_WANDER_AROUND', 'Twin', [
            'Every POKéMON CENTER here has a', 'flight desk. Fly to any town you', 'have been to before!']),
        ('OBJ_EVENT_GFX_SAILOR', 24, 29, 'MOVEMENT_TYPE_FACE_LEFT', 'Deckhand', [
            'The S.S. MERIDIAN sails to', 'LILYCOVE and back all day long.']),
    ]
    sc = f"""{NAME}_MapScripts::
\tmap_script MAP_SCRIPT_ON_TRANSITION, {NAME}_OnTransition
\t.byte 0

{NAME}_OnTransition:
\tsetflag {ctx.visited(NAME)}
\tend

{C}_Ship::
\tlock
\tfaceplayer
\tmsgbox {C}_Text_ShipAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, {C}_ShipNo
\tmsgbox {C}_Text_ShipGo, MSGBOX_DEFAULT
\tclosemessage
\twarp MAP_SOLMARA_SHIP_LOUNGE, 2, 11
\twaitstate
\trelease
\tend

{C}_ShipNo::
\tmsgbox {C}_Text_ShipNo, MSGBOX_DEFAULT
\trelease
\tend

{C}_Ranger::
\tlock
\tfaceplayer
\tcallnative Solmara_GetBadgeCount
\tgoto_if_eq VAR_RESULT, 8, {C}_RangerOpen
\tbuffernumberstring STR_VAR_1, VAR_RESULT
\tmsgbox {C}_Text_Ranger, MSGBOX_DEFAULT
\trelease
\tend

{C}_RangerOpen::
\tmsgbox {C}_Text_RangerOpen, MSGBOX_DEFAULT
\trelease
\tend

{C}_VictoryCheck::
\tlockall
\tcallnative Solmara_GetBadgeCount
\tgoto_if_eq VAR_RESULT, 8, {C}_VictoryOk
\tbuffernumberstring STR_VAR_1, VAR_RESULT
\tmsgbox {C}_Text_VictoryNo, MSGBOX_DEFAULT
\tclosemessage
\tapplymovement OBJ_EVENT_ID_PLAYER, {C}_StepBack
\twaitmovement 0
\treleaseall
\tend

{C}_VictoryOk::
\treleaseall
\tend

{C}_StepBack:
\twalk_down
\tstep_end

{C}_Text_ShipAsk:
{text('The S.S. MERIDIAN sails for', 'LILYCOVE CITY in HOENN. All aboard?')}
{C}_Text_ShipGo:
{text('Welcome aboard! Make yourself at', 'home in the lounge.')}
{C}_Text_ShipNo:
{text('We sail all day. Come back any', 'time!')}
{C}_Text_Ranger:
{text('VICTORY ROAD climbs to the', 'SOLMARA LEAGUE.', '',
      'Only trainers with all eight', 'SOLMARA badges may enter.', '',
      'You have {STR_VAR_1}. Come back with', 'all eight!').replace('{STR_VAR_1}', '{{STR_VAR_1}}').replace('{{', '{').replace('}}', '}')}
{C}_Text_RangerOpen:
{text('Eight SOLMARA badges! VICTORY ROAD', 'is open to you. Good luck up there!')}
{C}_Text_VictoryNo:
{text('VICTORY ROAD is for trainers with', 'all eight SOLMARA badges.', '',
      'You have {STR_VAR_1}.').replace('{{', '{').replace('}}', '}')}
"""
    for gfx, x, y, move, key, lines in people:
        a.obj(gfx, x, y, f'{C}_{key}', move, rx=1, ry=1)
        sc += talker(f'{C}_{key}', lines)
    a.sign(17, 21, f'{C}_SquareSign')
    sc += signpost(f'{C}_SquareSign', ['HARBOR SQUARE', 'Meet, trade and battle with', 'trainers from near and far!'])
    a.sign(20, 12, f'{C}_GymSign')
    sc += signpost(f'{C}_GymSign', ['PORT SOLACE POKéMON GYM', 'LEADER: MARINA', 'The tide that never turns!'])
    a.sign(22, 6, f'{C}_CaveSign')
    sc += signpost(f'{C}_CaveSign', ['VICTORY ROAD', 'To the SOLMARA LEAGUE.', 'Eight SOLMARA badges needed.'])
    a.connections = connections(NAME)
    a.write(map_id=map_id(NAME), layout_id='LAYOUT_PORT_SOLACE', music='MUS_DEWFORD', mapsec=MAPSEC,
            mapsec_name='PORT SOLACE', map_type='MAP_TYPE_TOWN', group=GROUP, scripts=sc)

    # Interiors.
    pokecenter(NAME, map_id(NAME), 1, MAPSEC, ctx.town_index(NAME), ITEMS, npcs=[
        ('OBJ_EVENT_GFX_GIRL_3', 3, 5, ['I came over on the ship with a', 'friend. We split up to cover', 'more ground!']),
        ('OBJ_EVENT_GFX_MAN_4', 10, 5, ['The flight desk only goes to', 'towns you have visited. Walk', 'there once first!'])])
    house(NAME, 1, map_id(NAME), 2, MAPSEC, [
        ('OBJ_EVENT_GFX_WOMAN_2', 3, 4, 'Woman', ['My husband built the pier. He', 'made it wide so a whole crowd', 'can stand on it at once!'], 'MOVEMENT_TYPE_FACE_DOWN'),
        ('OBJ_EVENT_GFX_SPECIES(WINGULL)', 6, 4, 'Wingull', ['WINGULL: Pihyuu!'], 'MOVEMENT_TYPE_WANDER_AROUND')])
    house(NAME, 2, map_id(NAME), 3, MAPSEC, [
        ('OBJ_EVENT_GFX_EXPERT_M', 5, 3, 'Captain', ['I captained the first ship from', 'HOENN. Back then you sailed alone.', '',
                                                     'Now the ship is full of trainers', 'chatting and trading. Much', 'better company!'], 'MOVEMENT_TYPE_FACE_DOWN')],
          layout='LAYOUT_HOUSE2')
    return a
