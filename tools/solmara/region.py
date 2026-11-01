"""Builds Solmara, the second region (docs/solmara.md): every map, script,
trainer and encounter, then writes the flag/trainer ids.

    python3 tools/solmara/region.py
"""
import importlib, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import sol  # noqa: E402

# The ring, in order: each town, then the route to the next one.
TOWNS = ['PortSolace', 'Gearholt', 'Ashcombe', 'Silkwood', 'Rosemead', 'Duskmere', 'Frostholm', 'Drakespire']
TOWN_NAMES = ['PORT SOLACE', 'GEARHOLT', 'ASHCOMBE', 'SILKWOOD', 'ROSEMEAD', 'DUSKMERE', 'FROSTHOLM', 'DRAKESPIRE']
MODULES = ['port_solace', 'gearholt', 'ashcombe', 'silkwood', 'rosemead', 'duskmere', 'frostholm', 'drakespire',
           'routes', 'gyms', 'league', 'ship']


class Ctx:
    def visited(self, town):
        return sol.REG.flag(f'VISITED_{sol.snake(town)}')

    def badge(self, n):
        return sol.REG.flag(f'BADGE_{n}')

    def town_index(self, town):
        return TOWNS.index(town)


def main():
    sol.ensure_group()
    ctx = Ctx()
    # Fixed ids first, so they never move: visited flags and badges.
    for t in TOWNS:
        ctx.visited(t)
    for n in range(1, 9):
        ctx.badge(n)
    import layout
    layout.link_all()
    for m in MODULES:
        if not os.path.exists(os.path.join(HERE, m + '.py')):
            print('skip (missing)', m)
            continue
        importlib.import_module(m).build(ctx)
        print('ok', m)
    badges = '\n'.join(f'#define FLAG_SOLMARA_BADGE_{n} FLAG_SOL_BADGE_{n}' for n in range(1, 9))
    sol.REG.write(badges)
    import flight
    flight.write(ctx, TOWNS, TOWN_NAMES)


if __name__ == '__main__':
    main()
