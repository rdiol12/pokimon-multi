# Adventure features

Game features from the fourth batch: trainer levels, daily quests,
crafting, ranching, dungeons, regional forms, treasure hunts, the online
Battle Frontier and the Hall of Fame wall. Each has its own section;
screenshots are made with `tools/adventure_shots/` (below).

The features with a screen of their own are under R > **ADVENTURE**
(`data/scripts/adventure/menu.inc`). Their save data is
`struct AdventureSaveData` at the end of SaveBlock3
(`include/adventure_save.h`). The overworld and battle hooks come in
through `src/adventure.c`, which the world features' hooks in
`src/world.c` call.

Numbers this batch owns (agreed with the other feature threads): flags
`0x264`–`0x27C` and `0x281`–`0x28D`, object local ids 230–235 on shared
maps, world leaderboard ids 8–15, netplay request types 160–191, and its
own EWRAM mailbox (`gAdventureMailbox`, magic `PKMNADV1`).

## Trainer levels

![Trainer levels: a level-up notice, the level card, the outfit menu, two players in outfits, and every outfit](images/adventure-trainer-levels.png)

Playing online earns trainer XP (`src/trainer_level.c`). It only counts
while connected to the server:

| | XP |
| --- | --- |
| Every minute in the overworld | 2 |
| Winning a battle against another player | 60 |
| Any other result against a player | 25 |
| A trade | 30 |
| Catching a Pokémon | 5 |
| Beating a trainer | 10 |

Level *n* needs 40 + 20·*n* XP to reach the next one, up to level 30
(about 9,900 XP in all). A notice shows each new level and what it gives.
R > ADVENTURE > TRAINER LEVEL lists every level, with the ones not reached
yet greyed out.

Most levels give an item. Five give a **perk**:

| Level | Perk |
| --- | --- |
| 5 | Trainers you beat pay 10% more |
| 10 | 10% off in Poké Marts |
| 15 | Eggs in the party hatch twice as fast |
| 20 | Trainers you beat pay 25% more (instead of 10%) |
| 25 | Wild Pokémon get one more shiny roll |

and five give an **outfit**: Autumn (3), Starlight (8), Ace (12),
Shadow (18) and Champion (30). R > ADVENTURE > OUTFITS changes clothes;
like START > STYLE, the player changes as the cursor moves. Outfits are
Brendan and May recoloured (the clothes, bandana and trim; colours taken
from Emerald's own palettes), for walking, both bikes, surfing, fishing,
watering and field moves. Each outfit is its own object graphics id, so
other players see it online with no change to the netplay protocol.
`tools/adventure/gen_outfits.py` writes the palettes and graphics
(`src/data/object_events/outfit_graphics_info*.h`).

## Screenshots

```sh
python3 tools/adventure_shots/run.py tools/adventure_shots/trainer_level.lua out/trainer_level 30000 2
```

`run.py` is `tools/world_shots/run.py` (headless libmgba, `Driver.scene`,
two players with a server when the last argument is 2) plus
`Driver.adventure{xp = ..., outfit = ...}`, which sets up a scene through
`gAdventureShowcase` (`src/adventure.c`).
