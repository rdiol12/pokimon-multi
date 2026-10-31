# Roaming trainers, charging wild Pokémon and quest boards

150 trainers ("rovers") live their own lives all over Hoenn. They walk the
maps with their lead Pokémon following them, fight and catch wild Pokémon,
level up, learn moves, evolve, use the Day Care, earn gym badges, battle each
other and the game's own trainers, get hurt (body-part injuries included) and
heal at Pokémon Centers. The player meets them anywhere and can:

- **watch** two of them (or one of them and a route trainer) battle: walk up
  to the fight on the map and press A. YES opens a real battle with both
  sides played by the AI. The result counts for both trainers.
- **talk** to a wandering one to see its badges, record and team, and
  **battle** it.
- **trade** with one: say no to a battle and it offers its weakest Pokémon
  for one of its favourite type (the in-game trade scene; the Pokémon you
  get has the rover as its trainer).
- see their **rankings and news** on the quest board in every town. They
  are not on the PvP ladder, which stays for players.
- make **rivals**: a rover the player beats remembers it. Online, a rival
  up to two maps away comes looking for them, shows a "!" when it spots
  the player, walks up and battles them (no saying no) until it wins one
  back. Rivals train harder.
- make **friends**: trading, exploring together and the like build
  friendship (per player, on the server). A friend greets the player
  warmly and has a present for them once a day (the best thing it
  crafted, else something from its bag).
- **team up** with one for a tag battle: with two other rovers on the map,
  the one talked to offers a 2-on-2 against them (the partner's front
  picture stands next to the player). Both opponents' records count it.
- **explore a cave floor together**: in a cave or the Rift Below, a rover
  talked to offers to come along. It follows a step behind (stepping
  aside when it's in the way), every wild Pokémon met comes in pairs and
  the rover fights beside the player (a 2-on-2 wild battle with it as the
  in-game partner), and when the player leaves the floor it says goodbye and shares a find
  (a Shard, Mushroom, Stardust, Escape Rope, now and then a Star Piece).
  Rovers dive in the Rift Below more often than they walk other caves.
- **buy from them and sell to them** on the market board and at the
  auction house (online). In towns with a Poké Mart, a rover with Pokémon
  at its ranch now and then lists one on the market at a fair price for
  its level, or puts one up for auction, and collects what it earned. A
  rover with money to spare buys players' listings and bids on their
  auctions when the price is fair, and gets its money back when outbid.
  Rovers don't buy from each other, and at most 8 of their listings and 4
  of their auctions are up at once, so the boards stay the players'.
- follow, enter or watch the **weekly Rover League** on any quest board
  (online). Every week (from Monday 00:00 UTC) the eight strongest rovers
  play a knockout bracket on the shared clock: quarterfinals until
  Wednesday, semifinals until Friday, the final until Sunday. Until the
  quarterfinals end a player can enter in place of one of the lowest
  seeds (up to four players) and plays their matches against the rover
  in their own game, for prize money (2,000 a round more each round, and
  10,000 more for winning the final). A rover-vs-rover match not played
  yet can be watched as a real AI-vs-AI battle, and what the player sees
  is its result. At each deadline the rest are decided by strength, and a
  player who didn't play their match loses it. Results go to the news
  (`server/rovers_league.py`).

Also here:

- **Charging wild Pokémon.** Now and then on a route or in a cave, a wild
  Pokémon from the map's own encounter table bursts out of the grass, shows a
  "!" and charges whoever is nearest. If it reaches the player, a battle
  starts; if it reaches a rover, the rover fights it on the map. About a third
  of them are calm and just wander (talk to one to battle it).
- **Quest boards.** A board stands next to every town's Pokémon Center. It
  lists the rampaging Pokémon (much stronger than the rest of their map) up to
  four maps away, and who cleared the last ones. Each one shows up on its map
  and charges from far away. Knock it out or catch it to clear the job; the
  reward is collected at any board. Rovers take these jobs too.

## Their own lives

Each rover carries money and a bag (`server/rovers_life.py`). On routes and
in caves it gathers Apricorns, Berries, Shards and Mushrooms, and crafts them
with the crafting recipes (`src/crafting.c`) into Apricorn Balls, Great
Balls, Potions, Super Potions, Full Heals, Revives and evolution stones.
Catching goes much better with a ball in the bag; it buys Poké Balls and
Potions at Pokémon Centers' marts when it runs short. Pokémon it can't carry
go to its ranch (up to six), where they bring back finds like the Hoenn
Ranch's (`src/ranch.c`) for the rover's favourite type; past that they go to
the Haven Reserve. In Petalburg, Slateport and Fortree it takes daily quests
(catch, train, win battles) for money, and in Mauville it sends a Pokémon on
a job that comes back stronger with pay. Winning a battle against another
rover wins prize money; gym badges pay too. The game is sent its battle
medicine (used in battles against it), ball count, ranch size and money,
which it mentions when you talk to it.

At the Day Care a rover breeds like a player does (`docs/breed_fusion.md`):
two of its Pokémon of different kinds make a **fusion** egg (the mother's
shape, the father's colors, a name from both, Wingull + Pikachu =
Wingchu), otherwise an ordinary egg. Breeders go there often. A fusion
keeps its father's species in the rover's team data (`fusion`, 2 bytes per
Pokémon on the wire), and the game sets `MON_DATA_FUSION_PARTNER` on it
wherever it battles, trades or is sold, so its colors, types, stats and
name come out as on a player's fusion. Teams saved before fusions still
load.

With the day/night code on the server (`server/world.py` `time_of_day`,
optional), most rovers rest after dark (healing if they're at a Pokémon
Center) and fewer are pulled onto the player's map; every fifth one is a
night owl and keeps going. Outdoors they only catch the night Pokémon at
night and the day ones by day, like the game's tags.

## How it works

The server (`server/rovers.py`) owns the rovers. Every ten minutes of real
time each rover may move to a connected map, train, catch something from the
map's wild table, take on the next gym, leave Pokémon at the Day Care, fight
another rover on the same map, or heal. It never runs more than 12 hours of
catch-up at once. Data:

- `server/data/rovers_cast.json` and `src/data/rovers_cast.h`: the 150 trainers
  (name, look, favourite type, style, first Pokémon), from
  `tools/rovers/make_cast.py`.
- `server/data/rovers_world.json.gz`: the map graph (connections and warps,
  through gatehouses), wild tables, Pokémon Centers, base forms and species
  names, from `tools/rovers/make_rover_world.py`. Run it again after adding
  maps.

When the player enters an outdoor map or cave, the game asks `ROVER HERE`
(the server may pull a rover or two in from next door) and plays them out
(`src/rovers.c`). The game knows evolutions and learnsets, so it brings a
rover's Pokémon up to date (a Pokémon the server levelled comes back evolved,
with moves chosen by the trainer's favourite type and the Pokémon's types)
and sends the team back (`ROVER SYNC`). What the player sees happen is sent as
`SYNC` (catches, level-ups, evolutions), `FIGHT` (rover vs rover), `BEAT`
(rover vs a game trainer or the player) and `LEAVE`.

Offline, a couple of rovers still show up now and then with teams matched to
the player's badges (their first Pokémon plus some from the map), and nothing
is kept. The quest boards need the server.

How many show up depends on the map: the game counts free object slots and
sprite palettes and shows up to four (each one is a trainer plus a Pokémon).

Watching a battle puts the left rover's team in the player's party for the
battle (the player's own team, name, gender and money are saved and put back
afterwards) and turns on the expansion's AI-vs-AI mode (`IsAiVsAiBattle`). No
experience is given and nothing is saved.

A rover's Pokémon on the market or at auction is a short note, not a
real `struct Pokemon` (the server can't build one, the game encrypts it):
"RVR", 1, the rover id, its 16-byte Pokémon, zeros up to 100 bytes
(`server/rovers_trade.py`). The game turns it into the real Pokémon, the
rover as its trainer, when it arrives (`Rovers_FromListing`, called from
`AddMarketMonToParty` in netplay.c and `ReceiveMon` in life_auction.c).

## Ids and hooks

- Object local ids 130-149 (`include/constants/rovers.h`), spawned in C; no
  map.json edits. Rover objects aren't removed when they walk off screen.
- No flags, vars or save data. EWRAM: about 1,200 bytes.
- Hooks: `Life_OverworldUpdate`/`Life_OnMapTransition` (src/life.c), talk
  scripts in `field_control_avatar.c`, `IsAiVsAiBattle` (battle_controllers.c),
  `CanInjureInBattle` (injury.c), `CB2_EndLifeTrainerBattle` and
  `BattleSetup_StartRoverWatch` (battle_setup.c), the AI-vs-AI player side's
  picture and party in `battle_controller_player_partner.c` (it looked at the
  multi-battle partner's party, which crashed on the first switch), the
  left trainer's send-out and withdraw messages (`battle_message.c`; the
  in-game partner's text read a name that isn't there and overran the stack)
  and no white-out text when the left rover loses (`BS_JumpIfNoWhiteOut`),
  and the trade scene's names for a rover trade (`trade.c`).
- Tag battles and cave partners: `gRoverTagTrainers` in
  `GetTrainerStructFromId` (include/data.h) gives the partner and the two
  opponents (ids `TRAINER_PARTNER(PARTNER_STEVEN)`, `ROVER_TAG_FOE_A/B`), so
  names, pictures and parties come out right; the partner's front picture
  in `battle_controller_player_partner.c` and its name in `frontier_util.c`.
  Cave partners: `Rovers_HasBuddy` in `TryDoDoubleWildBattle`
  (wild_encounter.c) and `DoStandardWildBattle`/`CB2_EndWildBattle`
  (battle_setup.c). The trainers are in EWRAM, not on the heap: battle
  start resets the heap (`MoveSaveBlocks_ResetHeap`).
- The Haven Reserve (`server/community_reserve.py`, other branch): a rover that
  lets a Pokémon go drops it there, and rovers in Verdanturf sometimes catch
  one. Optional: rovers.py works without it.

- The automated tests boot the game with `Netplay_IsTestBoot` (the e2e
  drivers and the screenshot runners): then no rovers, chargers, rampaging
  Pokémon or boards appear unless a screenshot scene asked for rovers
  (`gRoverShowcase`), so nothing gets in the way of the tests' set routes.

## Screenshots

`tools/rovers/shots/run.py` with the scenarios in `tools/rovers/shots/`
(run from `tools/netplay_bridge`). `gRoverShowcase` puts chosen rovers in a
chosen scene on the current map. `league.lua` needs the server's clock on
a Monday: `ROVERS_TIME_OFFSET` (seconds added to the server's time).
