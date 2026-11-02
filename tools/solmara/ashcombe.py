"""Ashcombe: a town of hot springs on the slope of a volcano, the Fire gym.
Route 202 south to Gearholt, Route 203 north to Silkwood. The open-air
spring in the middle of town heals everyone who soaks in it together."""
from town import *
from styles import volcanic

NAME = 'Ashcombe'
EXITS = {'down': 18, 'up': 18}


def sketch(a):
    a.rect(3, 3, 5, 5, '#')
    a.rect(32, 3, 5, 6, '#')


def extra(a, ctx):
    C = f'{NAME}_EventScript'
    a.stamp('LAYOUT_LAVARIDGE_TOWN', 1, 0, 8, 6, 16, 13)   # the open-air spring (Lavaridge\'s)
    a.obj('OBJ_EVENT_GFX_HOT_SPRINGS_OLD_WOMAN', 15, 18, f'{C}_Spring', 'MOVEMENT_TYPE_FACE_RIGHT')
    return f"""{C}_Spring::
\tlock
\tfaceplayer
\tmsgbox {C}_Text_SpringAsk, MSGBOX_YESNO
\tgoto_if_eq VAR_RESULT, NO, {C}_SpringNo
\tfadescreen FADE_TO_BLACK
\tplayfanfare MUS_HEAL
\tcallnative Area_SoakInHotSpring
\twaitfanfare
\tfadescreen FADE_FROM_BLACK
\tmsgbox {C}_Text_SpringDone, MSGBOX_DEFAULT
\trelease
\tend

{C}_SpringNo::
\tmsgbox {C}_Text_SpringNo, MSGBOX_DEFAULT
\trelease
\tend

{C}_Text_SpringAsk:
{text('The spring is big enough for', 'everyone. Care for a soak?')}
{C}_Text_SpringDone:
{text('Your POKéMON are rested and look', 'happier than ever!')}
{C}_Text_SpringNo:
{text("The water's always warm.")}
"""


def build(ctx):
    T = Town(name=NAME, ground='s', title='ASHCOMBE', w=40, h=34, style=volcanic(), exits=EXITS, sketch=sketch, extra=extra,
             mapsec='MAPSEC_ASHCOMBE', music='MUS_VERDANTURF',
             gym=(27, 10), pc=(8, 10), houses=[(6, 23), (28, 23)], plaza=(12, 21, 16, 6),
             leader='CINDRA', motto='Burning bright, burning together!',
             town_sign=(23, 9, ['ASHCOMBE', 'Hot springs for all, under the', 'smoking peak.']),
             people=[
                 ('OBJ_EVENT_GFX_OLD_WOMAN', 13, 13, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Whole groups of trainers soak', 'in the spring together after', 'beating CINDRA. Lovely to see!']),
                 ('OBJ_EVENT_GFX_HIKER', 31, 17, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'CINDRA\'s FIRE types are tough,', 'but WATER and ROCK moves cool', 'them right down.']),
                 ('OBJ_EVENT_GFX_BOY_1', 24, 28, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Route 203 north leads into', 'thick woods. SILKWOOD is up', 'there, full of BUG types!']),
                 ('OBJ_EVENT_GFX_WOMAN_4', 10, 18, 'MOVEMENT_TYPE_FACE_RIGHT', [
                     'Ash falls on the town every', 'morning. We sweep it into the', 'gardens. Great for berries!']),
             ],
             hidden=[(5, 12, 'ITEM_FIRE_STONE'), (35, 13, 'ITEM_HEAT_ROCK')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_HEAL_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_POTION',
                   'ITEM_FULL_HEAL', 'ITEM_BURN_HEAL', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_PICNICKER', 3, 5, ['My friend is still on Route 202.', 'I\'ll wait here and heal up.'])],
             house_people=[
                 [('OBJ_EVENT_GFX_MAN_1', 6, 4, 'Man', ['The volcano hasn\'t erupted in a', 'hundred years. It only smokes', 'to show off.'], 'MOVEMENT_TYPE_FACE_DOWN')],
                 [('OBJ_EVENT_GFX_LITTLE_GIRL', 4, 4, 'Girl', ['My NUMEL sleeps by the stove.', 'It says it\'s too cold outside!'], 'MOVEMENT_TYPE_WANDER_AROUND'),
                  ('OBJ_EVENT_GFX_SPECIES(NUMEL)', 6, 3, 'Numel', ['NUMEL: Nuuumel...'], 'MOVEMENT_TYPE_FACE_DOWN')],
             ])
    return build_town(ctx, T)
