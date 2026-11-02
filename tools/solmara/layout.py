"""How Solmara's maps join up: a ring of eight towns on a 3x3 grid with the
League in the middle.

    Duskmere  -205- Rosemead  -204- Silkwood
       |206                            |203
    Frostholm       (League)        Ashcombe
       |207            |VR             |202
    Drakespire -208- PortSolace -201- Gearholt

Every exit is 4 cells wide; EXITS in each module gives the first row (left
and right edges) or column (top and bottom edges) of each exit.
"""
import importlib
import sol

RING = [  # (map, side, map, module of each)
    ('PortSolace', 'right', 'Route201'), ('Route201', 'right', 'Gearholt'),
    ('Gearholt', 'up', 'Route202'), ('Route202', 'up', 'Ashcombe'),
    ('Ashcombe', 'up', 'Route203'), ('Route203', 'up', 'Silkwood'),
    ('Silkwood', 'left', 'Route204'), ('Route204', 'left', 'Rosemead'),
    ('Rosemead', 'left', 'Route205'), ('Route205', 'left', 'Duskmere'),
    ('Duskmere', 'down', 'Route206'), ('Route206', 'down', 'Frostholm'),
    ('Frostholm', 'down', 'Route207'), ('Route207', 'down', 'Drakespire'),
    ('Drakespire', 'right', 'Route208'), ('Route208', 'right', 'PortSolace'),
]
OPP = {'left': 'right', 'right': 'left', 'up': 'down', 'down': 'up'}


def exits(name):
    if name.startswith('Route'):
        return importlib.import_module('routes').EXITS[name]
    return importlib.import_module(sol.snake(name).lower()).EXITS


def link_all():
    for a, side, b in RING:
        try:
            ea, eb = exits(a)[side], exits(b)[OPP[side]]
        except (ImportError, KeyError):
            print('not linked yet:', a, side, b)
            continue
        sol.link(a, side, b, ea, eb)
