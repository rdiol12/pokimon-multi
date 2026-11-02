"""The S.S. Meridian between Lilycove (Hoenn) and Port Solace (Solmara).

A sailor in Lilycove's harbor takes champions aboard. The ship's lounge is
one shared map: everyone sailing either way is in it at the same time, so
it's a natural place to meet, chat and trade before landing. The steward
lets you off at either port.
"""
from sol import *

LOUNGE = 'SolmaraShip_Lounge'
ARRIVE_LOUNGE = (2, 11)
ARRIVE_LILYCOVE = (13, 12)


def build(ctx):
    S = f'{LOUNGE}_EventScript'
    steward = npc('OBJ_EVENT_GFX_SAILOR', 16, 11, f'{S}_Steward', 'MOVEMENT_TYPE_FACE_LEFT')
    steward['movement_range_x'] = steward['movement_range_y'] = 0
    objs = [steward]
    people = [
        ('OBJ_EVENT_GFX_EXPERT_M', 6, 2, 'MOVEMENT_TYPE_FACE_DOWN', ['Half the trainers on this ship', 'are heading to SOLMARA for the', 'first time. Exciting, isn\'t it?']),
        ('OBJ_EVENT_GFX_LASS', 12, 3, 'MOVEMENT_TYPE_WANDER_AROUND', ['I always trade with someone on', 'the crossing. You meet the', 'nicest people at sea!']),
        ('OBJ_EVENT_GFX_FAT_MAN', 9, 11, 'MOVEMENT_TYPE_FACE_UP', ['The crossing is quick, but the', 'buffet is quicker. Gone already!']),
        ('OBJ_EVENT_GFX_SPECIES(WINGULL)', 3, 3, 'MOVEMENT_TYPE_WANDER_AROUND', ['WINGULL: Pihyuu! It stowed away!']),
    ]
    sc = f"""{LOUNGE}_MapScripts::
\t.byte 0

{S}_Steward::
\tlock
\tfaceplayer
\tmessage {S}_Text_Where
\twaitmessage
\tdynmultipush {S}_Text_Solace, 0
\tdynmultipush {S}_Text_Lilycove, 1
\tdynmultipush {S}_Text_Stay, 2
\tdynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
\tgoto_if_eq VAR_RESULT, 0, {S}_ToSolace
\tgoto_if_eq VAR_RESULT, 1, {S}_ToLilycove
\tmsgbox {S}_Text_StayOn, MSGBOX_DEFAULT
\trelease
\tend

{S}_ToSolace::
\tmsgbox {S}_Text_Land, MSGBOX_DEFAULT
\tclosemessage
\tplayse SE_SHIP
\twarp {map_id('PortSolace')}, 21, 31
\twaitstate
\trelease
\tend

{S}_ToLilycove::
\tmsgbox {S}_Text_Land, MSGBOX_DEFAULT
\tclosemessage
\tplayse SE_SHIP
\twarp MAP_LILYCOVE_CITY_HARBOR, {ARRIVE_LILYCOVE[0]}, {ARRIVE_LILYCOVE[1]}
\twaitstate
\trelease
\tend

{S}_Text_Where:
{text('Welcome aboard the S.S. MERIDIAN!', 'Where shall I let you off?')}
{S}_Text_Solace:
\t.string "PORT SOLACE$"
{S}_Text_Lilycove:
\t.string "LILYCOVE$"
{S}_Text_Stay:
\t.string "STAY ABOARD$"
{S}_Text_StayOn:
{text('Enjoy the lounge! Talk to me', 'whenever you\'re ready to land.')}
{S}_Text_Land:
{text('Land ho! Mind your step.')}
"""
    for i, (gfx, x, y, move, lines) in enumerate(people):
        o = npc(gfx, x, y, f'{S}_Npc{i}', move)
        if move != 'MOVEMENT_TYPE_WANDER_AROUND':
            o['movement_range_x'] = o['movement_range_y'] = 0
        objs.append(o)
        sc += talker(f'{S}_Npc{i}', lines)
    interior(LOUNGE, map_id(LOUNGE), 'LAYOUT_SS_TIDAL_CORRIDOR', [], 'MAP_LILYCOVE_CITY_HARBOR', 0, objs, sc,
             'MAPSEC_PORT_SOLACE', GROUP, music='MUS_SAILING')

    # The sailor in Lilycove's harbor.
    add_object('LilycoveCity_Harbor', {'graphics_id': 'OBJ_EVENT_GFX_SAILOR', 'x': 14, 'y': 11, 'elevation': 3,
                                       'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0,
                                       'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                                       'trainer_sight_or_berry_tree_id': '0',
                                       'script': 'LilycoveCity_Harbor_EventScript_SolmaraSailor', 'flag': '0'})
    register_script_file('solmara_ship', f"""
@ The S.S. Meridian's sailor in Lilycove's harbor (tools/solmara/ship.py).
LilycoveCity_Harbor_EventScript_SolmaraSailor::
\tlock
\tfaceplayer
\tgoto_if_unset FLAG_SYS_GAME_CLEAR, LilycoveCity_Harbor_EventScript_SolmaraNotYet
\tmsgbox LilycoveCity_Harbor_Text_SolmaraAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, LilycoveCity_Harbor_EventScript_SolmaraNo
\tmsgbox LilycoveCity_Harbor_Text_SolmaraGo, MSGBOX_DEFAULT
\tclosemessage
\twarp {map_id(LOUNGE)}, {ARRIVE_LOUNGE[0]}, {ARRIVE_LOUNGE[1]}
\twaitstate
\trelease
\tend

LilycoveCity_Harbor_EventScript_SolmaraNo::
\tmsgbox LilycoveCity_Harbor_Text_SolmaraNo, MSGBOX_DEFAULT
\trelease
\tend

LilycoveCity_Harbor_EventScript_SolmaraNotYet::
\tmsgbox LilycoveCity_Harbor_Text_SolmaraNotYet, MSGBOX_DEFAULT
\trelease
\tend

LilycoveCity_Harbor_Text_SolmaraAsk:
{text('The S.S. MERIDIAN sails to the', 'SOLMARA region, far to the east.', '', 'Eight new gyms and a LEAGUE of', 'its own! All aboard?')}
LilycoveCity_Harbor_Text_SolmaraGo:
{text('Welcome aboard, Champion!')}
LilycoveCity_Harbor_Text_SolmaraNo:
{text('We sail all day. Come back any', 'time!')}
LilycoveCity_Harbor_Text_SolmaraNotYet:
{text('The S.S. MERIDIAN only takes', 'trainers who have beaten the', 'HOENN POKéMON LEAGUE.', '', 'Become Champion, then come back!')}
""")
