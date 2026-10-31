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
