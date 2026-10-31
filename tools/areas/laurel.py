"""Laurel City: the Rover League's city, an island reached by the league
ferry from Slateport. The Rover League Stadium stands at the top of the
avenue; the week's league rovers travel here to compete (server/rovers.py
sends them), and players enter, play and watch at the stadium's front desk.
Also: the Hall of Champions (past champions, from the league's news), the
Rovers' Lodge, the League Shop, a nurse in the stadium, and fans.

The town is stamped from the Battle Frontier's west side (its tileset) and
the stadium's lobby is the Battle Dome's.

    python3 tools/areas/laurel.py
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from build import *
from styles import TREES

CITY, STADIUM = 'LaurelCity', 'LaurelCity_Stadium'
HALL, LODGE, SHOP = 'LaurelCity_HallOfChampions', 'LaurelCity_RoversLodge', 'LaurelCity_LeagueShop'
MAPSEC = 'MAPSEC_LAUREL_CITY'
SRC = 'LAYOUT_BATTLE_FRONTIER_OUTSIDE_WEST'
W, H = 44, 52

style = Style('gTileset_General', 'gTileset_BattleFrontierOutsideWest', [SRC], wall_classes={'T': TREES})
a = Area(CITY, W, H, style, fill='~')

# Land: the island, with trees in the corners.
a.rect(2, 1, W - 4, 44, '.')
a.rect(2, 1, 6, 24, 'T')
a.rect(W - 8, 1, 6, 24, 'T')
a.rect(2, 41, 5, 4, 'T')
a.rect(W - 7, 41, 5, 4, 'T')
a.snap_trees()

# The stadium (the Battle Dome and its plaza) up top, the avenue of arches,
# flower beds, and the ferry dock at the bottom.
a.stamp(SRC, 5, 1, 26, 24, 9, 1)
a.stamp(SRC, 24, 46, 5, 15, 23, 25)
a.stamp(SRC, 17, 45, 5, 3, 14, 30)
a.stamp(SRC, 31, 45, 10, 3, 30, 30)
a.stamp(SRC, 10, 65, 21, 5, 15, 45)
# The Hall of Champions and the Rovers' Lodge (the west lounges), the
# League Shop (the mart).
a.stamp(SRC, 43, 40, 13, 6, 2, 34)
a.stamp(SRC, 49, 47, 7, 6, 35, 34)

# The dock's top: fence where the Frontier's gate building stood, and a gap
# onto the dock.
a.stamp(SRC, 12, 65, 8, 1, 28, 45)
for x in (24, 25, 26):
    a.fixed[(x, 45)] = a.fixed[(x, 47)]
# Half a lounge came in with the plaza's west edge: trees instead.
for y in range(16, 22):
    for x in (9, 10):
        a.fixed.pop((x, y), None)
        a.put(x, y, 'T')

STADIUM_DOOR = (23, 17)
HALL_DOOR, LODGE_DOOR, SHOP_DOOR = (4, 38), (12, 38), (37, 38)
ARRIVE = (25, 47)

a.warp(*STADIUM_DOOR, 'MAP_LAUREL_CITY_STADIUM', 0)
a.warp(*HALL_DOOR, 'MAP_LAUREL_CITY_HALL_OF_CHAMPIONS', 0)
a.warp(*LODGE_DOOR, 'MAP_LAUREL_CITY_ROVERS_LODGE', 0)
a.warp(*SHOP_DOOR, 'MAP_LAUREL_CITY_LEAGUE_SHOP', 0)
a.sign(STADIUM_DOOR[0] - 4, STADIUM_DOOR[1] + 1, f'{CITY}_EventScript_StadiumSign')


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


def talker(label, lines):
    return f"""{label}::
\tmsgbox {label}_Text, MSGBOX_NPC
\tend

{label}_Text:
{text(*lines)}
"""


C = f'{CITY}_EventScript'
a.obj('OBJ_EVENT_GFX_SAILOR', 23, 47, f'{C}_Ferry', 'MOVEMENT_TYPE_FACE_RIGHT')
NPCS = [
    ('OBJ_EVENT_GFX_YOUNGSTER', 20, 27, 'MOVEMENT_TYPE_WANDER_AROUND', 'Fan', [
        'The ROVER LEAGUE is the best show', 'in HOENN! Eight rovers, one', 'champion, every single week.']),
    ('OBJ_EVENT_GFX_LASS', 31, 28, 'MOVEMENT_TYPE_FACE_DOWN', 'Lass', [
        'The league rovers come here from', 'all over HOENN at the start of', 'the week.', '',
        'Talk to them! They love to chat', 'before a match.']),
    ('OBJ_EVENT_GFX_POKEFAN_M', 14, 36, 'MOVEMENT_TYPE_WANDER_AROUND', 'Pokefan', [
        'A rover that gets knocked out', 'packs up and heads back out on', 'the road. The champion stays to', 'celebrate until Sunday!']),
    ('OBJ_EVENT_GFX_OLD_MAN', 33, 41, 'MOVEMENT_TYPE_FACE_LEFT', 'OldMan', [
        'They call it LAUREL CITY after the', 'laurel wreath champions wear.', '',
        'The HALL OF CHAMPIONS keeps every', 'winner\'s name.']),
    ('OBJ_EVENT_GFX_CAMERAMAN', 28, 22, 'MOVEMENT_TYPE_FACE_UP', 'Cameraman', [
        'I film every final for the', 'evening news. Lights, camera,', 'battle!']),
    ('OBJ_EVENT_GFX_BLACK_BELT', 17, 24, 'MOVEMENT_TYPE_FACE_DOWN', 'BlackBelt', [
        'Players can take a low seed\'s', 'place until the quarterfinals', 'end on Wednesday.', '',
        'Bring your strongest team!']),
]
scripts = f"""{CITY}_MapScripts::
\t.byte 0

{C}_Ferry::
\tlock
\tfaceplayer
\tmsgbox {C}_Text_FerryAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, {C}_FerryNo
\tmsgbox {C}_Text_FerryGo, MSGBOX_DEFAULT
\tcloseMessage
\twarp MAP_SLATEPORT_CITY, 33, 14
\twaitstate
\trelease
\tend

{C}_FerryNo::
\tmsgbox {C}_Text_FerryNo, MSGBOX_DEFAULT
\trelease
\tend

{C}_StadiumSign::
\tmsgbox {C}_Text_StadiumSign, MSGBOX_SIGN
\tend

{C}_Text_FerryAsk:
{text('The league ferry goes back to', 'SLATEPORT. All aboard?')}
{C}_Text_FerryGo:
{text('Anchors aweigh!')}
{C}_Text_FerryNo:
{text('I\'ll be here when the match is', 'over.')}
{C}_Text_StadiumSign:
{text('ROVER LEAGUE STADIUM', 'Every week, eight rovers. One', 'champion.')}
""".replace('closeMessage', 'closemessage')
for gfx, x, y, move, name, lines in NPCS:
    a.obj(gfx, x, y, f'{C}_{name}', move, rx=1, ry=1)
    scripts += talker(f'{C}_{name}', lines)

a.write(map_id='MAP_LAUREL_CITY', layout_id='LAYOUT_LAUREL_CITY', music='MUS_B_FRONTIER', mapsec=MAPSEC,
        mapsec_name='LAUREL CITY', map_type='MAP_TYPE_CITY', group='gMapGroup_TownsAndRoutes', scripts=scripts)
print('ok city')

# ---------------------------------------------------------------- the stadium
G = 'gMapGroup_TownsAndRoutes'
S = f'{STADIUM}_EventScript'
nurse = npc('OBJ_EVENT_GFX_NURSE', 17, 10, f'{S}_Nurse', 'MOVEMENT_TYPE_FACE_DOWN', local_id='LOCALID_LAUREL_NURSE')
desk = npc('OBJ_EVENT_GFX_TEALA', 5, 10, 'Rovers_EventScript_LeagueDesk', 'MOVEMENT_TYPE_FACE_DOWN')
for o in (nurse, desk):
    o['movement_range_x'] = o['movement_range_y'] = 0
fans = [
    ('OBJ_EVENT_GFX_MAN_3', 1, 12, 'Announcer', [
        'And here they come, the best', 'rovers in HOENN!', '',
        'Ahem. Sorry. I practice my', 'announcing on visitors.']),
    ('OBJ_EVENT_GFX_LASS', 14, 14, 'Lass', [
        'I watched a rover match at the', 'desk. You see it happen, and', 'that\'s how it went for real!']),
    ('OBJ_EVENT_GFX_FAT_MAN', 18, 14, 'FatMan', [
        'The nurse here heals your team', 'before your match. Don\'t go in', 'tired!']),
    ('OBJ_EVENT_GFX_MANIAC', 8, 14, 'Maniac', [
        'Prize money goes up every round.', 'Win the final and there\'s a big', 'bonus on top!']),
]
objs = [desk, nurse]
st = f"""{STADIUM}_MapScripts::
\t.byte 0

{S}_Nurse::
\tsetvar VAR_0x800B, LOCALID_LAUREL_NURSE
\tcall Common_EventScript_PkmnCenterNurse
\twaitmessage
\twaitbuttonpress
\trelease
\tend

{S}_DeskSign::
\tmsgbox {S}_Text_DeskSign, MSGBOX_SIGN
\tend

{S}_NurseSign::
\tmsgbox {S}_Text_NurseSign, MSGBOX_SIGN
\tend

{S}_Text_DeskSign:
{text('ROVER LEAGUE: brackets, entry,', 'matches and watching.')}
{S}_Text_NurseSign:
{text('Rest your POKéMON before a match.')}
"""
for gfx, x, y, name, lines in fans:
    objs.append(npc(gfx, x, y, f'{S}_{name}', 'MOVEMENT_TYPE_FACE_DOWN' if name != 'Announcer' else 'MOVEMENT_TYPE_FACE_RIGHT'))
    objs[-1]['movement_range_x'] = objs[-1]['movement_range_y'] = 0
    st += talker(f'{S}_{name}', lines)
bgs = [{'type': 'sign', 'x': x, 'y': 10, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
        'script': f'{S}_{kind}Sign'} for x, kind in ((4, 'Desk'), (7, 'Desk'), (15, 'Nurse'), (18, 'Nurse'))]
interior(STADIUM, 'MAP_LAUREL_CITY_STADIUM', 'LAYOUT_BATTLE_FRONTIER_BATTLE_DOME_LOBBY', [(11, 16), (12, 16)],
         'MAP_LAUREL_CITY', 0, objs, st, MAPSEC, G, music='MUS_B_DOME_LOBBY', bgs=bgs)

# ---------------------------------------------------------------- the Hall of Champions
HS = f'{HALL}_EventScript'
curator = npc('OBJ_EVENT_GFX_GENTLEMAN', 8, 4, 'Rovers_EventScript_LeagueHall', 'MOVEMENT_TYPE_FACE_DOWN')
curator['movement_range_x'] = curator['movement_range_y'] = 0
hall_objs = [curator, npc('OBJ_EVENT_GFX_SCHOOL_KID_M', 4, 6, f'{HS}_Kid', 'MOVEMENT_TYPE_WANDER_AROUND'),
             npc('OBJ_EVENT_GFX_EXPERT_F', 10, 4, f'{HS}_Expert', 'MOVEMENT_TYPE_FACE_LEFT')]
hs = f"{HALL}_MapScripts::\n\t.byte 0\n\n" + talker(f'{HS}_Kid', [
    'When I grow up, my name\'s going', 'up in this hall!']) + talker(f'{HS}_Expert', [
    'A champion\'s name stays here', 'forever. Even a player\'s, if', 'they win the final!'])
interior(HALL, 'MAP_LAUREL_CITY_HALL_OF_CHAMPIONS', 'LAYOUT_BATTLE_FRONTIER_LOUNGE1', [(1, 7), (2, 7)],
         'MAP_LAUREL_CITY', 1, hall_objs, hs, MAPSEC, G, music='MUS_B_FRONTIER')

# ---------------------------------------------------------------- the Rovers' Lodge
LS = f'{LODGE}_EventScript'
lodge_objs = [npc('OBJ_EVENT_GFX_HIKER', 2, 5, f'{LS}_Hiker', 'MOVEMENT_TYPE_FACE_RIGHT'),
              npc('OBJ_EVENT_GFX_COOK', 6, 6, f'{LS}_Cook', 'MOVEMENT_TYPE_WANDER_AROUND'),
              npc('OBJ_EVENT_GFX_PICNICKER', 6, 4, f'{LS}_Picnicker', 'MOVEMENT_TYPE_FACE_DOWN')]
ls = f"{LODGE}_MapScripts::\n\t.byte 0\n\n" + talker(f'{LS}_Hiker', [
    'This is where the league rovers', 'sleep the night before a match.', '',
    'Some snore louder than a', 'WHISMUR!']) + talker(f'{LS}_Cook', [
    'I cook for the league rovers.', 'A good meal before a battle', 'makes all the difference.']) + talker(f'{LS}_Picnicker', [
    'I was a rover once. I made the', 'semifinals! Then my LOTAD', 'fell asleep mid-battle...'])
interior(LODGE, 'MAP_LAUREL_CITY_ROVERS_LODGE', 'LAYOUT_BATTLE_FRONTIER_LOUNGE2', [(4, 9)],
         'MAP_LAUREL_CITY', 2, lodge_objs, ls, MAPSEC, G, music='MUS_B_FRONTIER')

# ---------------------------------------------------------------- the League Shop
SS = f'{SHOP}_EventScript'
clerk = npc('OBJ_EVENT_GFX_MART_EMPLOYEE', 1, 3, f'{SS}_Clerk', 'MOVEMENT_TYPE_FACE_RIGHT')
clerk['movement_range_x'] = clerk['movement_range_y'] = 0
shop_objs = [clerk, npc('OBJ_EVENT_GFX_BOY_2', 8, 4, f'{SS}_Boy', 'MOVEMENT_TYPE_WANDER_AROUND')]
ss = f"""{SHOP}_MapScripts::
\t.byte 0

{SS}_Clerk::
\tlock
\tfaceplayer
\tmessage gText_HowMayIServeYou
\twaitmessage
\tpokemart {SS}_Items
\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
\trelease
\tend

\t.align 2
{SS}_Items:
\t.2byte ITEM_ULTRA_BALL
\t.2byte ITEM_HYPER_POTION
\t.2byte ITEM_MAX_POTION
\t.2byte ITEM_FULL_RESTORE
\t.2byte ITEM_REVIVE
\t.2byte ITEM_FULL_HEAL
\t.2byte ITEM_X_ATTACK
\t.2byte ITEM_X_DEFENSE
\t.2byte ITEM_X_SPEED
\t.2byte ITEM_X_SP_ATK
\t.2byte ITEM_X_SP_DEF
\t.2byte ITEM_GUARD_SPEC
\tpokemartlistend

""" + talker(f'{SS}_Boy', ['X items in a league match? Smart!', 'Every little bit helps.'])
interior(SHOP, 'MAP_LAUREL_CITY_LEAGUE_SHOP', 'LAYOUT_MART', [(3, 7), (4, 7)],
         'MAP_LAUREL_CITY', 3, shop_objs, ss, MAPSEC, G, music='MUS_POKE_MART')
print('ok interiors')

# ---------------------------------------------------------------- the league ferry, from Slateport
add_object('SlateportCity', {'graphics_id': 'OBJ_EVENT_GFX_SAILOR', 'x': 34, 'y': 14, 'elevation': 3,
                             'movement_type': 'MOVEMENT_TYPE_FACE_LEFT', 'movement_range_x': 0, 'movement_range_y': 0,
                             'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                             'script': 'SlateportCity_EventScript_LeagueFerry', 'flag': '0'})
register_script_file('laurel', f"""
@ The league ferry in Slateport City (tools/areas/laurel.py).
SlateportCity_EventScript_LeagueFerry::
\tlock
\tfaceplayer
\tmsgbox SlateportCity_Text_LeagueFerryAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, SlateportCity_EventScript_LeagueFerryNo
\tmsgbox SlateportCity_Text_LeagueFerryGo, MSGBOX_DEFAULT
\tclosemessage
\twarp MAP_LAUREL_CITY, {ARRIVE[0]}, {ARRIVE[1]}
\twaitstate
\trelease
\tend

SlateportCity_EventScript_LeagueFerryNo::
\tmsgbox SlateportCity_Text_LeagueFerryNo, MSGBOX_DEFAULT
\trelease
\tend

SlateportCity_Text_LeagueFerryAsk:
{text('This is the league ferry to LAUREL', 'CITY, home of the ROVER LEAGUE!', '', 'Want to go watch, or compete?')}
SlateportCity_Text_LeagueFerryGo:
{text('Off we go! Next stop, LAUREL CITY!')}
SlateportCity_Text_LeagueFerryNo:
{text('The ferry runs every day. Come', 'back any time!')}
""")
print('ok ferry')
