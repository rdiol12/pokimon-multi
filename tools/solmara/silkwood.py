"""Silkwood: a village in the tree canopy, the Bug gym. Route 203 south to
Ashcombe, Route 204 west to Rosemead."""
from town import *
from styles import jungle

NAME = 'Silkwood'
EXITS = {'down': 18, 'left': 15}


def sketch(a):
    for x, y in ((14, 4), (30, 13), (4, 26), (33, 28)):
        a.rect(x, y, 4, 4, 'T')
    a.ellipse(33, 24, 2.5, 2, '~')


def build(ctx):
    T = Town(name=NAME, flowers=24, title='SILKWOOD', w=40, h=34, style=jungle(), exits=EXITS, sketch=sketch,
             mapsec='MAPSEC_SILKWOOD', music='MUS_FORTREE',
             gym=(21, 4), pc=(6, 5), houses=[(27, 20), (7, 21)], plaza=(11, 12, 15, 8),
             leader='THISTLE', motto='Small wings, big dreams!',
             town_sign=(11, 10, ['SILKWOOD', 'Where the forest hums.']),
             people=[
                 ('OBJ_EVENT_GFX_BUG_CATCHER', 14, 22, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Players swap rare BUG POKéMON', 'right here in the square. I got', 'a SCIZOR from someone yesterday!']),
                 ('OBJ_EVENT_GFX_GIRL_2', 28, 15, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'THISTLE\'s gym is full of bushes.', 'Plenty of room to get around', 'other challengers though!']),
                 ('OBJ_EVENT_GFX_OLD_MAN', 6, 13, 'MOVEMENT_TYPE_FACE_DOWN', [
                     'West is Route 204, through the', 'marsh to ROSEMEAD. Wear boots!']),
                 ('OBJ_EVENT_GFX_PICNICKER', 23, 26, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'FIRE and FLYING moves are what', 'you want against BUG types.']),
             ],
             hidden=[(36, 10, 'ITEM_SILVER_POWDER'), (4, 19, 'ITEM_BIG_MUSHROOM')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_NET_BALL', 'ITEM_NEST_BALL', 'ITEM_HYPER_POTION',
                   'ITEM_MAX_POTION', 'ITEM_FULL_HEAL', 'ITEM_ANTIDOTE', 'ITEM_REVIVE', 'ITEM_MAX_REPEL'],
             pc_people=[('OBJ_EVENT_GFX_BUG_CATCHER', 10, 5, ['I counted forty BUG POKéMON on', 'the way here. Then I lost count.'])],
             house_people=[
                 [('OBJ_EVENT_GFX_WOMAN_2', 6, 4, 'Weaver', ['I weave cloth from SPINARAK silk.', 'Strong enough to hold a', 'SNORLAX!'], 'MOVEMENT_TYPE_FACE_DOWN')],
                 [('OBJ_EVENT_GFX_LITTLE_BOY', 4, 4, 'Boy', ['From our window you can see all', 'the way to the sea!'], 'MOVEMENT_TYPE_WANDER_AROUND')],
             ])
    return build_town(ctx, T)
