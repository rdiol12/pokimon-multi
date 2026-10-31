# Fifth batch: bonding, seasons, jobs, mining, photo safari, Team Eclipse, auctions, ghost trainers

Features that give players more to do with their Pokémon, offline and
online. Each has its own section; screenshots are made with
`tools/life_shots/` (below).

Numbers this batch owns (agreed with the other feature threads):
flags `0x290`–`0x2AF`, vars `0x4091`, `0x409B`, `0x409D`, `0x40A1`,
object local ids 216–229, and its own EWRAM mailbox (`gLifeMailbox`,
magic `PKMNLIF1`). Saved state is `struct LifeSaveData` in SaveBlock3
(`include/life_save.h`). Scripts live in `data/scripts/life/`, included
from `data/scripts/life.inc`; script→C calls use `callnative`.

## Bonding

![Petting Eevee and giving it a treat](images/life-bond.png)

R > **BOND** picks a party Pokémon and opens a bonding screen on a grassy
field (`src/life_bond.c`). Move the hand with the D-pad, then hold A and rub
the Pokémon to pet it; SELECT gives it a berry from the bag; L/R switches to
another party Pokémon.

- Each pet adds 1 friendship, up to 20 a day per Pokémon.
- A treat (up to 3 a day) adds 4 friendship, 8 if the berry's strongest
  flavor is one the Pokémon's nature likes, and 1 if it dislikes it.
- The hearts are the expansion's affection levels (`GetMonAffectionHearts`),
  and the battle bonuses are its `B_AFFECTION_MECHANICS`: three hearts and
  up may hang on to a knockout hit with 1 HP, four give 1.2× EXP and a 20%
  chance to shake off a status at the end of the turn, five add critical
  hits and some evasion. They never apply in link, Frontier or secret base
  battles, so PvP stays even.

The counters are kept per Pokémon (by personality) and reset each day.

## Seasons

![Route 102 and Petalburg in spring, summer, autumn and winter](images/life-seasons.png)

The real-world month (the cartridge clock) picks the season
(`src/life_seasons.c`): spring is March to May, summer June to August,
autumn September to November, winter December to February.

- **Scenery.** On outdoor maps that use the general tileset (all of
  Hoenn's towns and routes), the tree and grass colours of the tileset's
  palette 2 change: cherry blossom in spring, Emerald's own greens in
  summer, red and gold leaves on yellowed grass in autumn, snow-covered
  trees and ground in winter (`Seasons_TintPrimaryPalettes`, called from
  `LoadTilesetPalette` in `fieldmap.c`, before the day/night tint).
- **Snow.** In winter, outdoor maps with clear skies get light snow. Story
  weather, ash, sand, fog and the roaming rain fronts are left alone.
- **Seasonal Pokémon.** 15% of wild encounters outdoors are the season's
  Pokémon, at the map's level (evolved when the level is past the official
  evolution level). The last two of each list are rare.

  | Season | Pokémon | Rare |
  | --- | --- | --- |
  | Spring | Deerling (Spring), Cherubi, Flabébé, Combee, Budew | Cutiefly, Comfey |
  | Summer | Deerling (Summer), Sunkern, Volbeat, Illumise, Dewpider | Heracross, Yanma |
  | Autumn | Deerling (Autumn), Sewaddle, Pumpkaboo, Phantump, Foongus | Applin, Zorua |
  | Winter | Deerling (Winter), Snover, Cubchoo, Bergmite, Snom | Delibird, Frigibax |
- **Forms.** Deerling and Sawsbuck in the party change to the season's
  form, as in the main series.
- The first time the player steps outside in a new season, a notice says
  so ("Winter: Snow is falling in Hoenn!").

## Pokémon jobs

![Sending Geodude on a cave survey, the job list, and picking it up](images/life-jobs.png)

A job agent in the **Mauville Pokémon Center** (left of the counter)
sends a party Pokémon out on a timed errand (`src/life_jobs.c`). Up to
three Pokémon can be out at once; while out, they are kept in
`struct LifeSaveData.jobs` and leave the party (you can't send your last
Pokémon that can battle, or an Egg). R > **JOBS** shows who is out and how
long is left.

| Job | Hours | Best types | Pay | Can bring back |
| --- | --- | --- | --- | --- |
| Berry farm help | 1 | Grass, Bug, Ground | ¥300 | Oran, Pecha, Sitrus, Lum Berries |
| Parcel delivery | 2 | Flying, Normal, Electric | ¥900 | Poké, Great, Repeat Balls, Potions |
| Lifeguard duty | 2 | Water, Ice | ¥700 | Pearl, Big Pearl, Heart Scale, Dive Balls |
| Power plant shift | 4 | Electric, Steel, Fire | ¥1,600 | PP Up, Elixir, Thunder Stone, Cell Battery |
| Cave survey | 4 | Rock, Ground, Fighting | ¥1,200 | Hard Stone, Nugget, Star Piece, Everstone, Fire/Water/Leaf Stone |
| Night watch | 8 | Dark, Ghost, Psychic | ¥2,500 | Dusk Balls, Dusk Stone, Moon Stone, Rare Candy, Spell Tag |

When the time is up (by the clock, so it keeps going while the game is
off), talk to the agent: each Pokémon comes back with the pay, one of the
job's items (1–4), 3 friendship and EXP (hours × level × 20, plus 100).
A Pokémon of one of the job's types is "a natural": double items, double
pay and 1.5× EXP. If the party is full it goes to the PC. Levels gained on
a job don't teach moves or evolve the Pokémon (use the move relearner, or
level it once in battle).

## Underground mining

![The digger, a dig wall, uncovering treasures and the haul](images/life-mining.png)

Rocks with a glint in them sit in six caves: Granite Cave 1F and B1F,
Rusturf Tunnel, Fiery Path, Meteor Falls and Victory Road 1F
(`LifeMining_*` in `src/life_mining.c`, object scripts
`Life_EventScript_DigSpot0`–`5`). They need the digging tools, which the
digger next to the Rusturf Tunnel rock gives away. Each rock can be dug
once a day.

Digging opens a wall of rock, 13×6 cells, one to three layers deep, with
two to four treasures buried in it (the items' own icons). Move the cursor
with the D-pad:

- **A, the pick**: two layers off the cell, one off the four cells around it.
  Adds 2 cracks.
- **B, the hammer**: two layers off the cell, one off the eight around it.
  Adds 4 cracks and shakes the wall.
- **START** stops digging.

The wall gives way at 70 cracks. Every treasure that was completely
uncovered by then goes in the bag. Treasures (weighted): Red, Blue, Yellow
and Green Shards, Hard Stone, Everstone, Heart Scale, Star Piece, Nugget,
Oval Stone, the evolution stones (Fire, Water, Thunder, Leaf, Moon, Sun,
Ice, Dusk, Dawn, Shiny), Fire/Water/Electric/Grass Gems, and, rarer and 3×3
cells big, Root Fossil, Claw Fossil (both revived at the Devon lab) and Rare
Bone. The digger keeps count of what you've found.

The wall and treasures are drawn on BG1 (rock, in 16×16 blocks from
`graphics/life/mine_tiles.png`, drawn by `tools/life_shots/draw_art.py` in
the browns of Emerald's caves) over BG2 (earth); the treasure icons are
sprites between the two, so they show through as the rock is cleared.

## Photo safari

![The photographer, a Pokémon roaming the Safari Zone, three photos and the album](images/life-photo.png)

A photographer stands at the Safari Zone entrance on Route 121
(`Life_EventScript_PhotoRanger`, `src/life_photo.c`). For ¥500 they lend a
camera with 12 shots and send you into the Safari Zone. With the camera
there are no wild battles; instead up to four Pokémon from the area's
grass table roam around the player (object local ids 216–219, 1 in 64
shiny). Talking to one reminds you to use the camera.

**R** takes a photo of the best subject up to five tiles straight ahead
(a little to either side counts too, more the further away it is). The
photo opens full screen, framed by a viewfinder, with the score:

| | |
|---|---|
| Distance | 1 tile 350, 2 tiles 400, 3 tiles 300, 4 tiles 200, 5 tiles 100 |
| Framing | centered +200, one tile off +100 |
| Pose | facing you +300, on the move +200, side view +100, from behind 0 |
| Group | +150 for every other Pokémon of its kind in the shot |
| Rarity | catch rate 45 or less ×3, 120 or less ×2; shiny ×2 |

The picture follows the shot: close Pokémon are big, a Pokémon seen from
behind shows its back, a side view is turned, and a group stands
together. The best photo of each species goes in the album (R > **ALBUM**,
24 pages; a new species replaces the lowest-scoring page when it's full).

When the film runs out the player is walked back to the entrance (leaving
the Safari Zone any other way also ends the safari). The photographer
adds up the safari's score and gives a prize: a Nugget for 12000 or more,
an Ultra Ball for 7000, a Great Ball for 3500, otherwise a Poké Ball.

## Screenshots

```sh
python3 tools/life_shots/run.py tools/life_shots/bond.lua out/bond
python3 tools/life_shots/run.py tools/life_shots/seasons.lua out/seasons 400000
python3 tools/life_shots/run.py tools/life_shots/jobs.lua out/jobs
python3 tools/life_shots/run.py tools/life_shots/mining.lua out/mining
python3 tools/life_shots/run.py tools/life_shots/photo.lua out/photo
```

`run.py` is `tools/world_shots/run.py` with this batch's symbols added, so
scenarios use the same Driver (`Driver.scene` and friends). They can also
write `gLifeShowcase` (`src/life.c`): byte 1 sets the lead's friendship,
then byte 0 = 1 applies it on the next overworld frame. Byte 2 is the
season plus one (0 uses the clock); it applies on the next warp. Byte 3
= 1 finishes every Pokémon job. Bytes 4–8 pose a photo subject in the
Safari Zone during a photo safari: species (u16), tiles away, the
direction it faces (`DIR_*`, 0 = toward the player) and how many stand
side by side.

Art: `tools/life_shots/draw_art.py` draws this batch's sprites into
`graphics/life/`.
