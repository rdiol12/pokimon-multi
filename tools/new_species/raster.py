"""A tiny pixel-art renderer for drawing new Pokémon in the Emerald style.

Shapes (ellipses, polygons, tapered strokes) are drawn at 1x straight onto an
indexed canvas, each with a material: a ramp of palette indices from outline
to highlight. Round shapes are shaded like spheres lit from the top left (the
light every Emerald sprite uses), strokes like tubes, and every shape gets a
1px darker edge where it meets something else, plus a dark outline around
the whole sprite, as hand-drawn GBA sprites have.
"""
import math

LIGHT = (-0.55, -0.65, 0.52)
_n = math.sqrt(sum(c * c for c in LIGHT))
LIGHT = tuple(c / _n for c in LIGHT)


class Material:
    def __init__(self, outline, dark, mid, light, highlight=None, edge=None):
        self.outline, self.dark, self.mid, self.light = outline, dark, mid, light
        self.highlight = highlight if highlight is not None else light
        self.edge = edge if edge is not None else dark

    def tone(self, i):
        if i > 0.86:
            return self.highlight
        if i > 0.55:
            return self.light
        if i > 0.18:
            return self.mid
        return self.dark


class Canvas:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [[0] * w for _ in range(h)]
        self.mat = [[None] * w for _ in range(h)]

    def _put(self, x, y, color, mat):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = color
            self.mat[y][x] = mat

    def _fill(self, inside, shade, mat, edge=True, bbox=None):
        x0, y0, x1, y1 = bbox or (0, 0, self.w - 1, self.h - 1)
        x0, y0 = max(0, int(x0) - 1), max(0, int(y0) - 1)
        x1, y1 = min(self.w - 1, int(x1) + 1), min(self.h - 1, int(y1) + 1)
        mask = set()
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                if inside(x + 0.5, y + 0.5):
                    mask.add((x, y))
        for (x, y) in mask:
            c = mat.tone(shade(x + 0.5, y + 0.5))
            if edge and any((x + dx, y + dy) not in mask for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                c = mat.edge
            self._put(x, y, c, mat)
        return mask

    def ellipse(self, cx, cy, rx, ry, mat, clip=None, edge=True, flat=None, bias=0.0):
        def inside(x, y):
            if clip and not clip(x, y):
                return False
            return ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0

        def shade(x, y):
            if flat is not None:
                return flat
            nx, ny = (x - cx) / rx, (y - cy) / ry
            nz = math.sqrt(max(0.0, 1 - nx * nx - ny * ny))
            return nx * LIGHT[0] + ny * LIGHT[1] + nz * LIGHT[2] + bias
        return self._fill(inside, shade, mat, edge, (cx - rx, cy - ry, cx + rx, cy + ry))

    def polygon(self, pts, mat, edge=True, flat=0.4, grad=None):
        xs, ys = [p[0] for p in pts], [p[1] for p in pts]

        def inside(x, y):
            c = False
            j = len(pts) - 1
            for i in range(len(pts)):
                xi, yi = pts[i]
                xj, yj = pts[j]
                if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
                    c = not c
                j = i
            return c

        def shade(x, y):
            if grad:
                return grad(x, y)
            return flat
        return self._fill(inside, shade, mat, edge, (min(xs), min(ys), max(xs), max(ys)))

    def stroke(self, pts, w0, w1, mat, edge=True, steps=48):
        """A tube along a quadratic/cubic Bézier (3 or 4 points), w0 -> w1 wide."""
        path = []
        for k in range(steps + 1):
            t = k / steps
            if len(pts) == 3:
                (ax, ay), (bx, by), (cx, cy) = pts
                x = (1 - t) ** 2 * ax + 2 * (1 - t) * t * bx + t * t * cx
                y = (1 - t) ** 2 * ay + 2 * (1 - t) * t * by + t * t * cy
            else:
                (ax, ay), (bx, by), (cx, cy), (dx, dy) = pts
                u = 1 - t
                x = u ** 3 * ax + 3 * u * u * t * bx + 3 * u * t * t * cx + t ** 3 * dx
                y = u ** 3 * ay + 3 * u * u * t * by + 3 * u * t * t * cy + t ** 3 * dy
            path.append((x, y, w0 + (w1 - w0) * t))
        xs, ys = [p[0] for p in path], [p[1] for p in path]
        r = max(w0, w1)

        def nearest(x, y):
            best = None
            for i in range(len(path) - 1):
                ax, ay, aw = path[i]
                bx, by, bw = path[i + 1]
                vx, vy = bx - ax, by - ay
                L = vx * vx + vy * vy or 1e-9
                t = max(0.0, min(1.0, ((x - ax) * vx + (y - ay) * vy) / L))
                px, py = ax + vx * t, ay + vy * t
                d = math.hypot(x - px, y - py)
                w = aw + (bw - aw) * t
                side = ((x - px) * -vy + (y - py) * vx) / math.sqrt(L)
                if best is None or d - w / 2 < best[0]:
                    best = (d - w / 2, side / max(w / 2, 0.5))
            return best

        def inside(x, y):
            return nearest(x, y)[0] <= 0

        def shade(x, y):
            s = nearest(x, y)[1]
            # Tube lit from the left: the side towards the light is brighter.
            return 0.5 - 0.45 * s
        return self._fill(inside, shade, mat, edge, (min(xs) - r, min(ys) - r, max(xs) + r, max(ys) + r))

    def dot(self, x, y, color):
        self._put(int(x), int(y), color, None)

    def outline(self, color_of=None, default=1):
        """Dark outline on the sprite's outer edge (inside the shapes)."""
        edge = []
        for y in range(self.h):
            for x in range(self.w):
                if not self.px[y][x]:
                    continue
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nx, ny = x + dx, y + dy
                    if not (0 <= nx < self.w and 0 <= ny < self.h) or not self.px[ny][nx]:
                        edge.append((x, y))
                        break
        for x, y in edge:
            m = self.mat[y][x]
            self.px[y][x] = m.outline if m else default

    def shifted(self, dx, dy):
        out = Canvas(self.w, self.h)
        for y in range(self.h):
            for x in range(self.w):
                if self.px[y][x] and 0 <= x + dx < self.w and 0 <= y + dy < self.h:
                    out.px[y + dy][x + dx] = self.px[y][x]
                    out.mat[y + dy][x + dx] = self.mat[y][x]
        return out
