# Pastimes: minigames and side activities

Single-player activities around Hoenn. Each one has its own file in
`src/pastime_*.c` and `data/scripts/pastime/`, its saved state in
`struct PastimeSaveData` (`include/pastime_save.h`, at the end of
SaveBlock3), and a log under **R > PASTIMES** when it has one.

## How activities place NPCs

Activities don't edit `map.json` or use event flags. Each lists its NPCs
and objects as `struct PastimeSpot` (`include/pastime_spots.h`): map,
position, graphics, script, and an `isShown(arg)` check. `src/pastime.c`
spawns the spots of the current map while they are in view and their check
passes, with local ids 170 to 189, and talking to one runs its script with
`VAR_0x8004 = arg`. Add a new list to `sSpotLists` in `src/pastime.c`.

Hooks into shared code: `Pastime_Update`, `Pastime_TryInteract`,
`Pastime_OnCatch` and `Pastime_OnTrainerBattleEnd` from `src/world.c`, the
talk script in `src/field_control_avatar.c`, and the PASTIMES row of the R
menu (`data/scripts/world/menu.inc`).

## Screenshots

`tools/pastime_shots/run.py <scenario.lua> <out dir>` boots the ROM headless
and runs a scenario (same driver as `tools/world_shots/`), with helpers that
write a scene straight into `struct PastimeSaveData`.

## Lost and found

![Lost and found](images/pastime-lost-found.png)

Eight people in Hoenn towns lost something on a nearby route. The lost things
lie in plain sight as item balls; pick one up and bring it back to its owner
for a reward. A thing found before meeting its owner says whose it is.

| Owner | Town | Lost | Where | Reward |
|---|---|---|---|---|
| Lucy | Petalburg City | Hair Ribbon | Route 104, flower beds | Soothe Bell |
| Evan | Rustboro City | Notebook | Petalburg Woods | Lucky Egg |
| Sailor Hank | Slateport City | Old Compass | Route 109 beach | Mystic Water |
| Timmy | Mauville City | Toy Car | Route 110 | Magnet |
| Gramps Otto | Fallarbor Town | Reading Glasses | Route 113 | Wise Glasses |
| Wren | Fortree City | Bird Whistle | Route 119 | Sharp Beak |
| Mrs. Diane | Lilycove City | Wedding Ring | Route 121 | Big Nugget |
| Kai | Dewford Town | Shell Charm | Route 104, north | Shell Bell |

**R > PASTIMES > LOST & FOUND** lists the owners met and the things found,
with where each was lost. Screenshots: `tools/pastime_shots/lost_found.lua`.

## Museum donations

![Museum donations](images/pastime-museum.png)

The Lilycove Museum opens a natural history wing. Its curator (by the
stairs on 1F) takes three kinds of donations:

- **Fossils**: any of the 15 fossil items (Root, Claw, Helix, Dome, Old
  Amber, Skull, Armor, Cover, Plume, Jaw, Sail and the four Galar
  fossilized parts). The fossil leaves the bag.
- **Rare Pokémon**: show a legendary, mythical, Ultra Beast, Paradox,
  shiny or pseudo-legendary (Dragonite, Tyranitar, Salamence, Metagross,
  ...). It stays in the party; the museum records up to 12.
- **Photos** taken with R > PHOTO; the photo wall grows at 5, 15 and 30.

The first four fossils and first three rare Pokémon stand in the hall as
life-size models (overworld Pokémon sprites), each with a plaque. Every
fossil, rare Pokémon and photo wall section is an exhibit point; at 1, 3,
5, 8, 12, 16 and 22 points the curator gives a Nugget, Rare Candy, PP Max,
Big Nugget, Comet Shard, Ability Patch and finally a Master Ball.
**R > PASTIMES > MUSEUM** lists the exhibits. Screenshots:
`tools/pastime_shots/museum.lua`.

## Delivery jobs

![Delivery jobs](images/pastime-delivery.png)

HOENN EXPRESS has a PARCEL DESK outside the Pokémon Center in Oldale,
Petalburg, Rustboro, Slateport, Mauville, Fallarbor, Fortree and Lilycove
(plus a VIGOROTH mover at the Slateport head office). A desk offers a
parcel for another of those towns you've already visited. The time limit is
90 seconds plus 20 per step of distance on the region map, counted in play
time; pay is ¥300 plus ¥100 per step.

- Delivered in half the time: **EXPRESS**, +50% pay.
- In time: full pay. Late: half pay (a notice pops up when time runs out).
- Every fifth parcel also earns an item (Great Ball, Hyper Potion, Rare
  Candy, PP Up, Nugget, Ultra Ball, Big Nugget, in turn).

**R > PASTIMES > DELIVERY** shows the parcel, where it goes, the time left
and the parcels delivered. Talking to another desk while carrying one lets
you give the job up. Screenshots: `tools/pastime_shots/delivery.lua`.

## Mail courier routes

![Mail courier routes](images/pastime-mail.png)

The PIDGEY & WINGULL POST runs one route a day, rotating through Rustboro,
Slateport, Mauville and Lilycove (by the day of the clock). Its postmaster
(in each of those towns) tells you where today's route is; in that town
they hand out three letters for people around town, who only show up while
you carry their letter. With a Pidgey, Pidgeotto, Pidgeot, Wingull or
Pelipper in the party, the bird flies one letter over by itself and the
route pays a ¥200 tip on top of ¥600. Every third route also earns a stamp
of thanks (a feather, then Rare Candy). **R > PASTIMES > MAIL ROUTE** shows
today's letters and the totals. Screenshots: `tools/pastime_shots/mail.lua`.

## Battle quiz hall

![Battle quiz hall](images/pastime-quiz.png)

The QUIZMASTER in Mauville City runs ten-question rounds on type matchups:
"A FIRE move hits FERROTHORN (GRASS/STEEL)!", with the defender's picture,
answered SUPER EFFECTIVE, NORMAL, NOT VERY or NO EFFECT. The answers come
from the battle engine's own type chart (`GetTypeModifier`), and defenders
are 40 well-known Pokémon with a spread of types. A right answer scores 10
plus twice the current streak. The first round each day that scores 40, 80,
120 or 160 wins a Great Ball, Ultra Ball, PP Up or Rare Candy.

Each round's score goes to the server's **QUIZ CHAMPS** leaderboard (world
board 16 in `server/world.py`); **R > PASTIMES > QUIZ HALL** has MY RECORDS
(best score, rounds, perfect rounds, today's prize) and QUIZ CHAMPS.
`Pastime_ShowMonPic` is a picture box that can share the screen with a
multichoice menu (`showmonpic` can't). Screenshots:
`tools/pastime_shots/quiz.lua` (two players online).

## Pokémon hotel

![Pokémon hotel](images/pastime-hotel.png)

You're the new manager of the STARFALL HOTEL on Starfall Isle. The bellhop
in the plaza (south of the houses) takes you into the lobby, a new map
`MAP_STARFALL_ISLE_HOTEL` that reuses the Battle Frontier lounge layout;
leaving puts you back next to the bellhop (a warp arrival point added to
Starfall Isle).

Each day, two guests plus one per star sit in the lobby, each wanting
something (Fresh Water, Soda Pop, a Berry, a Potion, a Repel...). Give it
to them from your bag for a tip. At the front desk, collect the day's room
money (the star rate per guest, double for guests you served) and tips;
the guests then check out and new ones come the next day. The desk also
renovates the hotel for ¥3,000, ¥8,000, ¥20,000 and ¥45,000 up to five
stars: more guests and better rates, and at five stars STEVEN checks in,
asks for a Lava Cookie and gives a Metal Coat and a big tip.
**R > PASTIMES > HOTEL** shows the stars, totals and today's guests with
what they want. Screenshots: `tools/pastime_shots/hotel.lua`.

## Pokémon cafe

![Pokémon cafe](images/pastime-cafe.png)

The chef of CAFÉ SOLEIL stands on the Lilycove waterfront, east of the
Department Store. Bring Berries (and Moomoo Milk) and pick a dish:

| Dish | Ingredients | Effect |
| --- | --- | --- |
| Spicy Curry | 2 Cheri, 1 Oran | Attack +1 for 5 battles |
| Sour Salad | 2 Aspear, 1 Oran | Defense +1 for 5 battles |
| Dry Rice Ball | 2 Chesto, 1 Oran | Sp. Atk +1 for 5 battles |
| Bitter Soup | 2 Rawst, 1 Oran | Sp. Def +1 for 5 battles |
| Sweet Pancakes | 2 Pecha, 1 Moomoo Milk | Speed +1 for 5 battles |
| Rainbow Feast | 1 Sitrus, 1 Lum, 1 Moomoo Milk | every stat +1 for 3 battles |

The boost uses the totem-aura queue (`gQueuedStatBoosts`): at the start of
each of your next battles your lead Pokémon (and its partner in doubles)
gets the boost with the "aura flared to life" message. Online (link),
recorded, Battle Frontier and Safari battles don't count. The hook is
`Pastime_OnBattleStart()` in `CreateBattleStartTask` (`src/battle_setup.c`).
A new dish replaces the old one. **R > PASTIMES > CAFE** shows the dish in
effect, battles left, and every recipe (grey when you're missing an
ingredient). Screenshots: `tools/pastime_shots/cafe.lua`.

## Cooking contests

![Cooking contests](images/pastime-cooking.png)

The COOK-OFF judge stands next to the CAFÉ SOLEIL chef in Lilycove and runs
one contest a day. Pick one of the cafe's dishes (its ingredients are used
up), then chop, stir and plate: three rounds of the timing meter
(`src/world_meter.c`). The judges score 30 for the dish (40 for the Rainbow
Feast), 20 for each step done well and up to 10 for taste, against three
rival chefs. Win to move up a rank and get its prize:

| Rank | Meter | Rivals score | Prize |
| --- | --- | --- | --- |
| NORMAL | wide zone, slow | 45 to 60 | Rare Candy |
| GREAT | 3 tiles, faster | 57 to 72 | PP Max |
| ULTRA | 3 tiles, fast | 69 to 84 | Leftovers |
| MASTER | 2 tiles, fast | 80 to 95 | Ability Capsule (then Rare Candy) |

The best score goes to the server's **COOK-OFF CHAMPS** leaderboard (world
board 17 in `server/world.py`). **R > PASTIMES > COOK-OFF** has MY RECORDS
(rank, best score, wins, entries, today's contest) and COOK-OFF CHAMPS.
Screenshots: `tools/pastime_shots/cooking.lua` (two players online).

## Bug-catching contest

![Bug-catching contest](images/pastime-bug-contest.png)

The contest officer stands at the north edge of Route 119's big field of
tall grass (below the Weather Institute). Once a day, enter to get 20 Sport
Balls and 10 minutes on the clock (overworld time; battles and menus pause
it). While the contest is on, every wild Pokémon in Route 119's grass is a
Bug type: common ones (Caterpie, Weedle, Wurmple, Ledyba, Spinarak,
Kricketot, Sewaddle, Nincada, Blipbug), uncommon ones (Venonat, Paras,
Surskit, Volbeat, Illumise, Combee, Yanma, Cutiefly, Nymble) and rare ones
(Scyther, Pinsir, Heracross, Karrablast, Shelmet, Larvesta). You keep
everything you catch.

Each Bug caught scores 20, 50 or 100 for rarity, 3 a level, half its IV
total and 50 if it's shiny; your best one counts ("NEW BEST BUG!"). One
minute before the end, and when time's up, a notice tells you to go back
to the officer (or end early by talking to him). The judges compare your
best Bug with three rivals: 1st wins a Sun Stone, 2nd an Everstone, 3rd a
Sitrus Berry, 4th an Oran Berry. Leftover Sport Balls are collected.

The score goes to the server's **BUG CONTEST** board (world board 18 in
`server/world.py`). **R > PASTIMES > BUG CONTEST** shows the clock, your
best catch, best ever, wins and entries, and the board. Hooks:
`Pastime_ModifyWildSpecies` in `World_ModifyWildSpecies` (`src/world.c`);
the wild Pokémon is read at battle start (`Pastime_OnBattleStart`).
Screenshots: `tools/pastime_shots/bug_contest.lua` (two players online).

## Trading post

![Trading post](images/pastime-trading-post.png)

The trader stands at the end of the Slateport market row. Each day he has
three barters from a pool of 18 (5 Oran Berries for a Sitrus Berry, 5 Great
Balls for 2 Ultra Balls, a Star Piece for a Thunder Stone, 2 Heart Scales for
a Fire Stone...), each once a day. He also makes one **Pokémon swap** a
day: bring a Pokémon of the type he wants and he trades you a Pokémon from
another region at level 20, holding a gift (a Fire type for SNEASEL "FROSTY"
with a Quick Claw, a Water type for GROWLITHE "EMBER" with Charcoal, a
Flying type for DRATINI "NOODLE" with a Dragon Fang, and six more). The
traded Pokémon has OT TRADER and goes in the same party slot; the Pokédex
marks it caught. **R > PASTIMES > TRADING POST** lists today's offers (grey
once done) and the trades made. Screenshots: `tools/pastime_shots/trading.lua`.
