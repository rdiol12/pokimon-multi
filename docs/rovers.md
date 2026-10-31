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
- see their **rankings and news** on the quest board in every town, and on the
  PvP ladder (rover-vs-rover fights are rated like player battles).

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

## Ids and hooks

- Object local ids 130-149 (`include/constants/rovers.h`), spawned in C; no
  map.json edits. Rover objects aren't removed when they walk off screen.
- No flags, vars or save data. EWRAM: about 870 bytes.
- Hooks: `Life_OverworldUpdate`/`Life_OnMapTransition` (src/life.c), talk
  scripts in `field_control_avatar.c`, `IsAiVsAiBattle` (battle_controllers.c),
  `CanInjureInBattle` (injury.c), `CB2_EndLifeTrainerBattle` and
  `BattleSetup_StartRoverWatch` (battle_setup.c), the AI-vs-AI player side's
  picture and party in `battle_controller_player_partner.c` (it looked at the
  multi-battle partner's party, which crashed on the first switch), the
  left trainer's send-out and withdraw messages (`battle_message.c`; the
  in-game partner's text read a name that isn't there and overran the stack)
  and no white-out text when the left rover loses (`BS_JumpIfNoWhiteOut`).
- The Haven Reserve (`server/community_reserve.py`, other branch): a rover that
  lets a Pokémon go drops it there, and rovers in Verdanturf sometimes catch
  one. Optional: rovers.py works without it.

## Screenshots

`tools/rovers/shots/run.py` with the scenarios in `tools/rovers/shots/`
(run from `tools/netplay_bridge`). `gRoverShowcase` puts chosen rovers in a
chosen scene on the current map.
