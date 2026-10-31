"""Turns a terrain sketch into metatiles by learning from Hoenn's own maps.

A sketch is a grid of terrain classes ('#' wall/tree/rock, '~' water,
'"' tall grass, '.' floor, plus per-style classes such as 'p' for path).
For every cell, synth() looks at the 3x3 class neighbourhood and the
metatiles already chosen to its left and above, and picks the metatile the
real maps use most often in that situation. That way tree canopies, cliff
edges and shorelines join up the way Game Freak drew them.
"""
import math
from collections import Counter, defaultdict
import numpy as np
from tiles import Pair, layouts, load_blocks

WATER = set(range(0x10, 0x16))
GRASS = {0x02, 0x03}


class Style:
    def __init__(self, primary, secondary, corpus, classes=None, wall_classes=None, colors=False):
        """corpus: layout ids to learn from (same tileset pair).
        classes: {char: set of metatile ids} for extra floor classes.
        wall_classes: {char: set of metatile ids} for extra wall classes.
        colors: split open ground by colour into 'g' grass, 's' sand and
        'r' bare rock ('.' is everything else)."""
        self.colors = colors
        self.min_count = 15
        self.exclude = set()
        self.pattern_weight = 1
        self.uniform_plain = False  # cells deep inside one class take its most common metatile
        self.primary, self.secondary = primary, secondary
        self.pair = Pair(primary, secondary)
        self.classes = classes or {}
        self.wall_classes = wall_classes or {}
        self.models = [defaultdict(Counter) for _ in range(10)]
        self.H, self.V, self.D = defaultdict(Counter), defaultdict(Counter), defaultdict(Counter)
        self.elev = defaultdict(Counter)
        L = layouts()
        for lid in corpus:
            l = L[lid]
            if (l['primary_tileset'], l['secondary_tileset']) != (primary, secondary):
                raise ValueError(f'{lid} uses another tileset')
            self.learn(load_blocks(l))

    def cls(self, block):
        m, coll = block & 0x3FF, block >> 10 & 3
        if coll:
            for c, s in self.wall_classes.items():
                if m in s:
                    return c
        for c, s in self.classes.items():
            if m in s:
                return c
        beh = self.pair.attr(m) & 0xFF
        if beh in WATER:
            return '~'
        if coll:
            if self.colors:
                r, g, b = self.pair.metatile(m).reshape(-1, 3).mean(0)
                if g > r + 20:
                    return 'T'
            return '#'
        if beh in GRASS:
            return '"'
        if self.colors:
            r, g, b = self.pair.metatile(m).reshape(-1, 3).mean(0)
            if g > r + 20:
                return 'g'
            if r > 200 and g > 175 and b < 150:
                return 's'
            if r > g + 30:
                return 'r'
        return '.'

    def keys(self, pat, left, up, upright):
        c = pat[4]
        cross = pat[1] + pat[3] + pat[4] + pat[5] + pat[7]
        pat9 = pat[:9]
        return [(0, pat, left, up, upright), (1, pat, left, up), (2, pat, left), (3, pat, up), (4, pat),
                (5, pat9, left, up), (6, pat9), (7, cross, left, up), (8, cross), (9, c)]

    def learn(self, b):
        h, w = b.shape
        cl = np.array([[self.cls(int(v)) for v in row] for row in b])
        pad = np.pad(cl, 2, mode='edge')
        for y in range(h):
            for x in range(w):
                pat = ''.join(pad[y + 1 + dy, x + 1 + dx] for dy in range(3) for dx in range(3)) + pad[y, x + 2] + pad[y + 4, x + 2]
                left = int(b[y, x - 1]) & 0x3FF if x else -1
                up = int(b[y - 1, x]) & 0x3FF if y else -1
                upr = int(b[y - 1, x + 1]) & 0x3FF if y and x + 1 < w else -1
                m = int(b[y, x])
                if x: self.H[left][m & 0x3FF] += 1
                if y: self.V[up][m & 0x3FF] += 1
                if y and x: self.D[int(b[y - 1, x - 1]) & 0x3FF][m & 0x3FF] += 1
                for i, k in enumerate(self.keys(pat, left, up, upr)):
                    self.models[i][k[1:]][m & 0x3FF] += 1
                self.elev[m & 0x3FF][m & 0xFC00] += 1

    def choose(self, pat, left, up, upleft):
        keys = self.keys(pat, left, up, -1)
        cands = None
        for i in (1, 2, 3, 4, 5, 6, 8, 9):
            cands = self.models[i].get(keys[i][1:])
            if cands:
                cands = {m: n for m, n in cands.items() if m not in self.exclude}
            if cands:
                break
        if not cands:
            return None
        common = {m: n for m, n in cands.items()
                  if sum(self.elev[m].values()) >= self.min_count and m not in self.exclude}
        if common:
            cands = common
        if len(set(pat)) == 1 and self.uniform_plain:
            plain = {m: n for m, n in self.models[4].get((pat,), cands).items() if m not in self.exclude}
            if plain:
                return max(plain, key=plain.get)

        def score(m):
            sc = self.pattern_weight * math.log(1 + cands[m])
            for table, prev, wgt in ((self.H, left, 3), (self.V, up, 3), (self.D, upleft, 1)):
                if prev >= 0:
                    n = table[prev][m]
                    sc += wgt * (math.log(1 + n) if n else -4)
            return sc
        return max(cands, key=score)

    def synth(self, sketch, fixed=None):
        """sketch: list of strings. fixed: {(x, y): block} cells already chosen.
        Returns a uint16 array of map blocks (metatile | collision | elevation)."""
        h, w = len(sketch), len(sketch[0])
        rows = [sketch[0]] * 2 + list(sketch) + [sketch[-1]] * 2
        pad = [''.join([r[0]] * 2 + list(r) + [r[-1]] * 2) for r in rows]
        out = np.zeros((h, w), np.uint16)
        fixed = fixed or {}
        for y in range(h):
            for x in range(w):
                if (x, y) in fixed:
                    out[y, x] = fixed[(x, y)]
                    continue
                pat = ''.join(pad[y + 1 + dy][x + 1 + dx] for dy in range(3) for dx in range(3)) + pad[y][x + 2] + pad[y + 4][x + 2]
                left = int(out[y, x - 1]) & 0x3FF if x else -1
                up = int(out[y - 1, x]) & 0x3FF if y else -1
                upr = -1
                m = self.choose(pat, left, up, int(out[y - 1, x - 1]) & 0x3FF if x and y else -1)
                if m is None:
                    raise ValueError(f'no metatile for class {pat[4]!r} at {x},{y}')
                out[y, x] = m | self.elev[m].most_common(1)[0][0]
        return out
