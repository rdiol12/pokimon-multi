"""Prints a map's walkability as text, to pick spots for field-ability objects.

    python3 tools/field_ability_shots/mapgrid.py MAP_DIR_NAME [x0 y0 x1 y1]

  # wall  . floor  s sand  ~ water  i ice  " grass  O object event  H hidden item
"""
import json, os, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'areas'))
import tiles

ROOT = tiles.ROOT
BEH = [l.strip().rstrip(',') for l in open(f'{ROOT}/include/constants/metatile_behaviors.h') if l.strip().startswith('MB_')]

m = json.load(open(f'{ROOT}/data/maps/{sys.argv[1]}/map.json'))
lay = tiles.layouts()[m['layout']]
blocks = tiles.load_blocks(lay)
p, s = tiles.Tileset(lay['primary_tileset']), tiles.Tileset(lay['secondary_tileset'])
h, w = blocks.shape
x0, y0, x1, y1 = (int(v) for v in sys.argv[2:6]) if len(sys.argv) > 5 and sys.argv[2].isdigit() else (0, 0, w - 1, h - 1)
objs = {(o['x'], o['y']) for o in m.get('object_events', [])}
hid = {(b['x'], b['y']) for b in m.get('bg_events', []) if b.get('type') == 'hidden_item'}
print('    ' + ''.join(str(x % 10) for x in range(x0, x1 + 1)))
for y in range(y0, y1 + 1):
    row = ''
    for x in range(x0, x1 + 1):
        v = int(blocks[y, x]); mt, col, elev = v & 0x3FF, (v >> 10) & 3, v >> 12
        a = int(p.attrs[mt] if mt < 512 else s.attrs[mt - 512]) & 0xFF
        b = BEH[a] if a < len(BEH) else '?'
        c = '#' if col else '.'
        if not col:
            if 'SAND' in b: c = 's'
            elif 'WATER' in b or 'OCEAN' in b or 'POND' in b: c = '~'
            elif 'ICE' in b: c = 'i'
            elif 'GRASS' in b: c = '"'
        if (x, y) in objs: c = 'O'
        if (x, y) in hid: c = 'H'
        row += c
    print(f'{y:3d} {row}')

# Corners of open floor: blocked on two adjacent sides, open on the other two
# and on the diagonal between them, so an object there blocks no path.
if '--corners' in sys.argv:
    want = sys.argv[sys.argv.index('--corners') + 1]
    grid = {}
    for y in range(h):
        for x in range(w):
            v = int(blocks[y, x]); mt = v & 0x3FF
            a = int(p.attrs[mt] if mt < 512 else s.attrs[mt - 512]) & 0xFF
            b = BEH[a] if a < len(BEH) else '?'
            grid[x, y] = (bool((v >> 10) & 3) or (x, y) in objs, b, v >> 12)
    def open_(x, y): return (x, y) in grid and not grid[x, y][0] and 'WATER' not in grid[x, y][1]
    # Steps from the nearest warp (ledges and elevation ignored, no surfing).
    from collections import deque
    dist, q = {}, deque()
    for wp in m.get('warp_events', []):
        dist[wp['x'], wp['y']] = 0; q.append((wp['x'], wp['y']))
    while q:
        x, y = q.popleft()
        for nx, ny in ((x+1,y),(x-1,y),(x,y+1),(x,y-1)):
            if (nx, ny) not in dist and open_(nx, ny):
                dist[nx, ny] = dist[x, y] + 1; q.append((nx, ny))
    out = []
    for (x, y), (blk, b, e) in grid.items():
        if (x, y) not in dist: continue
        if blk or want not in b: continue
        for dx, dy in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
            if not open_(x - dx, y) and not open_(x, y - dy) and open_(x + dx, y) and open_(x, y + dy) and open_(x + dx, y + dy):
                out.append((x, y, b, e)); break
    print(' '.join(f'({x},{y} e{e} d{dist[x,y]})' for x, y, b, e in sorted(out, key=lambda t: (t[1], t[0]))))

# Open patches: tiles of a behavior whose 8 neighbours are all open too.
if '--open' in sys.argv:
    want = sys.argv[sys.argv.index('--open') + 1]
    def info(x, y):
        v = int(blocks[y, x]); mt = v & 0x3FF
        a = int(p.attrs[mt] if mt < 512 else s.attrs[mt - 512]) & 0xFF
        return bool((v >> 10) & 3) or (x, y) in objs, BEH[a] if a < len(BEH) else '?', v >> 12
    out = []
    for y in range(1, h - 1):
        for x in range(1, w - 1):
            if all(not info(x+dx, y+dy)[0] and want in info(x+dx, y+dy)[1] for dx in (-1,0,1) for dy in (-1,0,1)):
                out.append((x, y))
    print(len(out), ' '.join(f'({x},{y})' for x, y in out[::max(1, len(out)//60)]))
