# The living world

Ten systems that make Hoenn feel lived in. Code: `src/alive*.c`, scripts in
`data/scripts/alive.inc`, server side in `server/alive.py`. Save data is
`struct AliveSave` at the end of `SaveBlockExt` (`include/alive_save.h`).
Our objects use local ids 52-69 (`include/constants/alive.h`).

| # | System | File | What happens |
|---|--------|------|--------------|
| 1 | Visible wild Pokémon | `alive_wild.c` | Overworld encounters are on. Some Pokémon come in packs of 2-3; at night most sleep (Zzz) and start the battle asleep. Nocturnal ones stay awake. |
| 2 | Town routines | `alive_town.c` | Wandering townsfolk walk home through a door at 7-9 PM and come out at 5-8 AM. Poké Marts close 10 PM-6 AM. |
| 3 | NPCs who remember you | `alive_talk.c` | Townsfolk remember the player (96 slots) and remark once a day on new badges, an evolved or new lead, scars, an outfit, help given, or being a regular. Clerks and nurses don't. |
| 4 | Living news | `alive_news.c` | A crier by each Pokémon Center shouts headlines and hands out the paper; TVs sometimes show the news. Online it's the server's real events (`ANEWS`), offline the forecast and the player's deeds. |
| 5 | Ambient life | `alive_ambient.c` | Birds hop about and fly off when the player gets close, fish jump in water, flocks fly over. |
| 6 | Towns that grow | `alive_projects.c` | Eight towns have a foreman collecting money for a garden, park or plaza. Online everyone's gifts count (`APROJ`); when it's funded it's built on the map, and gives a daily gift. |
| 7 | Surprise events | `alive_events.c` | Up to 5 a day on routes: a stampede that drops berries, a lost child to walk to a town, a Pokémon stuck up a tree, a washed-up crate, a hurt wild Pokémon to heal (it may join you). Online, deeds make the news (`AEV`). |
| 8 | Weather changes the map | `alive_weather.c` | Rain opens puddles on routes, snow leaves footprints and drifts you push through, and on dry days small ponds dry up and sometimes leave an item. |
| 9 | Town Pokémon | `alive_town.c` | Resident Pokémon in towns (Zigzagoon digging by the Center, Vigoroth hauling boxes in Rustboro, Wingull on the shore...). They sleep at night and sometimes find you an item. |
| 10 | NPC chatter | `alive_talk.c` | Nearby townsfolk walk up to each other and chat in captions about the time of day, rovers, and the player. |

## Testing

- Server: `python3 -m unittest server/test_alive.py`
- Screenshots: `python3 tools/world_shots/run.py tools/alive_shots/alive.lua <out> 60000`.
  Scenes use `gAliveShowcase` (`include/alive.h`) to force a pack, an event,
  a chat, a finished project, a dry day or a weather.
- Test boots (the e2e tests) keep the original random encounters and turn the
  living world off unless `gAliveShowcase.living` is set.

## Art

The snow drift (`graphics/alive/snow_drift.png`) and the Zzz bubble
(`tools/alive_shots/sprites.py`) are new, drawn in the Emerald style.
