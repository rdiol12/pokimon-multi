"""Ice sliding puzzles: place rocks on an ice field so that the way across
takes a few slides, and check it can be solved.

The field is a w x h block of ice, walled on all sides except one gap on
the bottom row (the way in) and one on the top row (the way out). On ice
the player slides until the next cell is blocked or they reach a cell that
is not ice.
"""
import random
from collections import deque

DIRS = ((0, -1), (0, 1), (-1, 0), (1, 0))


def slide(ice, blocked, pos, d):
    x, y = pos
    nx, ny = x + d[0], y + d[1]
    if (nx, ny) in blocked or ((nx, ny) not in ice and not passable_floor(nx, ny)):
        return None
    x, y = nx, ny
    while (x, y) in ice:
        nx, ny = x + d[0], y + d[1]
        if (nx, ny) in blocked or ((nx, ny) not in ice and not passable_floor(nx, ny)):
            break
        x, y = nx, ny
    return x, y


_floor = set()


def passable_floor(x, y):
    return (x, y) in _floor


def solve(ice, rocks, start, goal, floor):
    """Fewest moves from start to goal (None if impossible)."""
    _floor.clear()
    _floor.update(floor)
    seen = {start: 0}
    q = deque([start])
    while q:
        p = q.popleft()
        if p == goal:
            return seen[p]
        for d in DIRS:
            n = slide(ice, rocks, p, d)
            if n and n not in seen and (n in ice or n in floor):
                seen[n] = seen[p] + 1
                q.append(n)
    return None


def make(x0, y0, w, h, gap_in, gap_out, rocks=8, min_moves=6, max_moves=12, seed=1, tries=20000):
    """Returns (rock cells, moves). gap_in/gap_out are x positions of the
    floor cells just below and above the field."""
    r = random.Random(seed)
    ice = {(x, y) for y in range(y0, y0 + h) for x in range(x0, x0 + w)}
    start, goal = (gap_in, y0 + h), (gap_out, y0 - 1)
    floor = {start, goal}
    best = None
    for _ in range(tries):
        rs = set(r.sample(sorted(ice - {(gap_in, y0 + h - 1), (gap_out, y0)}), rocks))
        n = solve(ice - rs, rs, start, goal, floor)
        if n and min_moves <= n <= max_moves:
            return rs, n
        if n and (best is None or n > best[1]):
            best = (rs, n)
    if best:
        return best
    raise ValueError('no solvable layout')
