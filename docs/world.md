# World and gameplay features

![Day and night (top) and roaming weather (bottom)](images/world-daynight-weather.png)

Features that make the world itself more alive, offline and online. Each has
its own section; screenshots are made with `tools/world_shots/` (below).

Numbers this batch owns (agreed with the other feature threads):
flags `0x030`–`0x04F`, vars `0x40F7`–`0x40FF`, object local ids 200–215
for other players' followers and mounts, netplay request/event types 96–127,
and its own EWRAM mailbox (`gWorldMailbox`, magic `PKMNWLD1`).

Script→C calls use `callnative`, so the shared `data/specials.inc` table is
left alone. Scripts live in `data/scripts/world/`, included from
`data/scripts/world.inc`.

## Day and night

The clock (the cartridge RTC) now changes which Pokémon appear:
`OW_TIME_OF_DAY_ENCOUNTERS` is on, and 13 areas have a night table
(`*_Night` in `src/data/wild_encounters.json`) with the official night
Pokémon at the same levels as by day:

| Area | At night |
|------|----------|
| Routes 101–104 | Hoothoot, Spinarak, Oddish, Murkrow (rare) |
| Petalburg Woods | Spinarak, Venonat, Hoothoot, Gastly, Morelull |
| Route 110 | Hoothoot, Gastly, Oddish, Murkrow |
| Route 113 | Houndour, Murkrow, Sneasel (rare) |
| Route 116 | Hoothoot, Venonat, Kricketot, Noibat |
| Route 117 | Volbeat, Illumise, Oddish, Morelull |
| Route 119 | Hoothoot, Noctowl, Murkrow, Misdreavus |
| Route 120 | Murkrow, Misdreavus, Absol, Gastly |
| Route 123, Mt. Pyre | Duskull, Misdreavus, Gastly, Haunter |

Morning, day and evening use the normal tables. The overworld is tinted by
time of day (the expansion's DNS).

Night-only visitors (shown from 8 pm, `DayNight_UpdateFlags` in
`src/world_daynight.c`):

- **Oldale Town**: a storyteller who explains which Pokémon come out at night
  and gives a Spell Tag once.
- **Lilycove City**: the Night Market, selling Dusk Balls, Moon and Dusk
  Stones, Reaper Cloth, Spell Tags and Black Glasses.
- **Fallarbor Town**: a stargazer who gives a Moon Stone once.

## Roaming weather

Three weather fronts drift across Hoenn, one map per hour of the clock
(`src/world_weather.c`): a **rainstorm** (Route 101 → Lilycove via Routes 103,
110, 118–121), a **sandstorm** (Route 117 → Route 114 via Route 111–113 and
Fallarbor) and a **snow squall** (Route 102 → Route 115 via Petalburg, Route
104, Rustboro and Route 116). Each walks to the end of its path and back. Its
position only depends on the clock, so players online at the same time find
it in the same place. The rainstorm also brings light rain to the maps next
to it. When the hour turns while you stand on a map, the sky changes on the
spot. Story weather (Groudon/Kyogre, the Route 119/123 cycles, underwater)
is never replaced.

While a front is over a map, 35% of wild encounters there are weather
Pokémon, at the map's levels and at the stage they'd have evolved to by then
(official evolution levels):

- Rain: Lotad, Wingull, Psyduck, Poliwag, Surskit, Marill, Wooper, Tympole;
  rare Goomy.
- Sandstorm: Sandshrew, Trapinch, Cacnea, Baltoy, Hippopotas, Silicobra,
  Dwebble; rare Gible, Larvitar.
- Snow: Snorunt, Snover, Swinub, Spheal, Cubchoo, Vanillite, Snom; rare
  Sneasel, Alolan Vulpix.

A weather watcher in Oldale Town (next to the Pokémon Center) tells where
each front is right now.

## Fishing and berry picking

A bite opens the reel meter (`src/world_meter.c`): stop the needle in the
green with A to hook the Pokémon. Better rods get a smaller, faster zone
(4 tiles for the Old Rod, 2 for the Super Rod) but the same official
encounter tables. Every hooked Pokémon has a size: its official Pokédex
height scaled 75–125% by its personality, so the same Pokémon always
measures the same. Landing it (catching it) counts it, and a new biggest
fish shows a "personal record" notice.

Before a berry tree is picked, a three-stop meter adds one berry per hit
(up to the tree's 31-berry limit). R > RECORDS lists the biggest fish,
fish landed, berries picked, best harvest and the other world records.

## Screenshots

```sh
python3 tools/world_shots/run.py tools/world_shots/daynight.lua out/daynight
```

`run.py` boots the ROM headless (libmgba, the netplay e2e harness) through
the test boot, then runs the scenario. `Driver.scene(hour, MAPS.MAP_X, x, y)`
sets the clock and warps by writing `gWorldShowcase` (`src/world.c`);
`Driver.findBattle`, `Driver.runAway` and `Driver.talk` do the rest.
