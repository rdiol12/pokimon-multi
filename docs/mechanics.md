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

## Rule sets (inverse battles and more)

![The rules referee, the rule list, an INVERSE battle where Fire is super effective on Water, Trick Room, and rule sets marked WON](images/mechanics-rule-sets.png)

A referee in the Slateport Battle Tent lobby runs battles under special
rules (`src/mechanics_battle.c`, `data/scripts/mechanics/rules.inc`):

| Rule set | What changes |
| --- | --- |
| INVERSE | The type chart is turned around (`B_FLAG_INVERSE_BATTLE`). |
| TRICK ROOM | Trick Room for the whole battle. |
| NO ITEMS | Magic Room for the whole battle, and the Bag can't be used. |
| WONDER ROOM | Wonder Room (Defense and Sp. Def swapped) for the whole battle. |
| MISTY FIELD | Misty Terrain for the whole battle. |

The opponent is a Battle Frontier trainer with three of their Pokémon, at
the average level of the player's three strongest. Battles play like the
Battle Tower: no EXP, no prize money, no whiting out, and the party is
healed before and after. Rule sets won are marked WON
(`gSaveBlock3Ptr->mechanics.rulesWon`); winning all five gives an Ability
Patch.

**Online PvP.** `Mechanics_SetBattleRules(RULESET_*)` (include/mechanics.h)
sets a rule set up for the next battle, and `Mechanics_ClearBattleRules()`
undoes it. For a link battle both games must set the same rule set before
the battle starts.

`Mechanics_StartFacilityBattle()` starts a battle against any Battle
Frontier trainer whose party is already in `gParties[B_TRAINER_OPPONENT_A]`
(`Mechanics_FillFacilityParty` makes one); the script gets the outcome in
`VAR_RESULT`. No trainer ids are used, so `TRAINERS_MAX` stays as it is.

## Screenshots

```sh
python3 tools/mechanics_shots/run.py tools/mechanics_shots/move_tutor.lua out/
python3 tools/mechanics_shots/run.py tools/mechanics_shots/rules.lua out/ 120000
```

The runner is `tools/world_shots/run.py` plus `Driver.fastText()` and
`Driver.step(key, name)`, `Driver.fight(name, shotEvery)` (battles with the
first move) and `Driver.mech{...}` (gMechanicsShowcase in src/mechanics.c:
fix moveless party Pokémon, set the lead's moves). It needs a built `pokeemerald.gba` and
`libmgba-dev`.
