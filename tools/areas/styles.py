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
