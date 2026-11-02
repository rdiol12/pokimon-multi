# Bigger maps for many players

Hoenn's towns were drawn for one player. With everyone online at once, the
small towns fill up fast and there is nowhere to stand around, trade or
battle. This upgrade grows the towns and keeps a tool for checking every map.

## Town squares

Nine towns grew a new district with a square where players meet:

| Town | Grew | Square |
| --- | --- | --- |
| Littleroot Town | 18 rows south (20x38) | Littleroot Green: a sandy square, a pond, flowers |
| Oldale Town | 16 columns east (36x20) | Oldale Square, through the gap in the east trees |
| Petalburg City | 16 rows south (30x46) | Petalburg Plaza, down the path past the south house |
| Dewford Town | 14 rows south (20x34) | Dewford Beach Square on the sand, down to the sea |
| Verdanturf Town | 14 rows south (20x34) | Verdanturf Meadow, a paved square in the grass |
| Mauville City | 16 rows in the middle (40x36) | Mauville Crossroads Square, between the city and Route 110 |
| Fallarbor Town | 14 rows south (20x34) | Fallarbor Ash Square, a sunken yard in the ash |
| Lavaridge Town | 14 rows south (20x34) | Lavaridge Hot Square, below the cliff |
| Fortree City | 14 rows south (40x34) | Fortree Clearing, the one open place on the ground |

Every square has:

- a **guide** who opens the ONLINE HUB or the PLAZA (offline she says how to
  join),
- a **matchmaker** who runs the QUICK MATCH list (a battle or a trade with
  anyone online who asked for the same), the same as the desk upstairs in
  every Pokémon Center,
- a **notice board**, a local or two with something to say about the town,
  and a hidden item (flags `FLAG_SQUARE_*_HIDDEN`).

Towns only grew south or east, so nothing already in them moved (warps,
people, fly spots, the pastimes' and living world's spots all keep their
coordinates). Mauville is the exception: its bottom two rows, where Route
110's road comes in, moved 16 rows down together with the item ball on them,
and the new district fills the gap.

The new ground is drawn by the area synthesiser (`tools/areas/synth.py`),
which learns from Game Freak's own maps with the same tileset, so tree lines,
cliffs and paths join up the way they do in the original. A handful of
one-off metatiles (bike racks, ladders, bits of houses) are kept out of the
new ground (`EXCLUDE` in `squares.py`).

### Tools

- `tools/map_upgrade/extend.py`: `Extension` grows any map on any side, keeps
  every old metatile, lets the synthesiser draw the new ground and the seam,
  and (when growing left or up) shifts the map's events and both sides of its
  connections.
- `tools/map_upgrade/squares.py`: the nine squares. The first run saves each
  town's original layout and map.json in `tools/map_upgrade/orig/`; every run
  starts from those, so a rerun puts back the saved map.json (redo any later
  hand edits there). It checks that every new person, sign and hidden item can
  be reached on foot (collision, elevation and water), and writes
  `data/scripts/town_squares.inc` and `include/constants/map_upgrade.h`.
- `tools/map_upgrade/audit.py`: checks every map for one-tile chokepoints on
  the walk between exits, people standing in those walkways or in front of
  doors, and story triggers. `--json out.json` writes the full list.

Map upgrade flags (`MAPUP_FLAG(n)`) use a block of Solmara's extended-save
flag space that Solmara doesn't use (ids 0x7600 up).

Screenshots: `python3 tools/areas/shots/run.py tools/map_upgrade/shots/squares.lua OUT 300000`.
