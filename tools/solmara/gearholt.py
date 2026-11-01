"""Gearholt: a factory town of steel and steam, the Steel gym. Route 201
west to Port Solace, Route 202 north to Ashcombe. The Battle Yard in the
middle of town is a fenced-off open square for battles between players."""
from town import *

NAME = 'Gearholt'
EXITS = {'left': 15, 'up': 18}


def style():
    return Style('gTileset_General', 'gTileset_Mauville',
                 ['LAYOUT_MAUVILLE_CITY', 'LAYOUT_VERDANTURF_TOWN', 'LAYOUT_ROUTE110', 'LAYOUT_ROUTE117'],
                 wall_classes={'T': TREES})


def sketch(a):
    a.ellipse(33, 28, 3, 2, '~')


def build(ctx):
    T = Town(name=NAME, flowers=16, title='GEARHOLT', w=40, h=34, style=style(), exits=EXITS, sketch=sketch,
             mapsec='MAPSEC_GEARHOLT', music='MUS_SLATEPORT', city=True,
             gym=(26, 4), pc=(7, 5), houses=[(5, 22), (25, 22)], plaza=(11, 11, 18, 10),
             leader='BRANT', motto='Forged in fire, tempered in steel!',
             town_sign=(15, 9, ['GEARHOLT', 'The town that never stops', 'working.', '',
                               'BATTLE YARD: the square is open', 'to all trainers. Battle away!']),
             people=[
                 ('OBJ_EVENT_GFX_MECHANIC' if False else 'OBJ_EVENT_GFX_MAN_4', 9, 18, 'MOVEMENT_TYPE_FACE_RIGHT', [
                     'This square used to be the ore', 'yard. Now trainers come here', 'to battle each other!']),
                 ('OBJ_EVENT_GFX_YOUNGSTER', 30, 13, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'I watch the battles in the yard', 'every day. Talk to a trainer to', 'challenge them!']),
                 ('OBJ_EVENT_GFX_WORKER_M' if False else 'OBJ_EVENT_GFX_HIKER', 33, 18, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'BRANT\'s STEEL types shrug off', 'almost anything. Fire and', 'Fighting moves melt them down!']),
                 ('OBJ_EVENT_GFX_LASS', 14, 25, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Route 202 north goes through the', 'dunes to ASHCOMBE. Bring water!']),
                 ('OBJ_EVENT_GFX_OLD_MAN', 21, 27, 'MOVEMENT_TYPE_FACE_UP', [
                     'In my day we had one gym and', 'one road. Now the whole ring is', 'open from the start!']),
             ],
             hidden=[(35, 30, 'ITEM_METAL_COAT'), (4, 12, 'ITEM_NUGGET')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_HEAVY_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_POTION',
                   'ITEM_FULL_HEAL', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_METAL_COAT', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_BLACK_BELT', 3, 5, ['I came from HOENN with my whole', 'club. We take a gym each and', 'compare notes!'])],
             house_people=[
                 [('OBJ_EVENT_GFX_MAN_2', 6, 4, 'Smith', ['I forge the gym\'s doors. Eight', 'trainers can walk in side by', 'side!'], 'MOVEMENT_TYPE_FACE_DOWN'),
                  ('OBJ_EVENT_GFX_SPECIES(MAGNEMITE)', 3, 4, 'Magnemite', ['MAGNEMITE: Bzzt bzzt!'], 'MOVEMENT_TYPE_WANDER_AROUND')],
                 [('OBJ_EVENT_GFX_WOMAN_3', 4, 4, 'Woman', ['The factory whistle blows at', 'noon. Everyone stops for lunch,', 'even the gym trainers!'], 'MOVEMENT_TYPE_FACE_DOWN')],
             ])
    return build_town(ctx, T)
