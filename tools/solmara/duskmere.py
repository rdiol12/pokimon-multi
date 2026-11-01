"""Duskmere: a misty harbour town on the dark side of the bay, the Ghost
gym. Route 205 east to Rosemead, Route 206 south to Frostholm."""
from town import *

NAME = 'Duskmere'
EXITS = {'right': 15, 'down': 18}


def style():
    return Style('gTileset_General', 'gTileset_Lilycove', ['LAYOUT_LILYCOVE_CITY', 'LAYOUT_ROUTE121', 'LAYOUT_ROUTE123'],
                 wall_classes={'T': TREES})


def sketch(a):
    a.rect(0, 0, 40, 3, '~')
    a.rect(0, 0, 3, 34, '~')
    a.rect(3, 3, 2, 31, 'T')
    a.rect(30, 26, 7, 5, 'T')


def build(ctx):
    T = Town(name=NAME, title='DUSKMERE', w=40, h=34, style=style(), exits=EXITS, sketch=sketch,
             mapsec='MAPSEC_DUSKMERE', music='MUS_SOOTOPOLIS', weather='WEATHER_FOG_HORIZONTAL',
             gym=(9, 5), pc=(26, 5), houses=[(7, 22), (26, 21)], plaza=(12, 12, 16, 8),
             leader='MORWYN', motto='The mist sees all who enter.',
             town_sign=(18, 10, ['DUSKMERE', 'The town in the mist.']),
             people=[
                 ('OBJ_EVENT_GFX_HEX_MANIAC', 30, 14, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'In the fog you can hardly tell', 'the other trainers from the', 'ghosts. Say hello anyway!']),
                 ('OBJ_EVENT_GFX_OLD_MAN', 13, 22, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'MORWYN\'s GHOST types vanish', 'when you swing at them. DARK', 'and GHOST moves hit them hard.']),
                 ('OBJ_EVENT_GFX_FISHERMAN', 6, 14, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'Strange shapes swim under the', 'fog at night. Try fishing!']),
                 ('OBJ_EVENT_GFX_GIRL_3', 22, 27, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Route 206 to the south climbs', 'into the snow. FROSTHOLM is cold!']),
             ],
             hidden=[(35, 9, 'ITEM_SPELL_TAG'), (8, 30, 'ITEM_DUSK_STONE')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_DUSK_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_POTION',
                   'ITEM_FULL_HEAL', 'ITEM_AWAKENING', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_HEX_MANIAC', 10, 5, ['Ghosts love a crowd. The more', 'players in town, the more they', 'come out to play...'])],
             house_people=[
                 [('OBJ_EVENT_GFX_OLD_WOMAN', 6, 4, 'Granny', ['My late husband\'s DUSKULL still', 'brings me tea every morning.'], 'MOVEMENT_TYPE_FACE_DOWN'),
                  ('OBJ_EVENT_GFX_SPECIES(DUSKULL)', 4, 4, 'Duskull', ['DUSKULL: ...'], 'MOVEMENT_TYPE_WANDER_AROUND')],
                 [('OBJ_EVENT_GFX_MAN_2', 4, 4, 'Man', ['The lighthouse keeper says the', 'fog came the day the town', 'was founded and never left.'], 'MOVEMENT_TYPE_FACE_DOWN')],
             ])
    return build_town(ctx, T)
