"""Rosemead: a garden town of flower beds and hedges, the Fairy gym.
Route 204 east to Silkwood, Route 205 west to Duskmere."""
from town import *
from styles import town

NAME = 'Rosemead'
EXITS = {'right': 15, 'left': 15}


def sketch(a):
    a.ellipse(33, 27, 3, 2, '~')
    a.ellipse(7, 28, 3, 1.6, '"')


def extra(a, ctx):
    a.scatter(0x004, 30, 3, 3, 37, 31, seed=5)
    return ''


def build(ctx):
    T = Town(name=NAME, ground='g', title='ROSEMEAD', w=40, h=34, style=town(), exits=EXITS, sketch=sketch, extra=extra,
             mapsec='MAPSEC_ROSEMEAD', music='MUS_PETALBURG',
             gym=(17, 4), pc=(7, 21), houses=[(6, 6), (28, 6), (26, 22)], plaza=(12, 12, 16, 8),
             leader='BELLE', motto='Grace that hits hard!',
             town_sign=(12, 10, ['ROSEMEAD', 'The garden of SOLMARA.']),
             people=[
                 ('OBJ_EVENT_GFX_WOMAN_1', 14, 22, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Every flower bed in town was', 'planted by a different trainer', 'passing through!']),
                 ('OBJ_EVENT_GFX_GENTLEMAN', 30, 16, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'BELLE\'s FAIRY types laugh at', 'DRAGON moves. STEEL and POISON', 'are what they fear.']),
                 ('OBJ_EVENT_GFX_LITTLE_GIRL', 22, 23, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'West is Route 205 through the', 'canyon. DUSKMERE is foggy and', 'a bit spooky!']),
                 ('OBJ_EVENT_GFX_POKEFAN_F', 9, 15, 'MOVEMENT_TYPE_FACE_RIGHT', [
                     'Players picnic in the square on', 'sunny days. Bring a friend!']),
             ],
             hidden=[(35, 9, 'ITEM_PIXIE_PLATE'), (4, 23, 'ITEM_MOON_STONE')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_LOVE_BALL', 'ITEM_FRIEND_BALL', 'ITEM_HYPER_POTION',
                   'ITEM_MAX_POTION', 'ITEM_FULL_HEAL', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_LASS', 10, 5, ['Have you counted your badges?', 'Only the order you choose is', 'yours. The challenge grows!'])],
             house_people=[
                 [('OBJ_EVENT_GFX_OLD_WOMAN', 6, 4, 'Granny', ['I\'ve grown roses here for sixty', 'years. They grow best with', 'company around!'], 'MOVEMENT_TYPE_FACE_DOWN')],
                 [('OBJ_EVENT_GFX_MAN_2', 4, 3, 'Man', ['BELLE was a contest star before', 'she became a leader.'], 'MOVEMENT_TYPE_FACE_DOWN'),
                  ('OBJ_EVENT_GFX_SPECIES(SYLVEON)', 6, 4, 'Sylveon', ['SYLVEON: Fiiii!'], 'MOVEMENT_TYPE_WANDER_AROUND')],
                 [('OBJ_EVENT_GFX_GIRL_1', 4, 4, 'Girl', ['I want to be a FAIRY type', 'trainer just like BELLE!'], 'MOVEMENT_TYPE_WANDER_AROUND')],
             ])
    return build_town(ctx, T)
