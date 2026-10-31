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

## Screenshots

```sh
python3 tools/life_shots/run.py tools/life_shots/bond.lua out/bond
```

`run.py` is `tools/world_shots/run.py` with this batch's symbols added, so
scenarios use the same Driver (`Driver.scene` and friends). They can also
write `gLifeShowcase` (`src/life.c`): byte 1 sets the lead's friendship,
then byte 0 = 1 applies it on the next overworld frame.

Art: `tools/life_shots/draw_art.py` draws this batch's sprites into
`graphics/life/`.
