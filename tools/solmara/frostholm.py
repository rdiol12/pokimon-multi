"""Frostholm: a snowbound mountain village, the Ice gym. Route 206 north to
Duskmere, Route 207 south to Drakespire."""
import copy
from town import *

NAME = 'Frostholm'
EXITS = {'up': 18, 'down': 18}


def style():
    st = Style('gTileset_General', 'gTileset_Fallarbor', ['LAYOUT_FALLARBOR_TOWN', 'LAYOUT_ROUTE114', 'LAYOUT_ROUTE115'],
               wall_classes={'T': TREES}, colors=True)
    st.exclude = {0xCF, 0xAF, 0x208, 0x350, 0x20E, 0x20F}
    st.secondary = 'gTileset_Frost'
    return st


def sketch(a):
    a.ellipse(31, 18, 3, 2, '~')
    a.rect(3, 3, 6, 5, 'T')


def build(ctx):
    T = Town(name=NAME, ground='.', title='FROSTHOLM', w=40, h=34, style=style(), exits=EXITS, sketch=sketch,
             mapsec='MAPSEC_FROSTHOLM', music='MUS_FALLARBOR', weather='WEATHER_SNOW',
             gym=(27, 8), pc=(9, 10), houses=[(6, 22), (27, 23)], plaza=(12, 14, 16, 7),
             leader='SIGRID', motto='Cold heart, clear mind!',
             town_sign=(23, 12, ['FROSTHOLM', 'Warm hearts in the cold north.']),
             people=[
                 ('OBJ_EVENT_GFX_EXPERT_M', 14, 23, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'Snowball fights in the square', 'every winter! Last year forty', 'trainers joined in.']),
                 ('OBJ_EVENT_GFX_GIRL_2', 25, 17, 'MOVEMENT_TYPE_FACE_LEFT', [
                     'SIGRID\'s ICE types hate FIRE,', 'FIGHTING, ROCK and STEEL moves.']),
                 ('OBJ_EVENT_GFX_HIKER', 7, 18, 'MOVEMENT_TYPE_FACE_RIGHT', [
                     'South, Route 207 runs through', 'the crags to DRAKESPIRE, home', 'of the DRAGON gym.']),
                 ('OBJ_EVENT_GFX_BOY_2', 22, 27, 'MOVEMENT_TYPE_WANDER_AROUND', [
                     'The gym floor is ice! Watch out,', 'you slide until you hit', 'something. Or someone!']),
             ],
             hidden=[(35, 27, 'ITEM_NEVER_MELT_ICE'), (5, 13, 'ITEM_ICE_STONE')],
             shop=['ITEM_ULTRA_BALL', 'ITEM_GREAT_BALL', 'ITEM_TIMER_BALL', 'ITEM_HYPER_POTION', 'ITEM_MAX_POTION',
                   'ITEM_FULL_HEAL', 'ITEM_ICE_HEAL', 'ITEM_REVIVE', 'ITEM_MAX_REPEL', 'ITEM_ESCAPE_ROPE'],
             pc_people=[('OBJ_EVENT_GFX_OLD_MAN', 10, 5, ['Come in, come in! Warm up by', 'the heater, all of you!'])],
             house_people=[
                 [('OBJ_EVENT_GFX_WOMAN_3', 6, 4, 'Woman', ['We bake bread for the trainers', 'crossing the mountain. Nobody', 'should travel hungry!'], 'MOVEMENT_TYPE_FACE_DOWN')],
                 [('OBJ_EVENT_GFX_LITTLE_BOY', 4, 4, 'Boy', ['My SNOM eats the frost off the', 'windows. It\'s very helpful!'], 'MOVEMENT_TYPE_WANDER_AROUND'),
                  ('OBJ_EVENT_GFX_SPECIES(SNOM)', 6, 3, 'Snom', ['SNOM: Snom!'], 'MOVEMENT_TYPE_FACE_DOWN')],
             ])
    return build_town(ctx, T)
