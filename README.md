# pokimon-multi

A Pokémon Emerald ROM hack with online multiplayer: see other players on the
overworld, battle them with your own party, trade Pokémon (for Pokémon or for
money), and climb the league leaderboard.

Based off RHH's pokeemerald-expansion 1.17.1 https://github.com/rh-hideout/pokeemerald-expansion/

## Building the ROM

Follow [INSTALL.md](INSTALL.md) to set up the toolchain, then run `make`. The
ROM is written to `pokeemerald.gba`. See [CREDITS.md](CREDITS.md) for everyone
behind pokeemerald-expansion and its
[documentation](https://rh-hideout.github.io/pokeemerald-expansion/) for
engine features.

## Upstream

The engine was imported from the `expansion/1.17.1` tag of
[pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion)
as a single commit. To pull in a newer release, diff the two tags upstream and
apply the patch here.
