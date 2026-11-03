"""Draws a map with the audit's findings on top: one-tile chokepoints on the
main walk in red, people in walkways in orange.

    python3 tools/map_upgrade/overlay.py MAP_ID out.png
"""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'areas'))
from PIL import ImageDraw
from audit import audit
from mapdata import maps, Grid, main_paths, width_at
from tiles import Pair, layouts, load_blocks, render


def main():
    map_id, out = sys.argv[1], sys.argv[2]
    folder, m = maps()[map_id]
    l = layouts()[m['layout']]
    im = render(Pair(l['primary_tileset'], l['secondary_tileset']), load_blocks(l))
    g = Grid(m['layout'])
    path, _ = main_paths(g, m)
    d = ImageDraw.Draw(im, 'RGBA')
    for x, y in path:
        if g.walkable(x, y):
            col = (255, 0, 0, 150) if width_at(g, x, y) == 1 else (255, 255, 0, 50)
            d.rectangle((x * 16, y * 16, x * 16 + 15, y * 16 + 15), fill=col)
    for b in audit(map_id)['blockers']:
        d.rectangle((b['x'] * 16, b['y'] * 16, b['x'] * 16 + 15, b['y'] * 16 + 15), outline=(255, 128, 0), width=3)
    im.save(out)


if __name__ == '__main__':
    main()
