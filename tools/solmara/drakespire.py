"""Drakespire: a stone city on the cliffs where dragons roost, the Dragon
gym. Route 207 north to Frostholm, Route 208 east to Port Solace."""
from town import *
from styles import woods

NAME = 'Drakespire'
EXITS = {'up': 18, 'right': 15}


def sketch(a):
    a.ellipse(8, 28, 3, 2, '~')


def build(ctx):
    T = Town(name=NAME, flowers=20, title='DRAKESPIRE', w=40, h=34, style=woods(), exits=EXITS, sketch=sketch,
             mapsec='MAPSEC_DRAKESPIRE', music='MUS_RUSTBORO', city=True,
             gym=(6, 5), pc=(27, 5), houses=[(27, 22), (14, 24)], plaza=(11, 12, 18, 9),
             leader='VALEN', motto='Wings of the ancient sky!',
             town_sign=(16, 10, ['DRAKESPIRE', 'Where dragons nest above the', 'clouds.']),
             people=[
                 ('OBJ_EVENT_GFX_EXPERT_F', 13, 22, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'VALEN\'s DRAGONS are the last', 'test before VICTORY ROAD. Or the', 'first, if you\'re brave!']),
                 ('OBJ_EVENT_GFX_BLACK_BELT', 31, 16, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'DRAGON types fear ICE, FAIRY and', 'other DRAGON moves.']),
                 ('OBJ_EVENT_GFX_SCHOOL_KID_M', 20, 27, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Route 208 east goes along the', 'coast back to PORT SOLACE.']),
                 ('OBJ_EVENT_GFX_OLD_MAN', 8, 16, 'MOVEMENT_TYPE_FACE_RIGHT', [
                     'From the top of the spire you', 'can see the whole ring of', 'towns, and the LEAGUE in the', 'middle!']),
             ],
             hidden=[(35, 28, 'ITEM_DRAGON_FANG'), (4, 13, 'ITEM_DRAGON_SCALE')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_QUICK_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_POTION',
                   'ITEM_FULL_HEAL', 'ITEM_FULL_RESTORE', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_COOLTRAINER_M' if False else 'OBJ_EVENT_GFX_MAN_3', 10, 5, ['I fought VALEN first. Lost. Then', 'I fought him with seven badges.', 'He was even tougher!'])],
             house_people=[
                 [('OBJ_EVENT_GFX_MAN_1', 6, 4, 'Man', ['A BAGON once tried to fly off', 'our roof. It just bonked its', 'head. It keeps trying!'], 'MOVEMENT_TYPE_FACE_DOWN')],
                 [('OBJ_EVENT_GFX_WOMAN_2', 4, 4, 'Woman', ['VALEN trains on the cliffs at', 'dawn. You can hear the roars', 'from here!'], 'MOVEMENT_TYPE_FACE_DOWN')],
             ])
    return build_town(ctx, T)
