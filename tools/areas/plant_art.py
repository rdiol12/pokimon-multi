"""Voltspire Power Plant's tileset: the New Mauville tiles (gTileset_BikeShop)
plus new pixel art in Emerald's style: breaker panels in three colours
(lever up or down), the break room's appliances a Rotom hides in (a TV, a
fridge, a washing machine, a fan and a lawn mower) and humming coils.

    python3 tools/areas/plant_art.py

Writes data/tilesets/secondary/plant/ and tools/areas/plant_tiles.json.
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from art import Canvas, pack, ROOT

FLOOR = 0x210             # New Mauville's floor shows under see-through pixels

PAL = [(115, 197, 164), (41, 41, 57), (82, 90, 106), (139, 148, 164), (197, 205, 213), (246, 246, 255),
       (205, 164, 41), (255, 222, 82), (41, 106, 205), (106, 172, 255), (197, 49, 41), (255, 115, 90),
       (49, 156, 74), (123, 222, 106), (238, 131, 41), (172, 230, 255)]
OUT, STEEL_D, STEEL, STEEL_L, WHITE, YEL_D, YEL, BLUE_D, BLUE, RED_D, RED, GRN_D, GRN, ORANGE, GLASS = range(1, 16)
WHITE_D = STEEL_L
COLOURS = {'yellow': (YEL_D, YEL), 'blue': (BLUE_D, BLUE), 'red': (RED_D, RED)}


def breaker(colour, on):
    """A breaker box on the wall: a coloured band, a lever and a lamp."""
    dark, light = COLOURS[colour]
    c = Canvas(16, 32)
    c.rect(1, 2, 14, 29, STEEL)
    c.rect(1, 2, 14, 3, STEEL_L)
    c.rect(1, 28, 14, 29, STEEL_D)
    c.rect(2, 5, 13, 8, light)                      # the colour band
    c.rect(2, 8, 13, 8, dark)
    c.rect(5, 11, 10, 25, STEEL_D)                  # the lever slot
    y = 12 if on else 20
    c.rect(4, y, 11, y + 3, WHITE)
    c.rect(4, y + 3, 11, y + 3, WHITE_D)
    c.disc(12, 26.5, 1.5, GRN if on else RED)       # the lamp
    c.outline(OUT)
    return c


def tv():
    c = Canvas(16, 32)
    c.rect(4, 26, 11, 31, STEEL_D)                  # the stand
    c.rect(1, 6, 14, 25, STEEL)
    c.rect(1, 6, 14, 7, STEEL_L)
    c.rect(3, 9, 12, 21, BLUE_D)
    c.rect(4, 10, 7, 12, GLASS)
    c.rect(10, 23, 11, 23, RED)
    c.line(5, 0, 8, 5, STEEL_D)                     # rabbit ears
    c.line(11, 0, 8, 5, STEEL_D)
    c.outline(OUT)
    return c


def fridge():
    c = Canvas(16, 32)
    c.rect(1, 1, 14, 31, WHITE)
    c.rect(1, 1, 2, 31, WHITE)
    c.rect(13, 1, 14, 31, WHITE_D)
    c.rect(1, 11, 14, 11, STEEL)
    c.rect(11, 4, 11, 9, STEEL_D)                   # handles
    c.rect(11, 14, 11, 22, STEEL_D)
    c.rect(1, 29, 14, 31, STEEL)
    c.outline(OUT)
    return c


def washer():
    c = Canvas(16, 32)
    c.rect(1, 6, 14, 31, WHITE)
    c.rect(13, 6, 14, 31, WHITE_D)
    c.rect(1, 6, 14, 10, STEEL_L)                   # the control strip
    c.disc(4, 8, 1.2, RED)
    c.disc(8, 8, 1.2, BLUE)
    c.disc(8, 20, 5.5, STEEL)                       # the round door
    c.disc(8, 20, 4.5, BLUE)
    c.disc(6.5, 18.5, 1.5, GLASS)
    c.outline(OUT)
    return c


def fan():
    c = Canvas(16, 32)
    c.rect(7, 16, 8, 28, STEEL)
    c.rect(3, 28, 12, 31, STEEL_D)
    c.disc(8, 9, 7, STEEL_D)
    c.disc(8, 9, 6, GLASS)
    for k in range(3):                               # blades
        t = k * 2 * np.pi / 3
        c.line(8, 9, 8 + 5 * np.cos(t), 9 + 5 * np.sin(t), STEEL_L, 2)
    c.disc(8, 9, 1.5, ORANGE)
    c.outline(OUT)
    return c


def mower():
    c = Canvas(16, 32)
    c.line(3, 6, 6, 20, STEEL_D, 2)                 # the handle
    c.rect(1, 5, 5, 6, STEEL)
    c.rect(2, 20, 14, 27, GRN)
    c.rect(2, 20, 14, 21, GRN)
    c.rect(2, 26, 14, 27, GRN_D)
    c.rect(9, 17, 12, 20, ORANGE)                   # the engine
    for x in (4, 12):
        c.disc(x, 29, 2.2, STEEL_D)
    c.outline(OUT)
    return c


def coil():
    """A humming Tesla coil on a base."""
    c = Canvas(16, 32)
    c.rect(2, 26, 13, 31, STEEL_D)
    c.rect(2, 26, 13, 26, STEEL_L)
    c.rect(5, 10, 10, 26, ORANGE)
    for y in range(11, 26, 2):
        c.rect(5, y, 10, y, YEL_D)
    c.disc(7.5, 6, 5, STEEL)
    c.disc(6, 4.5, 2, STEEL_L)
    c.px(2, 1, YEL)
    c.px(13, 2, YEL)
    c.px(1, 4, GLASS)
    c.outline(OUT)
    return c


OBJECTS = {f'breaker_{col}_{"on" if on else "off"}': (lambda col=col, on=on: breaker(col, on), 12, {})
           for col in COLOURS for on in (False, True)}
OBJECTS.update({
    'tv': (tv, 12, {}),
    'fridge': (fridge, 12, {}),
    'washer': (washer, 12, {}),
    'fan': (fan, 12, {}),
    'mower': (mower, 12, {}),
    'coil': (coil, 12, {}),
})


def main():
    pack('data/tilesets/secondary/bike_shop', 'data/tilesets/secondary/plant', OBJECTS, {12: PAL}, FLOOR,
         f'{ROOT}/tools/areas/plant_tiles.json', fill_free=True)


if __name__ == '__main__':
    main()
