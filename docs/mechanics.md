# Battle and Pokémon mechanics

New battle rules and Pokémon mechanics: move tutors, rule sets, level-capped
tournaments, color variants, personalities, battle styles, trainer skills,
signature moves, evolution quests, cave bosses, battle commentary and the
storm-calming quest. Each has its own section; screenshots are made with
`tools/mechanics_shots/` (below).

The code lives in `src/mechanics*.c`, scripts in `data/scripts/mechanics/`.
`src/mechanics.c` has the overworld hooks the world features' hooks in
`src/world.c` call (`Mechanics_Update`, `Mechanics_TryInteract`) and puts
people on existing maps from C (`sMechNpcs`), so no map files change.

Numbers this thread owns (agreed with the other feature threads): flags
`0x881`–`0x887` and `0x8E5`–`0x8EF`, object local ids 190–199.

## Move tutor in every Pokémon Center

![The move tutor: first meeting, the menu, picking a Pokémon, Treecko's Egg Moves, and the tutor in other centers](images/mechanics-move-tutor.png)

A MOVE TUTOR stands by the right wall of every Hoenn Pokémon Center. He
uses the expansion's move relearner menu (`Common_EventScript_MoveRelearner`
in `data/scripts/move_relearner.inc`), free of charge:

- **Level Up Moves**: moves the Pokémon could have learned by now.
- **Egg Moves**: its species' Egg Moves.
- **TM Moves**: TMs it can learn that are in the bag.
- **Tutor Moves**: the tutor moves it can learn.

`P_ENABLE_MOVE_RELEARNERS` is on, so the summary screen's relearn option
also offers all four kinds.

## Screenshots

```sh
python3 tools/mechanics_shots/run.py tools/mechanics_shots/move_tutor.lua out/
```

The runner is `tools/world_shots/run.py` plus `Driver.fastText()` and
`Driver.step(key, name)`. It needs a built `pokeemerald.gba` and
`libmgba-dev`.
