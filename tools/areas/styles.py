"""Terrain styles: which real maps each kind of area learns its look from."""
from synth import Style

# General tileset trees (Hoenn routes) and Fortree's dense jungle blocks.
TREES = set(range(0x1C0, 0x200)) | {0xC6, 0xC7, 0x0E, 0x0F, 0x16, 0x17}

# General's 3x3 edge sets (top-left corner; rows are 8 metatiles apart).
POND = 0xC8
SAND = 0x118
PLAZA = 0x100

_cache = {}


def jungle():
    """Fortree, Route 119 and Route 120: rainforest with treehouses."""
    if 'jungle' not in _cache:
        _cache['jungle'] = Style('gTileset_General', 'gTileset_Fortree',
                                 ['LAYOUT_ROUTE119', 'LAYOUT_ROUTE120', 'LAYOUT_FORTREE_CITY'],
                                 wall_classes={'T': TREES})
    return _cache['jungle']


def marsh():
    """Route 119, Route 120 and Fortree without the shallow bridge water: ponds stay one clean colour."""
    if 'marsh' not in _cache:
        _cache['marsh'] = Style('gTileset_General', 'gTileset_Fortree',
                                ['LAYOUT_ROUTE119', 'LAYOUT_ROUTE120', 'LAYOUT_FORTREE_CITY'],
                                wall_classes={'T': TREES})
        _cache['marsh'].exclude = {0x277, 0x0A1, 0x124, 0x29F, 0x071, 0x12C, 0x189, 0x0B1, 0x297, 0x28F, 0x218, 0x210, 0x120, 0x0B8, 0x291, 0x274, 0x0CA, 0x276, 0x28A, 0x27C, 0x0D8, 0x287}
    return _cache['marsh']


def woods():
    """Route 104, Route 116 and Petalburg Woods: round broadleaf trees."""
    if 'woods' not in _cache:
        _cache['woods'] = Style('gTileset_General', 'gTileset_Rustboro',
                                ['LAYOUT_ROUTE104', 'LAYOUT_ROUTE116', 'LAYOUT_PETALBURG_WOODS'],
                                wall_classes={'T': TREES})
    return _cache['woods']


def volcanic():
    """Lavaridge, Route 112, Mt. Chimney and Jagged Pass: red rock and ash."""
    if 'volcanic' not in _cache:
        _cache['volcanic'] = Style('gTileset_General', 'gTileset_Lavaridge',
                                   ['LAYOUT_LAVARIDGE_TOWN', 'LAYOUT_ROUTE112', 'LAYOUT_MT_CHIMNEY', 'LAYOUT_JAGGED_PASS'],
                                   wall_classes={'T': TREES}, colors=True)
        _cache['volcanic'].exclude = {0x202, 0x2BB, 0x304, 0x215, 0x113}  # river banks and signs
    return _cache['volcanic']


LAVA = {0x2BD, 0x333}


def magma():
    """Fiery Path and the Magma Hideout: red caverns with lava lakes."""
    if 'magma' not in _cache:
        _cache['magma'] = Style('gTileset_General', 'gTileset_Lavaridge',
                                ['LAYOUT_FIERY_PATH', 'LAYOUT_MAGMA_HIDEOUT_1F', 'LAYOUT_MAGMA_HIDEOUT_2F_1R',
                                 'LAYOUT_MAGMA_HIDEOUT_2F_2R', 'LAYOUT_MAGMA_HIDEOUT_3F_1R', 'LAYOUT_MAGMA_HIDEOUT_4F'],
                                wall_classes={'L': LAVA})
        _tune_magma(_cache['magma'])
    return _cache['magma']


def _tune_magma(st):
    st.pattern_weight = 2.5
    st.uniform_plain = True
    st.exclude |= {0x215, 0x364, 0x33B, 0x30E, 0x2AF, 0x35C, 0x3A8, 0x3A9, 0x397, 0x396, 0x39D, 0x25E, 0x256}  # machines, ladders, doors
    return st


def canyon():
    """Routes 114 and 115: terraced rock, rivers and a wooden bridge."""
    if 'canyon' not in _cache:
        _cache['canyon'] = Style('gTileset_General', 'gTileset_Fallarbor', ['LAYOUT_ROUTE114', 'LAYOUT_ROUTE115'],
                                 wall_classes={'T': TREES}, colors=True)
        _cache['canyon'].exclude = {0xCF, 0xAF, 0x208, 0x350, 0x20E, 0x20F}  # stairs, holes, sand
    return _cache['canyon']


ICE = 0x38D
ICE_ROCKS = {0x37D, 0x37E}


def ice():
    """Shoal Cave's ice room: frozen walls, snow and slippery ice."""
    if 'ice' not in _cache:
        st = Style('gTileset_General', 'gTileset_Cave', ['LAYOUT_SHOAL_CAVE_LOW_TIDE_ICE_ROOM'],
                   classes={'I': {ICE}, 'U': {0x381, 0x383, 0x304}}, wall_classes={'B': ICE_ROCKS})
        st.min_count = 1
        st.exclude = {0x28F, 0x37F, 0x38B, 0x386, 0x363, 0x377}  # cracks, ladders, the exit, strays
        _cache['ice'] = st
    return _cache['ice']


def desert():
    """Route 111's desert: sand, rock mesas and boulders."""
    if 'desert' not in _cache:
        st = Style('gTileset_General', 'gTileset_Mauville', ['LAYOUT_ROUTE111'], wall_classes={'T': TREES}, colors=True)
        _cache['desert'] = st
    return _cache['desert']


def ruins():
    """The Desert Ruins and the Sealed Chamber: sandstone halls."""
    if 'ruins' not in _cache:
        st = Style('gTileset_General', 'gTileset_Cave', ['LAYOUT_DESERT_RUINS', 'LAYOUT_SEALED_CHAMBER_OUTER_ROOM'])
        st.min_count = 1
        st.exclude = {0x229, 0x22A, 0x22B, 0x22C, 0x232, 0x233, 0x234, 0x235, 0x260, 0x261, 0x207,
                      0x208, 0x209, 0x20A, 0x218, 0x21A, 0x180, 0x189, 0x190, 0x181, 0x156, 0x147, 0x192}
        _cache['ruins'] = st
    return _cache['ruins']


def town():
    """Littleroot, Oldale and Petalburg with Routes 101-103: Hoenn's small towns."""
    if 'town' not in _cache:
        _cache['town'] = Style('gTileset_General', 'gTileset_Petalburg',
                               ['LAYOUT_PETALBURG_CITY', 'LAYOUT_OLDALE_TOWN', 'LAYOUT_LITTLEROOT_TOWN',
                                'LAYOUT_ROUTE101', 'LAYOUT_ROUTE102', 'LAYOUT_ROUTE103'],
                               wall_classes={'T': TREES}, colors=True)
    return _cache['town']


def abyss():
    """Hoenn's underwater routes: sea-floor trenches between rock walls,
    seaweed as the grass. Drawn with gTileset_Abyss, the same tiles plus the
    reef and sunken city art (tools/areas/abyss_art.py)."""
    if 'abyss' not in _cache:
        st = Style('gTileset_General', 'gTileset_Underwater',
                   ['LAYOUT_UNDERWATER_ROUTE124', 'LAYOUT_UNDERWATER_ROUTE126', 'LAYOUT_UNDERWATER_ROUTE127',
                    'LAYOUT_UNDERWATER_ROUTE125', 'LAYOUT_UNDERWATER_ROUTE128', 'LAYOUT_UNDERWATER_ROUTE129',
                    'LAYOUT_UNDERWATER_ROUTE105'],
                   classes={'"': {0x201, 0x281}})
        st.secondary = 'gTileset_Abyss'
        _cache['abyss'] = st
    return _cache['abyss']


MINE_LADDER_DOWN, MINE_LADDER_UP = 0x23F, 0x23E


def mine():
    """Granite Cave's brown rock: tunnels and caverns, drawn with
    gTileset_Mine (the same tiles plus the rails, carts and timber of
    tools/areas/mine_art.py)."""
    if 'mine' not in _cache:
        st = Style('gTileset_General', 'gTileset_Cave',
                   ['LAYOUT_GRANITE_CAVE_1F', 'LAYOUT_GRANITE_CAVE_B1F', 'LAYOUT_GRANITE_CAVE_B2F'])
        st.min_count = 1
        st.secondary = 'gTileset_Mine'
        st.exclude = {0x23F, 0x23E, 0x23D, 0x23C, 0x207, 0x22F, 0x0E8, 0x209, 0x252, 0x246, 0x23B, 0x23A, 0x242}
        _cache['mine'] = st
    return _cache['mine']


def plant():
    """New Mauville's halls: cream walls and grey floors, drawn with
    gTileset_Plant (the same tiles plus the breakers and appliances of
    tools/areas/plant_art.py)."""
    if 'plant' not in _cache:
        st = Style('gTileset_General', 'gTileset_BikeShop', ['LAYOUT_NEW_MAUVILLE_INSIDE'])
        st.min_count = 4
        st.secondary = 'gTileset_Plant'
        st.exclude = set(range(0x2B0, 0x2F8)) | {0x22E, 0x236, 0x24F, 0x23E, 0x269, 0x271}
        _cache['plant'] = st
    return _cache['plant']


def crystal():
    """Victory Road and Granite Cave in violet: the Prism Caverns, drawn with
    gTileset_Crystal (the cave tiles recoloured plus the crystals, pads and
    camp of tools/areas/crystal_art.py)."""
    if 'crystal' not in _cache:
        st = Style('gTileset_General', 'gTileset_Cave',
                   ['LAYOUT_VICTORY_ROAD_1F', 'LAYOUT_VICTORY_ROAD_B1F', 'LAYOUT_GRANITE_CAVE_B1F'])
        st.min_count = 1
        st.secondary = 'gTileset_Crystal'
        st.exclude = {0x23F, 0x23E, 0x23D, 0x23C, 0x207, 0x22F, 0x0E8, 0x209, 0x252, 0x246, 0x23B, 0x23A, 0x242,
                      0x2BB, 0x2BC, 0x2C3, 0x2C4, 0x204, 0x205}  # ladders, holes, exits and the plank bridges
        _cache['crystal'] = st
    return _cache['crystal']
