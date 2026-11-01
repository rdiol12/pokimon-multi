# Solmara, the second region

Solmara is reached after the Hoenn League. A sailor in Lilycove's harbor
(he needs `FLAG_SYS_GAME_CLEAR`) takes the player aboard the S.S. Meridian.
The ship's steward lets you off at Port Solace or back in Lilycove.

## The map

Eight towns sit on a ring of eight routes (201-208), laid out as a 3x3 grid
with the League in the middle:

| Town | Gym | Leader | Badge TM |
|------|-----|--------|----------|
| Port Solace | Water | MARINA | Water Pulse |
| Gearholt | Steel | BRANT | Steel Wing |
| Ashcombe | Fire | CINDRA | Flamethrower |
| Silkwood | Bug | THISTLE | Giga Drain |
| Rosemead | Fairy | BELLE | Calm Mind |
| Duskmere | Ghost | MORWYN | Shadow Ball |
| Frostholm | Ice | SIGRID | Ice Beam |
| Drakespire | Dragon | VALEN | Dragon Claw |

The gyms can be taken in any order. Every Solmara trainer gains 2 levels per
Solmara badge you hold (`SOLMARA_LEVELS_PER_BADGE`), so whichever gym is
fourth is a fourth-gym fight. Leaders offer a rematch at your current level.

With all eight badges, the Victory Road cave north of Port Solace opens. It
leads to the Solmara League: a lobby, four Elite Four rooms (VESPER,
NETTLE, DUNE, ORION) and the Champion SOLENNE. Clearing it records your time
and clear count and posts your party to the online Hall of Fame wall.

## Built for multiplayer

- No story beat is for one player only. Every gym, leader, item and the
  League can be done by everyone on the same map at the same time. Leaders
  and the champion can be fought again.
- Towns have an open square, 4-wide exits and wide routes, because remote
  players are solid objects and a 1-tile path would jam.
- Each Pokémon Center has a nurse, a shop counter, a market board and the
  SWANNA AIR flight desk, which flies to any town you have visited. No HM
  is needed to get around.
- The ship's lounge is one shared map for everyone sailing either way.
- The League lobby has a reporter who opens the online WATCH and LADDER
  screens.
- Ashcombe's hot spring heals the party and raises friendship
  (`Area_SoakInHotSpring`).
- Maps use at most local id 51 for their own objects. Ids 52-69 are left
  for the living-world system.

## Save data

Solmara doesn't use the full SaveBlock3 area flags. It uses its own
`struct SolmaraSaveData` at the end of `gSaveBlockExt`:

- 2048 flags at `0x7000+`, handled by `GetFlagPointer`. The top 256 are
  trainer flags.
- Trainer ids from area-trainer slot 256 up (`SOLMARA_TRAINER(n)`).
- Champion time and League clear count.

## Generating

All maps are generated, not hand-edited:

```
python3 tools/solmara/region.py
```

The generator writes the maps, layouts, scripts, trainers
(`src/data/solmara_trainers.party`), wild encounters and
`include/constants/solmara_ids.h`. Each town, route, the gyms, the League
and the ship are modules in `tools/solmara/`. Tiles are learned from real
Hoenn maps (`tools/areas/synth.py`), so the art matches Emerald. No new
graphics and no FRLG assets are used.

## Screenshots

```
python3 tools/areas/shots/run.py tools/areas/shots/solmara.lua OUT 40000
```

This goes through the ship, Port Solace, the Victory Road badge check, the
flight desk, every town, four routes, two gyms, Victory Road, the League
and a leader battle. `Driver.flag` can now set Solmara flags (`0x7000+`).
