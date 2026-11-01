# Original Hoenn, adjusted for many players

The original story was made for one player. Every player has their own save,
so one-time events, gifts, single items and legendaries never run out: each
player gets their own. What did break with other players around is fixed
here (`src/hoenn_multi.c`).

![Walking through a crowded Pokémon Center to the nurse, alone in your bedroom and in a gym, and friend travel refusing a place you haven't reached](images/hoenn-multi.png)

## Walk through other players

Other players and their following Pokémon used to be solid, so one person
standing in a door, in front of the nurse or on a one-tile bridge blocked
everyone. Now nobody bumps into them (`HoennMulti_PassesThrough`, checked in
`GetObjectObjectCollidesWith`), and they don't block a trainer's line of
sight either. You can still face them and press A to talk.

## Places where you don't see other players

Some places are a copy of your own (`sPrivateMaps`): another player there
would stand in your bedroom, walk through gym gates that are shut for you, or
wander into the Champion's room in the middle of your battle. Other players
aren't drawn in:

- the moving truck and both Littleroot houses,
- secret bases,
- the eight gyms,
- the Elite Four rooms, the halls between them, the Champion's room and the
  Hall of Fame,
- the Trick House puzzles,
- the Battle Frontier's corridors and battle rooms (the lobbies stay shared)
  and the Trainer Hill floors.

They also disappear during your own cutscenes: once a script moves someone,
other players stay hidden until it ends, so nobody stands in the middle of
the scene, and the scene gets their object slots.

## Friend travel follows the story

ONLINE > FRIENDS could warp you to any friend outdoors, so a new player could
jump to Ever Grande City or Sootopolis. Now it only goes to maps you've
already been to ("You haven't been there yet."). The game keeps one bit per
map in the extended save (`SaveBlockExt.visitedMaps`), set whenever you stand
on a map (not during hide-and-seek or a race, which take you there and back).
Saves from before this count the towns and cities you can fly to.

## Screenshots

`python3 tools/world_shots/hoenn_multi.py <out dir>` starts the server with
stand-in players around Hoenn (in the Oldale Pokémon Center aisle, at the
nurse and by the door, in Brendan's bedroom, in the Petalburg Gym, and a
friend on Route 104) and runs `tools/world_shots/hoenn_multi.lua`, which
fails if the player can't reach the nurse through them or if friend travel
goes somewhere new.
